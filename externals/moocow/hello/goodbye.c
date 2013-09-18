/* -*- Mode: C -*- */
/*=============================================================================*\
 * File: goodbye.c
 * Author: Bryan Jurish <moocow@ling.uni-potsdam.de>
 * Description: dummy external for autotools example
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
#include <m_pd.h>
#include "common/mooPdUtils.h"

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

/*=====================================================================
 * Constants
 *=====================================================================*/

/*=====================================================================
 * Structures and Types
 *=====================================================================*/
static t_class *goodbye_class;

typedef struct _goodbye
{
  t_object       x_obj;
} t_goodbye;


/*--------------------------------------------------------------------
 * new
 */
static void *goodbye_new(void)
{
  t_goodbye *x = (t_goodbye *)pd_new(goodbye_class);
  return x;
}

/*--------------------------------------------------------------------
 * bang
 */
static void goodbye_bang(PDEXT_UNUSED t_goodbye *x)
{
  post("goodbye, Pd!");
}

/*=====================================================================
 * Setup
 *=====================================================================*/
void goodbye_setup(void)
{
  post("");
  post("goodbye: example external version " PACKAGE_VERSION " by Bryan Jurish");
  post("goodbye: compiled by " PACKAGE_BUILD_USER " on " PACKAGE_BUILD_DATE);

  goodbye_class = class_new(gensym("goodbye"),
			     (t_newmethod)goodbye_new,
			     0,
			     sizeof(t_goodbye),
			     CLASS_DEFAULT,
			     0);

  //-- help method
  class_addmethod(goodbye_class, (t_method)goodbye_bang, gensym("bang"), A_NULL);
}
