
/*=============================================================================*\
 * File: gfsmArith.h
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

#ifndef _GFSM_ARITH_H
#define _GFSM_ARITH_H

#include <gfsmAutomaton.h>

/** \file gfsmArith.h
 *  \brief Arithmetic operations on automata
 */

/*======================================================================
 * Types
 */
/** Type representing an elementary arithmetic operation */
typedef enum {
  //-- real ops
  gfsmAONone,    ///< No operation:             \a w=w
  gfsmAOExp,     ///< Real Exponentiation:      \a w=exp(w)
  gfsmAOLog,     ///< Real Logarithm:           \a w=log(w)
  gfsmAONoNeg,   ///< Real Force-positive:      \a w=(w < 0 ? -w : w)
  gfsmAOAdd,     ///< Real Addition:            \a w=w+arg
  gfsmAOMult,    ///< Real Multiplication:      \a w=w*arg
  //-- semiring ops
  gfsmAOSRNoNeg,  ///< Semiring Force-positve:   \a w=(sr_less(sr_zero,w) ? sr_zero : w)
  gfsmAOSRPlus,   ///< Semiring Addition:        \a w=sr_plus(w,arg)
  gfsmAOSRTimes   ///< Semiring Multiplication:  \a w=sr_times(w,arg)
} gfsmArithOp;

/** \brief Type representing all parameters for a generic automaton arithmetic operation */
typedef struct {
  gfsmAutomaton    *fsm;            ///< Automaton
  gfsmArithOp       op;             ///< Operation
  gfsmWeight        arg;            ///< 2nd operation argument (if any)
  gboolean          do_zero;        ///< operate on semiring-zeroes?
} gfsmArithParams;


/*======================================================================
 * Methods: arithmetic: generic
 */
///\name Arithmetic (Generic)
//@{

//------------------------------
/** Perform a generic arithmetic operation on \a fsm.
 *  \note Destructively alters \a fsm.
 *
 *  \param fsm      Automaton
 *  \param op       Operation
 *  \parma arg      Argument of operation (if any)
 *  \param lo       Perform only for arcs with lower label \a lo (gfsmNoLabel for any label)
 *  \param hi       Perform only for arcs with upper label \a hi (gfsmNoLabel for any label)
 *  \param do_arcs  Perform operation on arc weights
 *  \param do_final Perform operation on final weights
 *  \param do_zero  Perform operation on zero weights
 *
 *  \returns modified \a fsm
 */
gfsmAutomaton *gfsm_automaton_arith(gfsmAutomaton    *fsm,
				    gfsmArithOp       op,
				    gfsmWeight        arg,
				    gfsmLabelVal      lo,
				    gfsmLabelVal      hi,
				    gboolean          do_arcs,
				    gboolean          do_final,
				    gboolean          do_zero);

//------------------------------
/** Perform a generic arithmetic operation on state \a qid in \a fsm.
 *  \note Destructively alters \a fsm.
 *
 *  \param fsm      Automaton
 *  \param qid      State-id in \a fsm, or gfsmNoState for all states
 *  \param op       Operation
 *  \parma arg      Argument of operation (if any)
 *  \param lo       Perform only for arcs with lower label \a lo (gfsmNoLabel for any label)
 *  \param hi       Perform only for arcs with upper label \a hi (gfsmNoLabel for any label)
 *  \param do_arcs  Perform operation on arc weights
 *  \param do_final Perform operation on final weights
 *  \param do_zero  Perform operation on zero weights
 *
 *  \returns modified \a fsm
 */
gfsmAutomaton *gfsm_automaton_arith_state(gfsmAutomaton    *fsm,
					  gfsmStateId       qid,
					  gfsmArithOp       op,
					  gfsmWeight        arg,
					  gfsmLabelVal      lo,
					  gfsmLabelVal      hi,
					  gboolean          do_arcs,
					  gboolean          do_final,
					  gboolean          do_zero);


//------------------------------
/** Perform a generic arithmetic operation on final weights
 *  \returns params->fsm
 */
gfsmAutomaton *gfsm_automaton_arith_final(gfsmAutomaton    *fsm,
					  gfsmArithOp       op,
					  gfsmWeight        arg,
					  gboolean          do_zero);

//------------------------------
/** Perform a generic arithmetic operation on a gfsmWeight.
 *  \returns result of operation
 */
gfsmWeight gfsm_weight_arith(gfsmSemiring *sr,
			     gfsmArithOp   op,
			     gfsmWeight    w1,
			     gfsmWeight    w2,
			     gboolean      do_zero);

//@}
#endif /* _GFSM_ARITH_H */
