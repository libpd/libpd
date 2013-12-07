
/*=============================================================================*\
 * File: gfsmArcIter.h
 * Author: Bryan Jurish <moocow@ling.uni-potsdam.de>
 * Description: finite state machine library: arc iterators
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

/** \file gfsmArcIter.h
 *  \brief Iterate over outgoing arcs of an automaton state.
 */

#ifndef _GFSM_ARCITER_H
#define _GFSM_ARCITER_H

#include <gfsmAutomaton.h>

/*======================================================================
 * Types: Arc iterators
 */
/// Abstract type for arc iterators
typedef struct {
  gfsmAutomaton *fsm;    /**< fsm holding these arcs */
  gfsmState     *state;  /**< state holding these arcs */
  gfsmArcList   *arcs;   /**< pointer to node for current arc */
} gfsmArcIter;

/*======================================================================
 * Methods: Arc iterators: open/close
 */
///\name Arc Iterators: Constructors etc.
//@{
/** Open a ::gfsmArcIter \a aip for the outgoing arcs from state with ID \a qid in the automaton \a fsm.
 *  \param aip Pointer to the ::gfsmArcIter to be opened; assumed to be already allocated
 *  \param fsm Automaton containing the state whose outgoing arcs are to be opened
 *  \param qid ID of the state whose outgoing arcs are to be opened
 *
 *  \note
 *   \li Arc iterators may be silently invalidated by destructive operations
 *   \li The arc iterator should be closed with gfsm_arciter_close() when it is no longer needed.
 *   \li Caller is responsible for allocation and freeing of \a *aip.
 */
GFSM_INLINE
void gfsm_arciter_open(gfsmArcIter *aip, gfsmAutomaton *fsm, gfsmStateId stateid);

/** "Open" an arc iterator for the outgoing arcs from a state pointer into \a fsm
 *  \deprecated prefer gfsm_arciter_open()
 */
GFSM_INLINE
void gfsm_arciter_open_ptr(gfsmArcIter *aip, gfsmAutomaton *fsm, gfsmState *stateptr);

/** Close a ::gfsmArcIter \a aip if already opened, otherwise does nothing.
 *  \param aip The ::gfsmArcIter to be closed.
 *  \note
 *   \li If multiple copies of a ::gfsmArcIter exist, only one needs to be closed.
 *   \li Currently does nothing useful; in future versions this function may
 *       be required to free temporary allocations, etc.
 */
GFSM_INLINE
void gfsm_arciter_close(gfsmArcIter *aip);

/** Copy positional data from \a src to \a dst.
 * \param src The ::gfsmArcIter from which to copy positional data
 * \param dst The ::gfsmArcIter to which positional data is to be written
 * \note
 *  \li Only the position pointed to should be copied by this method,
 *      and not the underlying data.
 *  \li If you use this method to copy ::gfsmArcIter positions,
 *      you should subsequently call gfsm_arciter_close() on only
 *      \e one of them!
 */
GFSM_INLINE
gfsmArcIter *gfsm_arciter_copy(gfsmArcIter *dst, const gfsmArcIter *src);

/* Create and return a new (shallow) copy of a ::gfsmArcIter.
 * \param src The ::gfsmArcIter whose positional data is to be duplicated.
 * \note
 *  \li Only the position pointed to should be copied by this method,
 *      and not the underlying data.
 *  \li If you use this method to copy ::gfsmArcIter positions,
 *      you should subsequently call gfsm_arciter_close() on only
 *      \e one of them!
 */
GFSM_INLINE
gfsmArcIter *gfsm_arciter_clone(const gfsmArcIter *src);

//@}

/*======================================================================
 * Methods: Arc iterators: Accessors
 */
///\name Arc Iterators: Accessors
//@{

/** Check validity of a ::gfsmArcIter* \a aip.
 *  \param aip The ::gfsmArcIter whose status is to be queried.
 *  \returns a true value if \a aip is considered valid, FALSE otherwise.
 */
GFSM_INLINE
gboolean gfsm_arciter_ok(const gfsmArcIter *aip);

/** Position the ::gfsmArcIter \a aip to the next available outgoing arc for which it was opened.
 *  \param aip The ::gfsmArcIter to be incremented.
 */
GFSM_INLINE
void gfsm_arciter_next(gfsmArcIter *aip);

/** Reset an arc iterator to the first outgoing arc for which it was initially opened.
 *  \param aip the ::gfsmArcIter to be reset
 */
GFSM_INLINE
void gfsm_arciter_reset(gfsmArcIter *aip);

/** Get current arc associated with a :gfsmArcIter, or NULL if none is available.
 *  \param aip The ::gfsmArcIter to be 'dereferenced'.
 *  \returns A pointer to the current ::gfsmArc 'pointed to' by \a aip, or NULL if
 *           no more arcs are available.
 *  \note
 *   \li In future versions, a ::gfsmAutomaton implementation will be free to return
 *       a dynamically generated arc here: there is no general
 *       guarantee that modifications to the ::gfsmArc returned by this
 *       function will be propagated to the underlying ::gfsmAutomaton.
 *   \li It is expected to remain the case that for the default automaton implementation class,
 *       the arcs returned by this function should be modifiable in-place.
 */
GFSM_INLINE
gfsmArc *gfsm_arciter_arc(const gfsmArcIter *aip);

/** Remove the arc referred to by a ::gfsmArcIter \a aip from the associated ::gfsmAutomaton,
 *  and position \aip to the next available arc, if any.
 *  \param aip The ::gfsmArcIter whose 'current' arc is to be removed.
 */
GFSM_INLINE
void gfsm_arciter_remove(gfsmArcIter *aip);


/** Position an arc-iterator to the current or next arc with lower label \a lo.
 *  \param aip The ::gfsmArcIter to reposition
 *  \param lo  Lower arc label to seek
 *  \note
 *    Currently just wraps gfsm_arciter_ok(), gfsm_arciter_next() and gfsm_arciter_arc()
 *    in a linear search from the current position.
 */
void gfsm_arciter_seek_lower(gfsmArcIter *aip, gfsmLabelVal lo);

/** Position an arc-iterator to the current or next arc with upper label \a hi.
 *  \param aip The ::gfsmArcIter to reposition
 *  \param lo  Upper arc label to seek
 *  \note
 *    Currently just wraps gfsm_arciter_ok(), gfsm_arciter_next() and gfsm_arciter_arc()
 *    in a linear search from the current position.
 */
void gfsm_arciter_seek_upper(gfsmArcIter *aip, gfsmLabelVal hi);

/** Position an arc-iterator to the current or next arc with lower label \a lo and upper label \a hi.
 *  If either \a lo or \a hi is ::gfsmNoLabel, no matching will be performed on the corresponding arc label(s).
 *  \param aip The ::gfsmArcIter to reposition
 *  \param lo  Lower arc label to seek, or ::gfsmNoLabel to ignore lower labels
 *  \param hi  Upper arc label to seek, or ::gfsmNoLabel to ignore upper labels
 *  \note
 *    Default implementation wraps gfsm_arciter_ok(), gfsm_arciter_next() and gfsm_arciter_arc()
 *    in a linear search from the current position.
 */
void gfsm_arciter_seek_both(gfsmArcIter *aip, gfsmLabelVal lo, gfsmLabelVal hi);

/// Typedef for user-seek functions
typedef gboolean (*gfsmArcIterSeekFunc) (gfsmArcIter *aip, gpointer data);

/** Position an arc-iterator to the next arc for which (*seekfunc)(arciter,data) returns TRUE.
 *  \note
 *    Just wraps gfsm_arciter_ok() and gfsm_arciter_next()
 *    in a linear search from the current position.
 */
void gfsm_arciter_seek_user(gfsmArcIter *aip,
			    gfsmArcIterSeekFunc seekfunc,
			    gpointer data);


//@}

//-- inline definitions
#ifdef GFSM_INLINE_ENABLED
# include <gfsmArcIter.hi>
#endif

#endif /* _GFSM_ARCITER_H */

