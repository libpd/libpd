/* -*- Mode: C -*- */
/*=============================================================================*\
 * File: array2rawbytes.c
 * Author: Bryan Jurish <moocow@ling.uni-potsdam.de>
 * Description: convert byte-valued atom lists to wchar_t-valued atom lists
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

#include <string.h>
#include <wchar.h>
#include <stdlib.h>
#include <m_pd.h>

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "mooPdUtils.h"
#include "pdstringUtils.h"

/* black magic for Microsoft's compiler */
#ifdef _MSC_VER
#pragma warning( disable : 4244 )
#pragma warning( disable : 4305 )
#endif

/*--------------------------------------------------------------------
 * DEBUG
 *--------------------------------------------------------------------*/

/*=====================================================================
 * Constants
 *=====================================================================*/
static char *array2rawbytes_banner = "array2rawbytes: pdstring version " PACKAGE_VERSION " by Bryan Jurish";

#define ARRAY2RAWBYTES_DEFAULT_BUFLEN PDSTRING_DEFAULT_BUFLEN

/*=====================================================================
 * Structures and Types: any2string
 *=====================================================================*/

static t_class *array2rawbytes_class;

typedef struct _array2rawbytes
{
  t_object          x_obj;
  t_symbol         *x_name;    //-- name of source array
  t_pdstring_bytes  x_bytes;   //-- byte "buffer", really just aliases array t_float*
  t_pdstring_atoms  x_atoms;   //-- output atom buffer
  t_outlet         *x_outlet;
} t_array2rawbytes;


/*=====================================================================
 * Methods
 *=====================================================================*/

/*--------------------------------------------------------------------
 * get OFFSET LENGTH
 */
static void array2rawbytes_get(t_array2rawbytes *x, t_float offset_f, t_float len_f)
{
  int offset=offset_f, len=len_f, fvecsize, cvecsize;
  t_garray *a;
  t_float *fvec;
  unsigned char *cvec;

  //-- sanity check(s)
  if (!(a = (t_garray *)pd_findbyclass(x->x_name, garray_class))) {
    pd_error(x, "array2rawbytes_get: no such array '%s'", x->x_name->s_name);
    return;
  }

  //-- get float* from array
  if (!garray_getfloatarray(a, &fvecsize, &fvec))
    pd_error(x,"array2rawbytes: bad template for write to array '%s'", x->x_name->s_name);

  //-- get character offset & len
  cvec     = (unsigned char*)fvec;
  cvecsize = fvecsize*sizeof(t_float)/sizeof(unsigned char);
  post("array2rawbytes[x=%p]: char data: cvecsize=%d", x, cvecsize);

  //-- tweak out-of-bounds values
  post("array2rawbytes[x=%p]: pre-tweak: offset=%d, len=%d", x, offset, len);

  offset %= cvecsize;
  if (len <= 0 || offset+len >= cvecsize)
    len = cvecsize - offset;

  post("array2rawbytes[x=%p]: post-tweak: offset=%d, len=%d", x, offset, len);

  //-- munge x_bytes
  x->x_bytes.b_buf = cvec+offset;
  x->x_bytes.b_len = len;

  //-- convert bytes -> atoms
  pdstring_bytes2atoms(x, &x->x_atoms, &x->x_bytes, PDSTRING_EOS_NONE);

  //-- output
  outlet_list(x->x_outlet, &s_list, x->x_atoms.a_len, x->x_atoms.a_buf);
}


/*--------------------------------------------------------------------
 * new
 */
static void *array2rawbytes_new(t_symbol *arrayname)
{
    t_array2rawbytes *x = (t_array2rawbytes *)pd_new(array2rawbytes_class);

    //-- init
    x->x_name = arrayname;
    pdstring_bytes_init(&x->x_bytes, 0); //-- gets clobbered by array data
    pdstring_atoms_init(&x->x_atoms, PDSTRING_DEFAULT_BUFLEN);

    //-- outlets
    x->x_outlet = outlet_new(&x->x_obj, &s_list);

    return (void *)x;
}

/*--------------------------------------------------------------------
 * set ARRAYNAME
 */
static void array2rawbytes_set(t_array2rawbytes *x, t_symbol *arrayname)
{
  x->x_name = arrayname;
}

/*--------------------------------------------------------------------
 * free
 */
static void array2rawbytes_free(t_array2rawbytes *x)
{
  pdstring_atoms_clear(&x->x_atoms);
  outlet_free(x->x_outlet);
  return;
}

/*--------------------------------------------------------------------
 * setup: guts
 */
void array2rawbytes_setup_guts(void)
{
  //-- class
  array2rawbytes_class = class_new(gensym("array2rawbytes"),
				 (t_newmethod)array2rawbytes_new,
				 (t_method)array2rawbytes_free,
				 sizeof(t_array2rawbytes),
				 CLASS_DEFAULT,
				 A_DEFSYM,                   //-- arrayname
				 0);

  //-- methods
  class_addmethod(array2rawbytes_class, (t_method)array2rawbytes_get, gensym("get"), A_DEFFLOAT, A_DEFFLOAT, 0);
  class_addmethod(array2rawbytes_class, (t_method)array2rawbytes_set, gensym("set"), A_DEFSYM, 0);
  
  //-- help symbol
  //class_sethelpsymbol(array2rawbytes_class, gensym("array2rawbytes-help.pd")); //-- breaks pd-extended help lookup
}

/*--------------------------------------------------------------------
 * setup
 */
void array2rawbytes_setup(void)
{
  post(array2rawbytes_banner);
  array2rawbytes_setup_guts();
}
