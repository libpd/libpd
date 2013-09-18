
/*=============================================================================*\
 * File: gfsmSet.h
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

/** \file gfsmSet.h
 *  \brief Abstract set type using GTree
 */

#ifndef _GFSM_SET_H
#define _GFSM_SET_H

#include <gfsmMem.h>

/*======================================================================
 * Types
 */
/** \brief Type for sets of pointers
 *  \detail really just an ugly wrapper for GTree
 */
typedef GTree gfsmSet;


/*======================================================================
 * Constructors etc.
 */
///\name gfsmSet: Constructors etc.
//@{
/** Create and return a new set */
GFSM_INLINE
gfsmSet *gfsm_set_new_full(GCompareDataFunc key_cmp_func, gpointer key_cmp_data, GDestroyNotify key_free_func);

/** gfsm_set_new(key_compare_func): create and return a new set
 *  (returned set will not free elements)
 */
GFSM_INLINE
gfsmSet *gfsm_set_new(GCompareFunc key_cmp_func);

/** Copy set \a src to \a dst. \returns \a dst */
GFSM_INLINE
gfsmSet *gfsm_set_copy(gfsmSet *dst, gfsmSet *src);

/** Utilitiy for gfsm_set_copy() */
gboolean gfsm_set_copy_foreach_func(gpointer key, gpointer value, gfsmSet *dst);

/** clear a set */
void gfsm_set_clear(gfsmSet *set);

/** Destroy a set
 *  \code void gfsm_set_free(gfsmSet *set); \endcode
 */
#define gfsm_set_free g_tree_destroy

//@}


/*======================================================================
 * Accessors
 */
///\name gfsmSet: Accessors
//@{
/** check set membership */
#define gfsm_set_contains(set,key) g_tree_lookup(set,key)

/** insert a new key into the set */
#define gfsm_set_insert(set,key) g_tree_insert(set,key,(gpointer)1)

/** get size of set */
#define gfsm_set_size(set) g_tree_nnodes(set)

/** Remove an element from a set */
#define gfsm_set_remove(set,key) g_tree_remove(set,key)

/** Traversal (see g_tree_foreach) */
#define gfsm_set_foreach(set,func,data) g_tree_foreach(set,func,data)
//@}

/*======================================================================
 * set: Algebra
 */
///\name gfsmSet: Algebra
//@{

/** Add all elements of set \a set2 to \a set1.
 *  If \a dupfunc is non-NULL, it will be used to copy elements from \a set2,
 *  otherwise elements will be copied as literal gpointer values.
 * \returns altered \a set1
 */
GFSM_INLINE
gfsmSet *gfsm_set_union(gfsmSet *set1, gfsmSet *set2, gfsmDupFunc dupfunc);

/** Remove all elements in \a set2 from \a set1.
 * \returns altered \a set1 */
GFSM_INLINE
gfsmSet *gfsm_set_difference(gfsmSet *set1, gfsmSet *set2);

/** Remove all elements from \a set1 which are not also in \a set2.
 * \returns altered \a set1 */
gfsmSet *gfsm_set_intersection(gfsmSet *set1, gfsmSet *set2);

//@}


/*======================================================================
 * Converters
 */
///\name gfsmSet: converters
//@{

/** Get a GSList of a set's elements */
GFSM_INLINE
GSList *gfsm_set_to_slist(gfsmSet *set);

/** Low-level utilitity for gfsm_set_to_slist() */
gboolean gfsm_set_to_slist_foreach_func(gpointer key, gpointer value, GSList **dst);

/** Append a set's elements to a GPtrArray */
GFSM_INLINE
void gfsm_set_to_ptr_array(gfsmSet *set, GPtrArray *array);

/** Low-level foreach utilitity for gfsm_set_to_array() */
gboolean gfsm_set_to_ptr_array_foreach_func(gpointer key, gpointer value, GPtrArray *dst);
//@}

/*======================================================================
 * Debugging
 */
#ifdef GFSM_DEBUG_ENABLED
#include <stdio.h>

///\name gfsmSet: debugging
//@{

/** Dump contents of a gfsmSet using '%u' to a FILE* */
void gfsm_set_print_uint(gfsmSet *set, FILE *f);

//@}
#endif /* GFSM_DEBUG_ENABLED */

//-- inline definitions
#ifdef GFSM_INLINE_ENABLED
# include <gfsmSet.hi>
#endif

#endif /* _GFSM_SET_H */
