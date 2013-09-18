
/*=============================================================================*\
 * File: gfsmIndexed.h
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

/** \file gfsmIndexed.h
 *  \brief First stab at indexed automata
 */

#ifndef _GFSM_INDEXED_H
#define _GFSM_INDEXED_H

#include <gfsmArcIndex.h>

/*======================================================================
 * Types
 */

/// Type for an indexed automaton.
typedef struct {
  //-- gfsmAutomaton compatibility
  gfsmAutomatonFlags  flags;              /**< automaton flags, for ::gfsmAutomaton compatibility */
  gfsmSemiring       *sr;                 /**< semiring used for arc weight computations */
  gfsmStateId         root_id;            /**< id of root state, or gfsmNoState if not defined */
  //
  //-- Basic data
  //gfsmBitVector      *state_is_valid;     /* per-state validity flags */
  gfsmWeightVector   *state_final_weight; /**< State final weight, or sr->zero */
  gfsmArcTableIndex  *arcs;               /**< Arc storage (sorted primarily by source state) */
} gfsmIndexedAutomaton;

/*======================================================================
 * Methods: gfsmIndexedAutomaton: constructors, etc.
 */
/// \name Constructors etc.
//@{

/** Create a new ::gfsmIndexedAutomaton, specifying some basic automaton & index structure */
GFSM_INLINE
gfsmIndexedAutomaton *gfsm_indexed_automaton_new_full(gfsmAutomatonFlags flags,
						      gfsmSRType         srtype,
						      gfsmStateId        n_states,
						      guint              n_arcs);

/** Create a new indexed automaton, using some default values */
GFSM_INLINE
gfsmIndexedAutomaton *gfsm_indexed_automaton_new(void);

/** Copy a ::gfsmIndexedAutomaton \a src to \a dst.  \returns \a dst */
gfsmIndexedAutomaton *gfsm_indexed_automaton_copy(gfsmIndexedAutomaton *dst, gfsmIndexedAutomaton *src);

/** Create and return an exact clone of a ::gfsmIndexedAutomaton */
GFSM_INLINE
gfsmIndexedAutomaton *gfsm_indexed_automaton_clone(gfsmIndexedAutomaton *xfsm);

/** Clear a ::gfsmIndexedAutomaton */
GFSM_INLINE
void gfsm_indexed_automaton_clear(gfsmIndexedAutomaton *xfsm);

/** Free a ::gfsmIndexedAutomaton */
GFSM_INLINE
void gfsm_indexed_automaton_free(gfsmIndexedAutomaton *xfsm);

//@}

/*======================================================================
 * Methods: Import & Export
 */
/// \name Import & Export
//@{

/** Populate a ::gfsmIndexedAutomaton from a ::gfsmAutomaton
 *  \param fsm source automaton
 *  \param xfsm destination indexed automaton,
 *         may be passed as NULL to create a new ::gfsmIndexedAutomaton
 *  \returns (new) indexed automaton \a xfsm
 *  \note implicitly clears \a xfsm
 */
gfsmIndexedAutomaton *gfsm_automaton_to_indexed(gfsmAutomaton *fsm, gfsmIndexedAutomaton *xfsm);

/** Export a ::gfsmIndexedAutomaton to a ::gfsmAutomaton
 *  \param xfsm source indexed automaton
 *  \param fsm destination :.gfsmAutomaton
 *         may be passed as NULL to create a new ::gfsmAutomaton
 *  \returns (new) automaton \a fsm
 *  \note implicitly clears \a fsm
 */
gfsmAutomaton *gfsm_indexed_to_automaton(gfsmIndexedAutomaton *xfsm, gfsmAutomaton *fsm);

//@}

/*======================================================================
 * Methods: Accessors: gfsmIndexedAutomaton
 */
/// \name Accessors: gfsmIndexedAutomaton
//@{

/** Reserve space for at least \a n_states states */
GFSM_INLINE
void gfsm_indexed_automaton_reserve_states(gfsmIndexedAutomaton *xfsm, gfsmStateId n_states);

/** Reserve space for at least \a n_arcs arcs */
GFSM_INLINE
void gfsm_indexed_automaton_reserve_arcs(gfsmIndexedAutomaton *xfsm, guint n_arcs);

/** (re-)sort arcs in a ::gfsmIndexedAutomaton */
GFSM_INLINE
void gfsm_indexed_automaton_sort(gfsmIndexedAutomaton *xfsm, gfsmArcCompMask sort_mask);

//@}

/*======================================================================
 * gfsmAutomaton API: Automaton properties
 */
/// \name gfsmAutomaton API: automaton properties
//@{

/** Get pointer to the semiring associated with this automaton */
#define gfsm_indexed_automaton_get_semiring(xfsm) (xfsm->sr)

/** Set the semiring associated with this automaton */
GFSM_INLINE
gfsmSemiring *gfsm_indexed_automaton_set_semiring(gfsmIndexedAutomaton *xfsm, gfsmSemiring *sr);

/** Set the semiring associated with this automaton by semiring-type */
GFSM_INLINE
void gfsm_indexed_automaton_set_semiring_type(gfsmIndexedAutomaton *xfsm, gfsmSRType srtype);

/** Get number of states (constant time) */
GFSM_INLINE
gfsmStateId gfsm_indexed_automaton_n_states(gfsmIndexedAutomaton *xfsm);

/** Get total number of arcs (constant time) */
GFSM_INLINE
guint gfsm_indexed_automaton_n_arcs(gfsmIndexedAutomaton *xfsm);

/** Get Id of root node, or gfsmNoState if undefined */
GFSM_INLINE
gfsmStateId gfsm_indexed_automaton_get_root(gfsmIndexedAutomaton *xfsm);

/** Set Id of root node, creating state if necessary */
GFSM_INLINE
void gfsm_indexed_automaton_set_root(gfsmIndexedAutomaton *xfsm, gfsmStateId qid);

//@}

/*======================================================================
 * Methods: Accessors: gfsmAutomaton API: States
 */
/// \name gfsmAutomaton API: States
//@{

/** Check whether automaton has a state with ID \a qid. */
GFSM_INLINE
gboolean gfsm_indexed_automaton_has_state(gfsmIndexedAutomaton *xfsm, gfsmStateId qid);

/** Ensures that state \a id exists \returns \a qid */
GFSM_INLINE
gfsmStateId gfsm_indexed_automaton_ensure_state(gfsmIndexedAutomaton *xfsm, gfsmStateId qid);

/* Remove the state with id \a qid, if any.
 * Currently does nothing.
 */
GFSM_INLINE
void gfsm_indexed_automaton_remove_state(gfsmIndexedAutomaton *fsm, gfsmStateId qid);

/** Set boolean final-state flag. \returns (void) */
#define gfsm_indexed_automaton_set_final_state(xfsm,qid,is_final) \
  gfsm_indexed_automaton_set_final_state_full((xfsm),(qid),(is_final),(xfsm)->sr->one)

/** Set final weight. \returns (void) */
GFSM_INLINE
void gfsm_indexed_automaton_set_final_state_full(gfsmIndexedAutomaton *fsm,
						 gfsmStateId    qid,
						 gboolean       is_final,
						 gfsmWeight     final_weight);

/** Lookup final weight. \returns TRUE iff state \a id is final, and sets \a *wp to its final weight. */
GFSM_INLINE
gboolean gfsm_indexed_automaton_lookup_final(gfsmIndexedAutomaton *fsm, gfsmStateId id, gfsmWeight *wp);

/** Is \a qid final in \a xfsm?  Really just wraps gfsm_indexed_automaton_lookup_final() */
GFSM_INLINE
gboolean gfsm_indexed_automaton_state_is_final(gfsmIndexedAutomaton *xfsm, gfsmStateId qid);

/** Get final weight for \a qid final in \a xfsm?  Really just wraps gfsm_indexed_automaton_lookup_final() */
GFSM_INLINE
gfsmWeight gfsm_indexed_automaton_get_final_weight(gfsmIndexedAutomaton *xfsm, gfsmStateId qid);

/** Get number of outgoing arcs. \returns guint */
GFSM_INLINE
guint gfsm_indexed_automaton_out_degree(gfsmIndexedAutomaton *fsm, gfsmStateId qid);

//@}

/*======================================================================
 * ArcRange
 */
///\name gfsmArcRange interface
//@{

/** Open a ::gfsmArcRange for outgoing arcs from state \a qid in \a xfsm */
GFSM_INLINE
void gfsm_arcrange_open_indexed(gfsmArcRange *range, gfsmIndexedAutomaton *xfsm, gfsmStateId qid);

//@}

//-- inline definitions
#ifdef GFSM_INLINE_ENABLED
# include <gfsmIndexed.hi>
#endif

#endif /* _GFSM_INDEXED_H */
