
/*=============================================================================*\
 * File: gfsmAutomaton.h
 * Author: Bryan Jurish <moocow@ling.uni-potsdam.de>
 * Description: finite state machine library: automata
 *
 * Copyright (c) 2004-2007 Bryan Jurish.
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

/** \file gfsmAutomaton.h
 *  \brief Automaton definitions and low-level access
 */

#ifndef _GFSM_AUTOMATON_H
#define _GFSM_AUTOMATON_H

#include <gfsmAlphabet.h>
#include <gfsmState.h>
#include <gfsmWeightMap.h>
#include <gfsmBitVector.h>

/*======================================================================
 * Types
 */

/** \brief Automaton status flags
 *  \detail
 *  \todo convert ::gfsmAutomatonFlags flags to 3-valued domain: unknown,true,false
 *  \todo add better checking & access for automaton status flags
 */
typedef struct {
  guint32 is_transducer     : 1;       /**< whether this automaton is a transducer */
  guint32 is_weighted       : 1;       /**< whether this automaton is weighted */
  guint32 is_deterministic  : 1;       /**< whether fsm is known to be deterministic */
  guint32 sort_mode         : 24;      /**< new-style sort mode (a ::gfsmArcCompMask) */
  guint32 unused            : 5;       /**< reserved */
} gfsmAutomatonFlags;

/** \brief "Heavy" automaton type
 *  
 *  All automata are stored as weighted transducers.
 */
typedef struct {
  //-- basic data
  gfsmAutomatonFlags  flags;     /**< automaton flags */
  gfsmSemiring       *sr;        /**< semiring used for arc weight computations */
  GArray             *states;    /**< vector of automaton states */
  gfsmWeightMap      *finals;    /**< map from final state-Ids to final weights */
  gfsmStateId         root_id;   /**< ID of root node, or gfsmNoState if not defined */
} gfsmAutomaton;

/*======================================================================
 * Constants
 */
/** Default initial automaton size (number of states) */
extern const gfsmStateId gfsmAutomatonDefaultSize;

/** Default initial automaton flags */
extern const gfsmAutomatonFlags gfsmAutomatonDefaultFlags;

/** Default semiring for automaton arc weights */
extern const gfsmSRType gfsmAutomatonDefaultSRType;

/*======================================================================*/
/// \name API: Constructors etc.
//@{

/** Create a new ::gfsmAutomaton, preallocating \a n_states states */
GFSM_INLINE
gfsmAutomaton *gfsm_automaton_new_full(gfsmAutomatonFlags flags, gfsmSRType srtype, gfsmStateId n_states);

/** Create and return a new ::gfsmAutomaton, using default flags, semiring type and size */
GFSM_INLINE
gfsmAutomaton *gfsm_automaton_new(void);

/** Create a new gfsmAutomaton as a deep exact copy of \a fsm.
 *  \param fsm automaton to be cloned
 *  \returns new deep copy of \a src
 */
GFSM_INLINE
gfsmAutomaton *gfsm_automaton_clone(gfsmAutomaton *fsm);

/** Assign non-structural contents (flags, semiring) of \a src to \a dst,
 *  without altering \a dst's topology.
 *  \param dst target automaton
 *  \param src source automaton
 *  \returns modified \a dst
 *  \warning Earlier versions of this function also set the root state of \a dst
 *           to that of \a src, but you should no longer rely on this being the case!
 */
GFSM_INLINE
gfsmAutomaton *gfsm_automaton_copy_shallow(gfsmAutomaton *dst, gfsmAutomaton *src);

/** Assign the contents of fsm \a src to fsm \a dst \returns \a dst */
gfsmAutomaton *gfsm_automaton_copy(gfsmAutomaton *dst, gfsmAutomaton *src);

/** Create a new ::gfsmAutomaton whose non-structural contents match those of \a fsm.
 *  \param fsm source automaton
 *  \returns new automaton whose non-structural fields match those of \a fsm
 */
GFSM_INLINE
gfsmAutomaton *gfsm_automaton_shadow(gfsmAutomaton *fsm);

/** Swap the contents of automata \a fsm1 and \a fsm2 */
GFSM_INLINE
void gfsm_automaton_swap(gfsmAutomaton *fsm1, gfsmAutomaton *fsm2);

/** Clear an automaton */
void gfsm_automaton_clear(gfsmAutomaton *fsm);

/** Destroy an automaton: all associated states and arcs will be freed. */
GFSM_INLINE
void gfsm_automaton_free(gfsmAutomaton *fsm);
//@}


/*======================================================================*/
/// \name API: Automaton Semiring
//@{

/** Get pointer to the semiring associated with this automaton */
GFSM_INLINE
gfsmSemiring *gfsm_automaton_get_semiring(gfsmAutomaton *fsm);

/** Set the semiring associated with this automaton
 *  \param fsm automaton to modify
 *  \param sr  semiring to be copied into \a fsm->sr
 *  \returns pointer to the (new) semiring for \a fsm
 *  \note
 *   \li Implicitly frees the semiring previously associated with \a fsm, if any.
 *  \warning
 *    Prior to libgfsm-v0.0.9 this function returned the parameter \a sr itself
 */
GFSM_INLINE
gfsmSemiring *gfsm_automaton_set_semiring(gfsmAutomaton *fsm, gfsmSemiring *sr);

/** Set the semiring associated with this automaton by type.
 *  \param fsm    automaton whose semiring is to be set
 *  \param srtype type of new semiring
 *  \note
 *   \li If \a fsm's semiring is already of type \a srtype, this function does nothing.
 *   \li If \a srtype is ::gfsmSRTUser, \a fsm's new semiring will be unitialized
 *   \li Implicitly frees the semiring previously associated with \a fsm, if any.
 */
GFSM_INLINE
void gfsm_automaton_set_semiring_type(gfsmAutomaton *fsm, gfsmSRType srtype);

//@}

/*======================================================================*/
///\name API: Automaton Structure
//@{

/** Reserve space for at least \a n_states states (may do nothing)
 *  \param fsm automaton to modify
 *  \param n_states number of states to reserve, if supported by implementation
 */
GFSM_INLINE
void gfsm_automaton_reserve_states(gfsmAutomaton *fsm, gfsmStateId n_states);

/** Backwards-compatible alias for gfsm_automaton_reserve_states()
 *  \deprecated in favor of gfsm_automaton_reserve_states()
 */
#define gfsm_automaton_reserve(fsm,n_states) gfsm_automaton_reserve_states((fsm),(n_states))


/** Reserve space for at least \a n_arcs arcs
 *  \param fsm automaton to modify
 *  \param n_arcs number of arcs to reserve
 *  \note
 *   Currently does nothing.
 */
GFSM_INLINE
void gfsm_automaton_reserve_arcs(gfsmAutomaton *fsm, guint n_arcs);

/** Get the number of states in an automaton (modulo 'gaps' in state ID numbering).
 *  \param fsm automaton to examine
 *  \returns
 *    The least ::gfsmStateId \b q such that for all <b>r >= q</b>,
 *    \a fsm has no state with ID \b r.
 */
GFSM_INLINE
gfsmStateId gfsm_automaton_n_states(gfsmAutomaton *fsm);

/** Get number of final states in \a fsm */
GFSM_INLINE
gfsmStateId gfsm_automaton_n_final_states(gfsmAutomaton *fsm);

/** Get total number of arcs in \a fsm. 
 *  \note
 *   Currently just a wrapper for gfsm_automaton_n_arcs_full(), so time is <em>O(n_arcs)</em>
 */
GFSM_INLINE
guint gfsm_automaton_n_arcs(gfsmAutomaton *fsm);

/** Get ID of root state, or ::gfsmNoState if undefined */
GFSM_INLINE
gfsmStateId gfsm_automaton_get_root(gfsmAutomaton *fsm);

/** Set ID of root state, creating state if necessary.
 *  \param fsm         automaton whose root state is to be set
 *  \param new_root_id ID of new root state
 *   \li If \a new_root_id is ::gfsmNoState, \a fsm is marked as 'unrooted'
 *   \li otherwise, a new state with ID \a new_root_id is implicitly created if none already existed
 */
GFSM_INLINE
void gfsm_automaton_set_root(gfsmAutomaton *fsm, gfsmStateId new_root_id);

/** Call a user-defined function \a func for each final state of \a fsm.
 *  \param fsm  automaton whose final states are to be traversed
 *  \param func \c GTraverseFunc for final states, as for \c g_tree_foreach()
 *  \param data user data for \a func
 *  \warning
 *    \a func may \e not directly alter any final weights of \a fsm.
 *    See GLib documentaion of g_tree_foreach() for details and a workaround.
 *  \note
 *    \a func will be called as <tt>(*func)(gpointer final_stateid, gpointer final_weight, gpointer data)</tt>;
 *    that is, both the ::gfsmStateId \a final_stateid and the final weight \a final_weight will be encoded
 *    as (gpointers).  They can be decoded with GPOINTER_TO_UINT() and gfsm_ptr2weight(), respectively, e.g.
\code
gboolean my_final_func(gpointer id_p, gpointer fw_p, gpointer data) {
  gfsmStateId final_id     = GPOINTER_TO_UINT(id_p);  //-- decode state id
  gfsmWeight  final_weight = gfsm_ptr2weight(fw_p);   //-- decode final weight
  do_something_interesting();                         //-- ... whatever ...
  return FALSE;                                       //-- continue traversal
}
\endcode
*   \see gfsm_automaton_finals_to_array()
 */
GFSM_INLINE
void gfsm_automaton_finals_foreach(gfsmAutomaton *fsm, GTraverseFunc func, gpointer data);

/** Get a GArray of ::gfsmStateWeightPair values for final states of \a fsm.
 *  \param fsm   automaton from which to extract final states
 *  \param array array to be populated, or NULL to allocate a new array
 *  \returns new \a array, or a newly allocated ::gfsmStateWeightPairArray
 *  \note
 *    Caller is responsible for freeing the array returned when it is no longer needed.
 */
GFSM_INLINE
gfsmStateWeightPairArray* gfsm_automaton_finals_to_array(gfsmAutomaton *fsm, gfsmStateWeightPairArray *array);

//@}

/*======================================================================*/
/** \name API: Automaton Properties
 *  Currently quite sketchy; better tracking and checking of automaton flags should be implemented.
 */
//@{

/** True iff automaton is a transducer */
#define gfsm_automaton_is_transducer(fsm) ((fsm)->flags.is_transducer)

/** True iff automaton is weighted */
#define gfsm_automaton_is_weighted(fsm) ((fsm)->flags.is_weighted)

/** Get current automaton arc-sort mode (primary sort) */
#define gfsm_automaton_sortmode(fsm) \
    ((gfsmArcSortMode)(gfsm_acmask_nth((fsm)->flags.sort_mode,0)))

//  ((gfsmArcSortMode)((fsm)->flags.sort_mode))

/** Get verbose summary arc information (linear time w/ number of arcs)
 *  \param[in]  fsm automaton to examine
 *  \param[out] n_lo_epsilon on return holds number of arcs with lower label ::gfsmEpsilon, or NULL
 *  \param[out] n_hi_epsilon on return holds number of arcs with upper label ::gfsmEpsilon, or NULL
 *  \param[out] n_both_epsilon on return holds number of arcs with both lower and upper labels ::gfsmEpsilon, or NULL
 *  \returns total number of arcs
 */
guint gfsm_automaton_n_arcs_full(gfsmAutomaton *fsm,
				 guint *n_lo_epsilon,
				 guint *n_hi_epsilon,
				 guint *n_both_epsilon);

/** Low-level utility function for gfsm_automaton_is_cyclic() */
gboolean gfsm_automaton_is_cyclic_state(gfsmAutomaton *fsm,
					gfsmStateId    qid,
					gfsmBitVector *visited,
					gfsmBitVector *completed);

/** Test whether automaton is cyclic */
gboolean gfsm_automaton_is_cyclic(gfsmAutomaton *fsm);

/** Test whether automaton is acyclic */
#define gfsm_automaton_is_acyclic(fsm) (!gfsm_automaton_is_cyclic(fsm))

/** Extract automaton-internal labels to \a alph.  If \a alph is NULL,
 *  a new default alphabet will be created and returned (you will need to
 *  free it yourself).
 *
 *  The alphabet should be able to match literal label values to themselves
 *  (so don't pass a string alphabet)
 *
 *  \param[in]  fsm automaton to examine
 *  \param[in]  which determines which label side(s) to extract
 *  \param[out] alph alphabet to which labels are extracted, or NULL to create a new alphabet
 *
 * \returns \a alph, or a newly allocated and populated alphabet
 */
gfsmAlphabet *gfsm_automaton_get_alphabet(gfsmAutomaton *fsm,
					  gfsmLabelSide  which,
					  gfsmAlphabet  *alph);


/** Renumber states of the automaton \a fsm.
 *  Destructively alters \c fsm.
 *  On return, \a fsm should have no 'gaps' in its state enumeration function, and its
 *  root state should have the ID 0 (zero).
 */
void gfsm_automaton_renumber_states(gfsmAutomaton *fsm);

/** Renumber states of an FSM using user-specified state-ID map \a old2new.
 *  Destructively alters \c fsm.
 *  \param fsm
 *   Automaton whose states are to be renumbered
 *  \param old2new
 *   GArray of ::gfsmStateId such that <tt>qid_new=old2new[qid_old]</tt>.
 *   \a qid_new may be ::gfsmNoState to ignore the corresponding \a qid_old
 *  \param n_new_states
 *   Maximum \a qid_new ::gfsmStateId value in \a old2new, or 0 (zero) to auto-compute.
 */
void gfsm_automaton_renumber_states_full(gfsmAutomaton *fsm, GArray *old2new, gfsmStateId n_new_states);

//@}

/*======================================================================*/
///\name API: gfsmState
//@{

/** Open and return a pointer to a ::gfsmState struct for ::gfsmStateId \a qid in \a fsm.
 *  \warning
 *   The pointer returned should be closed with gfsm_automaton_close_state()
 *  \param fsm automaton from which to draw state information
 *  \param qid ID of state to be opened
 *  \returns ::gfsmState* for state with ID \a qid in \a fsm, or NULL if no such state exists.
 *  \deprecated
 *    prefer gfsm_automaton_has_state(), gfsm_automaton_state_is_final(), gfsm_arciter_open() etc.
 */
GFSM_INLINE
gfsmState *gfsm_automaton_open_state(gfsmAutomaton *fsm, gfsmStateId qid);

/** Open and return a pointer to a ::gfsmState struct for ::gfsmStateId \a qid in \a fsm,
 *  creating state if it does not already exists.
 *  \warning
 *   The pointer returned should be closed with gfsm_automaton_close_state().
 *  \param fsm automaton from which to draw state information
 *  \param qid ID of state to be opened
 *  \returns ::gfsmState* for (possibly new) state with ID \a qid in \a fsm
 *  \deprecated
 *    prefer gfsm_automaton_has_state(), gfsm_automaton_state_is_final(), gfsm_arciter_open() etc.
 */
GFSM_INLINE
gfsmState *gfsm_automaton_open_state_force(gfsmAutomaton *fsm, gfsmStateId qid);

/** Close a pointer to a ::gfsmState opened with gfsm_automaton_open_state() for \a fsm.
 *  \param fsm automaton from which state was opened.
 *  \param qp  pointer as returned by gfsm_automaton_open_state()
 *  \note Currently does nothing.
 */
GFSM_INLINE
void gfsm_automaton_close_state(gfsmAutomaton *fsm, gfsmState *qp);

/** Backwards-compatible alias for gfsm_automaton_open_state().
 *  This alias is expected to disappear when calling gfsm_automaton_close_state() becomes mandatory.
 */
#define gfsm_automaton_find_state(fsm,qid) gfsm_automaton_open_state((fsm),(qid))

/** Backwards-compatible alias for gfsm_automaton_open_state().
 *  This alias is expected to disappear when calling gfsm_automaton_close_state() becomes mandatory.
 */
#define gfsm_automaton_find_state_const(fsm,qid) ((const gfsmState*)(gfsm_automaton_open_state((fsm),(qid))))

/** Backwards-compatible alias for gfsm_automaton_open_state_force()
 *  This alias is expected to disappear when calling gfsm_automaton_close_state() becomes mandatory.
 */
#define gfsm_automaton_get_state(fsm,qid) gfsm_automaton_open_state_force((fsm),(qid))

//@}

/*======================================================================*/
/// \name API: Automaton States
//@{

/*--------------------------------------------------------------
 * has_state()
 */
GFSM_INLINE
gboolean gfsm_automaton_has_state(gfsmAutomaton *fsm, gfsmStateId qid);

/** Add a new state, specifying state ID.
 *  \param fsm automaton to modify
 *  \param qid  ID of new state, or ::gfsmNoState to use the first available state ID.
 *  \returns Id of the (new) state
 *  \note
 *   \li Implicitly sets \a fsm's root state if \a fsm was previously unrooted.
 *   \li Does nothing if \a fsm already has a state with ID \a qid.
 */
GFSM_INLINE
gfsmStateId gfsm_automaton_add_state_full(gfsmAutomaton *fsm, gfsmStateId qid);

/** Ensures that state \a id exists \returns \a qid
 *  Really just an alias for gfsm_automaton_add_state_full().
 */
GFSM_INLINE
gfsmStateId gfsm_automaton_ensure_state(gfsmAutomaton *fsm, gfsmStateId qid);

/** Add a new state to \a fsm.
 *  Really just an alias for \code gfsm_automaton_add_state_full(fsm,gfsmNoState) \endcode
 */
GFSM_INLINE
gfsmStateId gfsm_automaton_add_state(gfsmAutomaton *fsm);

/** Remove the state with id \a qid, if any.
 *  \param fsm automaton from which to remove a state
 *  \param qid ID of the state to be removed
 *  \note
 *   Any incoming arcs for state \a qid are NOT removed,
 *   although any outgoing arcs are removed and freed.
 */
GFSM_INLINE
void gfsm_automaton_remove_state(gfsmAutomaton *fsm, gfsmStateId qid);

/** Lookup final weight for state with ID \a qid in automaton \a fsm.
 *  \param fsm automaton to examine
 *  \param qid ID of state to examine
 *  \param  wp output parameter for final weight
 *  \returns
 *     TRUE if state \a qid is final, FALSE otherwise
 */
GFSM_INLINE
gboolean gfsm_automaton_lookup_final(gfsmAutomaton *fsm, gfsmStateId qid, gfsmWeight *wp);

/** Check whether the state with ID \a qid is final in \a fsm.
 *  Really just a wrapper for gfsm_automaton_lookup_final().
 *  \param fsm automaton to examine
 *  \param qid ID of state to check for finality
 *  \returns TRUE if \a qid is final in \a fsm, FALSE otherwise.
 */
GFSM_INLINE
gboolean gfsm_automaton_state_is_final(gfsmAutomaton *fsm, gfsmStateId qid);

/** Backwards-compatible alias for gfsm_automaton_state_is_final() */
#define gfsm_automaton_is_final_state(fsm,qid) gfsm_automaton_state_is_final((fsm),(qid))

/** Get final weight. \returns final weight if state \a qid is final, else \a fsm->sr->zero */
GFSM_INLINE
gfsmWeight gfsm_automaton_get_final_weight(gfsmAutomaton *fsm, gfsmStateId qid);

/** Set final-weight and/or final-states membership flag for state with ID \a qid in \a fsm.
 * \param fsm automaton to modify
 * \param qid ID of state to modified
 * \param is_final whether state should be considered final
 * \param final_weight
 *    If \a is_final is true, final weight for state. 
 *    Otherwise, final weight is implicitly <tt>fsm->sr->zero</tt>
 */
GFSM_INLINE
void gfsm_automaton_set_final_state_full(gfsmAutomaton *fsm,
					 gfsmStateId    qid,
					 gboolean       is_final,
					 gfsmWeight     final_weight);

/** Backwards-compatble wrapper for <code>gfsm_automaton_set_final_state_fulll(fsm,qid,is_final,fsm->sr->one)</code>
 *  \see gfsm_automaton_set_final_state_full()
 */
GFSM_INLINE
void gfsm_automaton_set_final_state(gfsmAutomaton *fsm, gfsmStateId qid, gboolean is_final);

/** Get number of outgoing arcs from \a qid in \a fsm */
GFSM_INLINE
guint gfsm_automaton_out_degree(gfsmAutomaton *fsm, gfsmStateId qid);

//@}

/*======================================================================*/
/// \name Accessors: Automaton Arcs
//@{

/** Add an arc from state with ID \a qid1 to state with ID \a qid2
 *  on labels (\a lo,\a hi) with weight \a w.
 *  Missing states should be implicitly created.
 *  \param fsm Automaton to modify
 *  \param qid1 ID of source state
 *  \param qid2 ID of target state
 *  \param lo   Lower label
 *  \param hi   Upper label
 *  \param w    Arc weight
 */
GFSM_INLINE
void gfsm_automaton_add_arc(gfsmAutomaton *fsm,
			    gfsmStateId qid1,
			    gfsmStateId qid2,
			    gfsmLabelId lo,
			    gfsmLabelId hi,
			    gfsmWeight  w);

/** Add an arc given pointers \a sp to the state and \a link to a
 *  single-element arclist to be added. 
 *   No states are implicitly created.
 *
 *  \deprecated prefer gfsm_automaton_add_arc()
 */
GFSM_INLINE
void gfsm_automaton_add_arc_node(gfsmAutomaton *fsm,
				 gfsmState     *sp,
				 gfsmArcList   *node);


/** Sort all arcs in an automaton by one of the built-in comparison functions.
 *  \param fsm  Automaton to modify
 *  \param mode Specifies built-in arc comparison priorities
 *  \returns modified \a fsm
 *  \note
 *    \li Does nothing if \code (mode==gfsmASMNone || mode==fsm->flags.sort_mode) \endcode
 *    \li Really just a wrapper for gfsm_automaton_arcsort_full()
 */
GFSM_INLINE
gfsmAutomaton *gfsm_automaton_arcsort(gfsmAutomaton *fsm, gfsmArcCompMask mode);

/** Sort all arcs in an automaton by a user-specified comparison function.
 *  \param fsm
 *    Automaton to modify
 *  \param cmpfunc
 *    3-way comparison function, called as \a (*cmpfunc)(gfsmArc *a1, gfsmArc *a2, gpointer data)
 *    to compare arcs \a a1 and \a a2.
 *  \param data
 *    User data for \a cmpfunc
 *  \returns
 *    Modified \a fsm
 */
void gfsm_automaton_arcsort_full(gfsmAutomaton *fsm, GCompareDataFunc cmpfunc, gpointer data);

/** Alias for gfsm_automaton_arcsort_full() */
#define gfsm_automaton_arcsort_with_data(fsm,cmpfunc,data) gfsm_automaton_arcsort_full((fsm),(cmpfunc),(data))

//@}

//-- inline definitions
#ifdef GFSM_INLINE_ENABLED
# include <gfsmAutomaton.hi>
#endif

#endif /* _GFSM_AUTOMATON_H */
