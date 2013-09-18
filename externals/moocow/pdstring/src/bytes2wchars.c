/* -*- Mode: C -*- */
/*=============================================================================*\
 * File: bytes2wchars.c
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
static char *bytes2wchars_banner = "bytes2wchars: pdstring version " PACKAGE_VERSION " by Bryan Jurish";

#define BYTES2WCHARS_DEFAULT_BUFLEN PDSTRING_DEFAULT_BUFLEN

/*=====================================================================
 * Structures and Types: any2string
 *=====================================================================*/

static t_class *bytes2wchars_class;

typedef struct _bytes2wchars
{
  t_object          x_obj;
  t_pdstring_bytes  x_bytes;  //-- byte buffer
  t_pdstring_wchars x_wchars; //-- wide character buffer
  t_pdstring_atoms  x_atoms;  //-- atoms to output
  t_outlet         *x_outlet;
} t_bytes2wchars;


/*=====================================================================
 * Utilities
 *=====================================================================*/

/*--------------------------------------------------------------------
 * bytes2wchars_atoms()
 */
static void bytes2wchars_atoms(t_bytes2wchars *x, int argc, t_atom *argv)
{
  t_pdstring_atoms src = {argv,argc,0};

  /*-- convert atoms -> bytes -> wchars -> atoms --*/
  pdstring_atoms2bytes (x, &x->x_bytes, &src, PDSTRING_EOS_NONE);
  pdstring_bytes2wchars(x, &x->x_wchars, &x->x_bytes);
  pdstring_wchars2atoms(x, &x->x_atoms, &x->x_wchars);
}


/*=====================================================================
 * Methods
 *=====================================================================*/

/*--------------------------------------------------------------------
 * anything
 */
static void bytes2wchars_anything(t_bytes2wchars *x, MOO_UNUSED t_symbol *sel, int argc, t_atom *argv)
{
  bytes2wchars_atoms(x, argc, argv);

  /*-- output --*/
  outlet_anything(x->x_outlet, &s_list, x->x_atoms.a_len, x->x_atoms.a_buf);
}


/*--------------------------------------------------------------------
 * new
 */
static void *bytes2wchars_new(MOO_UNUSED t_symbol *sel, int argc, t_atom *argv)
{
    t_bytes2wchars *x = (t_bytes2wchars *)pd_new(bytes2wchars_class);
    int bufsize = PDSTRING_DEFAULT_BUFLEN;

    //-- args: 0: bufsize
    if (argc > 0) {
      int initial_bufsize = atom_getintarg(0,argc,argv);
      if (initial_bufsize > 0) bufsize = initial_bufsize;
    } 

    //-- allocate
    pdstring_bytes_init(&x->x_bytes, bufsize);
    pdstring_wchars_init(&x->x_wchars, bufsize);
    pdstring_atoms_init(&x->x_atoms, bufsize);

    //-- outlets
    x->x_outlet      = outlet_new(&x->x_obj, &s_list);

    return (void *)x;
}

/*--------------------------------------------------------------------
 * free
 */
static void bytes2wchars_free(t_bytes2wchars *x)
{
  pdstring_bytes_clear(&x->x_bytes);
  pdstring_wchars_clear(&x->x_wchars);
  pdstring_atoms_clear(&x->x_atoms);
  outlet_free(x->x_outlet);
  return;
}

/*--------------------------------------------------------------------
 * setup: guts
 */
void bytes2wchars_setup_guts(void)
{
  //-- class
  bytes2wchars_class = class_new(gensym("bytes2wchars"),
				 (t_newmethod)bytes2wchars_new,
				 (t_method)bytes2wchars_free,
				 sizeof(t_bytes2wchars),
				 CLASS_DEFAULT,
				 A_GIMME,                   //-- initial_bufsize
				 0);

  //-- methods
  class_addanything(bytes2wchars_class, (t_method)bytes2wchars_anything);
  
  //-- help symbol
  //class_sethelpsymbol(bytes2wchars_class, gensym("bytes2wchars-help.pd")); //-- breaks pd-extended help lookup
}

/*--------------------------------------------------------------------
 * setup
 */
void bytes2wchars_setup(void)
{
  post(bytes2wchars_banner);
  bytes2wchars_setup_guts();
}
