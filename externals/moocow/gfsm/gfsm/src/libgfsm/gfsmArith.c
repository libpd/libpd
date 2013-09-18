/*=============================================================================*\
 * File: gfsmArith.c
 * Author: Bryan Jurish <moocow@ling.uni-potsdam.de>
 * Description: finite state machine library
 *
 * Copyright (c) 2005-2007 Bryan Jurish.
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

#include <math.h>
#include <glib.h>
#include <gfsmArith.h>
#include <gfsmArcIter.h>


/*======================================================================
 * Methods: arithmetic: Generic
 */

//--------------------------------------------------------------
gfsmAutomaton *gfsm_automaton_arith(gfsmAutomaton    *fsm,
				    gfsmArithOp       op,
				    gfsmWeight        arg,
				    gfsmLabelVal      lo,
				    gfsmLabelVal      hi,
				    gboolean          do_arcs,
				    gboolean          do_final,
				    gboolean          do_zero)
{
  if (op == gfsmAONone) return fsm; //-- dummy operation

  //-- arc weights
  if (do_arcs) {
    gfsmStateId qid;

    if (fsm->flags.sort_mode == gfsmASMWeight)
      fsm->flags.sort_mode = gfsmASMNone; //-- arc-weights may be destructively altered

    for (qid=0; qid < fsm->states->len; qid++) {
      gfsmArcIter ai;
      for (gfsm_arciter_open(&ai,fsm,qid), gfsm_arciter_seek_both(&ai,lo,hi);
	   gfsm_arciter_ok(&ai);
	   gfsm_arciter_next(&ai), gfsm_arciter_seek_both(&ai,lo,hi))
	{
	  gfsmArc *arc = gfsm_arciter_arc(&ai);
	  arc->weight  = gfsm_weight_arith(fsm->sr, op, arc->weight, arg, do_zero);
	}
    }
  }

  //-- final weights
  if (do_final) gfsm_automaton_arith_final(fsm, op, arg, do_zero);

  return fsm;
}

//--------------------------------------------------------------
gfsmAutomaton *gfsm_automaton_arith_state(gfsmAutomaton    *fsm,
					  gfsmStateId       qid,
					  gfsmArithOp       op,
					  gfsmWeight        arg,
					  gfsmLabelVal      lo,
					  gfsmLabelVal      hi,
					  gboolean          do_arcs,
					  gboolean          do_final,
					  gboolean          do_zero)
{
  if (qid==gfsmNoState) return gfsm_automaton_arith(fsm,op,arg,lo,hi,do_arcs,do_final,do_zero);

  //-- arc weights
  if (do_arcs) {
    gfsmArcIter ai;

    if (fsm->flags.sort_mode == gfsmASMWeight)
      fsm->flags.sort_mode = gfsmASMNone; //-- arc-weights may be destructively altered

    for (gfsm_arciter_open(&ai,fsm,qid), gfsm_arciter_seek_both(&ai,lo,hi);
	 gfsm_arciter_ok(&ai);
	 gfsm_arciter_next(&ai), gfsm_arciter_seek_both(&ai,lo,hi))
      {
	gfsmArc *arc = gfsm_arciter_arc(&ai);
	arc->weight  = gfsm_weight_arith(fsm->sr, op, arc->weight, arg, do_zero);
      }
  }

  //-- final weight
  if (do_final) {
    gfsm_automaton_set_final_state_full(fsm,
					qid,
					gfsm_automaton_is_final_state(fsm, qid),
					gfsm_weight_arith(fsm->sr,
							  op,
							  gfsm_automaton_get_final_weight(fsm, qid),
							  arg,
							  do_zero));
  }

  return fsm;
}

//--------------------------------------------------------------
gfsmWeight gfsm_weight_arith(gfsmSemiring *sr,
			     gfsmArithOp   op,
			     gfsmWeight    w1,
			     gfsmWeight    w2,
			     gboolean      do_zero)
{
  if (!do_zero && w1==sr->zero) return w1;

  switch (op) {

  case gfsmAOExp:   ///< Exponentiate
    return expf(w1);
    break;

  case gfsmAOLog:   ///< Logarithm
    return logf(w1);
    break;

  case gfsmAONoNeg:   ///< Real force-positive
    return (w1 < 0 ? (-w1) : w1);
    break;

  case gfsmAOAdd: ///< Real Addition
    return w1+w2;
    break;

  case gfsmAOMult: ///< Real Multiplication
    return w1*w2;
    break;

  case gfsmAOSRNoNeg: ///< Semiring Force positive
    return (gfsm_sr_less(sr,sr->zero,w1) ? sr->zero : w1);
    break;

  case gfsmAOSRPlus:   ///< Semiring Addition
    return gfsm_sr_plus(sr,w1,w2);
    break;

  case gfsmAOSRTimes:  ///< Semiring Multiplication
    return gfsm_sr_times(sr,w1,w2);
    break;

  case gfsmAONone:  ///< No operation
  default:
    return w1;
  }
  return w1; //-- should never happen
}



/*======================================================================
 * Methods: arithmetic: final
 */

//--------------------------------------------------------------
gboolean _gfsm_automaton_arith_final_foreach_func(gfsmStateId      id,
						  gpointer         pw,
						  gfsmArithParams *params)
{
  gfsmWeight w = gfsm_ptr2weight(pw);
  gfsm_weightmap_insert(params->fsm->finals,
			GUINT_TO_POINTER(id),
			gfsm_weight_arith(params->fsm->sr,
					  params->op,
					  w,
					  params->arg,
					  params->do_zero));
  return FALSE;
}


//--------------------------------------------------------------
gfsmAutomaton *gfsm_automaton_arith_final(gfsmAutomaton    *fsm,
					  gfsmArithOp       op,
					  gfsmWeight        arg,
					  gboolean          do_zero)
{
  gfsmArithParams params = { fsm, op, arg, do_zero };
  g_tree_foreach(fsm->finals,
		 (GTraverseFunc)_gfsm_automaton_arith_final_foreach_func,
		 &params);
  return fsm;
}

