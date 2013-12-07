
/*
 *   Pure Data Packet header file. List class
 *   Copyright (c) by Tom Schouten <tom@zwizwa.be>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

/* who can live without a list, hmm? 

   this is sorth of a compromise between lists, queues,
   stacks, lisp and forth. list contain pdp atoms
   (floats, ints, symbols, pointers, packets or lists) */


#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "pdp_list.h"
#include "pdp_symbol.h"
#include "pdp_packet.h"
#include "pdp_type.h"
#include "pdp_types.h"
#include "pdp_mem.h"
#include "pdp_post.h"
#include "pdp_debug.h"

#define D if (0)


t_pdp_fastalloc *_fast_atom_allocator;
t_pdp_fastalloc *_fast_list_allocator;




static void _pdp_list_dealloc(t_pdp_list *l)
{
  pdp_fastalloc_save_atom(_fast_list_allocator, l);
}

// allocator macros
#define PDP_ATOM_ALLOC() pdp_fastalloc_new_atom(_fast_atom_allocator)
#define PDP_ATOM_DEALLOC(x) pdp_fastalloc_save_atom(_fast_atom_allocator, x)
#define PDP_LIST_ALLOC() pdp_fastalloc_new_atom(_fast_list_allocator)
#define PDP_LIST_DEALLOC(x) _pdp_list_dealloc(x)
//#define PDP_LIST_DEALLOC(x) pdp_fastalloc_save_atom(_fast_list_allocator, x)




/* some private helper methods */

/* list pool setup */
void pdp_list_setup(void)
{



  /* create fast allocators */
  _fast_atom_allocator = pdp_fastalloc_new(sizeof(t_pdp_atom));
  _fast_list_allocator = pdp_fastalloc_new(sizeof(t_pdp_list));

  /* testing code */
  if (0){
      char *next;
      t_pdp_list *l = pdp_tree_from_cstring("( een twee (3 vier ()) vijf (6.0)", &next);
      if (!l){
	  pdp_post("parse error:");
	  pdp_post(next);
      }
      else{
	  pdp_list_print(l);
      }
      exit(1);
  }

 
}



/* create a list */
t_pdp_list* pdp_list_new(int elements)
{
    t_pdp_atom *a = 0;
    t_pdp_list *l = PDP_LIST_ALLOC();
    l->elements = 0;


    if (elements){
	a = PDP_ATOM_ALLOC();
	l->elements++;
	a->t = a_undef;
	a->w.w_int = 0;
	a->next = 0;
	elements--;
    }
    l->first = a;
    l->last = a;

    while (elements--){
	a = PDP_ATOM_ALLOC();
	l->elements++;
	a->t = a_undef;
	a->w.w_int = 0;
	a->next = l->first;
	l->first = a;
    }

    return l;
}

/* clear a list */
void pdp_list_clear(t_pdp_list *l)
{
    t_pdp_atom *a = l->first; 
    t_pdp_atom *next_a;

    while(a){
	next_a = a->next;
	PDP_ATOM_DEALLOC(a);
	a = next_a;
    }

    l->first = 0;
    l->last = 0;
    l->elements = 0;

}

/* destroy a list */
void pdp_list_free(t_pdp_list *l)
{
    if (l){
	pdp_list_clear(l);
	PDP_LIST_DEALLOC(l);
    }
}


/* destroy a (sub)tree */
void pdp_tree_free(t_pdp_list *l)
{
  if (l) {
    pdp_tree_clear(l);
    PDP_LIST_DEALLOC(l);
  }
}

/* clear a tree */
void pdp_tree_clear(t_pdp_list *l)
{
    t_pdp_atom *a = l->first; 
    t_pdp_atom *next_a;


    while(a){
      if (a->t == a_list){
	  pdp_tree_free(a->w.w_list);
      }
      next_a = a->next;
      PDP_ATOM_DEALLOC(a);
      a = next_a;
    }

    l->first = 0;
    l->last = 0;
    l->elements = 0;

}

/* BEGIN PARSER CODE */

/* real whitespace handling */
static inline int _is_whitespace(char c){return (c == ' ' || c == '\n' || c == '\t');}
static inline void _skip_real_whitespace(char **c){while (_is_whitespace(**c)) (*c)++;}

/* comment handling */
static inline int _is_left_comment(char c) {return (c == '#');}
static inline int _is_right_comment(char c) {return (c == '\n');}
static inline void _skip_comment(char **c)
{
    if (!_is_left_comment(**c)) return;
    (*c)++;
    while (!_is_right_comment(**c)){
	if (!**c) return; // no terminating newline
	(*c)++;
    }
    (*c)++;
}

/* comment + whitespace handling */
static inline void _skip_whitespace(char **c)
{
    char *prev_c;
    /* skip comments and whitespace until the
       pointer stops moving */
    do {
	prev_c = *c;
	_skip_real_whitespace(c);
	_skip_comment(c);
    } while (prev_c != *c);
}

static inline int _is_left_separator(char c) {return (c == '(');}
static inline int _is_right_separator(char c) {return (c == ')');}
static inline int _is_terminator(char c) {return (c == 0);}

/* the end of an atom is marked by a separator */
static inline int _is_separator(char c) {return (_is_terminator(c) 
						 || _is_left_separator(c) 
						 || _is_right_separator(c) 
						 || _is_whitespace(c));}


/* parse a single pure atom from a zero terminated string
   a pure atom is either a number (int or float) xor a symbol
*/

static inline void _parse_pure_atom(t_pdp_atom *a, char *c)
{
    char *next;

    /* check if the string has a decimal point */
    int has_decimal = 0;
    char *c2;
    for(c2 = c; *c2; c2++){
	if (*c2 == '.') { has_decimal = 1; break; }
    }
    
    /* try parsing as a number (int or float) first */
    if (has_decimal){ // try float
	float f = strtod(c, &next);
	if (next[0] == 0){ // got everything?
	    D pdp_post("parsing float %f", f);
	    a->t = a_float;
	    a->w = (t_pdp_word)f;
	    return;
	}
    }
    else { // try int
	int i = strtol(c, &next, 0);
	if (next[0] == 0){ // got everything?
	    D pdp_post("parsing int %d", i);
	    a->t = a_int;
	    a->w = (t_pdp_word)i;
	    return;
	}
    }


    /* number parsing failed: it's a symbol */
    D pdp_post("parsing symbol %s", c);
    a->t = a_symbol;
    a->w = (t_pdp_word)pdp_gensym(c);

}

t_pdp_atom *pdp_atom_new(void){t_pdp_atom *a = PDP_ATOM_ALLOC(); a->next = 0; return a;}
void pdp_atom_free(t_pdp_atom *a){PDP_ATOM_DEALLOC(a);}

/* there are two parser methods: parse an atom and parse a list
   both can call each other recursively.
   the atoms and list are allocated with pdp_list_new and
   pdp_atom_new respectively */

t_pdp_atom *pdp_atom_from_cstring(char *chardef, char **next)
{
    t_pdp_atom *a = 0;

    /* skip whitespace and check if there's anything left */
    _skip_whitespace(&chardef);
    if (!chardef[0] || _is_right_separator(*chardef)) goto done;


    /* check if it's a list atom */
    if(_is_left_separator(*chardef)){
	t_pdp_list *l =  pdp_tree_from_cstring(chardef, &chardef);
	if (l){
	    a = pdp_atom_new();
	    a->t = a_list;
	    a->w.w_list = l;
	}

    }

    /* we have a pure atom, copy it to a temp buffer */
    else{
	int n = 0;
	while (!_is_separator(chardef[n])) n++;
	if (!n) goto done;
	else {
	    char tmp[n+1];
	    strncpy(tmp, chardef, n);
	    tmp[n] = 0;
	    a = pdp_atom_new();
	    _parse_pure_atom(a, tmp);
	    chardef += n;
	}

    }    

 done:
    if (next) *next = chardef;
    return a;
	
}

/* check if a tree (list of lists) matches a certain type syntax
   types:

      symbol -> a_sym;
      int    -> a_int;
      float  -> a_float;
      packet -> a_packet;
      list   -> a_list;
      ...    -> zero or more times the preceeding elements in the list
*/



/* create a list from a character string */
t_pdp_list *pdp_tree_from_cstring(char *chardef, char **next)
{
    t_pdp_list *l = pdp_list_new(0);
    t_pdp_atom *a = 0;

    D pdp_post ("creating list from char: %s", chardef);

    /* find opening parenthesis and skip it*/
    _skip_whitespace(&chardef);
    if (!_is_left_separator(*chardef)) goto error; else chardef++;

    /* chardef now points at the first atom, start adding atoms */
    while(1){
	a = pdp_atom_from_cstring(chardef, &chardef);
	if (a)pdp_list_add_back_atom(l, a);
	else break;
    }

    /* skip whitespace and find closing parenthesis */
    _skip_whitespace(&chardef);
    if (!_is_right_separator(*chardef)) goto error; else chardef++;
    if (next) *next = chardef;
    return l;

 error:
    /* end of string encountered: parse error */
    D pdp_post("parse error: %s", chardef);
    if (next) *next = chardef;
    pdp_tree_free(l); //this will free all sublists too
    return 0; // parse error

    

}

/* END PARSER CODE */

// this assumes syntax's syntax is correct
int pdp_tree_check_syntax(t_pdp_list *list, t_pdp_list *syntax)
{

    t_pdp_atom *la = 0;
    t_pdp_atom *sa = 0;

    t_pdp_symbol *ellipsis = pdp_gensym("...");
    t_pdp_symbol *wildcard = pdp_gensym("*");

    /* handle empty lists */
    if (list->elements == 0){

	/* check if syntax list is empty */
	if (syntax->elements == 0) goto match;

	/* check if syntax list has ellipsis */
	if (syntax->last->t == a_symbol &&
	    syntax->last->w.w_symbol == ellipsis) goto match;

	/* default: no match */
	goto nomatch;
    }


    /* loop over list and syntax list */
    for (la = list->first, sa = syntax->first; 
	 la && sa; 
	 la = la->next, sa = sa->next){

	D pdp_post("pdp_tree_check_syntax: starting check");

    checkatom:
	/* what do we expect for this atom ? */
	switch(sa->t){
	case a_list:
	    D pdp_post("expecting list");
	    /* we need to recurse down the tree */
	    /* exit if the current list to check
	       does not have a sublist */
	    if (la->t != a_list) {
		D pdp_post("not a list");
		goto nomatch;
	    }

	    /* recurse and exit if no match */
	    D pdp_post("checking sublist");
	    if (!pdp_tree_check_syntax(la->w.w_list, sa->w.w_list)){
		D pdp_post("sublist does not match");
		goto nomatch;
	    }

	    break;

	case a_symbol:

	    /* if ellipsis, rewind */
	    if (ellipsis == sa->w.w_symbol){
		D pdp_post("got ellipsis");
		/* check if we're not looping */
		if (sa == syntax->first){
		    D pdp_post("ellipsis at start of list");
		    goto nomatch;
		}
		/* try again */
		sa = syntax->first;
		D pdp_post("ellipsis rewind");
		goto checkatom;
	    }

	    else if (wildcard == sa->w.w_symbol){
		D pdp_post("got wildcard");
	    }

	    /* ordinary atom: check type */
	    else{
		D pdp_post("expecting %s", sa->w.w_symbol->s_name);
		switch(la->t){
		    
		case a_int:
		    if (sa->w.w_symbol != pdp_gensym("int")) goto nomatch; break;
		case a_float:
		    if (sa->w.w_symbol != pdp_gensym("float")) goto nomatch; break;
		case a_symbol:
		    if (sa->w.w_symbol != pdp_gensym("symbol")) goto nomatch; break;
		case a_packet:
		    if (sa->w.w_symbol != pdp_gensym("packet")) goto nomatch; break;
		case a_list:
		    if (sa->w.w_symbol != pdp_gensym("list")) goto nomatch; break;
		    
		default:
		    goto nomatch;
		}
		D pdp_post("OK");
	    }

	    break;
	    
	default:
	    D pdp_post("syntax syntax error");
	    pdp_list_print(syntax);
	    goto nomatch; // incorrect syntax description
	}

    }

    /* loop ended because one of the lists was finished */
    /* only two cases can be valid: la == 0 and (sa == 0 or ellipsis) */

    if (la != 0){
	D pdp_post("not end of list -> no match");
	goto nomatch;
    }
    
    if (sa == 0) goto match;

    if (!(sa->t == a_symbol && sa->w.w_symbol == ellipsis)){
	D pdp_post("syntax list not in ellipsis position -> no match");
	goto nomatch;
    }
    
    
    /* exits */
 match:
    D pdp_post("pdp_tree_check_syntax: match");
    return 1;
 nomatch:
    D pdp_post("pdp_tree_check_syntax: no match");
    return 0;

}



/* traversal */
void pdp_list_apply(t_pdp_list *l, t_pdp_atom_method m)
{
  t_pdp_atom *a;
  if (!l) return;
  for (a=l->first; a; a=a->next) m(a);
}

void pdp_tree_apply(t_pdp_list *l, t_pdp_atom_method m) 
{
  t_pdp_atom *a;
  if (!l) return;
  for (a=l->first; a; a=a->next){
    if (a->t == a_list) pdp_tree_apply(a->w.w_list, m);
    else m(a);
  }
}

void pdp_list_apply_word_method(t_pdp_list *l, 
				t_pdp_word_type type, t_pdp_word_method wm)
{
  t_pdp_atom *a;
  if (!l) return;
  for (a=l->first; a; a=a->next){
    if (a->t == type) wm(a->w);
  }
}
void pdp_list_apply_pword_method(t_pdp_list *l, 
				t_pdp_word_type type, t_pdp_pword_method pwm)
{
  t_pdp_atom *a;
  if (!l) return;
  for (a=l->first; a; a=a->next){
    if (a->t == type) pwm(&a->w);
  }
}

void pdp_tree_apply_word_method(t_pdp_list *l, 
				t_pdp_word_type type, t_pdp_word_method wm) 
{
  t_pdp_atom *a;
  if (!l) return;
  for (a=l->first; a; a=a->next){
    if (a->t == a_list) pdp_tree_apply_word_method(a->w.w_list, type, wm);
    else if (a->t == type) wm(a->w);
  }
}
void pdp_tree_apply_pword_method(t_pdp_list *l, 
				t_pdp_word_type type, t_pdp_pword_method pwm) 
{
  t_pdp_atom *a;
  if (!l) return;
  for (a=l->first; a; a=a->next){
    if (a->t == a_list) pdp_tree_apply_pword_method(a->w.w_list, type ,pwm);
    else if (a->t == type) pwm(&a->w);
  }
}

static void _atom_packet_mark_unused(t_pdp_atom *a)
{
  if (a->t == a_packet){
    pdp_packet_mark_unused(a->w.w_packet);
    a->w.w_packet = -1;
  }
}

static void _atom_packet_copy_ro(t_pdp_atom *a)
{
    int p;
    if (a->t == a_packet){
	a->w.w_packet = pdp_packet_copy_ro(a->w.w_packet);
    }
}

void pdp_tree_strip_packets  (t_pdp_list *l)
{
   pdp_tree_apply(l, _atom_packet_mark_unused);
}

static void _pdp_tree_copy_ro_packets (t_pdp_list *l)
{
    pdp_tree_apply(l, _atom_packet_copy_ro);
}

t_pdp_list *pdp_tree_copy_ro(t_pdp_list *l)
{
    t_pdp_list *l2 = pdp_tree_copy(l);
    _pdp_tree_copy_ro_packets(l2);
    return l2;
}

static void _pdp_atomlist_fprint(FILE* f, t_pdp_atom *a);

static void _pdp_atom_fprint(FILE* f, t_pdp_atom *a)
{
    if (!a){
	fprintf(f, "<NULL ATOM>");
	return;
    }

    switch(a->t){
	/* generic atoms */
    case a_symbol:   fprintf(f, "%s",a->w.w_symbol->s_name); break;
    case a_float:    fprintf(f, "%f",a->w.w_float); break;
    case a_int:      fprintf(f, "%d",a->w.w_int); break;
    case a_packet:   fprintf(f, "#<pdp %d %s>",a->w.w_packet,
			     pdp_packet_get_description(a->w.w_packet)->s_name); break;
    case a_pointer:   fprintf(f, "#<0x%08x>", a->w.w_int); break;
    case a_list:     _pdp_atomlist_fprint(f, a->w.w_list->first); break;
    case a_atom_pointer:
	fprintf(f, "->");
	_pdp_atom_fprint(f, a->w.w_atom_pointer);
	break;
    case a_undef:     fprintf(f, "<undef>"); break;

	/* forth atoms */
    case a_forthword: fprintf(f, "#<forth word 0x%08x>", a->w.w_int); break;
    case a_vmword:    fprintf(f, "#<vm word 0x%08x>", a->w.w_int); break;
    case a_vmmacro:   fprintf(f, "#<vm macro 0x%08x>", a->w.w_int); break;

	
    default:         fprintf(f, "<unknown type>"); break;
    }
}

/* debug */
static void _pdp_atomlist_fprint(FILE* f, t_pdp_atom *a)
{
    fprintf(f, "(");
    while (a){
	_pdp_atom_fprint(f,a);
       	a = a->next;
	if (a) fprintf(f, " ");
    }
    fprintf(f, ")");
}

void _pdp_list_fprint(FILE* f, t_pdp_list *l)
{
    _pdp_atomlist_fprint(f, l->first);
    fprintf(f, "\n");
}

void pdp_list_print(t_pdp_list *l)
{
    _pdp_list_fprint(stderr, l);
}

void pdp_atom_print(t_pdp_atom *a)
{
    _pdp_atom_fprint(stderr, a);
}

/* public list operations */




/* add a atom/word to the start of the list */
void pdp_list_add_atom(t_pdp_list *l, t_pdp_atom *a)
{
    a->next = l->first;
    l->first = a;
    l->elements++;
    if (!l->last) l->last = a;
}

void pdp_list_add(t_pdp_list *l, t_pdp_word_type t, t_pdp_word w)
{
    t_pdp_atom *a = PDP_ATOM_ALLOC();
    a->t = t;
    a->w = w;
    pdp_list_add_atom(l, a);
}


/* add a word to the end of the list */
void pdp_list_add_back_atom(t_pdp_list *l, t_pdp_atom *a)
{
    
    l->elements++;
    a->next = 0;
    if (l->last){
	l->last->next = a;
    }
    else{
	l->first = a;
    }
    l->last = a;
}

void pdp_list_add_back(t_pdp_list *l, t_pdp_word_type t, t_pdp_word w)
{
    t_pdp_atom *a = PDP_ATOM_ALLOC();
    a->w = w;
    a->t = t;
    pdp_list_add_back_atom(l, a);
}

/* get list size */
int pdp_list_size(t_pdp_list *l)
{
    return l->elements;
}




/* pop: return first item and remove */
t_pdp_atom  *pdp_list_pop_atom(t_pdp_list *l)
{
    t_pdp_atom *a = l->first;
    if (!a) return a;

    l->first = a->next;
    l->elements--;
    if (!l->first) l->last = 0;
    a->next = 0; // detach
    return a;
}


/* pop: return first item and remove */
t_pdp_word pdp_list_pop(t_pdp_list *l)
{
    t_pdp_atom *a = pdp_list_pop_atom(l);
    t_pdp_word w=a->w;
    PDP_ATOM_DEALLOC(a);
    return w;
}




/* pop from one list and push to other */
void pdp_list_pop_push(t_pdp_list *source, t_pdp_list *dest)
{
    t_pdp_atom *a = source->first;

    /* pop atom */
    if (--(source->elements)){source->last = 0;}
    source->first = a->next;

    /* push atom */
    a->next = dest->first;
    if (dest->elements++) {dest->last = a;}
    dest->first = a;

    return;

}


/* return element at index */
t_pdp_word pdp_list_index(t_pdp_list *l, int indx)
{
    t_pdp_atom *a;
    for (a = l->first; indx--; a = a->next);
    return a->w;
}





/* remove an element from a list */
void pdp_list_remove(t_pdp_list *l, t_pdp_word_type t, t_pdp_word w)
{
    t_pdp_atom head;
    t_pdp_atom *a;
    t_pdp_atom *kill_a;
    head.next = l->first;

    for(a = &head; a->next; a = a->next){
	if (a->next->w.w_int == w.w_int && a->next->t == t){
	    kill_a = a->next;        // element to be killed
	    a->next = a->next->next; // remove link
	    PDP_ATOM_DEALLOC(kill_a);
	    l->elements--;
	    l->first = head.next;    // restore the start pointer
	    if (l->last == kill_a) { // restore the end pointer
		l->last = (a != &head) ? a : 0;
	    }

	    break;
	}
    }
    
}





/* copy a list */
t_pdp_list* pdp_tree_copy_reverse(t_pdp_list *list)
{
    t_pdp_list *newlist = pdp_list_new(0);
    t_pdp_atom *a;
    for (a = list->first; a; a = a->next)
	if (a->t == a_list){
	    pdp_list_add(newlist, a->t, 
			 (t_pdp_word)pdp_tree_copy_reverse(a->w.w_list));
	}
	else{
	    pdp_list_add(newlist, a->t, a->w);
	}
    return newlist;
}
t_pdp_list* pdp_list_copy_reverse(t_pdp_list *list)
{
    t_pdp_list *newlist = pdp_list_new(0);
    t_pdp_atom *a;
    for (a = list->first; a; a = a->next)
      pdp_list_add(newlist, a->t, a->w);
    return newlist;
}

t_pdp_list* pdp_tree_copy(t_pdp_list *list)
{
    t_pdp_list *newlist = pdp_list_new(list->elements);
    t_pdp_atom *a_src = list->first;
    t_pdp_atom *a_dst = newlist->first;

    while(a_src){
	a_dst->t = a_src->t;
	if (a_dst->t == a_list){ //recursively copy sublists (tree copy)
	    a_dst->w.w_list = pdp_tree_copy(a_src->w.w_list);
	}
	else{
	    a_dst->w = a_src->w;
	}
	a_src = a_src->next;
	a_dst = a_dst->next;
    }

    return newlist;
}
t_pdp_list* pdp_list_copy(t_pdp_list *list)
{
    t_pdp_list *newlist = pdp_list_new(list->elements);
    t_pdp_atom *a_src = list->first;
    t_pdp_atom *a_dst = newlist->first;

    while(a_src){
	a_dst->t = a_src->t;
	a_dst->w = a_src->w;
	a_src = a_src->next;
	a_dst = a_dst->next;
    }
    return newlist;
}

void pdp_list_join (t_pdp_list *l, t_pdp_list *tail)
{
    if (tail->elements){
	l->elements += tail->elements;
	if (l->last){
	    l->last->next = tail->first;
	    l->last = tail->last;
	}
	else {
	    l->first = tail->first;
	    l->last = tail->last;
	}
    }
    PDP_LIST_DEALLOC(tail); //delete the tail header
}

void pdp_list_cat (t_pdp_list *l, t_pdp_list *tail)
{
    t_pdp_list *tmp = pdp_list_copy(tail);
    pdp_list_join(l, tmp);
}


/* in place reverse: atoms stay the same
   they are just relinked. so pointers will stay accurate */
void pdp_list_reverse(t_pdp_list *l)
{
    t_pdp_list tmp;
    t_pdp_atom *a;
    tmp.first = l->first;
    tmp.last = l->last;
    tmp.elements = l->elements;
    l->first = 0;
    l->last = 0;
    l->elements = 0;
    while (a = pdp_list_pop_atom(&tmp)){
	pdp_list_add_atom(l, a);
    }
}

void pdp_list_reverse_old(t_pdp_list *l)
{
    t_pdp_list *l2 = pdp_list_copy_reverse(l);
    pdp_list_clear(l);
    l->first = l2->first;
    l->last = l2->last;
    l->elements = l2->elements;
    _pdp_list_dealloc(l2);
}

/* check if a list contains an element */
int pdp_list_contains(t_pdp_list *list, t_pdp_word_type t, t_pdp_word w)
{
    t_pdp_atom *a;
    for(a = list->first; a; a=a->next){
	if (a->w.w_int == w.w_int && a->t == t) return 1;
    }
    return 0;
}

/* add a thing to the start of the list if it's not in there already */
void pdp_list_add_to_set(t_pdp_list *list, t_pdp_word_type t, t_pdp_word w)
{
    if (!pdp_list_contains(list, t, w))
	pdp_list_add(list, t, w);
}




