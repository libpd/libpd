
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

/* the pdp list is composed of atoms. 
   the default atom is a pointer.
   lists can be recursed into trees.

   note: all functions that return t_pdp_word and don't take a type argument
   obviously don't perform any type checking. if you have heterogenous lists,
   you should use atom iterators or direct access.

   functions starting with "pdp_tree" recurse through sublists.
   functions starting with "pdp_list" stay at the top level.

*/



#ifndef PDP_LIST_H
#define PDP_LIST_H

struct _pdp_list;
struct _pdp_atom;

/* THE LIST OBJECT */

typedef enum {
    /* generic atoms */
    a_undef = 0,
    a_pointer,
    a_float,
    a_int,
    a_symbol,
    a_packet,
    a_list,
    a_atom_pointer,

    /* forth atoms */
    a_forthword,         /* forth word operating on a stack */
    a_vmword,            /* forth word operating on a virtual machine */
    a_vmmacro            /* forth word operating on vm, manipilating current def */
} t_pdp_word_type;

typedef union _pdp_word
{
    void*                   w_pointer;
    float                   w_float;
    int                     w_int;
    struct _pdp_symbol*     w_symbol;
    int                     w_packet;
    struct _pdp_list*       w_list;
    struct _pdp_atom*       w_atom_pointer;

} t_pdp_word;

/* a list element */
typedef struct _pdp_atom
{
    struct _pdp_atom *next;
    t_pdp_word w;
    t_pdp_word_type t;
} t_pdp_atom;

/* a list container */
typedef struct _pdp_list
{
    int elements;
    t_pdp_atom *first;
    t_pdp_atom *last;
    
} t_pdp_list;


/* CONVENTION: trees stacks and lists.

   * all operations with "list" in them operate on flat lists. all the
     items contained in the list are either pure atoms (floats, ints, or symbols) 
     or references (packets, pointers, lists)

   * all operations with "tree" in them, operate on recursive lists (trees)
     all sublists of the list (tree) are owned by the parent list, so you can't
     build trees from references to other lists.
 
   * stacks are trees (the forth can have tree's on a stack, or have recursive stacks)
     (WAS: stacks are by definition flat lists, so they can not contains sublists)

*/
     
typedef void (*t_pdp_atom_method)(t_pdp_atom *);
typedef void (*t_pdp_word_method)(t_pdp_word);
typedef void (*t_pdp_pword_method)(t_pdp_word *);
typedef void (*t_pdp_free_method)(void *);

/* creation / destruction */
t_pdp_atom*        pdp_atom_new           (void);
void               pdp_atom_free          (t_pdp_atom *);
t_pdp_list*        pdp_list_new           (int elements);
void               pdp_list_free          (t_pdp_list *l);
void               pdp_list_clear         (t_pdp_list *l);
void               pdp_tree_free          (t_pdp_list *l);
void               pdp_tree_clear         (t_pdp_list *l);



/* call a free method on all pointers in a tree */
void               pdp_tree_strip_pointers (t_pdp_list *l, t_pdp_free_method f);

/* strip all packets from a tree. i.e. call pdp_packet_mark_unused on them */
void               pdp_tree_strip_packets  (t_pdp_list *l);

/* copy a tree, and copy all packets readonly */
t_pdp_list        *pdp_tree_copy_ro(t_pdp_list *l);

t_pdp_list*        pdp_tree_from_cstring(char *chardef, char **nextchar);

/* check type syntax of list */
int pdp_tree_check_syntax(t_pdp_list *list, t_pdp_list *syntax);
t_pdp_atom *pdp_atom_from_cstring(char *chardef, char **nextchar);
//void pdp_atom_from_cstring(t_pdp_atom *a, char *string);


/* traversal routines (map functions) */
/* use these in conjunction with gcc local functions
   if there's ever a portability problem: add a void* data argument to implement closures */
void pdp_list_apply       (t_pdp_list *l, t_pdp_atom_method am);
void pdp_tree_apply       (t_pdp_list *l, t_pdp_atom_method am);
void pdp_list_apply_word_method  (t_pdp_list *l, t_pdp_word_type t, t_pdp_word_method wm);
void pdp_tree_apply_word_method  (t_pdp_list *l, t_pdp_word_type t, t_pdp_word_method wm);
void pdp_list_apply_pword_method (t_pdp_list *l, t_pdp_word_type t, t_pdp_pword_method pwm);
void pdp_tree_apply_pword_method (t_pdp_list *l, t_pdp_word_type t, t_pdp_pword_method pwm);


/* copy: (reverse) copies a list. */ 
/* list copy is flat. pointers and packets are copied. so you need to
   ensure reference consistency yourself. */

t_pdp_list*        pdp_list_copy          (t_pdp_list *l);
t_pdp_list*        pdp_list_copy_reverse  (t_pdp_list *l);
t_pdp_list*        pdp_tree_copy          (t_pdp_list *l);
t_pdp_list*        pdp_tree_copy_reverse  (t_pdp_list *l);


/* cat: this makes a copy of the second list and adds it at the end of the first one */
void               pdp_list_cat           (t_pdp_list *l, t_pdp_list *tail);

/* information */
int     pdp_list_contains                 (t_pdp_list *l, t_pdp_word_type t, t_pdp_word w);
int     pdp_list_size                     (t_pdp_list *l);
void    pdp_list_print                    (t_pdp_list *l);
void    pdp_atom_print                    (t_pdp_atom *a);

/* access */
void          pdp_list_add           (t_pdp_list *l, t_pdp_word_type t, t_pdp_word w);
void          pdp_list_add_back      (t_pdp_list *l, t_pdp_word_type t, t_pdp_word w);
void          pdp_list_add_to_set    (t_pdp_list *l, t_pdp_word_type t, t_pdp_word w);
void          pdp_list_remove        (t_pdp_list *l, t_pdp_word_type t, t_pdp_word w);

void pdp_list_add_atom(t_pdp_list *l, t_pdp_atom *a);
void pdp_list_add_back_atom(t_pdp_list *l, t_pdp_atom *a);

/* these don't do error checking. out of bound == error */
t_pdp_atom   *pdp_list_pop_atom      (t_pdp_list *l);
t_pdp_word    pdp_list_pop           (t_pdp_list *l);
t_pdp_word    pdp_list_index         (t_pdp_list *l, int indx);
void          pdp_list_pop_push      (t_pdp_list *source, t_pdp_list *dest);

/* some aliases */
#define pdp_list_add_front pdp_list_add
#define pdp_list_push      pdp_list_add
#define pdp_list_queue     pdp_list_add_end
#define pdp_list_unqueue   pdp_list_pop

/* util */
void pdp_list_reverse(t_pdp_list *l);

/* generic atom iterator */
#define PDP_ATOM_IN(list,atom)              for (atom = list->first ; atom ; atom = atom->next)

/* fast single type iterators */

/* generic */
#define PDP_WORD_IN(list, atom, word, type) for (atom=list->first ;atom && ((word = atom -> w . type) || 1); atom=atom->next)

/* type specific */
#define PDP_POINTER_IN(list, atom, x) PDP_WORD_IN(list, atom, x, w_pointer)
#define PDP_INT_IN(list, atom, x)     PDP_WORD_IN(list, atom, x, w_int)
#define PDP_FLOAT_IN(list, atom, x)   PDP_WORD_IN(list, atom, x, w_float)
#define PDP_SYMBOL_IN(list, atom, x)  PDP_WORD_IN(list, atom, x, w_symbol)
#define PDP_PACKET_IN(list, atom, x)  PDP_WORD_IN(list, atom, x, w_packet)
#define PDP_LIST_IN(list, atom, x)    PDP_WORD_IN(list, atom, x, w_list)


/* some macros for the pointer type */

#define pdp_list_add_pointer(l,p)          pdp_list_add(l, a_pointer, ((t_pdp_word)((void *)(p))))
#define pdp_list_add_back_pointer(l,p)     pdp_list_add_back(l, a_pointer, ((t_pdp_word)((void *)(p))))
#define pdp_list_add_pointer_to_set(l,p)   pdp_list_add_to_set(l, a_pointer, ((t_pdp_word)((void *)(p))))
#define pdp_list_remove_pointer(l,p)       pdp_list_remove(l, a_pointer, ((t_pdp_word)((void *)(p))))
#define pdp_list_contains_pointer(l,p)     pdp_list_contains(l, a_pointer, ((t_pdp_word)((void *)(p))))

/* atom access */
#define PDP_LIST_ATOM_0(x) ((x)->first)
#define PDP_LIST_ATOM_1(x) ((x)->first->next)
#define PDP_LIST_ATOM_2(x) ((x)->first->next->next)
#define PDP_LIST_ATOM_3(x) ((x)->first->next->next->next)
#define PDP_LIST_ATOM_4(x) ((x)->first->next->next->next->next)

/* array like setters */
static inline void pdp_atom_set(t_pdp_atom *a, t_pdp_word_type t, t_pdp_word w) {a->t = t; a->w = w;}
static inline void pdp_list_set_0(t_pdp_list *l, t_pdp_word_type t, t_pdp_word w) {pdp_atom_set(l->first, t, w);}
static inline void pdp_list_set_1(t_pdp_list *l, t_pdp_word_type t, t_pdp_word w) {pdp_atom_set(l->first->next, t, w);}
static inline void pdp_list_set_2(t_pdp_list *l, t_pdp_word_type t, t_pdp_word w) {pdp_atom_set(l->first->next->next, t, w);}
  

/* evaluator (tiny lisp) */



typedef t_pdp_list* (*t_pdp_list_evaluator_function)(t_pdp_list *);


#endif
