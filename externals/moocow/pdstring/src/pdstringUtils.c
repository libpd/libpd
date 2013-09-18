/* -*- Mode: C -*- */
/*=============================================================================*\
 * File: pdstringUtils.c
 * Author: Bryan Jurish <moocow@ling.uni-potsdam.de>
 * Description: pdstring: common utilities
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

#ifndef PDSTRING_UTILS_C
#define PDSTRING_UTILS_C

#include <string.h>
#include <m_pd.h>
#include <stdlib.h>
#include "mooPdUtils.h"
#include "pdstringUtils.h"

/*=====================================================================
 * Debugging
 *=====================================================================*/
#define PDSTRING_UTILS_DEBUG 1
//#undef  PDSTRING_UTILS_DEBUG

#ifdef PDSTRING_UTILS_DEBUG
# define PDSDEBUG(x) x
#else
# define PDSDEBUG(x)
#endif

/*=====================================================================
 * Constants : see header
 *=====================================================================*/

/*=====================================================================
 * Structures & Types : see header
 *=====================================================================*/

/*=====================================================================
 * Initialization
 *=====================================================================*/

/*---------------------------------------------------------------------
 * bytes
 */
PDSTRING_STATIC void pdstring_bytes_clear(t_pdstring_bytes *b)
{
  if (b->b_alloc) freebytes(b->b_buf, (b->b_alloc)*sizeof(unsigned char));
  b->b_buf   = NULL;
  b->b_len   = 0;
  b->b_alloc = 0;
}
PDSTRING_STATIC void pdstring_bytes_realloc(t_pdstring_bytes *b, size_t n)
{
  pdstring_bytes_clear(b);
  b->b_buf   = n ? (unsigned char*)getbytes(n*sizeof(unsigned char)) : NULL;
  b->b_alloc = n;
}
PDSTRING_STATIC void pdstring_bytes_init(t_pdstring_bytes *b, size_t n)
{
  pdstring_bytes_clear(b);
  pdstring_bytes_realloc(b,n);
}

/*---------------------------------------------------------------------
 * wchars
 */
PDSTRING_STATIC void pdstring_wchars_clear(t_pdstring_wchars *w)
{
  if (w->w_alloc) freebytes(w->w_buf, (w->w_alloc)*sizeof(wchar_t));
  w->w_buf   = NULL;
  w->w_len   = 0;
  w->w_alloc = 0;
}
PDSTRING_STATIC void pdstring_wchars_realloc(t_pdstring_wchars *w, size_t n)
{
  pdstring_wchars_clear(w);
  w->w_buf   = n ? (wchar_t*)getbytes(n*sizeof(wchar_t)) : NULL;
  w->w_alloc = n;
}
PDSTRING_STATIC void pdstring_wchars_init(t_pdstring_wchars *w, size_t n)
{
  pdstring_wchars_clear(w);
  pdstring_wchars_realloc(w,n);
}

/*---------------------------------------------------------------------
 * atoms
 */
PDSTRING_STATIC void pdstring_atoms_clear(t_pdstring_atoms *a)
{
  if (a->a_alloc) freebytes(a->a_buf, (a->a_alloc)*sizeof(t_atom));
  a->a_buf   = NULL;
  a->a_len   = 0;
  a->a_alloc = 0;
}
PDSTRING_STATIC void pdstring_atoms_realloc(t_pdstring_atoms *a, size_t n)
{
  pdstring_atoms_clear(a);
  a->a_buf   = n ? (t_atom*)getbytes(n*sizeof(t_atom)) : NULL;
  a->a_alloc = n;
}
PDSTRING_STATIC void pdstring_atoms_init(t_pdstring_atoms *a, size_t n)
{
  pdstring_atoms_clear(a);
  pdstring_atoms_realloc(a,n);
}

/*---------------------------------------------------------------------
 * floatarray
 */
PDSTRING_STATIC void pdstring_floatarray_clearvec(t_pdstring_floatarray *fa)
{
  fa->fa_garray = NULL;
  fa->fa_vlen = 0;
  fa->fa_vec = NULL;
}
PDSTRING_STATIC void pdstring_floatarray_clear(t_pdstring_floatarray *fa)
{
  fa->fa_name = &s_;
  fa->fa_offset = 0;
  pdstring_floatarray_clearvec(fa);
}
PDSTRING_STATIC void pdstring_floatarray_realloc(t_pdstring_floatarray *fa, size_t n)
{
  if (fa->fa_name == &s_) return;
  pdstring_floatarray_getvec(NULL,fa);
  if (fa->fa_offset < 0) fa->fa_offset += fa->fa_vlen;
  if (fa->fa_vlen < fa->fa_offset + (int)n) {
    garray_resize(fa->fa_garray, fa->fa_offset+(int)n);
    fa->fa_vec = NULL;
    pdstring_floatarray_getvec(NULL,fa);
  }
}
PDSTRING_STATIC void pdstring_floatarray_init(t_pdstring_floatarray *fa, size_t n)
{
  pdstring_floatarray_clearvec(fa);
  pdstring_floatarray_realloc(fa,n);
}
PDSTRING_STATIC void pdstring_floatarray_getvec(t_object *x, t_pdstring_floatarray *fa)
{
  //-- get array
  if (!(fa->fa_garray = (t_garray *)pd_findbyclass(fa->fa_name, garray_class))) {
    pd_error(x, "pdstring_floatarray_getvec(): no such array '%s'", fa->fa_name->s_name);
    pdstring_floatarray_clearvec(fa);
    return;
  }
  //-- get float vector & size
  if (!garray_getfloatarray(fa->fa_garray, &fa->fa_vlen, &fa->fa_vec)) {
    pd_error(x, "pdstring_floatarray_getvec(): garray_getfloatarray() failed for array '%s'", fa->fa_name->s_name);
    return;
  }
}


/*=====================================================================
 * Utilities
 *=====================================================================*/

/*--------------------------------------------------------------------
 * pdstring_any2bytes()
 *  + x is used for error reporting
 *  + uses x_binbuf for conversion
 *  + selector sel is added to binbuf too, if it is none of  {NULL, &s_float, &s_list, &s_}
 *  + x_binbuf may be NULL, in which case a temporary t_binbuf is created and used
 *    - in this case, output bytes are copied into *dst, reallocating if required
 *  + if x_binbuf is given and non-NULL, dst may be NULL.
 *    - if dst is non-NULL, its values will be clobbered by those returned by
 *      binbuf_gettext()
 */
PDSTRING_STATIC void pdstring_any2bytes(void *x, t_pdstring_bytes *dst, t_symbol *sel, t_pdstring_atoms *src, t_binbuf *x_binbuf)
{
  int bb_is_tmp=0;

  //-- create temporary binbuf?
  if (!x_binbuf) {
    x_binbuf = binbuf_new();
    bb_is_tmp = 1;
  }

  //-- prepare binbuf
  binbuf_clear(x_binbuf);

  //-- binbuf_add(): selector
  if (sel && sel != &s_float && sel != &s_list && sel != &s_) {
    t_atom a;
    SETSYMBOL((&a), sel);
    binbuf_add(x_binbuf, 1, &a);
  }

  //-- binbuf_add(): src atoms
  binbuf_add(x_binbuf, src->a_len, src->a_buf);

  //-- output: get text string
  if (bb_is_tmp) {
    //-- temporary binbuf: copy text
    char *text;
    int   len;
    binbuf_gettext(x_binbuf, &text, &len);

    //-- reallocate?
    if ( dst->b_alloc < (size_t)len )
      pdstring_bytes_realloc(dst, len + PDSTRING_BYTES_GET);

    //-- copy
    memcpy(dst->b_buf, text, len*sizeof(char));
    dst->b_len = len;

    //-- cleanup
    binbuf_free(x_binbuf);
    if (text) freebytes(text,len);
  }
  else if (dst) {
    //-- permanent binbuf: clobber dst
    pdstring_bytes_clear(dst);
    binbuf_gettext(x_binbuf, ((char**)((void*)(&dst->b_buf))), &dst->b_len);
    dst->b_alloc = dst->b_len;
  }
}


/*--------------------------------------------------------------------
 * pdstring_bytes2any()
 *  + uses x_binbuf for conversion
 *  + x_binbuf may be NULL, in which case a temporary t_binbuf is created and used
 *    - in this case, output atoms are copied into *dst, reallocating if required
 *  + if x_binbuf is given and non-NULL, dst may be NULL.
 *    - if dst is non-NULL, its values will be clobbered by those returned by
 *      binbuf_getnatom() and binbuf_getvec()
 */
PDSTRING_STATIC void pdstring_bytes2any(void *x, t_pdstring_atoms *dst, t_pdstring_bytes *src, t_binbuf *x_binbuf)
{
  int bb_is_tmp=0;

  //-- create temporary binbuf?
  if (!x_binbuf) {
    x_binbuf = binbuf_new();
    bb_is_tmp = 1;
  }

  //-- populate binbuf
  binbuf_clear(x_binbuf);
  binbuf_text(x_binbuf, (char*)src->b_buf, src->b_len);
  //PDSDEBUG(post("bytes2any[dst=%p,src=%p,bb=%p]: binbuf_print: ", dst,src,x_binbuf));
  //PDSDEBUG(binbuf_print(x_binbuf));

  //-- populate atom list
  if (bb_is_tmp) {
    //-- temporary binbuf: copy atoms
    t_atom *argv = binbuf_getvec(x_binbuf);
    int     argc = binbuf_getnatom(x_binbuf);

    //-- reallocate?
    if ( dst->a_alloc < (size_t)argc )
      pdstring_atoms_realloc(dst, argc + PDSTRING_ATOMS_GET);

    //-- copy
    memcpy(dst->a_buf, argv, argc*sizeof(t_atom));
    dst->a_len = argc;

    //-- cleanup
    binbuf_free(x_binbuf);
  }
  else if (dst) {
    //-- permanent binbuf: clobber dst
    dst->a_buf = binbuf_getvec(x_binbuf);
    dst->a_len = binbuf_getnatom(x_binbuf);
    dst->a_alloc = 0;  //-- don't try to free this
  }
}


/*--------------------------------------------------------------------
 * pdstring_atoms2bytes()
 *  + always appends a final NUL byte to *dst_buf, even if src_argv doesn't contain one
 *  + returns number of bytes actually written to *dst_buf, __including__ implicit trailing NUL
 */
PDSTRING_STATIC int pdstring_atoms2bytes(void *x, t_pdstring_bytes *dst, t_pdstring_atoms *src, t_float x_eos)
{
  t_atom *argv = src->a_buf;
  int     argc = src->a_len;
  unsigned char *s;
  int     new_len=0;

  /*-- re-allocate? --*/
  if (dst->b_alloc <= (size_t)(argc+1))
    pdstring_bytes_realloc(dst, argc + 1 + PDSTRING_BYTES_GET);

  /*-- get byte string --*/
  for (s=dst->b_buf, new_len=0; argc > 0; argc--, argv++, s++, new_len++)
    {
      *s = atom_getfloat(argv);
      if ((x_eos<0 && !*s) || (*s==x_eos)) { break; } /*-- hack: truncate at first EOS char --*/
    }
  *s = '\0'; /*-- always append terminating NUL */
  dst->b_len = new_len;

  return new_len+1;
}

/*--------------------------------------------------------------------
 * pdstring_atoms2wchars()
 *  + always appends a final NUL wchar_t to dst->w_buf, even if src->a_buf doesn't contain one
 *  + returns number of bytes actually written to dst->w_buf, __including__ implicit trailing NUL
 *  + but dst->w_len does NOT include implicit trailing NUL
 */
PDSTRING_STATIC int pdstring_atoms2wchars(void *x, t_pdstring_wchars *dst, t_pdstring_atoms *src, t_float x_eos)
{
  t_atom *argv = src->a_buf;
  int     argc = src->a_len;
  int     new_len=0;
  wchar_t *s;

  /*-- re-allocate? --*/
  if (dst->w_alloc <= (size_t)(argc+1))
    pdstring_wchars_realloc(dst, argc + 1 + PDSTRING_WCHARS_GET);

  /*-- get wchar_t string --*/
  for (s=dst->w_buf, new_len=0; argc > 0; argc--, argv++, s++, new_len++)
    {
      *s = atom_getfloat(argv);
      if ((x_eos<0 && !*s) || (*s==x_eos)) { break; } /*-- hack: truncate at first EOS char --*/
    }
  *s = L'\0'; /*-- always append terminating NUL */
  dst->w_len = new_len;

  return new_len+1;
}


/*--------------------------------------------------------------------
 * pdstring_bytes2wchars()
 */
PDSTRING_STATIC int pdstring_bytes2wchars(void *x, t_pdstring_wchars *dst, t_pdstring_bytes *src)
{
  size_t bi, wi;

  //-- re-allocate?
  if ( dst->w_alloc < (size_t)src->b_len )
    pdstring_wchars_realloc(dst, src->b_len + PDSTRING_WCHARS_GET);

  //-- convert
  //PDSDEBUG(post("\nbytes2wchars[dst=%p,src=%p]: init", dst,src);)
  mbtowc(NULL,NULL,0); //-- re-initialize conversion state for mbtowc()
  for (bi=0,wi=0; bi<(size_t)src->b_len; wi++) {
    int nbytes = mbtowc(dst->w_buf+wi, (char*)(src->b_buf+bi), src->b_len-bi);
    if (nbytes <= 0) {
      if (nbytes < 0) {
	pd_error(x,"pdstring_bytes2wchars(): malformed byte string \"%s\" at char '%c' - copying literal byte", src->b_buf, src->b_buf[bi]);
      }
      dst->w_buf[wi] = src->b_buf[bi];
      nbytes = 1;
    }
    bi += nbytes;
    //PDSDEBUG(post("bytes2wchars[dst=%p,src=%p]: loop[bi=%d,wi=%d,src=%s]: nbytes=%d,wc=%u", dst,src, bi,wi,src, nbytes,dst->w_buf[wi]));
  }
  dst->w_len = wi;
  return wi;
}

/*--------------------------------------------------------------------
 * pdstring_wchars2bytes()
 */
PDSTRING_STATIC int pdstring_wchars2bytes(void *x, t_pdstring_bytes *dst, t_pdstring_wchars *src)
{
  size_t bi, wi;

  //-- re-allocate?
  if ( dst->b_alloc < src->w_len * MB_CUR_MAX )
    pdstring_bytes_realloc(dst, src->w_len * MB_CUR_MAX + PDSTRING_WCHARS_GET);

  //-- convert
  for (bi=0,wi=0; wi < (size_t)src->w_len; wi++) {
    int nbytes = wctomb((char*)dst->b_buf+bi, src->w_buf[wi]);
    if (nbytes <= 0) {
      if (nbytes < 0) {
	pd_error(x,"pdstring_wchars2bytes(): malformed wide character (%u) - bashing to byte", src->w_buf[wi]);
      }
      dst->b_buf[bi] = src->w_buf[wi];
      nbytes = 1;
    }
    bi += nbytes;
  }
  dst->b_len = bi;
  return bi;
}


/*--------------------------------------------------------------------
 * pdstring_bytes2atoms()
 *  + implicitly appends x_eos if >= 0 and != PDSTRING_EOS_NONE
 */
PDSTRING_STATIC void pdstring_bytes2atoms(void *x, t_pdstring_atoms *dst, t_pdstring_bytes *src, t_float x_eos)
{
  int i;

  //-- re-allocate?
  if ( dst->a_alloc <= (size_t)src->b_len )
    pdstring_atoms_realloc(dst, src->b_len + 1 + PDSTRING_ATOMS_GET);

  //-- convert
  for (i=0; i < src->b_len; i++) {
    SETFLOAT((dst->a_buf+i), src->b_buf[i]);
  }
  dst->a_len = src->b_len;

  //-- append eos atom?
  if (x_eos >= 0 && x_eos != PDSTRING_EOS_NONE) {
    SETFLOAT(dst->a_buf+dst->a_len, x_eos);
    dst->a_len++;
  }
}

/*--------------------------------------------------------------------
 * pdstring_wchars2atoms()
 */
PDSTRING_STATIC void pdstring_wchars2atoms(void *x, t_pdstring_atoms *dst, t_pdstring_wchars *src)
{
  int i;

  //-- re-allocate?
  if ( dst->a_alloc < (size_t)src->w_len )
    pdstring_atoms_realloc(dst, src->w_len + PDSTRING_ATOMS_GET);

  //-- convert
  for (i=0; i < src->w_len; i++) {
    SETFLOAT((dst->a_buf+i), src->w_buf[i]);
  }
  dst->a_len = src->w_len;
}


#endif /* PDSTRING_UTILS_C */
