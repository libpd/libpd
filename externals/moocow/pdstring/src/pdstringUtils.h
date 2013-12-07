/* -*- Mode: C -*- */
/*=============================================================================*\
 * File: pdstringUtils.h
 * Author: Bryan Jurish <moocow@ling.uni-potsdam.de>
 * Description: pdstring: common utilities (static include)
 *
 * Copyright (c) 2009 Bryan Jurish.
 *
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file "COPYING", in this distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *=============================================================================*/

#ifndef PDSTRING_UTILS_H
#define PDSTRING_UTILS_H

#include <string.h>
#include <m_pd.h>
#include <stdlib.h>
#include "mooPdUtils.h"

/*=====================================================================
 * Constants
 *=====================================================================*/

/* PDSTRING_EOS_NONE
 *  + "safe" float value to use as x_eos if no truncation is desired
 */
#define PDSTRING_EOS_NONE 1e38f

/* PDSTRING_DEFAULT_BUFLEN
 *  + common default buffer length
 */
#define PDSTRING_DEFAULT_BUFLEN 256

/* PDSTRING_DEFAULT_GET
 *  + common default buffer grow length
 */
#define PDSTRING_DEFAULT_GET 256

/* PDSTRING_BYSTES_GET
 *  + number of extra bytes to get when buffer must grow
 */
#define PDSTRING_BYTES_GET PDSTRING_DEFAULT_GET
#define PDSTRING_WCHARS_GET PDSTRING_DEFAULT_GET
#define PDSTRING_ATOMS_GET PDSTRING_DEFAULT_GET

//#define PDSTRING_STATIC static
#define PDSTRING_STATIC

/*=====================================================================
 * Structures & Types
 *=====================================================================*/

/* t_pdstring_bytes
 *  + a byte-string buffer
 */
typedef struct _pdstring_bytes {
  unsigned char *b_buf;    //-- byte-string buffer
  int            b_len;    //-- current length of b_buf
  size_t         b_alloc;  //-- allocated size of b_buf
} t_pdstring_bytes;

/* t_pdstring_wchars
 *  + a wide character buffer
 */
typedef struct _pdstring_wchars {
  wchar_t       *w_buf;    //-- wide character buffer
  int            w_len;    //-- current length of w_buf
  size_t         w_alloc;  //-- allocated size of w_buf
} t_pdstring_wchars;

/* t_pdstring_atoms
 *  + an atom-list buffer
 */
typedef struct _pdstring_atoms {
  t_atom        *a_buf;    //-- t_atom buffer (aka argv)
  int            a_len;    //-- current length of a_buf (aka argc)
  size_t         a_alloc;  //-- allocated size of a_buf
} t_pdstring_atoms;

/* t_pdstring_floatarray
 *  + read/store strings to t_array (of t_float)
 */
typedef struct _pdstring_floatarray {
  t_symbol      *fa_name;   //-- name bound to array
  int            fa_offset; //-- offset at which to begin operation
  t_garray      *fa_garray; //-- underlying t_garray*, populated by pdstring_floatarray_getarray()
  int            fa_vlen;   //-- length of fa_vec (number of points), populated by pdstring_floatarray_getvec()
  t_float       *fa_vec;    //-- underlying t_float*, populated by pdstring_floatarray_getvec()
} t_pdstring_floatarray;


/*=====================================================================
 * Initialization
 *=====================================================================*/

/*---------------------------------------------------------------------
 * bytes
 */
PDSTRING_STATIC void pdstring_bytes_clear(t_pdstring_bytes *b);
PDSTRING_STATIC void pdstring_bytes_realloc(t_pdstring_bytes *b, size_t n);
PDSTRING_STATIC void pdstring_bytes_init(t_pdstring_bytes *b, size_t n);

/*---------------------------------------------------------------------
 * wchars
 */
PDSTRING_STATIC void pdstring_wchars_clear(t_pdstring_wchars *w);
PDSTRING_STATIC void pdstring_wchars_realloc(t_pdstring_wchars *w, size_t n);
PDSTRING_STATIC void pdstring_wchars_init(t_pdstring_wchars *w, size_t n);

/*---------------------------------------------------------------------
 * atoms
 */
PDSTRING_STATIC void pdstring_atoms_clear(t_pdstring_atoms *a);
PDSTRING_STATIC void pdstring_atoms_realloc(t_pdstring_atoms *a, size_t n);
PDSTRING_STATIC void pdstring_atoms_init(t_pdstring_atoms *a, size_t n);

/*---------------------------------------------------------------------
 * floatarray
 */
PDSTRING_STATIC void pdstring_floatarray_clear(t_pdstring_floatarray *a);             //-- clears all data (no free)
PDSTRING_STATIC void pdstring_floatarray_realloc(t_pdstring_floatarray *a, size_t n); //-- calls garray_resize()
PDSTRING_STATIC void pdstring_floatarray_init(t_pdstring_floatarray *a, size_t n);    //-- if a->fa_name is set
PDSTRING_STATIC void pdstring_floatarray_getvec(t_object *x, t_pdstring_floatarray *a); //-- gets underlying pointers


/*=====================================================================
 * Conversions
 *=====================================================================*/

/*--------------------------------------------------------------------
 * pdstring_any2bytes()
 *  + x is used for error reporting
 *  + uses x_binbuf for conversion
 *  + selector sel is added to binbuf too, if it is none of  {NULL, &s_float, &s_list, &s_}
 *  + x_binbuf may be NULL, in which case a temporary t_binbuf is created and used
 *    - in this case, output bytes are copied into *dst, reallocating if required
 *  + if x_binbuf is given and non-NULL, dst may be NULL.
 *    - if dst is non-NULL, its values will be clobbered by those returned by binbuf_gettext()
 */
PDSTRING_STATIC void pdstring_any2bytes(void *x, t_pdstring_bytes *dst, t_symbol *sel, t_pdstring_atoms *src, t_binbuf *x_binbuf);


/*--------------------------------------------------------------------
 * pdstring_bytes2any()
 *  + uses x_binbuf for conversion
 *  + x_binbuf may be NULL, in which case a temporary t_binbuf is created and used
 *    - in this case, output atoms are copied into *dst, reallocating if required
 *  + if x_binbuf is given and non-NULL, dst may be NULL.
 *    - if dst is non-NULL, its values will be clobbered by those returned by binbuf_getnatom() & binbuf_getvec()
 */
PDSTRING_STATIC void pdstring_bytes2any(void *x, t_pdstring_atoms *dst, t_pdstring_bytes *src, t_binbuf *x_binbuf);

/*--------------------------------------------------------------------
 * pdstring_atoms2bytes()
 *  + always appends a final NUL byte to *dst_buf, even if src_argv doesn't contain one
 *  + returns number of bytes actually written to *dst_buf, __including__ implicit trailing NUL
 */
PDSTRING_STATIC int pdstring_atoms2bytes(void *x, t_pdstring_bytes *dst, t_pdstring_atoms *src, t_float x_eos);

/*--------------------------------------------------------------------
 * pdstring_atoms2wchars()
 *  + always appends a final NUL wchar_t to dst->w_buf, even if src->a_buf doesn't contain one
 *  + returns number of bytes actually written to dst->w_buf, __including__ implicit trailing NUL
 *  + but dst->w_len does NOT include implicit trailing NUL
 */
PDSTRING_STATIC int pdstring_atoms2wchars(void *x, t_pdstring_wchars *dst, t_pdstring_atoms *src, t_float x_eos);

/*--------------------------------------------------------------------
 * pdstring_bytes2wchars()
 */
PDSTRING_STATIC int pdstring_bytes2wchars(void *x, t_pdstring_wchars *dst, t_pdstring_bytes *src);

/*--------------------------------------------------------------------
 * pdstring_wchars2bytes()
 */
PDSTRING_STATIC int pdstring_wchars2bytes(void *x, t_pdstring_bytes *dst, t_pdstring_wchars *src);

/*--------------------------------------------------------------------
 * pdstring_bytes2atoms()
 *  + implicitly appends x_eos if >= 0 and != PDSTRING_EOS_NONE
 */
PDSTRING_STATIC void pdstring_bytes2atoms(void *x, t_pdstring_atoms *dst, t_pdstring_bytes *src, t_float x_eos);

/*--------------------------------------------------------------------
 * pdstring_wchars2atoms()
 */
PDSTRING_STATIC void pdstring_wchars2atoms(void *x, t_pdstring_atoms *dst, t_pdstring_wchars *src);

#endif /* PDSTRING_UTILS_H */
