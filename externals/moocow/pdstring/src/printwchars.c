/* -*- Mode: C -*- */
/*=============================================================================*\
 * File: printwchars.c
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
#include <wchar.h>
#include <m_pd.h>

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "mooPdUtils.h"
#include "pdstringUtils.h"

/*=====================================================================
 * Constants & Globals
 *=====================================================================*/
static char *printwchars_banner = "printwchars: pdstring version " PACKAGE_VERSION " by Bryan Jurish";

/*=====================================================================
 * Structures and Types: any2string
 *=====================================================================*/

static t_class *printwchars_class;

typedef struct _printwchars
{
  t_object           x_obj;
  t_symbol          *x_prefix;
} t_printwchars;


/*=====================================================================
 * Methods
 *=====================================================================*/

/*--------------------------------------------------------------------
 * anything
 */
static void printwchars_anything(t_printwchars *x, MOO_UNUSED t_symbol *sel, int argc, t_atom *argv)
{
  startpost("%s/%s: ", x->x_prefix->s_name, sel->s_name);
  for (; argc > 0; argc--, argv++) {
    wchar_t c = atom_getfloat(argv);
    startpost("%C", c);
  }
  endpost();
}

#if 0
static void printwchars_anything_v1(t_printwchars *x, MOO_UNUSED t_symbol *sel, int argc, t_atom *argv)
{
  t_pdstring_atoms arg_atoms = { argv, argc, 0 };
  pdstring_atoms2wchars(x, &x->x_wchars, &arg_atoms, PDSTRING_EOS_NONE);
  post("%S/%S: %S", x->x_prefix->s_name, sel->s_name, x->x_wchars.w_buf);
}
#endif

/*--------------------------------------------------------------------
 * new
 */
static void *printwchars_new(t_symbol *prefix)
{
    t_printwchars *x = (t_printwchars *)pd_new(printwchars_class);

    if (prefix == &s_) prefix = gensym("printwchars");
    x->x_prefix = prefix;

    //pdstring_wchars_init(&x->x_wchars, PDSTRING_DEFAULT_BUFLEN);
    return (void *)x;
}

/*--------------------------------------------------------------------
 * free
 */
static void printwchars_free(t_printwchars *x)
{
  //pdstring_wchars_clear(&x->x_wchars);
  return;
}

/*--------------------------------------------------------------------
 * setup: guts
 */
void printwchars_setup_guts(void)
{
  //-- check/set "initialized" flag
  //if (PRINTWCHARS_INITIALIZED) return;
  //PRINTWCHARS_INITIALIZED = 1;

  //-- class
  printwchars_class = class_new(gensym("printwchars"),
			       (t_newmethod)printwchars_new,
			       (t_method)printwchars_free,
			       sizeof(t_printwchars),
			       CLASS_DEFAULT,
			       A_DEFSYM,                   //-- print-prefix
			       0);

  //-- alias
  //class_addcreator((t_newmethod)printwchars_new, gensym("printstring"), A_GIMME, 0);
  
  //-- methods
  class_addanything(printwchars_class, (t_method)printwchars_anything);

  //-- help symbol
  //class_sethelpsymbol(printwchars_class, gensym("printwchars-help.pd")); //-- breaks pd-extended help lookup
}

/*--------------------------------------------------------------------
 * setup
 */
void printwchars_setup(void)
{
  post(printwchars_banner);
  printwchars_setup_guts();
}
