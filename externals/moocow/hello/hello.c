/* -*- Mode: C -*- */
/*=============================================================================*\
 * File: hello.c
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
static t_class *hello_class;

typedef struct _hello
{
  t_object       x_obj;
} t_hello;


/*=====================================================================
 * Multilib (see also _setup() method)
 *=====================================================================*/
#ifndef WANT_OBJECT_EXTERNALS
# include "goodbye.c"
#endif

/*--------------------------------------------------------------------
 * new
 */
static void *hello_new(void)
{
  t_hello *x = (t_hello *)pd_new(hello_class);
  return x;
}

/*--------------------------------------------------------------------
 * bang
 */
static void hello_bang(PDEXT_UNUSED t_hello *x)
{
  post("hello, Pd!");
}

/*=====================================================================
 * Setup
 *=====================================================================*/
void hello_setup(void)
{
  post("");
  post("hello: example external version " PACKAGE_VERSION " by Bryan Jurish");
  post("hello: compiled by " PACKAGE_BUILD_USER " on " PACKAGE_BUILD_DATE);

  hello_class = class_new(gensym("hello"),
			     (t_newmethod)hello_new,
			     0,
			     sizeof(t_hello),
			     CLASS_DEFAULT,
			     0);

  //-- help method
  class_addmethod(hello_class, (t_method)hello_bang, gensym("bang"), A_NULL);

#ifndef WANT_OBJECT_EXTERNALS
  goodbye_setup();
#endif
}
