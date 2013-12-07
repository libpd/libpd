
/*=============================================================================*\
 * File: gfsmStateSet.h
 * Author: Bryan Jurish <moocow@ling.uni-potsdam.de>
 * Description: finite state machine library
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

/** \file gfsmStateSet.h
 *  \brief Sets of (gfsmStateId)s
 */

#ifndef _GFSM_STATESET_H
#define _GFSM_STATESET_H

#include <gfsmAutomaton.h>

/*======================================================================
 * Types
 */

/** \brief typedef for sets of automaton state-Ids
 * \detail current implementation uses a sorted array
*/
typedef GArray gfsmStateSet;

/** \brief typedef for weighted sets of automaton state-Ids */
typedef struct {
  gfsmStateSet *set;    /**< Set of gfsmStateIds */
  gfsmWeight    weight; /**< Weight of this set */
} gfsmWeightedStateSet;


/*======================================================================
 * Constants
 */
///\name Constants
//@{
/** Default initial size of state-sets */
extern const guint gfsmStateSetDefaultSize;
//@}

/*======================================================================
 * Methods: Constructors etc.
 */
/// \name Constructors etc.
//@{

/** Create and return a new state-set, giving initial reserved size */
GFSM_INLINE
gfsmStateSet *gfsm_stateset_sized_new(guint isize);

/** Create and return a new state-set */
GFSM_INLINE
gfsmStateSet *gfsm_stateset_new(void);

/** Create and return a new singleton state-set */
GFSM_INLINE
gfsmStateSet *gfsm_stateset_new_singleton(gfsmStateId id);

/** clear a state-set */
GFSM_INLINE
void gfsm_stateset_clear(gfsmStateSet *sset);

/** Create and return an exact copy of a state-set */
GFSM_INLINE
gfsmStateSet *gfsm_stateset_clone(gfsmStateSet *src);

/** destroy a state-set */
GFSM_INLINE
void gfsm_stateset_free(gfsmStateSet *sset);

//@}

/*======================================================================
 * Methods: Accessors
 */
///\name Accessors
//@{

/** Get minimum element of a state-set */
GFSM_INLINE
gfsmStateId gfsm_stateset_min(gfsmStateSet *sset);

/** Get number of elements in a state-set */
GFSM_INLINE
guint gfsm_stateset_size(gfsmStateSet *sset);

/** Check whether a state-id is contained in a state-set */
GFSM_INLINE
gboolean gfsm_stateset_contains(gfsmStateSet *sset, gfsmStateId id);

/** Insert a single state-id into a state-set.
 *  \returns true iff \a sset already contained \a id
 */
gboolean gfsm_stateset_insert(gfsmStateSet *sset, gfsmStateId id);

/** Assign \a sset1 to be the union of itself with \a sset2 */
gfsmStateSet *gfsm_stateset_union(gfsmStateSet *sset1, gfsmStateSet *sset2);

/** Remove a state-id from a state-set
 * \returns true iff \a sset contained \a id
 */
gboolean gfsm_stateset_remove(gfsmStateSet *sset, gfsmStateId id);

/** Equality test */
gboolean gfsm_stateset_equal(gfsmStateSet *sset1, gfsmStateSet *sset2);
//@}

/*======================================================================
 * Methods: Iteration
 */
///\name Iteration
//@{

/// state-set iterator type
typedef gfsmStateId* gfsmStateSetIter;

/** Iterator access: get current state-id, or gfsmNoState if none defined */
#define gfsm_stateset_iter_id(sseti) \
  ((sseti) ? *(sseti) : gfsmNoState)

/** Check validity of a state-set iterator */
#define gfsm_stateset_iter_ok(sseti) \
  ((sseti)!=NULL)

/** Get first state in the state-set.
 *  \returns iterator (value) pointing to the first state in the
 *  state-set.
 */
#define gfsm_stateset_iter_begin(sset) \
  ((gfsmStateId*)((sset)->data))

/** Increment a state-set iterator one position. */
#define gfsm_stateset_iter_next(sset,sseti) \
  ((++(sseti)) < (((gfsmStateId*)((sset)->data))+((sset)->len)) \
   ? (sseti) \
   : NULL)

/** Find an iterator pointing to the element for \a id in \a sset,
 *  or a bad iterator if no such element exists */
gfsmStateSetIter gfsm_stateset_find(gfsmStateSet *sset, gfsmStateId id);

//@}

/*======================================================================
 * Methods: Utiltiies
 */
///\name Utilities
//@{

/// typedef for iteration functions: return TRUE to stop iteration
typedef gboolean (*gfsmStateSetForeachFunc) (gfsmStateId id, gpointer data);

/** General iteration utilitiy for state-sets */
void gfsm_stateset_foreach(gfsmStateSet *sset, gfsmStateSetForeachFunc func, gpointer data);

/** Hashing function for state-sets */
guint gfsm_stateset_hash(gfsmStateSet *sset);

//@}

/*======================================================================
 * Methods: Automaton
 */
///\name Methods: Automaton access
//@{

/** Populate a state-set representing targets of arcs with
 *  lower label \a lo and upper label \a hi leaving state with id \a id
 *  in automaton \a fsm.
 *
 *  If either \a lo or \a hi is gfsmNoLabel, the corresponding labels
 *  will be ignored.
 *
 *  Note that this method does not clear \a sset.
 */
void gfsm_stateset_populate(gfsmStateSet *sset,
			    gfsmAutomaton *fsm,
			    gfsmStateId     id,
			    gfsmLabelVal    lo,
			    gfsmLabelVal    hi);

/** Convenience macro to populate a state-set from I/O-epsilon arcs */
#define gfsm_stateset_populate_eps(sset,fsm,id) \
  gfsm_stateset_populate((sset),(fsm),(id),gfsmEpsilon,gfsmEpsilon)

/** Returns true iff some \a id in \a sset is a final state in \a fsm */
gboolean gfsm_stateset_has_final_state(gfsmStateSet *sset, gfsmAutomaton *fsm);

/** Lookup sum of final weights in \a fsm of states \a id in \a sset
 *  Returns TRUE iff at least one state in \a sset is final, and
 *  sets \a *wp to the sum of final weights.
 */
gboolean gfsm_stateset_lookup_final_weight(gfsmStateSet *sset, gfsmAutomaton *fsm, gfsmWeight *wp);

//@}

//-- inline definitions
#ifdef GFSM_INLINE_ENABLED
# include <gfsmStateSet.hi>
#endif

#endif /* _GFSM_STATESET_H */
