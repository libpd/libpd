/*=============================================================================*\
 * File: gfsmPaths.h
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

/** \file gfsmPaths.h
 *  \brief Path discovery & enumeration
 */

#ifndef _GFSM_PATHS_H
#define _GFSM_PATHS_H

#include <gfsmAutomaton.h>

/*======================================================================
 * Types: paths
 */

/// Type for an automaton path
typedef struct {
  gfsmLabelVector *lo;  /**< lower label sequence */
  gfsmLabelVector *hi;  /**< upper label sequence */
  gfsmWeight       w;   /**< weight attached to this path */
} gfsmPath;



/*======================================================================
 * Methods: Path Utilities
 */

///\name Path Utilities
//@{

//------------------------------
/** Copy gfsmLabelVector. \returns \a dst */
gfsmLabelVector *gfsm_label_vector_copy(gfsmLabelVector *dst, gfsmLabelVector *src);

/** Duplicate a gfsmLabelVector. \returns \a dst */
#define gfsm_label_vector_dup(src) \
  gfsm_label_vector_copy(g_ptr_array_sized_new(src->len), src)

/** Reverse a gfsmLabelVector. \returns \a v */
gfsmLabelVector *gfsm_label_vector_reverse(gfsmLabelVector *v);

//------------------------------
/** Create and return a new gfsmPath, specifying components
 *  If either of \a lo or \a hi are NULL, a new vector will be created.
 */
gfsmPath *gfsm_path_new_full(gfsmLabelVector *lo, gfsmLabelVector *hi, gfsmWeight w);

/** Create and return a new empty gfsmPath, specifying semiring. */
#define gfsm_path_new(sr) \
  gfsm_path_new_full(NULL,NULL,gfsm_sr_one(sr))

/** Create and return a new gfsmPath as a copy of an existing gfsmPath */
gfsmPath *gfsm_path_new_copy(gfsmPath *p1);

/** Create and return a new gfsmPath, appending to an existing path */
gfsmPath *gfsm_path_new_append(gfsmPath *p1, gfsmLabelVal lo, gfsmLabelVal hi, gfsmWeight w, gfsmSemiring *sr);

/** Create and return a new gfsmPath as a copy of an existing gfsmPath with weight multiplied by \a w */
gfsmPath *gfsm_path_new_times_w(gfsmPath *p1, gfsmWeight w, gfsmSemiring *sr);

/** Append an arc to a gfsmPath */
void gfsm_path_push(gfsmPath *p, gfsmLabelVal lo, gfsmLabelVal hi, gfsmWeight w, gfsmSemiring *sr);

/** Pop an arc from a gfsmPath */
void gfsm_path_pop(gfsmPath *p, gfsmLabelVal lo, gfsmLabelVal hi);

/** 3-way path comparison function. */
gint gfsm_path_compare_data(const gfsmPath *p1, const gfsmPath *p2, gfsmSemiring *sr);

/** Reverse a gfsmPath */
gfsmPath *gfsm_path_reverse(gfsmPath *p);

/** Destroy a gfsmPath */
void gfsm_path_free(gfsmPath *p);
//@}

/*======================================================================
 * Methods: Automaton Serialization
 */

///\name Automaton Serialization
//@{

//------------------------------
/** Serialize a gfsmAutomaton to a set of (gfsmPath*)s.
 *  Really just a wrapper for gfsm_automaton_paths_full()
 *
 *  \param fsm   Acyclic automaton to be serializd
 *  \param paths output set or NULL
 *
 *  \returns \a paths if non-NULL, otherwise a new gfsmSet*.
 */
gfsmSet *gfsm_automaton_paths(gfsmAutomaton *fsm, gfsmSet *paths);

/** Serialize a gfsmAutomaton to a set of (gfsmPath*)s.
 *
 *  Causes deep recursion for cyclic automata.
 *  Returns a gfsmSet whose elements are (gfsmPath*)s.
 *  allocated with g_new().  It is the caller's responsibility to free the
 *  returned objects.
 *
 *  \param fsm   Acyclic automaton to be serializd
 *  \param which Which side of arc-labels to serialize
 *  \param paths output set or NULL
 *
 *  \returns \a paths if non-NULL, otherwise a new gfsmSet*.
 */
gfsmSet *gfsm_automaton_paths_full(gfsmAutomaton *fsm, gfsmSet *paths, gfsmLabelSide which);


/** Recursive guts for gfsm_automaton_paths() */
gfsmSet *_gfsm_automaton_paths_r(gfsmAutomaton *fsm,
				 gfsmSet       *paths,
				 gfsmLabelSide  which, 
				 gfsmStateId    q,
				 gfsmPath      *path);


//------------------------------
/** Convert a gfsmPathSet to a list of (char*)s.
 *  \a abet_lo and \a abet_hi should be (gfsmStringAlphabet*)s.
 */
GSList *gfsm_paths_to_strings(gfsmSet *paths,
			      gfsmAlphabet *abet_lo,
			      gfsmAlphabet *abet_hi,
			      gfsmSemiring       *sr,
			      gboolean warn_on_undefined,
			      gboolean att_style,
			      GSList *strings);

/** \brief Utility struct for gfsm_paths_to_strings() */
typedef struct gfsmPathsToStringsOptions_ {
  gfsmAlphabet *abet_lo;  ///< should be a gfsmStringAlphabet*
  gfsmAlphabet *abet_hi;  ///< should be a gfsmStringAlphabet*
  gfsmSemiring       *sr; ///< semiring for weight-based set sorting
  gboolean warn_on_undefined;  ///< warn on undefined symbols?
  gboolean att_style;          ///< use ATT-style output?
  GSList *strings;             ///< output list
} gfsmPathsToStringsOptions;

/** backwards compatible type alias */
#define _gfsm_paths_to_strings_options gfsmPathsToStringsOptions_

/** Utility for gfsm_paths_to_strings() */
gboolean _gfsm_paths_to_strings_foreach_func(gfsmPath *path,
					     gpointer value_dummy,
					     gfsmPathsToStringsOptions *opts);

/** Append string for a single gfsmPath* to a GString,
 *  which may be NULL to allocate a new string.
 *  \returns \a gs if non-NULL, otherwise a new GString*.
 *  \warning it is the caller's responsibility to free the returned GString*.
 */
GString *gfsm_path_to_gstring(gfsmPath     *path,
			      GString      *gs,
			      gfsmAlphabet *abet_lo,
			      gfsmAlphabet *abet_hi,
			      gfsmSemiring *sr,
			      gboolean      warn_on_undefined,
			      gboolean      att_style);

/** Allocate and return a new string (char*) for a single gfsmPath*.
 *  \returns new (char*) representing \a path.
 *  \warning it is the callers responsibility to free the returned \a char*.
 */
char *gfsm_path_to_string(gfsmPath     *path,
			  gfsmAlphabet *abet_lo,
			  gfsmAlphabet *abet_hi,
			  gfsmSemiring *sr,
			  gboolean      warn_on_undefined,
			  gboolean      att_style);


//------------------------------

/** Extract upper side of all paths from a Viterbi trellis.
 *
 *  Returns a gfsmSet whose elements are (gfsmPath*)s.
 *  allocated with g_new().  It is the caller's responsibility to free the
 *  returned objects.
 *
 *  \returns \a paths if non-NULL, otherwise a new gfsmSet*.
 */
#define gfsm_viterbi_trellis_paths(trellis,paths) \
  gfsm_viterbi_trellis_paths_full((trellis),(paths),gfsmLSUpper)


/** Extract all paths from a Viterbi trellis.
 *
 *  Returns a gfsmSet whose elements are (gfsmPath*)s.
 *  allocated with g_new().  It is the caller's responsibility to free the
 *  returned objects.
 *
 *  \returns \a paths if non-NULL, otherwise a new gfsmSet*.
 */
gfsmSet *gfsm_viterbi_trellis_paths_full(gfsmAutomaton *trellis, gfsmSet *paths, gfsmLabelSide which);


/** Extract the upper-side of the best path from a Viterbi trellis.
 *  \returns \a path if non-NULL, otherwise a new gfsmPath*.
 */
#define gfsm_viterbi_trellis_bestpath(trellis,path) \
  gfsm_viterbi_trellis_bestpath_full((trellis),(path),gfsmLSUpper)

/** Extract the best path from a Viterbi trellis.
 *  \returns \a path if non-NULL, otherwise a new gfsmPath*.
 */
gfsmPath *gfsm_viterbi_trellis_bestpath_full(gfsmAutomaton *trellis, gfsmPath *path, gfsmLabelSide which);

/** Guts for gfsm_viterbi_trellis_*path*() */
void _gfsm_viterbi_trellis_bestpath_r(gfsmAutomaton *trellis,
				      gfsmPath      *path,
				      gfsmLabelSide  which,
				      gfsmStateId    qid);

//@}


#endif /* _GFSM_PATHS_H */
