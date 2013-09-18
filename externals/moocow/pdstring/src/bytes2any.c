/* -*- Mode: C -*- */
/*=============================================================================*\
 * File: bytes2any.c
 * Author: Bryan Jurish <moocow@ling.uni-potsdam.de>
 * Description: convert strings to pd messages
 *
 * Copyright (c) 2004-2009 Bryan Jurish.
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
/*#define BYTES2ANY_DEBUG 1*/
/*#undef BYTES2ANY_DEBUG*/

#ifdef BYTES2ANY_DEBUG
# define S2ADEBUG(x) x
#else
# define S2ADEBUG(x)
#endif

#define BYTES2ANY_DEFAULT_BUFLEN PDSTRING_DEFAULT_BUFLEN

/*=====================================================================
 * Constants & Globals
 *=====================================================================*/
static char *bytes2any_banner = "bytes2any: pdstring version " PACKAGE_VERSION " by Bryan Jurish";

static int BYTES2ANY_INITIALIZED = 0;

/*=====================================================================
 * Structures and Types: any2string
 *=====================================================================*/

static t_class *bytes2any_class;

typedef struct _bytes2any
{
  t_object       x_obj;
  t_pdstring_bytes x_bytes; //-- byte buffer: {b_buf~x_text,b_len~?,b_alloc~x_size}
  t_float        x_eos;     //-- eos byte value
  t_binbuf      *x_binbuf;
  t_inlet       *x_eos_in;
  t_outlet      *x_outlet;
  t_outlet      *x_outlet_done;
} t_bytes2any;


/*=====================================================================
 * Utilities
 *=====================================================================*/

/*--------------------------------------------------------------------
 * bytes2any_atoms()
 */
static void bytes2any_atoms(t_bytes2any *x, int argc, t_atom *argv)
{
  t_pdstring_atoms src = {argv,argc,argc};
  pdstring_atoms2bytes(x, &(x->x_bytes), &src, x->x_eos);
  pdstring_bytes2any(x, NULL, &(x->x_bytes), x->x_binbuf);
  int x_argc;
  t_atom *x_argv;

  /*-- output --*/
  x_argc = binbuf_getnatom(x->x_binbuf);
  x_argv = binbuf_getvec(x->x_binbuf);
  if (x_argc && x_argv->a_type == A_SYMBOL) {
    outlet_anything(x->x_outlet,
		    x_argv->a_w.w_symbol,
		    x_argc-1,
		    x_argv+1);
  }
  else {
    outlet_anything(x->x_outlet,
		    &s_list,
		    x_argc,
		    x_argv);
  }
}


/*=====================================================================
 * Methods
 *=====================================================================*/

/*--------------------------------------------------------------------
 * anything
 */
static void bytes2any_anything(t_bytes2any *x, MOO_UNUSED t_symbol *sel, int argc, t_atom *argv)
{
  int i0=0, i;

  /*-- scan & output --*/
  if (x->x_eos >= 0) {
    for (i=i0; i < argc; i++) {
      if (((int)atom_getfloatarg(i,argc,argv))==((int)x->x_eos)) {
	bytes2any_atoms(x, i-i0, argv+i0);
	i0=i+1;
      }
    }
  }

  if (i0 < argc) {
    bytes2any_atoms(x, argc-i0, argv+i0);
  }

  outlet_bang(x->x_outlet_done);
}


/*--------------------------------------------------------------------
 * new
 */
static void *bytes2any_new(MOO_UNUSED t_symbol *sel, int argc, t_atom *argv)
{
    t_bytes2any *x = (t_bytes2any *)pd_new(bytes2any_class);
    int bufsize    = BYTES2ANY_DEFAULT_BUFLEN;

    //-- defaults
    x->x_binbuf = binbuf_new();
    x->x_eos    = -1;

    //-- args: 0: bufsize
    if (argc > 0) {
      int initial_bufsize = atom_getintarg(0,argc,argv);
      if (initial_bufsize > 0) bufsize = initial_bufsize;
      x->x_eos = -1;   //-- backwards-compatibility hack: no default eos character if only bufsize is specified
    } 
    //-- args: 1: separator
    if (argc > 1) {
      x->x_eos = atom_getfloatarg(1,argc,argv);
    }

    //-- allocate x_bytes
    pdstring_bytes_init(&x->x_bytes, bufsize);

    //-- inlets
    x->x_eos_in = floatinlet_new(&x->x_obj, &x->x_eos);

    //-- outlets
    x->x_outlet      = outlet_new(&x->x_obj, &s_list);
    x->x_outlet_done = outlet_new(&x->x_obj, &s_bang);

    //-- debug
    S2ADEBUG(post("bytes2any_new: x=%p, binbuf=%p, bytes.alloc=%d", x, x->x_eos, x->x_binbuf, x->x_bytes.b_alloc));

    return (void *)x;
}

/*--------------------------------------------------------------------
 * free
 */
static void bytes2any_free(t_bytes2any *x)
{
  pdstring_bytes_clear(&x->x_bytes);
  binbuf_free(x->x_binbuf);
  inlet_free(x->x_eos_in);
  outlet_free(x->x_outlet_done);
  outlet_free(x->x_outlet);
  return;
}

/*--------------------------------------------------------------------
 * setup: guts
 */
void bytes2any_setup_guts(void)
{
  if (BYTES2ANY_INITIALIZED) return;

  //-- class
  bytes2any_class = class_new(gensym("bytes2any"),
			       (t_newmethod)bytes2any_new,
			       (t_method)bytes2any_free,
			       sizeof(t_bytes2any),
			       CLASS_DEFAULT,
			       A_GIMME,                     //-- initial_bufsize, eos_char
			       0);

  //-- alias
#ifndef WANT_OBJECT_EXTERNALS
  class_addcreator((t_newmethod)bytes2any_new, gensym("string2any"), A_GIMME, 0);
#endif
  
  //-- methods
  class_addanything(bytes2any_class, (t_method)bytes2any_anything);

  //-- help symbol
  //class_sethelpsymbol(bytes2any_class, gensym("bytes2any-help.pd")); //-- breaks pd-extended help lookup

  //-- set flag
  BYTES2ANY_INITIALIZED = 1;
}

/*--------------------------------------------------------------------
 * setup
 */
void bytes2any_setup(void)
{
  post(bytes2any_banner);
  bytes2any_setup_guts();
}

#if 0
/*--
  this (with symlink install) causes pd-extended 0.41.4 20090209 to puke.
  bug report by hcs, Thu, 12 Feb 2009
*/
/*--------------------------------------------------------------------
 * setup (string2any alias)
 */
void string2any_setup(void) {
  post("string2any_setup(): WARNING: names are in flux!");
  post("string2any_setup(): Prefer [bytes2any] over [string2any].");
  bytes2any_setup();
}
#endif
