/*
 *   Pure Data Packet system implementation. : code implementing pdp's namespace (symbols)
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

#include <string.h>
#include <pthread.h>
#include "pdp_symbol.h"
#include "pdp_list.h"
#include "pdp_debug.h"

// some extra prototypes
void *pdp_alloc(int size);
void pdp_dealloc(void *data);

// the symbol hash mutex
static pthread_mutex_t pdp_hash_mutex;

#define HASHSIZE 1024
static t_pdp_symbol *pdp_symhash[HASHSIZE];


#define LOCK pthread_mutex_lock(&pdp_hash_mutex)
#define UNLOCK pthread_mutex_unlock(&pdp_hash_mutex)


static void _pdp_symbol_init(t_pdp_symbol *s)
{
    memset(s, 0, sizeof(*s));
    s->s_forth.t = a_undef;
}


/* shamelessly copied from pd src and made thread safe */
t_pdp_symbol *_pdp_dogensym(char *s, t_pdp_symbol *oldsym)
{
    t_pdp_symbol **sym1, *sym2;
    unsigned int hash1 = 0,  hash2 = 0;
    int length = 0;
    char *s2 = s;
    while (*s2)
    {
        hash1 += *s2;
        hash2 += hash1;
        length++;
        s2++;
    }
    sym1 = pdp_symhash + (hash2 & (HASHSIZE-1));

    /* lock hash */
    LOCK;

    while (sym2 = *sym1)
    {
        if (!strcmp(sym2->s_name, s)) goto gotit;
        sym1 = &sym2->s_next;
    }
    if (oldsym){
	sym2 = oldsym;
    }
    else
    {
        sym2 = (t_pdp_symbol *)pdp_alloc(sizeof(*sym2));
	_pdp_symbol_init(sym2);
        sym2->s_name = pdp_alloc(length+1);
        sym2->s_next = 0;
        strcpy(sym2->s_name, s);
    }
    *sym1 = sym2;

 gotit:

    /* unlock hash */
    UNLOCK;
    return (sym2);
}

t_pdp_symbol *pdp_gensym(char *s)
{
    return(_pdp_dogensym(s, 0));
}


/* connect a parsed typelist to a symbol type name
   1 = succes, 0 = error (symbol already connected) */
int pdp_symbol_set_typelist(t_pdp_symbol *s, t_pdp_list *typelist)
{
    int status = 0;
    LOCK;
    if (!s->s_type){
	s->s_type = typelist;
	status = 1;
    }
    UNLOCK;
    return status;
}


void pdp_symbol_apply_all(t_pdp_symbol_iterator it)
{
    int i;
    for (i=0; i<HASHSIZE; i++){
	t_pdp_symbol *s;
	for (s = pdp_symhash[i]; s; s=s->s_next){
	    it(s);
	}
	
    }
}

t_pdp_symbol _pdp_sym_wildcard;
t_pdp_symbol _pdp_sym_float;
t_pdp_symbol _pdp_sym_int;
t_pdp_symbol _pdp_sym_symbol;
t_pdp_symbol _pdp_sym_packet;
t_pdp_symbol _pdp_sym_pointer;
t_pdp_symbol _pdp_sym_invalid;
t_pdp_symbol _pdp_sym_list;
t_pdp_symbol _pdp_sym_question_mark;
t_pdp_symbol _pdp_sym_atom;
t_pdp_symbol _pdp_sym_null;
t_pdp_symbol _pdp_sym_quote_start;
t_pdp_symbol _pdp_sym_quote_end;
t_pdp_symbol _pdp_sym_return;
t_pdp_symbol _pdp_sym_nreturn;
t_pdp_symbol _pdp_sym_defstart;
t_pdp_symbol _pdp_sym_defend;
t_pdp_symbol _pdp_sym_if;
t_pdp_symbol _pdp_sym_then;
t_pdp_symbol _pdp_sym_local;
t_pdp_symbol _pdp_sym_forth;
t_pdp_symbol _pdp_sym_call;
t_pdp_symbol _pdp_sym_push;
t_pdp_symbol _pdp_sym_pop;

static void _sym(char *name, t_pdp_symbol *s)
{
    t_pdp_symbol *realsym;
    _pdp_symbol_init(s);
    s->s_name = name;
    realsym = _pdp_dogensym(name, s); 
    PDP_ASSERT(realsym == s); // if this fails, the symbol was already defined
}

void pdp_symbol_setup(void)
{
    // create mutexes
    pthread_mutex_init(&pdp_hash_mutex, NULL);

    // init symbol hash
    memset(pdp_symhash, 0, HASHSIZE * sizeof(t_pdp_symbol *));

    // setup predefined symbols (those that have direct pointer access for speedup)
    _sym("*",        &_pdp_sym_wildcard);
    _sym("float",    &_pdp_sym_float);
    _sym("int",      &_pdp_sym_int);
    _sym("symbol",   &_pdp_sym_symbol);
    _sym("packet",   &_pdp_sym_packet);
    _sym("pointer",  &_pdp_sym_pointer);
    _sym("invalid",  &_pdp_sym_invalid);
    _sym("list",     &_pdp_sym_list);
    _sym("?",        &_pdp_sym_question_mark);
    _sym("atom",     &_pdp_sym_atom);
    _sym("null",     &_pdp_sym_null);
    _sym("[",        &_pdp_sym_quote_start);
    _sym("]",        &_pdp_sym_quote_end);
    _sym("ret",      &_pdp_sym_return);
    _sym("nret",     &_pdp_sym_nreturn);
    _sym(":",        &_pdp_sym_defstart);
    _sym(";",        &_pdp_sym_defend);
    _sym("if",       &_pdp_sym_if);
    _sym("then",     &_pdp_sym_then);
    _sym("local",    &_pdp_sym_local);
    _sym("forth",    &_pdp_sym_forth);
    _sym("call",     &_pdp_sym_call);
    _sym("push",     &_pdp_sym_push);
    _sym("pop",      &_pdp_sym_pop);

}


