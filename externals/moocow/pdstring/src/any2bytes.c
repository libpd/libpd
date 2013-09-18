/* -*- Mode: C -*- */
/*=============================================================================*\
 * File: any2bytes.c
 * Author: Bryan Jurish <moocow@ling.uni-potsdam.de>
 * Description: convert pd messages to strings (dynamic allocation)
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
/*#define ANY2BYTES_DEBUG 1*/

#ifdef ANY2BYTES_DEBUG
# define A2SDEBUG(x) x
#else
# define A2SDEBUG(x)
#endif

#define ANY2BYTES_DEFAULT_BUFLEN PDSTRING_DEFAULT_BUFLEN


/*=====================================================================
 * Structures and Types: any2bytes
 *=====================================================================*/
static t_class *any2bytes_class;

typedef struct _any2bytes
{
  t_object       x_obj;
  t_pdstring_bytes x_bytes;  //-- byte buffer
  t_pdstring_atoms x_atoms;  //-- atom buffer (for output)
  t_float          x_eos;      //-- EOS character to add (<0 for none)
  t_binbuf        *x_binbuf;
  t_inlet         *x_eos_in;
  t_outlet        *x_outlet;
} t_any2bytes;


static int ANY2BYTES_INITIALIZED = 0;

/*=====================================================================
 * Constants
 *=====================================================================*/
static char *any2bytes_banner = "any2bytes: pdstring version " PACKAGE_VERSION " by Bryan Jurish";

/*=====================================================================
 * Methods
 *=====================================================================*/

/*--------------------------------------------------------------------
 * anything
 */
static void any2bytes_anything(t_any2bytes *x, t_symbol *sel, int argc, t_atom *argv)
{
  //-- convert any -> bytes -> atoms
  t_pdstring_atoms arg_atoms = {argv,argc,0};
  pdstring_bytes_clear(&x->x_bytes);
  pdstring_any2bytes(x, &x->x_bytes, sel, &arg_atoms, x->x_binbuf);
  pdstring_bytes2atoms(x, &x->x_atoms, &x->x_bytes, x->x_eos);

  //-- output
  outlet_list(x->x_outlet, &s_list, x->x_atoms.a_len, x->x_atoms.a_buf);
}

/*--------------------------------------------------------------------
 * new
 */
static void *any2bytes_new(MOO_UNUSED t_symbol *sel, int argc, t_atom *argv)
{
    t_any2bytes *x = (t_any2bytes *)pd_new(any2bytes_class);
    int bufsize = ANY2BYTES_DEFAULT_BUFLEN;

    //-- defaults
    x->x_eos      = 0;

    //-- args: 0: bufsize
    if (argc > 0) {
      int initial_bufsize = atom_getintarg(0, argc, argv);
      if (initial_bufsize > 0) { bufsize = initial_bufsize; }
    }
    //-- args: 1: eos
    if (argc > 1) {
      x->x_eos = atom_getfloatarg(1, argc, argv);
    }

    //-- allocate
    pdstring_bytes_init(&x->x_bytes, 0); //-- x_bytes gets clobbered by binbuf_gettext()
    pdstring_atoms_init(&x->x_atoms, bufsize);
    x->x_binbuf = binbuf_new();

    //-- inlets
    x->x_eos_in = floatinlet_new(&x->x_obj, &x->x_eos);

    //-- outlets
    x->x_outlet = outlet_new(&x->x_obj, &s_list);

    //-- report
    A2SDEBUG(post("any2bytes_new(): x=%p, eos=%d, binbuf=%p", x, x->x_eos, x->x_binbuf));

    return (void *)x;
}

/*--------------------------------------------------------------------
 * free
 */
static void any2bytes_free(t_any2bytes *x)
{
  pdstring_bytes_clear(&x->x_bytes);
  pdstring_atoms_clear(&x->x_atoms);
  binbuf_free(x->x_binbuf);
  inlet_free(x->x_eos_in);
  outlet_free(x->x_outlet);
  return;
}

/*--------------------------------------------------------------------
 * setup (guts)
 */
void any2bytes_setup_guts(void)
{
  if (ANY2BYTES_INITIALIZED) return;

  //-- class
  any2bytes_class = class_new(gensym("any2bytes"),
			      (t_newmethod)any2bytes_new,
			      (t_method)any2bytes_free,
			      sizeof(t_any2bytes),
			      CLASS_DEFAULT,
			      A_GIMME,                   //-- initial_bufsize, eos_char
			      0);

  //-- alias
#ifndef WANT_OBJECT_EXTERNALS
  class_addcreator((t_newmethod)any2bytes_new, gensym("any2string"), A_GIMME, 0);
#endif
  
  //-- methods
  class_addanything(any2bytes_class, (t_method)any2bytes_anything);
  
  //-- help symbol
  //class_sethelpsymbol(any2bytes_class, gensym("any2bytes-help.pd")); //-- breaks pd-extended help lookup

  //-- set flag
  ANY2BYTES_INITIALIZED = 1;
}


/*--------------------------------------------------------------------
 * setup
 */
void any2bytes_setup(void)
{
  post(any2bytes_banner);
  any2bytes_setup_guts();
}

#if 0
/*--
  this (with symlink install) causes pd-extended 0.41.4 20090209 to puke.
  bug report by hcs, Thu, 12 Feb 2009
*/
/*--------------------------------------------------------------------
 * setup (any2string alias)
 */
void any2string_setup(void) {
  post("any2string_setup(): WARNING: names are in flux!");
  post("any2string_setup(): Prefer [any2bytes] over [any2string].");
  any2bytes_setup();
}
#endif
