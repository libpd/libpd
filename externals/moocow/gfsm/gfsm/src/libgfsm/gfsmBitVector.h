
/*=============================================================================*\
 * File: gfsmBitVector.h
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

/** \file gfsmBitVector.h
 *  \brief Bit vector utilities using GArray
 */

#ifndef _GFSM_BITVECTOR_H
#define _GFSM_BITVECTOR_H

#include <gfsmIO.h>

/*======================================================================
 * Types
 */
/// bit vector type: really just a wrapper for GArray
typedef GArray gfsmBitVector;

/*======================================================================
 * Utilities
 */
///\name Utilities
//@{
/**  Low-level utility to convert bit-indices to byte-indices (starting from 0) */
GFSM_INLINE
guint gfsm_bitvector_bits2bytes_(guint nbits);

/** Low-level utility to convert byte-sizes to bit-sizes */
GFSM_INLINE
guint gfsm_bitvector_bytes2bits_(guint nbytes);

//@}

/*======================================================================
 * Constructors etc.
 */
///\name Constructors etc.
//@{

/** Create a new bit vector of length 0 */
GFSM_INLINE
gfsmBitVector *gfsm_bitvector_new(void);

/** Create a new bit vector of length 0 with reserved space for at least \a nbits bits */
GFSM_INLINE
gfsmBitVector *gfsm_bitvector_sized_new(guint nbits);

/** Resize a bit vector \a bv to at least \a nbits bits (rounded to next byte boundary) */
GFSM_INLINE
void gfsm_bitvector_resize(gfsmBitVector *bv, guint nbits);

/** Get current size (in bits) of a bit vector \a bv */
GFSM_INLINE
guint gfsm_bitvector_size(gfsmBitVector *bv);

/** Clear a bit vector */
GFSM_INLINE
void gfsm_bitvector_clear(gfsmBitVector *bv);

/** Set all bits to zero. \returns altered bv */
GFSM_INLINE
gfsmBitVector *gfsm_bitvector_zero(gfsmBitVector *bv);

/** Set all bits to one. \returns altered bv */
GFSM_INLINE
gfsmBitVector *gfsm_bitvector_one(gfsmBitVector *bv);

/** Destroy a bit vector */
GFSM_INLINE
void gfsm_bitvector_free(gfsmBitVector *bv);
//@}

/*======================================================================
 * Accessors
 */
///\name Accessors
//@{

/** Get the value (0 or 1) of the bit at index \a i in vector \a bv.
 */
GFSM_INLINE
gboolean gfsm_bitvector_get(gfsmBitVector *bv, guint i);

/** Set the value (0 or 1) of the bit at index \a i to boolean value \a v in vector \a bv.
 *  Formerly implemented as a macro which evaluates its arguments multiple times.
 */
GFSM_INLINE
void gfsm_bitvector_set(gfsmBitVector *bv, guint i, gboolean v);

//@}

/*======================================================================
 * I/O
 */
///\name I/O
//@{

/** Write the contents of a ::gfsmBitVector to a (binary) ::gfsmIOHandle.
 *  \param bv bit-vector to write
 *  \param ioh handle to which data is to be written
 *  \param errp if an error occurs, \a *errp will hold an error message
 *  \returns true on success
 */
gboolean gfsm_bitvector_write_bin_handle(gfsmBitVector *bv, gfsmIOHandle *ioh, gfsmError **errp);

/** Read the contents of a ::gfsmBitVector from a (binary) ::gfsmIOHandle.
 *  \param bv bit-vector into which data is to be read
 *  \param ioh handle from which data is to be read
 *  \param errp if an error occurs, \a *errp will hold an error message
 *  \returns true on success
 */
gboolean gfsm_bitvector_read_bin_handle(gfsmBitVector *bv, gfsmIOHandle *ioh, gfsmError **errp);

//@}

//-- inline definitions
#ifdef GFSM_INLINE_ENABLED
# include <gfsmBitVector.hi>
#endif

#endif /* _GFSM_BITVECTOR_H */
