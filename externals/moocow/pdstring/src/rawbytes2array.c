/* -*- Mode: C -*- */
/*=============================================================================*\
 * File: rawbytes2array.c
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
static char *rawbytes2array_banner = "rawbytes2array: pdstring version " PACKAGE_VERSION " by Bryan Jurish";

#define RAWBYTES2ARRAY_DEFAULT_BUFLEN PDSTRING_DEFAULT_BUFLEN

/*=====================================================================
 * Structures and Types: any2string
 *=====================================================================*/

static t_class *rawbytes2array_class;

typedef struct _rawbytes2array
{
  t_object          x_obj;
  t_symbol         *x_name;   //-- name of output array
  //t_pdstring_bytes  x_bytes;  //-- byte buffer
  //t_outlet         *x_outlet;
} t_rawbytes2array;


/*=====================================================================
 * Methods
 *=====================================================================*/

/*--------------------------------------------------------------------
 * anything
 */
static void rawbytes2array_anything(t_rawbytes2array *x, MOO_UNUSED t_symbol *sel, int argc, t_atom *argv)
{
  int i0,fvecsize;
  t_garray *a;
  t_float *fvec;
  unsigned char *cvec;

  //-- sanity check(s)
  if (!(a = (t_garray *)pd_findbyclass(x->x_name, garray_class))) {
    pd_error(x, "rawbytes2array: no such array '%s'", x->x_name->s_name);
    return;
  }

  //-- prepare
  i0 = atom_getfloat(argv);
  argc--;
  argv++;

 //-- resize?
  //garray_resize(a, argc);

  //-- get pointer
  if (!garray_getfloatarray(a, &fvecsize, &fvec))
    pd_error(x,"rawbytes2array: bad template for write to array '%s'", x->x_name->s_name);

  //-- write raw bytes
  for (cvec=(unsigned char*)(fvec+i0); argc > 0 && cvec < (unsigned char*)(fvec+fvecsize); cvec++,argv++,argc--)
    {
      *cvec = atom_getfloat(argv);
    }
}


/*--------------------------------------------------------------------
 * new
 */
static void *rawbytes2array_new(t_symbol *arrayname)
{
    t_rawbytes2array *x = (t_rawbytes2array *)pd_new(rawbytes2array_class);

    //-- init
    x->x_name = arrayname;

    return (void *)x;
}

/*--------------------------------------------------------------------
 * set ARRAYNAME
 */
static void rawbytes2array_set(t_rawbytes2array *x, t_symbol *arrayname)
{
  x->x_name = arrayname;
}

/*--------------------------------------------------------------------
 * free
 */
static void rawbytes2array_free(t_rawbytes2array *x)
{
  return;
}

/*--------------------------------------------------------------------
 * setup: guts
 */
void rawbytes2array_setup_guts(void)
{
  //-- class
  rawbytes2array_class = class_new(gensym("rawbytes2array"),
				 (t_newmethod)rawbytes2array_new,
				 (t_method)rawbytes2array_free,
				 sizeof(t_rawbytes2array),
				 CLASS_DEFAULT,
				 A_DEFSYM,                   //-- arrayname
				 0);

  //-- methods
  class_addanything(rawbytes2array_class, (t_method)rawbytes2array_anything);
  class_addmethod(rawbytes2array_class, (t_method)rawbytes2array_set, gensym("set"), A_DEFSYM, 0);
  
  //-- help symbol
  //class_sethelpsymbol(rawbytes2array_class, gensym("rawbytes2array-help.pd")); //-- breaks pd-extended help lookup
}

/*--------------------------------------------------------------------
 * setup
 */
void rawbytes2array_setup(void)
{
  post(rawbytes2array_banner);
  rawbytes2array_setup_guts();
}
