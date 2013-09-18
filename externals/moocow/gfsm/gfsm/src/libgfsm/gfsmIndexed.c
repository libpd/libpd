
/*=============================================================================*\
 * File: gfsmIndexed.c
 * Author: Bryan Jurish <moocow@ling.uni-potsdam.de>
 * Description: finite state machine library: arc indices
 *
 * Copyright (c) 2007 Bryan Jurish.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *=============================================================================*/

#include <gfsmIndexed.h>
#include <gfsmArcIter.h>

//-- no-inline definitions
#ifndef GFSM_INLINE_ENABLED
# include <gfsmIndexed.hi>
#endif

/*======================================================================
 * Constructors etc.
 */

//----------------------------------------
gfsmIndexedAutomaton *gfsm_indexed_automaton_copy(gfsmIndexedAutomaton *dst, gfsmIndexedAutomaton *src)
{
  if (!dst) {
    dst = gfsm_indexed_automaton_new_full(src->flags,
					  src->sr->type,
					  gfsm_indexed_automaton_n_states(src),
					  gfsm_indexed_automaton_n_arcs(src));
  }
  else {
    gfsm_indexed_automaton_clear(dst);
    gfsm_indexed_automaton_reserve_states(dst, gfsm_indexed_automaton_n_states(src));
    gfsm_indexed_automaton_reserve_arcs  (dst, gfsm_indexed_automaton_n_arcs(src)  );
  }

  //-- copy: flags, semiring, root
  dst->flags = src->flags;
  gfsm_indexed_automaton_set_semiring(dst, src->sr);
  dst->root_id = src->root_id;

  //-- copy: tables
  gfsm_weight_vector_copy  (dst->state_final_weight, src->state_final_weight);
  gfsm_arc_table_index_copy(dst->arcs, src->arcs);

  return dst;
}

/*======================================================================
 * Methods: Import & Export
 */

//----------------------------------------
gfsmIndexedAutomaton *gfsm_automaton_to_indexed(gfsmAutomaton *fsm, gfsmIndexedAutomaton *xfsm)
{
  //-- maybe allocate new indexed automaton
  if (xfsm==NULL) {
    xfsm = gfsm_indexed_automaton_new_full(fsm->flags,
					   fsm->sr->type,
					   gfsm_automaton_n_states(fsm),
					   gfsm_automaton_n_arcs(fsm)
					   );
  } else {
    gfsm_indexed_automaton_clear(xfsm);
    xfsm->flags = fsm->flags;
    gfsm_indexed_automaton_reserve_states(xfsm,gfsm_automaton_n_states(fsm));
    gfsm_indexed_automaton_reserve_arcs(xfsm,gfsm_automaton_n_arcs(fsm));
  }
  gfsm_indexed_automaton_set_semiring(xfsm,fsm->sr); //-- copy semiring

  //-- set root id
  xfsm->root_id = fsm->root_id;

  //-- index final weights
  gfsm_automaton_to_final_weight_vector(fsm, xfsm->state_final_weight);
  gfsm_automaton_to_arc_table_index(fsm, xfsm->arcs);

  //-- sort arcs (no!)
  //gfsm_indexed_automaton_sort(xfsm, xfsm->flags.sort_mode);


  return xfsm;
}



//----------------------------------------
gfsmAutomaton *gfsm_indexed_to_automaton(gfsmIndexedAutomaton *xfsm, gfsmAutomaton *fsm)
{
  gfsmStateId qid;
  gfsmWeight  srzero;

  //-- maybe allocate new automaton
  if (fsm==NULL) {
    fsm = gfsm_automaton_new_full(xfsm->flags, xfsm->sr->type, gfsm_indexed_automaton_n_states(xfsm));
  } else {
    gfsm_automaton_clear(fsm);
    fsm->flags = xfsm->flags;
    gfsm_automaton_set_semiring(fsm, gfsm_semiring_copy(xfsm->sr));
    gfsm_automaton_reserve(fsm, gfsm_indexed_automaton_n_states(xfsm));
  }

  //-- set root id
  fsm->root_id = xfsm->root_id;

  //-- update state-wise
  srzero = xfsm->sr->zero;
  for (qid=0; qid < xfsm->state_final_weight->len; qid++) {
    gfsmArcRange range;

    //-- state_final_weight
    gfsmWeight fw = g_array_index(xfsm->state_final_weight,gfsmWeight,qid);
    if (fw != srzero) { gfsm_automaton_set_final_state_full(fsm,qid,TRUE,fw); }

    //-- arcs
    for (gfsm_arcrange_open_indexed(&range, xfsm, qid); gfsm_arcrange_ok(&range); gfsm_arcrange_next(&range)) {
      gfsmArc *a = gfsm_arcrange_arc(&range);
      gfsm_automaton_add_arc(fsm,a->source,a->target,a->lower,a->upper,a->weight);
    }
    gfsm_arcrange_close(&range);
  }

  return fsm;
}

/*======================================================================
 * Methods: Accessors: gfsmIndexedAutomaton
 */
//-- inlined

/*======================================================================
 * Methods: Accessors: gfsmAutomaton API: Automaton
 */
//-- inlined

/*======================================================================
 * Methods: Accessors: gfsmAutomaton API: States
 */
//-- inlined

/*======================================================================
 * I/O
 */
