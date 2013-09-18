/*=============================================================================*\
 * File: pd_paths.c
 * Author: Bryan Jurish <moocow@ling.uni-potsdam.de>
 * Description: finite state automata for Pd: paths
 *
 * Copyright (c) 2006 Bryan Jurish.
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

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <pd_paths.h>

/*=====================================================================
 * Structures and Types
 *=====================================================================*/
static t_class *pd_gfsm_paths_class;


/*=====================================================================
 * pd_gfsm_paths: Methods
 *=====================================================================*/

/*=====================================================================
 * Constructors, etc.
 */

/*--------------------------------------------------------------------
 * new()
 */
static void *pd_gfsm_paths_new(t_symbol *sel, int argc, t_atom *argv)
{
  t_symbol *name  = &s_;
  t_symbol *rname = &s_;
  t_pd_gfsm_paths *x = (t_pd_gfsm_paths*)pd_new(pd_gfsm_paths_class);

  //-- defaults
  x->x_automaton_pd = NULL;
  x->x_result_pd    = NULL;
  x->x_labels       = g_ptr_array_sized_new(PD_GFSM_PATHS_DEFAULT_LABELS_LENGTH);
  x->x_labels->len  = 0;

  //-- args
  if (argc > 0) name  = atom_getsymbolarg(0,argc,argv);
  if (argc > 1) rname = atom_getsymbolarg(1,argc,argv);

  //-- bindings
  x->x_automaton_pd = pd_gfsm_automaton_pd_get(name);
  x->x_automaton_pd->x_refcnt++;

  x->x_result_pd = pd_gfsm_automaton_pd_get(rname);
  x->x_result_pd->x_refcnt++;

  //-- outlets
  x->x_valout = outlet_new(&x->x_obj, &s_anything);  //-- value outlet

  return (void *)x;
}

/*--------------------------------------------------------------------
 * free
 */
static void pd_gfsm_paths_free(t_pd_gfsm_paths *x)
{
  pd_gfsm_automaton_pd_release(x->x_automaton_pd);
  pd_gfsm_automaton_pd_release(x->x_result_pd);
  g_ptr_array_free(x->x_labels,TRUE);

  //-- do we need to do this?
  outlet_free(x->x_valout);
}

/*=====================================================================
 * Basic Accessors
 */

/*--------------------------------------------------------------------
 * automaton
 */
static void pd_gfsm_paths_automaton(t_pd_gfsm_paths *x, t_symbol *name)
{
  if (name == x->x_automaton_pd->x_name) return;
  pd_gfsm_automaton_pd_release(x->x_automaton_pd);
  x->x_automaton_pd = pd_gfsm_automaton_pd_get(name);
  ++x->x_automaton_pd->x_refcnt;
}

/*--------------------------------------------------------------------
 * result
 */
static void pd_gfsm_paths_result(t_pd_gfsm_paths *x, t_symbol *rname)
{
  if (rname == x->x_result_pd->x_name) return;
  pd_gfsm_automaton_pd_release(x->x_result_pd);
  x->x_result_pd = pd_gfsm_automaton_pd_get(rname);
  ++x->x_result_pd->x_refcnt;
}


/*=====================================================================
 * Paths
 */
static void pd_gfsm_paths_paths(t_pd_gfsm_paths *x, t_symbol *sel, int argc, t_atom *argv)
{
  //gfsmAutomaton  *fsm  = x->x_automaton_pd->x_automaton;
  guint i;

  //-- ensure labels is cleared & sufficiently allocated
  if (argc > x->x_labels->len) g_ptr_array_set_size(x->x_labels, argc);
  x->x_labels->len = 0;
  for (i=0; i < argc; i++) {
    gfsmLabelVal lab = atom_getfloat(argv+i);
    //if (lab==gfsmEpsilon) continue; //-- ignore epsilons (?)
    g_ptr_array_add(x->x_labels, (gpointer)lab);
  }

  //-- actual paths
  gfsm_automaton_paths(x->x_automaton_pd->x_automaton,
			x->x_labels,
			x->x_result_pd->x_automaton);

  //-- bang: report that we're done
  outlet_bang(x->x_valout);
}


/*=====================================================================
 * Setup
 */

/*--------------------------------------------------------------------
 * setup()
 */
void pd_gfsm_paths_setup(void)
{
  //-- class
  pd_gfsm_paths_class = class_new(gensym("gfsm_paths"),
				  (t_newmethod)pd_gfsm_paths_new,
				  (t_method)pd_gfsm_paths_free,
				  sizeof(t_pd_gfsm_paths),
				  CLASS_DEFAULT,
				  A_GIMME, A_NULL);

  //-- methods: automaton, result
  class_addmethod(pd_gfsm_paths_class,
		  (t_method)pd_gfsm_paths_automaton,
		  gensym("automaton"),
		  A_DEFSYM, A_NULL);
  class_addmethod(pd_gfsm_paths_class,
		  (t_method)pd_gfsm_paths_result,
		  gensym("result"),
		  A_DEFSYM, A_NULL);

  //-- methods: paths
  class_addmethod(pd_gfsm_paths_class,
		  (t_method)pd_gfsm_paths_paths,
		  gensym("paths"),
		  A_GIMME, A_NULL);
  class_addmethod(pd_gfsm_paths_class,
		  (t_method)pd_gfsm_paths_paths,
		  &s_list,
		  A_GIMME, A_NULL);

  //-- help symbol
  class_sethelpsymbol(pd_gfsm_paths_class, gensym("gfsm_paths-help.pd"));
}
