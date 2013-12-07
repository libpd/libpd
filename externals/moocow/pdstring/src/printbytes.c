/* -*- Mode: C -*- */
/*=============================================================================*\
 * File: printbytes.c
 * Author: Bryan Jurish <moocow@ling.uni-potsdam.de>
 * Description: print byte-strings using post()
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
#include <m_pd.h>

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "mooPdUtils.h"
#include "pdstringUtils.h"


/*=====================================================================
 * Constants & Globals
 *=====================================================================*/
static char *printbytes_banner = "printbytes: pdstring version " PACKAGE_VERSION " by Bryan Jurish";

/*=====================================================================
 * Structures and Types: any2string
 *=====================================================================*/

static t_class *printbytes_class;

typedef struct _printbytes
{
  t_object           x_obj;
  t_symbol          *x_prefix;
  //t_pdstring_bytes   x_bytes;
} t_printbytes;


/*=====================================================================
 * Methods
 *=====================================================================*/

/*--------------------------------------------------------------------
 * anything
 */
static void printbytes_anything(t_printbytes *x, MOO_UNUSED t_symbol *sel, int argc, t_atom *argv)
{
  startpost("%s/%s: ", x->x_prefix->s_name, sel->s_name);
  for (; argc > 0; argc--, argv++) {
    char c = atom_getfloat(argv);
    startpost("%c", c);
  }
  endpost();
}

#if 0
static void printbytes_anything_v1(t_printbytes *x, MOO_UNUSED t_symbol *sel, int argc, t_atom *argv)
{
  t_pdstring_atoms arg_atoms = { argv, argc, 0 };
  pdstring_atoms2bytes(x, &x->x_bytes, &arg_atoms, PDSTRING_EOS_NONE);
  post("%s/%s: %s", x->x_prefix->s_name, sel->s_name, x->x_bytes.b_buf);
}
#endif


/*--------------------------------------------------------------------
 * new
 */
static void *printbytes_new(t_symbol *prefix)
{
    t_printbytes *x = (t_printbytes *)pd_new(printbytes_class);

    if (prefix == &s_) prefix = gensym("printbytes");
    x->x_prefix = prefix;

    //pdstring_bytes_init(&x->x_bytes, PDSTRING_DEFAULT_BUFLEN);
    return (void *)x;
}

/*--------------------------------------------------------------------
 * free
 */
static void printbytes_free(t_printbytes *x)
{
  //pdstring_bytes_clear(&x->x_bytes);
  return;
}

/*--------------------------------------------------------------------
 * setup: guts
 */
void printbytes_setup_guts(void)
{
  //-- check/set "initialized" flag
  //if (PRINTBYTES_INITIALIZED) return;
  //PRINTBYTES_INITIALIZED = 1;

  //-- class
  printbytes_class = class_new(gensym("printbytes"),
			       (t_newmethod)printbytes_new,
			       (t_method)printbytes_free,
			       sizeof(t_printbytes),
			       CLASS_DEFAULT,
			       A_DEFSYM,                     //-- print-prefix
			       0);

  //-- alias
  //class_addcreator((t_newmethod)printbytes_new, gensym("printstring"), A_GIMME, 0);
  
  //-- methods
  class_addanything(printbytes_class, (t_method)printbytes_anything);

  //-- help symbol
  //class_sethelpsymbol(printbytes_class, gensym("printbytes-help.pd")); //-- breaks pd-extended help lookup
}

/*--------------------------------------------------------------------
 * setup
 */
void printbytes_setup(void)
{
  post(printbytes_banner);
  printbytes_setup_guts();
}
