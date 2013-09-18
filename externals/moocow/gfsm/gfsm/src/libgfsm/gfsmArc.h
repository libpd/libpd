/*=============================================================================*\
 * File: gfsmArc.h
 * Author: Bryan Jurish <moocow@ling.uni-potsdam.de>
 * Description: finite state machine library: arcs
 *
 * Copyright (c) 2004-2008 Bryan Jurish.
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

/** \file gfsmArc.h
 *  \brief Arc (transition) definitions & utilities
 */

#ifndef _GFSM_ARC_H
#define _GFSM_ARC_H

#include <gfsmSemiring.h>
#include <stdarg.h>

/// "Heavy" arc structure
typedef struct {
  gfsmStateId       source;  /**< ID of source node */
  gfsmStateId       target;  /**< ID of target node */
  gfsmLabelId       lower;   /**< Lower label */
  gfsmLabelId       upper;   /**< Upper label */
  gfsmWeight        weight;  /**< arc weight */
} gfsmArc;

/// Type for identifying arc-label "sides" in a transducer (lower vs. upper)
typedef enum {
  gfsmLSBoth  = 0, ///< Both sides (lower and upper)
  gfsmLSLower = 1, ///< Lower side only
  gfsmLSUpper = 2  ///< Upper side only
} gfsmLabelSide;

//----------------------------------------------------------------------
// arc sorting (new)

/// Enum type for elementary builtin comparisons on ::gfsmArc
/** \since v0.0.10 */
typedef enum {
  gfsmACNone    = 0x0,  /**< '_': no comparison at all */
  //
  //-- Forward (ascending)
  gfsmACLower   = 0x1,  /**< 'l': compare lower labels (ascending order) */
  gfsmACUpper   = 0x2,  /**< 'u': compare upper labels (ascending order) */
  gfsmACWeight  = 0x3,  /**< 'w': compare semiring weights (ascending order) */
  gfsmACSource  = 0x4,  /**< 's': compare source states (if supported and meaningful, ascending order) */
  gfsmACTarget  = 0x5,  /**< 't': compare target states (if supported and meaningful, ascending order) */
  gfsmACUser    = 0x6,  /**< 'x': pseudo-field indicating a user-defined comparison */
  gfsmACUnused1 = 0x7,  /**< unused */
  //
  //-- Reverse (descending)
  gfsmACReverse = 0x8,  /**< not really a comparison: the bit 0x8 is set for all "reverse" comparisons */
  gfsmACLowerR  = 0x9,  /**< 'L': compare lower labels (descending order) */
  gfsmACUpperR  = 0xa,  /**< 'U': compare upper labels (descending order) */
  gfsmACWeightR = 0xb,  /**< 'W': compare semiring weights (descending order) */
  gfsmACSourceR = 0xc,  /**< 'S': compare source states (if supported and meaningful, descending order) */
  gfsmACTargetR = 0xd,  /**< 'T': compare target states (if supported and meaningful, descending order) */
  gfsmACUserR   = 0xe,  /**< 'X': pseudo-field for reversed user-defined comparisons, for symmetry */
  //
  //-- Pseudo-comparisons
  gfsmACAll     = 0xf   /**< '*': pseudo-field for mask of all known elementary comparisons */
} gfsmArcComp;

/** \brief Prioritized list of up to 5 elementary arc comparisons packed as a guint32
 *  \detail
 *   The primary comparison is encoded as a ::gfsmArcComp in the least significant
 *   4-bit nybble of the integer, the secondary comparison is left-shifted by 4 bits,
 *   and so on.
 */
typedef guint32 gfsmArcCompMask;

/** Number of bits to left-shift ::gfsmArcCompMask for each arc sub-comparison using ::gfsmArcComp encoding */
#define gfsmACShift 4

/** Maximum number of ::gfsmArcComp fields supported by ::gfsmArcCompMask (pragmatic definition) */
#define gfsmACMaxN 6

/** Useful aliases for builtin arcsort modes, including backwards-compatible ::gfsmASMLower etc. */
typedef enum {
  gfsmASMNone   = 0x0,                                                                       /**< no sort */
  gfsmASMLower  = (gfsmACLower|(gfsmACUpper<<gfsmACShift)|(gfsmACTarget<<(2*gfsmACShift))),  /**< (lower,upper,target) */
  gfsmASMUpper  = (gfsmACUpper|(gfsmACLower<<gfsmACShift)|(gfsmACTarget<<(2*gfsmACShift))),  /**< (upper,lower,target) */
  gfsmASMWeight = gfsmACWeight,                                                              /**< (weight) */
  gfsmASMLowerWeight = (gfsmACLower|(gfsmACWeight<<gfsmACShift)),                            /**< (lower,weight) */
  gfsmASMUpperWeight = (gfsmACUpper|(gfsmACWeight<<gfsmACShift)),                            /**< (upper,weight) */
  gfsmASMUser        = gfsmACUser,                                                           /**< user-defined sort */
} gfsmArcSortModeE;

/// Semi-compatible typedef for arc sort mode
typedef gfsmArcCompMask gfsmArcSortMode;

/// Data for new-style generic arc comparison
typedef struct {
  gfsmArcCompMask       mask;               /**< Comparison precedence */
  gfsmSemiring         *sr;                 /**< Semiring to use for weight comparisons (if any) */
  GCompareDataFunc    user_compare_func;    /**< User comparison function (if any) */
  gpointer            user_data;            /**< User data (for \c user_compare_func) */
} gfsmArcCompData;

/*======================================================================
 * Methods: Arcs: Constructors etc.
 */

/// \name Arcs: Constructors etc.
//@{
/** Create and return a new (empty) ::gfsmArc */
GFSM_INLINE
gfsmArc *gfsm_arc_new(void);

/** Initialize a ::gfsmArc
 * \param a arc to initialize
 * \param src ID of source state
 * \param dst ID of target state
 * \param lo  ID of lower label
 * \param hi  ID of upper label
 * \param w   arc weight
 * \returns initialized arc \a a
 */
GFSM_INLINE
gfsmArc *gfsm_arc_init(gfsmArc *a,
		       gfsmStateId src,
		       gfsmStateId dst,
		       gfsmLabelId lo,
		       gfsmLabelId hi,
		       gfsmWeight wt);

/** Convenience macro to simultaneously create and initialize a ::gfsmArc
 * \param src ID of source state
 * \param dst ID of target state
 * \param lo  ID of lower label
 * \param hi  ID of upper label
 * \param w   arc weight
 * \returns newly allocated and initalized ::gfsmArc
 */
GFSM_INLINE
gfsmArc *gfsm_arc_new_full(gfsmStateId src, gfsmStateId dst, gfsmLabelVal lo, gfsmLabelVal hi, gfsmWeight wt);

/** Create an exact copy of the ::gfsmArc \a src */
GFSM_INLINE
gfsmArc *gfsm_arc_clone(gfsmArc *src);

/** Backwards-compatible convenience alias for gfsm_arc_clone() */
#define gfsm_arc_copy(src) gfsm_arc_clone(src)

/** Destroy a ::gfsmArc \a a */
GFSM_INLINE
void gfsm_arc_free(gfsmArc *a);
//@}

/*======================================================================
 * Methods: Arc: Accessors
 */
///\name Arc Accessors
//@{

/** Get source node of an arc -- may be gfsmNoState */
#define gfsm_arc_source(arcptr) ((arcptr)->source)

/** Get target node of an arc -- may be gfsmNoState */
#define gfsm_arc_target(arcptr) ((arcptr)->target)

/** Get lower label of an arc -- may be gfsmNoLabel */
#define gfsm_arc_lower(arcptr) ((arcptr)->lower)

/** Get upper label of an arc -- may be gfsmNoLabel */
#define gfsm_arc_upper(arcptr) ((arcptr)->upper)

/** Get weight of an arc -- may be gfsmNoWeight */
#define gfsm_arc_weight(arcptr) ((arcptr)->weight)

//@}


/*======================================================================
 * Arc Comparison Utilities
 */
///\name Arc Comparison Utilities
//@{

/** Generic 3-way comparison on arcs (inline version)
 *  \param a1 first arc to compare
 *  \param a2 second arc to compare
 *  \param acdata specifies comparison priorities
 *  \returns
 *    negative, zero, or positive integer depending on whether
 *    \a a1 is less-than, equal-to, or greater-than \a a2 according to \a acdata.
 */
GFSM_INLINE
gint gfsm_arc_compare_bymask_inline(gfsmArc *a1, gfsmArc *a2, gfsmArcCompData *acdata);

/** Generic 3-way comparison on arcs (extern version)
 *  Really just a wrapper for gfsm_arc_compare_mask_inline()
 */
gint gfsm_arc_compare_bymask(gfsmArc *a1, gfsmArc *a2, gfsmArcCompData *acdata);

/** Guts for gfsm_arc_compare_bymask_inline(): compare arcs w.r.t a single attribute \a cmp.
 *  \note gcc deems these calls "unlikely" and refuses to inline...
 */
GFSM_INLINE
gint gfsm_arc_compare_bymask_1_(gfsmArc *a1, gfsmArc *a2, gfsmArcComp cmp, gfsmArcCompData *acdata);

/** Parse a NUL-terminated string into a ::gfsmArcCompMask
 *  \param maskchars
 *    A NUL-terminated string representing the precedence among elementary comparisons.
 *    Each character represents a single elementary comparison.
 *    The primary comparison is the first character of the string.
 *    Correspondence of characters to comparisons is:
 *    \li 'l' ::gfsmACLower
 *    \li 'u' ::gfsmACUpper
 *    \li 'w' ::gfsmACWeight
 *    \li 's' ::gfsmACSource
 *    \li 't' ::gfsmACTarget
 *    \li 'x' ::gfsmACUser
 *    \li 'L' ::gfsmACLowerR
 *    \li 'U' ::gfsmACUpperR
 *    \li 'W' ::gfsmACWeightR
 *    \li 'S' ::gfsmACSourceR
 *    \li 'T' ::gfsmACTargetR
 *    \li 'X' ::gfsmACUserR
 *  \returns a ::gfsmArcCompMask for \a maskchars
 */
gfsmArcCompMask gfsm_acmask_from_chars(const char *maskchars);

/** Create and return a ::gfsmArcCompMask from a variable argument list of ::gfsmArcComp
 *  The argument list must be terminated with a zero (e.g. ::gfsmACNone)
 *  \param cmp0 primary comparison
 *  \param ...  secondary, tertiary, ... comparisons
 *  \returns ::gfsmArcCompMask for specified comparisons
 */
gfsmArcCompMask gfsm_acmask_from_args(gfsmArcComp cmp0, ...);

/** Create a partial ::gfsmArcCompMask for \a cmp as the \a nth (sub-)comparison */
GFSM_INLINE
gfsmArcCompMask gfsm_acmask_new(gfsmArcComp cmp, gint nth);

/** Get \a nth (sub-)comparison from a ::gfsmArcCompMask */
GFSM_INLINE
gfsmArcComp gfsm_acmask_nth(gfsmArcCompMask m, gint nth);

/** Get basic \a nth basic (sub-)comparison from a ::gfsmArcCompMask, disregarding sort order */
GFSM_INLINE
gfsmArcComp gfsm_acmask_nth_comp(gfsmArcCompMask m, gint nth);

/** Get sort order for \a nth (sub-)comparison of a ::gfsmArcCompMask.
 *  \returns a true value if \a nth sub-comparison is reversed (descending order), otherwise FALSE.
 */
GFSM_INLINE
gboolean gfsm_acmask_nth_reverse(gfsmArcCompMask m, gint nth);

/** Get single character representing the \a nth field of arc comparison mask \a m, as for gfsm_acmask_from_chars() */
GFSM_INLINE
gchar gfsm_acmask_nth_char(gfsmArcCompMask m, gint nth);

/** Get a static human-readable string representing the \a nth field of arc comparison mask \a m */
const gchar *gfsm_acmask_nth_string(gfsmArcCompMask m, gint nth);

/** Populate a character string representing a ::gfsmArcCompMask.
 *  \param mask[in]   mask to convert to a string
 *  \param chars[out] character string representing \a mask, in the format acepted by gfsm_acmask_from_chars(),
 *                    or NULL to allocate a new string.  If specified and non-NULL, \a chars should be
 *                    long enough to hold ::gfsmACMaxN+1 characters, since a terminating NUL
 *                    is implicitly added as the final character.
 *  \returns \a chars if specified, otherwise a newly allocated string.
 *  \note User is responsible for freeing the returned string with g_free() when it is no longer needed.
 */
gchar *gfsm_acmask_to_chars(gfsmArcCompMask m, gchar *chars);

/** Populate a GString* with a human-readable representation of a ::gfsmArcCompMask.
 *  \param mask[in]   mask to convert to a string
 *  \param gstr[out]  GString representing \a mask, in human-readable format,
 *                    or NULL to allocate a new GString*.
*  \returns \a gstr if specified, otherwise a newly allocated GString*.
 *  \note User is responsible for freeing the returned GString* with g_string_free() when it is no longer needed.
 */
GString *gfsm_acmask_to_gstring(gfsmArcCompMask m, GString *gstr);

/** Backwards-compatible arc sort mode name resolution function
 *  \deprecated in favor of gfsm_acmask_nth_string(), gfsm_acmask_to_gstring(), gfsm_acmask_to_chars()
 */
GFSM_INLINE
const gchar *gfsm_arc_sortmode_to_name(gfsmArcCompMask m);

//@}

//-- inline definitions
#ifdef GFSM_INLINE_ENABLED
# include <gfsmArc.hi>
#endif

#endif /* _GFSM_ARC_H */
