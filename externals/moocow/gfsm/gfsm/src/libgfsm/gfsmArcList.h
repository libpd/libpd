
/*=============================================================================*\
 * File: gfsmArcList.h
 * Author: Bryan Jurish <moocow@ling.uni-potsdam.de>
 * Description: finite state machine library: arc lists
 *  + formerly defined in gfsmArc.h
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

/** \file gfsmArcList.h
 *  \brief Definitions & utilities for arc lists, <b>Deprecated</b>
 *  \detail
 *  \deprecated
 *    in favor of gfsm_automaton_add_arc() and ::gfsmArcIter interface
 *  \see gfsmAutomaton.h, gfsmArcIter.h
 */

#ifndef _GFSM_ARC_LIST_H
#define _GFSM_ARC_LIST_H

#include <gfsmArc.h>

/// "Heavy" arc-list structure, no longer using GSList
typedef struct gfsmArcListNode_ {
  gfsmArc arc;                    /**< current arc */
  struct gfsmArcListNode_ *next;  /**< next node in the list */
} gfsmArcListNode;

/// Alias for gfsmArcListNode
typedef gfsmArcListNode gfsmArcList;


/*======================================================================
 * Methods: Arc List: Constructors etc.
 */
/// \name Arc List: Constructors etc.
//@{

/** Prepend the node \a nod to the ::gfsmArcList \a al
 *  \returns a pointer to the new 1st element of the arclist
 *  \deprecated in favor of gfsm_automaton_add_arc(), gfsm_arciter_insert()
 */
GFSM_INLINE
gfsmArcList *gfsm_arclist_prepend_node(gfsmArcList *al, gfsmArcList *nod);

/** Allocate and return a new arc-list node */
GFSM_INLINE
gfsmArcList *gfsm_arclist_new_full(gfsmStateId  src,
				   gfsmStateId  dst,
				   gfsmLabelVal lo,
				   gfsmLabelVal hi,
				   gfsmWeight   wt,
				   gfsmArcList  *nxt);


/** Insert an arc into a (possibly sorted) arclist.
 *  \param al ::gfsmArcList into which a new arc is to be inserted
 *  \param src source state id for the new arc
 *  \param dst target state id for the new arc
 *  \param lo  lower label for the new arc
 *  \param hi  upper label for the new arc
 *  \parm  wt  weight for the new arc
 *  \param acdata comparison data for 'smart' sorted insertion
 *  \returns a pointer to the (possibly new) 1st node of the arc list
 *  \deprecated in favor of gfsm_automaton_add_arc(), gfsm_arciter_insert()
 */
GFSM_INLINE
gfsmArcList *gfsm_arclist_insert(gfsmArcList *al,
				 gfsmStateId  src,
				 gfsmStateId  dst,
				 gfsmLabelVal lo,
				 gfsmLabelVal hi,
				 gfsmWeight   wt,
				 gfsmArcCompData *acdata);


/** Insert a single arc-link into a (possibly sorted) arclist.
 *  \param al   arc list into which \a link is to be inserted
 *  \param link arc list node to insert
 *  \param acdata sort data for 'smart' sorted insertion
 *  \returns a pointer to the (possibly new) 1st element of the arclist
 *  \deprecated in favor of gfsm_automaton_add_arc(), gfsm_arciter_insert()
 */
GFSM_INLINE
gfsmArcList *gfsm_arclist_insert_node(gfsmArcList *al, gfsmArcList *nod, gfsmArcCompData *acdata);

/** Low-level guts for gfsm_arclist_insert(), gfsm_arclist_insert_node() */
gfsmArcList *gfsm_arclist_insert_node_sorted(gfsmArcList *al, gfsmArcList *link, gfsmArcCompData *acdata);

/** Create and return a (deep) copy of an existing arc-list */
gfsmArcList *gfsm_arclist_clone(gfsmArcList *src);


/** Destroy an arc-list node and all subsequent nodes */
void gfsm_arclist_free(gfsmArcList *al);

/* Free a single node of an arc-list */
//void gfsm_arclist_free_node(gfsmArcList *nod);

//@}

/*======================================================================
 * Methods: Arc List: Accessors
 */
///\name Arc List: Access & Manipulation
//@{

/** Get the arc pointer for an arclist -- may be \c NULL
 *  \deprecated in favor of ::gfsmArcIter interface
 *  \see gfsmArcIter.h
 */
#define gfsm_arclist_arc(al) \
  ((al) ? (&(al->arc)) : NULL)

//  ((al) ? ((gfsmArc*)((al)->data)) : NULL)

/** Concatenate 2 arc-lists
 *  \param al1 initial sublist
 *  \param al2 final sublist
 *  \returns pointer to head of the concatenated list
 */
gfsmArcList *gfsm_arclist_concat(gfsmArcList *al1, gfsmArcList *al2);

/** Splice a single node out from a ::gfsmArcList.
 *  \param al  arc list
 *  \param nod node to extract
 *  \returns pointer to head of the new arc list, without \a nod
 *  \warning removed \a nod is not freed!
 *  \see gfsm_arclist_delte_node()
 */
gfsmArcList *gfsm_arclist_remove_node(gfsmArcList *al, gfsmArcList *nod);

/** Remove and free a single node from a ::gfsmArcList.
 *  \param al  arc list
 *  \param nod node to extract
 *  \returns pointer to head of the new arc list, without \a nod
 */
GFSM_INLINE
gfsmArcList *gfsm_arclist_delete_node(gfsmArcList *al, gfsmArcList *nod);

/** Reverse a ::gfsmArcList
 *  \param al  arc list to reverse
 *  \returns pointer to head of the reversed arc list
 */
gfsmArcList *gfsm_arclist_reverse(gfsmArcList *al);

//@}


/*======================================================================
 * Methods: Arc List: Utilities
 */
///\name Arc List: Utilities
//@{

/** Get length of an arc-list \a al (linear time) */
guint gfsm_arclist_length(gfsmArcList *al);
 //  Signature: <tt>guint gfsm_arclist_length(gfsmArcList *al)</tt>
//#define gfsm_arclist_length g_slist_length

/** Sort an arclist \a al using one of the builtin sort modes as specified by \a acdata.
 *  \param al    arc list to sort
 *  \param acdata sort data for builtin comparison
 *  \returns pointer to the new head of the sorted arc list
 */
GFSM_INLINE
gfsmArcList *gfsm_arclist_sort(gfsmArcList *al, gfsmArcCompData *acdata);

/** Sort an arclist \a al using a user-defined arc comparison function.
 *  \param al       arc list to sort
 *  \param cmpfunc  3-way comparison function on ::gfsmArc* for sorting
 *  \param data     additional data for \a cmpfunc
 *  \returns pointer to the new head of the sorted arc list
 */
GFSM_INLINE
gfsmArcList *gfsm_arclist_sort_with_data(gfsmArcList *al, GCompareDataFunc cmpfunc, gpointer data);

/** Alias for gfsm_arclist_sort_with_data() */
#define gfsm_arclist_sort_full gfsm_arclist_sort_with_data

/** low-level guts for gfsm_arclist_sort() */
gfsmArcList *gfsm_arclist_sort_real (gfsmArcList *list, GFunc compare_func, gpointer user_data);


//@}

//-- inline definitions
#ifdef GFSM_INLINE_ENABLED
# include <gfsmArcList.hi>
#endif

#endif /* _GFSM_ARC_LIST_H */
