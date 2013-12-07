
/*=============================================================================*\
 * File: gfsmArcIndex.h
 * Author: Bryan Jurish <moocow@ling.uni-potsdam.de>
 * Description: finite state machine library: arc indices
 *
 * Copyright (c) 2006-2007 Bryan Jurish.
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

/** \file gfsmArcIndex.h
 *  \brief Arc (transition) index utilities
 */

#ifndef _GFSM_ARCINDEX_H
#define _GFSM_ARCINDEX_H

#include <gfsmAutomaton.h>
#include <gfsmIO.h>

/*======================================================================
 * ReverseArcIndex
 */
///\name gfsmReverseArcIndex
//@{

/// Reverse arc-index type
/**  \a element at \a qto is a GSList*
 *    which contains a data element \a gfsmArc* \a arc={qfrom,qto,lo,hi,w}
 *    whenever source \a fsm contains an arc    \a arc={qfrom,qto,lo,hi,w}
 *    from \a qfrom.
 *
 *  \note
 *   arc data pointed to is shared by source automaton
 *   and the ::gfsmReverseArcIndex!
 */
typedef GPtrArray gfsmReverseArcIndex;

/** Create and return a new ::gfsmReverseArcIndex
 * \note
 *   Caller is responsible for freeing the returned index when it is no longer needed.
 */
GFSM_INLINE
gfsmReverseArcIndex *gfsm_reverse_arc_index_new(void);

/** Create a new ::gfsmReverseArcIndex, given number of states to be indexed
 * \note
 *   Caller is responsible for freeing the returned index when it is no longer needed.
 */
GFSM_INLINE
gfsmReverseArcIndex *gfsm_reverse_arc_index_sized_new(gfsmStateId n_states);

/** Populate a reversed arc index for \a fsm.
 * \param fsm source automaton
 * \param rarcs
 *   Reverse arc index.
 *   May be passed as NULL to create a new arc index.
 * \returns
 *   \a rarcs if non-NULL, otherwise a new reverse arc index for \a fsm.
 * \note
 *   Caller is responsible for freeing the returned index when it is no longer needed.
 */
gfsmReverseArcIndex *gfsm_automaton_to_reverse_arc_index(gfsmAutomaton *fsm, gfsmReverseArcIndex *rarcs);

/** Backwards-compatible alias for gfsm_automaton_to_reverse_arc_inde() */
#define gfsm_automaton_reverse_arc_index gfsm_automaton_to_reverse_arc_index

/** Free a ::gfsmReverseArcIndex
 *  \param rarcs
 *    reverse arc-index to be freed
 *  \param free_lists
 *    If true, associated arc-lists will be freed.
 */
void gfsm_reverse_arc_index_free(gfsmReverseArcIndex *rarcs, gboolean free_lists);

//@}


/*======================================================================
 * gfsmFinalWeightIndex
 */
///\name gfsmFinalWeightIndex
//@{

/** GArray of ::gfsmWeight, indexed e.g. by ::gfsmStateId */
typedef GArray gfsmWeightVector;

/** Create a new (empty) ::gfsmWeightVector
 * \note
 *   Caller is responsible for freeing \a wv when it is no longer needed.
 */
GFSM_INLINE
gfsmWeightVector *gfsm_weight_vector_new(void);

/** Create a new (empty) ::gfsmWeightVector, specifying initial size
 * \note
 *   Caller is responsible for freeing the returned index when it is no longer needed.
 */
GFSM_INLINE
gfsmWeightVector *gfsm_weight_vector_sized_new(guint size);

/** Copy a ::gfsmWeightVector \a src to \a dst. \returns \a dst */
GFSM_INLINE
gfsmWeightVector *gfsm_weight_vector_copy(gfsmWeightVector *dst, gfsmWeightVector *src);

/** Create and return an exact clone of a ::gfsmWeightVector */
GFSM_INLINE
gfsmWeightVector *gfsm_weight_vector_clone(gfsmWeightVector *src);


/** Set size of a ::gfsmWeightVector */
GFSM_INLINE
void gfsm_weight_vector_resize(gfsmWeightVector *wv, guint size);

/** Populate a ::gfsmWeightVector of state final weights in a ::gfsmAutomaton
 * \param fsm source automaton
 * \param wv
 *   Final weight index
 *   May be passed as NULL to create a new index.
 * \returns \a wv if non-NULL, otherwise a new final weight index for \a fsm.
 */
gfsmWeightVector *gfsm_automaton_to_final_weight_vector(gfsmAutomaton *fsm, gfsmWeightVector *wv);

/** Free a ::gfsmWeightVector */
GFSM_INLINE
void gfsm_weight_vector_free(gfsmWeightVector *wv);

/** Write the contents of a ::gfsmWeightVector to a (binary) ::gfsmIOHandle.
 *  \param wv weight vector to write
 *  \param ioh handle to which data is to be written
 *  \param errp if an error occurs, \a *errp will hold an error message
 *  \returns true on success
 */
gboolean gfsm_weight_vector_write_bin_handle(gfsmWeightVector *wv, gfsmIOHandle *ioh, gfsmError **errp);

/** Read the contents of a ::gfsmWeightVector from a (binary) ::gfsmIOHandle.
 *  \param wv weight vector into which data is to be read
 *  \param ioh handle from which data is to be read
 *  \param errp if an error occurs, \a *errp will hold an error message
 *  \returns true on success
 */
gboolean gfsm_weight_vector_read_bin_handle(gfsmWeightVector *wv, gfsmIOHandle *ioh, gfsmError **errp);

//@}

/*======================================================================
 * gfsmArcTable
 */
///\name gfsmArcTable
//@{

/// Type for dedicated block-wise storage of ::gfsmArc data: GArray of ::gfsmArc
typedef GArray gfsmArcTable;

/** Create and return a new (empty) ::gfsmArcTable */
GFSM_INLINE
gfsmArcTable *gfsm_arc_table_new(void);

/** Create and return a new (empty) ::gfsmArcTable, specifying size */
GFSM_INLINE
gfsmArcTable *gfsm_arc_table_sized_new(guint n_arcs);

/** Resize a ::gfsmArcTable */
GFSM_INLINE
void gfsm_arc_table_resize(gfsmArcTable *tab, guint n_arcs);

/** Copy a ::gfsmArcTable \a src to \a dst.  \returns \a dst. */
GFSM_INLINE
gfsmArcTable *gfsm_arc_table_copy(gfsmArcTable *dst, gfsmArcTable *src);

/** Create and return an exact copy of a ::gfsmArcTable \a src */
GFSM_INLINE
gfsmArcTable *gfsm_arc_table_clone(gfsmArcTable *src);

/** Free a ::gfsmArcTable */
GFSM_INLINE
void gfsm_arc_table_free(gfsmArcTable *tab);

/** Populate a :gfsmArcTable by copying arcs from \a fsm
 * \param fsm source automaton
 * \param tab
 *   arc table to populate.
 *   May be passed as NULL to create a new arc table.
 * \returns
 *   \a tab if non-NULL, otherwise a new ::gfsmArcTable for \a fsm.
 * \note
 *   Caller is responsible for freeing \a tab when it is no longer needed.
 */
gfsmArcTable *gfsm_automaton_to_arc_table(gfsmAutomaton *fsm, gfsmArcTable *tab);

/** Sort all arcs in a ::gfsmArcTable using a user-specified comparison function */
GFSM_INLINE
void gfsm_arc_table_sort_with_data(gfsmArcTable *tab, GCompareDataFunc compare_func, gpointer data);

/** Sort arcs by comparison priority in a ::gfsmArcTable */
GFSM_INLINE
void gfsm_arc_table_sort_bymask(gfsmArcTable *tab, gfsmArcCompMask m, gfsmSemiring *sr);

/** Write the contents of a ::gfsmArcTable to a (binary) ::gfsmIOHandle.
 *  \param tab table to write
 *  \param ioh handle to which data is to be written
 *  \param errp if an error occurs, \a *errp will hold an error message
 *  \returns true on success
 */
gboolean gfsm_arc_table_write_bin_handle(gfsmArcTable *tab, gfsmIOHandle *ioh, gfsmError **errp);

/** Read the contents of a ::gfsmArcTable from a (binary) ::gfsmIOHandle.
 *  \param tab table into which data is to be read
 *  \param ioh handle from which data is to be read
 *  \param errp if an error occurs, \a *errp will hold an error message
 *  \returns true on success
 */
gboolean gfsm_arc_table_read_bin_handle(gfsmArcTable *tab, gfsmIOHandle *ioh, gfsmError **errp);

//@}

/*======================================================================
 * gfsmArcTableIndex
 */
///\name gfsmArcTableIndex
//@{

/// Basic type for dedicated arc storage state-based arc index
typedef struct {
  gfsmArcTable *tab;              /**< arc table, sorted by (source,...) */
  GPtrArray    *first;            /**< \a first[q] is address of first element of \a arcs->data for state \a q (a ::gfsmArc*) */
} gfsmArcTableIndex;

/** Create and return a new (empty) ::gfsmArcTableIndex */
GFSM_INLINE
gfsmArcTableIndex *gfsm_arc_table_index_new(void);

/** Create and return a new (empty) ::gfsmArcTableIndex, specifying sizes */
GFSM_INLINE
gfsmArcTableIndex *gfsm_arc_table_index_sized_new(gfsmStateId n_states, guint n_arcs);

/** Resize a ::gfsmArcTableIndex */
GFSM_INLINE
void gfsm_arc_table_index_resize(gfsmArcTableIndex *tab, gfsmStateId n_states, guint n_arcs);

/** Get number of states allocated for a ::gfsmArcTableIndex */
GFSM_INLINE
gfsmStateId gfsm_arc_table_index_n_states(gfsmArcTableIndex *tabx);

/** Get number of arcs allocated for a ::gfsmArcTableIndex */
GFSM_INLINE
guint gfsm_arc_table_index_n_arcs(gfsmArcTableIndex *tabx);

/** Copy a ::gfsmArcTableIndex \a src to \a dst.  \returns \a dst. */
gfsmArcTableIndex *gfsm_arc_table_index_copy(gfsmArcTableIndex *dst, gfsmArcTableIndex *src);

/** Create and return an exact copy of a ::gfsmArcTableIndex \a src */
GFSM_INLINE
gfsmArcTableIndex *gfsm_arc_table_index_clone(gfsmArcTableIndex *src);

/** Free a ::gfsmArcTableIndex */
GFSM_INLINE
void gfsm_arc_table_index_free(gfsmArcTableIndex *tabx);

/** Populate a ::gfsmArcTableIndex by indexing outgoing arcs from each state in \a fsm.
 * \param fsm source automaton
 * \param tabx
 *   Indexed arc table to populate.
 *   May be passed as NULL to create a new indexed arc table.
 * \returns
 *   \a tabx if non-NULL, otherwise a new index for \a fsm.
 * \note
 *   \li Caller is responsible for freeing \a tabx when it is no longer needed.
 */
gfsmArcTableIndex *gfsm_automaton_to_arc_table_index(gfsmAutomaton *fsm, gfsmArcTableIndex *tabx);

/** Sort arcs state-wise in a ::gfsmArcTableIndex */
void gfsm_arc_table_index_sort_with_data(gfsmArcTableIndex *tabx, GCompareDataFunc compare_func, gpointer data);

/** Sort arcs state-wise by field priority in a ::gfsmArcTableIndex.
 *  Really just a wrapper for gfsm_arc_table_index_sort_with_data()
 */
GFSM_INLINE
void gfsm_arc_table_index_sort_bymask(gfsmArcTableIndex *tabx, gfsmArcCompMask m, gfsmSemiring *sr);

/** Get number of outgoing arcs from state \a qid in \a tabx */
GFSM_INLINE
guint gfsm_arc_table_index_out_degree(gfsmArcTableIndex *tabx, gfsmStateId qid);

/** Write the contents of a ::gfsmArcTableIndex to a (binary) ::gfsmIOHandle.
 *  \param tabx index to write
 *  \param ioh handle to which data is to be written
 *  \param errp if an error occurs, \a *errp will hold an error message
 *  \returns true on success
 */
gboolean gfsm_arc_table_index_write_bin_handle(gfsmArcTableIndex *tabx, gfsmIOHandle *ioh, gfsmError **errp);

/** Read the contents of a ::gfsmArcTableIndex from a (binary) ::gfsmIOHandle.
 *  \param tabx table into which data is to be read
 *  \param ioh handle from which data is to be read
 *  \param errp if an error occurs, \a *errp will hold an error message
 *  \returns true on success
 */
gboolean gfsm_arc_table_index_read_bin_handle(gfsmArcTableIndex *tabx, gfsmIOHandle *ioh, gfsmError **errp);

//@}

/*======================================================================
 * gfsmArcRange
 */
///\name gfsmArcRange
//@{

/// Type for searching and iterating over arcs in a ::gfsmArcTable
typedef struct {
  gfsmArc *min; /**< First (current) arc in range */
  gfsmArc *max; /**< First arc \b not in range */
} gfsmArcRange;

/** Open a ::gfsmArcRange for all outgoing arcs from state \a qid in \a tabx */
GFSM_INLINE
void gfsm_arcrange_open_table_index(gfsmArcRange *range, gfsmArcTableIndex *tabx, gfsmStateId qid);

/** Close a ::gfsmArcRange (currently does nothing really useful) */
GFSM_INLINE
void gfsm_arcrange_close(gfsmArcRange *range);

/** Check validity of a ::gfsmArcRange */
GFSM_INLINE
gboolean gfsm_arcrange_ok(gfsmArcRange *range);

/** Get current arc from a ::gfsmArcRange, which is assumed to be valid */
GFSM_INLINE
gfsmArc *gfsm_arcrange_arc(gfsmArcRange *range);

/** Increment current arc of a ::gfsmArcRange */
GFSM_INLINE
void gfsm_arcrange_next(gfsmArcRange *range);

//@}

/*======================================================================
 * inline definitions
 */
#ifdef GFSM_INLINE_ENABLED
# include <gfsmArcIndex.hi>
#endif

/*======================================================================
 * END
 */
#endif /* _GFSM_ARCINDEX_H */
