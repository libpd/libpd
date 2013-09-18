/*=============================================================================*\
 * File: gfsmSemiring.h
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

/** \file gfsmSemiring.h
 *  \brief semiring types & operations
 */

#ifndef _GFSM_SEMIRING_H
#define _GFSM_SEMIRING_H

#include <gfsmCommon.h>
#include <float.h>

/*======================================================================
 * Semiring: types
 */
/** Builtin semiring types
 *  \see fsmcost(3)
 */
typedef enum {
  gfsmSRTUnknown  = 0,  ///< unknown semiring (should never happen)
  gfsmSRTBoolean  = 1,  ///< boolean semiring <set:{0,1}, plus:||, times:&&, less:>, zero:0, one:1>
  gfsmSRTLog      = 2,  ///< negative log semiring <set:[-inf,inf], plus:-log(e^-x+e^-y), times:+, less:<, zero:inf, one:0>
  gfsmSRTReal     = 3,  ///< real semiring: <set:[0,inf], plus:+, times:*, less:<, zero:0, one:1>
  gfsmSRTTrivial  = 4,  ///< trivial semiring <set:{0}, plus:+, times:+, less:!=, zero:0, one:0>
  gfsmSRTTropical = 5,  ///< tropical semiring: <set:[-inf,inf], plus:min, times:+, less:<, zero:inf, one:0>
  gfsmSRTPLog     = 6,  ///< positive log semiring <set:[-inf,inf], plus:log(e^x+e^y), times:+, less:>, zero:-inf, one:0>
  gfsmSRTUser     = 256 ///< user-defined semiring
} gfsmSRType;

/*======================================================================
 * Semiring: types: structs
 */
/// struct to represent a builtin semi-ring for gfsm arc weights
typedef struct {
  gfsmSRType type;    /**< type of this semiring */
  gfsmWeight zero;    /**< nil element of this semiring (identity for '+') */
  gfsmWeight one;     /**< unity element of this semiring (idendity for '*') */
} gfsmSemiring;

/*======================================================================
 * Semiring: types: functions
 */
/** Type for user-defined semiring unary predicates (i.e. member) */
typedef gboolean (*gfsmSRUnaryPredicate) (gfsmSemiring *sr, gfsmWeight x);

/// Type for user-defined semiring binary predicates (i.e. equal) */
typedef gboolean (*gfsmSRBinaryPredicate) (gfsmSemiring *sr, gfsmWeight x, gfsmWeight y);

/// Type for user-defined semiring unary operations */
typedef gfsmWeight (*gfsmSRUnaryOp) (gfsmSemiring *sr, gfsmWeight x);

/// Type for user-defined semiring binary operations */
typedef gfsmWeight (*gfsmSRBinaryOp) (gfsmSemiring *sr, gfsmWeight x, gfsmWeight y);


/*======================================================================
 * Semiring: types: user structs
 */
/// User-defined semirings for gfsm operations
typedef struct {
  gfsmSemiring sr;                  /**< inheritance magic */

  //-- user-defined semirings *must* set these functions
  gfsmSRBinaryPredicate equal_func; /**< equality predicate */
  gfsmSRBinaryPredicate less_func;  /**< order predicate */
  gfsmSRBinaryOp        plus_func;  /**< addition operation */
  gfsmSRBinaryOp        times_func; /**< multiplication operation */
} gfsmUserSemiring;

/*======================================================================
 * Semiring: methods: constructors etc.
 */
///\name Constructors etc.
//@{

/** Create, initialize (for builtin types), and return new semiring of type \a type */
GFSM_INLINE
gfsmSemiring *gfsm_semiring_new(gfsmSRType type);

/** Initialize and return a builtin semiring */
GFSM_INLINE
void gfsm_semiring_init(gfsmSemiring *sr, gfsmSRType type);

/** Initialize and return a semiring */
GFSM_INLINE
gfsmUserSemiring *gfsm_user_semiring_new(gfsmSRBinaryPredicate equal_func,
					 gfsmSRBinaryPredicate less_func,
					 gfsmSRBinaryOp        plus_func,
					 gfsmSRBinaryOp        times_func);

/** Copy a semiring */
GFSM_INLINE
gfsmSemiring *gfsm_semiring_copy(gfsmSemiring *sr);

/** Destroy a gfsmSemiring */
GFSM_INLINE
void gfsm_semiring_free(gfsmSemiring *sr);
//@}

/*======================================================================
 * Semiring: general accessors
 */
///\name General Accessors
//@{

/** Get 'zero' element of the ::gfsmSemiring* \a sr */
GFSM_INLINE
gfsmWeight gfsm_sr_zero(gfsmSemiring *sr);

/** Get 'one' element of the ::gfsmSemiring* \a sr */
GFSM_INLINE
gfsmWeight gfsm_sr_one(gfsmSemiring *sr);

/** Check equality of elements \a x and \a y with respect to ::gfsmSemiring* \a sr */
GFSM_INLINE
gboolean gfsm_sr_equal(gfsmSemiring *sr, gfsmWeight x, gfsmWeight y);

/** Check semiring element order */
GFSM_INLINE
gboolean gfsm_sr_less(gfsmSemiring *sr, gfsmWeight x, gfsmWeight y);

/** 3-way comparison for semiring values */
gint gfsm_sr_compare(gfsmSemiring *sr, gfsmWeight x, gfsmWeight y);

/** Semiring addition */
GFSM_INLINE
gfsmWeight gfsm_sr_plus(gfsmSemiring *sr, gfsmWeight x, gfsmWeight y);

/** Semiring multiplication */
GFSM_INLINE
gfsmWeight gfsm_sr_times(gfsmSemiring *sr, gfsmWeight x, gfsmWeight y);
//@}

/*======================================================================
 * Semiring: methods: string utilities
 */
///\name String utilities
//@{
/** Convert symbolic name of a semiring to a gfsmSRType */
gfsmSRType gfsm_sr_name_to_type(const char *name);

/** Convert a gfsmSRType to a (constant) symbolic name */
gchar *gfsm_sr_type_to_name(gfsmSRType type);
//@}

/*======================================================================
 * Semiring: methods: general utilities
 */
///\name General utilities
//@{
/** stable log addition.
 *  \returns log(exp(x)+exp(y))
 */
GFSM_INLINE
gfsmWeight gfsm_log_add(gfsmWeight x, gfsmWeight y);
//@}

//-- inline definitions
#ifdef GFSM_INLINE_ENABLED
# include <gfsmSemiring.hi>
#endif

#endif /* _GFSM_SEMIRING_H */
