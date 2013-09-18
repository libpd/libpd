/*=============================================================================*\
 * File: pd_gfsm.c
 * Author: Bryan Jurish <moocow@ling.uni-potsdam.de>
 * Description: finite state automata for Pd
 *
 * Copyright (c) 2004-2009 Bryan Jurish.
 *
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * See file LICENSE for further informations on licensing terms.
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

#include <pd_gfsm.h>
#include <pd_alphabet.h>

#define USE_AUTOMATA

#ifdef USE_AUTOMATA
# include <pd_automaton.h>
# include <pd_state.h>
#endif

#ifdef HAVE_CONFIG_H
# include <noconfig.h>
# include <config.h>
#endif

/*=====================================================================
 * Structures and Types
 *=====================================================================*/
static t_class *pd_gfsm_dummy_class;

/*=====================================================================
 * pd_gfsm_dummy
 *=====================================================================*/

/*--------------------------------------------------------------------
 * pd_gfsm_dummy: new()
 */
static void *pd_gfsm_dummy_new(void)
{
  t_pd_gfsm_dummy *x = (t_pd_gfsm_dummy *)pd_new(pd_gfsm_dummy_class);
  return (void *)x;
}

/*--------------------------------------------------------------------
 * pd_gfsm_dummy: free()
 */
static void pd_gfsm_dummy_free(GFSM_UNUSED t_pd_gfsm_dummy *x)
{}

/*--------------------------------------------------------------------
 * pd_gfsm_dummy: setup()
 */
void gfsm_setup(void)
{
  //-- banner
  post("");
  post("gfsm: finite state machine externals v%s by Bryan Jurish", PACKAGE_VERSION);
  post("gfsm: using " PD_GFSM_WHICH " libgfsm v%s", gfsm_version_string);
  post("gfsm: compiled by " PACKAGE_BUILD_USER " on " PACKAGE_BUILD_DATE);

  //-- library
  pd_gfsm_alphabet_setup();
#ifdef USE_AUTOMATA
  pd_gfsm_automaton_setup();
  pd_gfsm_state_setup();
#endif

  //-- class (dummy)
  pd_gfsm_dummy_class = class_new(gensym("gfsm"),
				  (t_newmethod)pd_gfsm_dummy_new,
				  (t_method)pd_gfsm_dummy_free,
				  sizeof(t_pd_gfsm_dummy),
				  CLASS_DEFAULT,
				  A_NULL);
  
  //-- help symbol
  class_sethelpsymbol(pd_gfsm_dummy_class, gensym("gfsm-help.pd"));
}
