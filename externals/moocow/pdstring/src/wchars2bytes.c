/* -*- Mode: C -*- */
/*=============================================================================*\
 * File: wchars2bytes.c
 * Author: Bryan Jurish <moocow@ling.uni-potsdam.de>
 * Description: convert wchar_t-valued atom lists to byte-valued atom lists
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
static char *wchars2bytes_banner = "wchars2bytes: pdstring version " PACKAGE_VERSION " by Bryan Jurish";

#define WCHARS2BYTES_DEFAULT_BUFLEN PDSTRING_DEFAULT_BUFLEN

/*=====================================================================
 * Structures and Types: any2string
 *=====================================================================*/

static t_class *wchars2bytes_class;

typedef struct _wchars2bytes
{
  t_object          x_obj;
  t_pdstring_wchars x_wchars; //-- wide character buffer
  t_pdstring_bytes  x_bytes;  //-- byte buffer
  t_pdstring_atoms  x_atoms;  //-- atoms to output
  t_outlet         *x_outlet;
} t_wchars2bytes;


/*=====================================================================
 * Methods
 *=====================================================================*/

/*--------------------------------------------------------------------
 * anything
 */
static void wchars2bytes_anything(t_wchars2bytes *x, MOO_UNUSED t_symbol *sel, int argc, t_atom *argv)
{
  //-- convert arg_atoms -> wchars -> bytes -> atoms 
  t_pdstring_atoms arg_atoms = {argv,argc,0};
  pdstring_atoms2wchars(x, &x->x_wchars, &arg_atoms, PDSTRING_EOS_NONE);
  pdstring_wchars2bytes(x, &x->x_bytes, &x->x_wchars);
  pdstring_bytes2atoms (x, &x->x_atoms, &x->x_bytes, PDSTRING_EOS_NONE);

  //-- output
  outlet_anything(x->x_outlet, &s_list, x->x_atoms.a_len, x->x_atoms.a_buf);
}


/*--------------------------------------------------------------------
 * new
 */
static void *wchars2bytes_new(MOO_UNUSED t_symbol *sel, int argc, t_atom *argv)
{
    t_wchars2bytes *x = (t_wchars2bytes *)pd_new(wchars2bytes_class);
    int bufsize = PDSTRING_DEFAULT_BUFLEN;

    //-- args: 0: bufsize
    if (argc > 0) {
      int initial_bufsize = atom_getintarg(0,argc,argv);
      if (initial_bufsize > 0) bufsize = initial_bufsize;
    } 

    //-- allocate
    pdstring_wchars_init(&x->x_wchars, bufsize);
    pdstring_bytes_init (&x->x_bytes,  bufsize);
    pdstring_atoms_init (&x->x_atoms,  bufsize);

    //-- outlets
    x->x_outlet = outlet_new(&x->x_obj, &s_list);

    return (void *)x;
}

/*--------------------------------------------------------------------
 * free
 */
static void wchars2bytes_free(t_wchars2bytes *x)
{
  pdstring_wchars_clear(&x->x_wchars); 
  pdstring_bytes_clear(&x->x_bytes);
  pdstring_atoms_clear(&x->x_atoms);
  outlet_free(x->x_outlet);
  return;
}

/*--------------------------------------------------------------------
 * setup: guts
 */
void wchars2bytes_setup_guts(void)
{
  //-- class
  wchars2bytes_class = class_new(gensym("wchars2bytes"),
				 (t_newmethod)wchars2bytes_new,
				 (t_method)wchars2bytes_free,
				 sizeof(t_wchars2bytes),
				 CLASS_DEFAULT,
				 A_GIMME,                   //-- initial_bufsize
				 0);

  //-- methods
  class_addanything(wchars2bytes_class, (t_method)wchars2bytes_anything);
  
  //-- help symbol
  //class_sethelpsymbol(wchars2bytes_class, gensym("wchars2bytes-help.pd")); //-- breaks pd-extended help lookup
}

/*--------------------------------------------------------------------
 * setup
 */
void wchars2bytes_setup(void)
{
  post(wchars2bytes_banner);
  wchars2bytes_setup_guts();
}
