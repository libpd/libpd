/*
 *   Pure Data Packet system implementation. : symbol and namespace stuff
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

#ifndef _PDP_SYMBOL_
#define _PDP_SYMBOL_


/* pdp's symbols are derived from pd's symbols
   there is one symbol hash. each symbol has
   a meaning in several name spaces.

   * forth words
   * type description lists (for accelerating type matching)


*/

#include "pdp_list.h"




/* the pdp symbol type */
typedef struct _pdp_symbol
{
    /* next */
    struct _pdp_symbol *s_next;

    /* the symbol name */
    char *s_name;

    /* forth symbol->atom */
    struct _pdp_atom s_forth;

    /* packet handling cache */
    struct _pdp_list *s_type;                // a parsed type description: a/b/c -> (a,b,c)
    struct _pdp_list *s_reusefifo;           // packet pool fifo for this type


} t_pdp_symbol;



#ifdef __cplusplus
extern "C"
{
#endif

/* namespace stuff */
int pdp_symbol_set_typelist(t_pdp_symbol *s, struct _pdp_list *typelist);

/* get symbol from char */
t_pdp_symbol *pdp_gensym(char *s);

/* iterate over all symbols */
typedef void (*t_pdp_symbol_iterator)(t_pdp_symbol *s);
void pdp_symbol_apply_all(t_pdp_symbol_iterator ir);

// don't use these directly, use the macros
extern t_pdp_symbol _pdp_sym_wildcard;
extern t_pdp_symbol _pdp_sym_float;
extern t_pdp_symbol _pdp_sym_int;
extern t_pdp_symbol _pdp_sym_symbol;
extern t_pdp_symbol _pdp_sym_packet;
extern t_pdp_symbol _pdp_sym_pointer;
extern t_pdp_symbol _pdp_sym_list;
extern t_pdp_symbol _pdp_sym_invalid;
extern t_pdp_symbol _pdp_sym_question_mark;
extern t_pdp_symbol _pdp_sym_atom;
extern t_pdp_symbol _pdp_sym_null;
extern t_pdp_symbol _pdp_sym_quote_start;
extern t_pdp_symbol _pdp_sym_quote_end;
extern t_pdp_symbol _pdp_sym_return;
extern t_pdp_symbol _pdp_sym_nreturn;
extern t_pdp_symbol _pdp_sym_defstart;
extern t_pdp_symbol _pdp_sym_defend;
extern t_pdp_symbol _pdp_sym_if;
extern t_pdp_symbol _pdp_sym_then;
extern t_pdp_symbol _pdp_sym_local;
extern t_pdp_symbol _pdp_sym_forth;
extern t_pdp_symbol _pdp_sym_call;
extern t_pdp_symbol _pdp_sym_push;
extern t_pdp_symbol _pdp_sym_pop;


#ifdef __cplusplus
}
#endif

// these symbols are used a lot in critical parts
// optimize later

#define PDP_SYM_WILDCARD       &_pdp_sym_wildcard
#define PDP_SYM_FLOAT          &_pdp_sym_float
#define PDP_SYM_INT            &_pdp_sym_int
#define PDP_SYM_SYMBOL         &_pdp_sym_symbol
#define PDP_SYM_PACKET         &_pdp_sym_packet
#define PDP_SYM_POINTER        &_pdp_sym_pointer
#define PDP_SYM_LIST           &_pdp_sym_list
#define PDP_SYM_INVALID        &_pdp_sym_invalid
#define PDP_SYM_QUESTION_MARK  &_pdp_sym_question_mark
#define PDP_SYM_ATOM           &_pdp_sym_atom
#define PDP_SYM_NULL           &_pdp_sym_null
#define PDP_SYM_QUOTE_START    &_pdp_sym_quote_start
#define PDP_SYM_QUOTE_END      &_pdp_sym_quote_end
#define PDP_SYM_RETURN         &_pdp_sym_return
#define PDP_SYM_NRETURN        &_pdp_sym_nreturn
#define PDP_SYM_DEF_START      &_pdp_sym_defstart
#define PDP_SYM_DEF_END        &_pdp_sym_defend
#define PDP_SYM_IF             &_pdp_sym_if
#define PDP_SYM_THEN           &_pdp_sym_then
#define PDP_SYM_LOCAL          &_pdp_sym_local
#define PDP_SYM_FORTH          &_pdp_sym_forth
#define PDP_SYM_CALL           &_pdp_sym_call
#define PDP_SYM_PUSH           &_pdp_sym_push
#define PDP_SYM_POP            &_pdp_sym_pop

#endif

