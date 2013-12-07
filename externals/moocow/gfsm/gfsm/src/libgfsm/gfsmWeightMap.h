
/*=============================================================================*\
 * File: gfsmWeightMap.h
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

/** \file gfsmWeightMap.h
 *  \brief Abstract map from gpointers to gfsmWeights using GTree
 */

#ifndef _GFSM_WEIGHTMAP_H
#define _GFSM_WEIGHTMAP_H

#include <gfsmSet.h>
#include <gfsmSemiring.h>

/*======================================================================
 * Types
 */
/** \brief Type for maps from arbitrary data to gfsmWeights with a balanced binary tree.
 *  \detail really just an ugly wrapper for GTree
 */
typedef GTree gfsmWeightMap;

/** \brief Structure for mapping arbitrary data to gfsmWeights with a hash.
 *  \detail really just an ugly wrapper for GHashTable
 */
typedef struct {
  GHashTable   *table;   ///< hash table which does the dirty work
  gfsmDupFunc   key_dup; ///< key copying function
} gfsmWeightHash;

/** \brief Union type for converting between gfsmWeight and gpointer.
 *  \detail Requires that sizeof(gpointer)>=sizeof(gfsmWeight) in order to work properly.
 */
typedef union {
  gfsmWeight w; /**< Interpret underlying binary data as a gfsmWeight */
  gpointer   p; /**< Interpret underlying binary data as a gpointer   */
} gfsmWeightOrPointer;

/** \brief Type for a GArray of ::gfsmStateWeightPair */
typedef GArray gfsmStateWeightPairArray;

/*======================================================================
 * gfsmWeight <-> gpointer conversions
 */

///\name gfsmWeight <-> gpointer Conversions
//@{

/** Convert a gpointer to a gfsmWeight */
GFSM_INLINE
gfsmWeight gfsm_ptr2weight(const gpointer p);

/** Macro to convert gfsmWeight->gpointer */
GFSM_INLINE
gpointer gfsm_weight2ptr(const gfsmWeight w);

//@}

/*======================================================================
 * gfsmWeightMap: Constructors etc.
 */

///\name gfsmWeightMap: Constructors etc.
//@{

/** Create and return a new ::gfsmWeightMap
 */
GFSM_INLINE
gfsmWeightMap *gfsm_weightmap_new_full(GCompareDataFunc key_cmp_func,
				       gpointer         key_cmp_data,
				       GDestroyNotify   key_free_func);

/** Create and return a new weightmap which does not stored keys. */
GFSM_INLINE
gfsmWeightMap *gfsm_weightmap_new(GCompareFunc key_cmp_func);

/** Copy weightmap \a src to \a dst. \returns \a dst */
GFSM_INLINE
gfsmWeightMap *gfsm_weightmap_copy(gfsmWeightMap *dst, gfsmWeightMap *src);

/** Clear a ::gfsmWeightMap */
GFSM_INLINE
void gfsm_weightmap_clear(gfsmWeightMap *wm);

/** Destroy a weightmap */
GFSM_INLINE
void gfsm_weightmap_free(gfsmWeightMap *wm);
//@}


/*======================================================================
 * Accessors
 */
///\name gfsmWeightmap: Accessors
//@{

/** lookup: check weightmap membership */
GFSM_INLINE
gboolean gfsm_weightmap_contains(gfsmWeightMap *weightmap, gconstpointer key);

/** extended lookup: get weight associated with key */
GFSM_INLINE
gboolean gfsm_weightmap_lookup(gfsmWeightMap *weightmap, gconstpointer key, gfsmWeight *wp);

/** insert a new key->weight mapping into the weightmap */
//#define _gfsm_weightmap_insert(weightmap,key,w) g_tree_insert((weightmap),((gpointer)(key)),gfsm_weight2ptr(w))

/** insert a new key->weight mapping into the weightmap */
GFSM_INLINE
void gfsm_weightmap_insert(gfsmWeightMap *weightmap, gconstpointer key, gfsmWeight w);

/** Get size of weightmap */
#define gfsm_weightmap_size(weightmap) g_tree_nnodes(weightmap)

/** Remove an element from a weightmap */
#define gfsm_weightmap_remove(weightmap,key) g_tree_remove((weightmap),((gpointer)(key)))

/** Traversal (see g_tree_foreach) */
#define gfsm_weightmap_foreach(weightmap,func,data) g_tree_foreach((weightmap),(func),(data))

/** Copy contents of a ::gfsmWeightMap into a ::gfsmStateWeightPairArray
 *  \param weightmap weightmap to examine
 *  \param array     array to be populated, or NULL to allocate a new array
 *  \returns \a array, or a newly allocated ::gfsmStateWeightPairArray
 *  \note Caller is responsible for freeing \a array when it is no longer needed.
 */
gfsmStateWeightPairArray *gfsm_weightmap_to_array(gfsmWeightMap *weightmap, gfsmStateWeightPairArray *array);

//@}



/*======================================================================
 * gfsmWeightHash: Constructors etc.
 */
///\name gfsmWeightHash: Constructors etc.
//@{
/** Create and return a new hashing weight-map  */
GFSM_INLINE
gfsmWeightHash *gfsm_weighthash_new_full(gfsmDupFunc key_dup_func,
					 GHashFunc   key_hash_func,
					 GEqualFunc  key_equal_func,
					 GDestroyNotify key_destroy_func);

/** create & return a new hashing weightmap (returned map will not copy or free keys */
#define gfsm_weighthash_new(key_hash_f,key_equal_f) \
  gfsm_weighthash_new_full(NULL,(key_hash_f),(key_equal_f),NULL)

/** clear a weight-hash */
GFSM_INLINE
void gfsm_weighthash_clear(gfsmWeightHash *wh);

/** destroy a weight-hash */
GFSM_INLINE
void gfsm_weighthash_free(gfsmWeightHash *wh);
//@}


/*======================================================================
 * gfsmWeightHash: Accessors
 */
///\name gfsmWeightHash: Accessors
//@{

/** extended lookup: get weight associated with key */
GFSM_INLINE
gboolean gfsm_weighthash_lookup(gfsmWeightHash *wh, gconstpointer key, gfsmWeight *wp);

/** insert a key->weight mapping into the weighthash */
GFSM_INLINE
void gfsm_weighthash_insert(gfsmWeightHash *wh, gconstpointer key, gfsmWeight w);

/** Possibly insert a key->weight mapping into the weighthash
 *  The mapping \a (key=>w) is inserted if either no mapping for \a key exists in \a wh,
 *  or if \a w is strictly less-than the stored weight for \a key according to \a sr.
 *
 *  \returns TRUE if the mapping was updated, otherwise FALSE.
 */
GFSM_INLINE
gboolean gfsm_weighthash_insert_if_less(gfsmWeightHash *wh, gconstpointer key, gfsmWeight w, gfsmSemiring *sr);

/** Possibly insert a key->weight mapping into the weighthash
 *  The mapping \a (key=>w) is inserted if no mapping for \a key exists in \a wh.
 *  Otherwise, the stored weight \a (stored_w) for \a key is set to \a (w+stored_w)
 *  just in case \a (w+stored_w) is strictly less than \a stored_w for \a key according to \a sr.
 *
 *  \returns TRUE if the mapping was updated, otherwise FALSE.
 */
GFSM_INLINE
gboolean gfsm_weighthash_insert_sum_if_less(gfsmWeightHash *wh, gconstpointer key, gfsmWeight w, gfsmSemiring *sr);

/** Traversal (see g_hash_table_foreach) */
#define gfsm_weighthash_foreach(wh,func,data) \
  g_hash_table_foreach((wh)->table,(func),(data))

//@}

//-- inline definitions
#ifdef GFSM_INLINE_ENABLED
# include <gfsmWeightMap.hi>
#endif

#endif /* _GFSM_WEIGHTMAP_H */
