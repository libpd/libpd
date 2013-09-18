/*=============================================================================*\
 * File: pd_algebra.c
 * Author: Bryan Jurish <moocow@ling.uni-potsdam.de>
 * Description: finite state automata for Pd
 *
 * Copyright (c) 2004 Bryan Jurish.
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

#include <pd_gfsm.h>
#include <pd_automaton.h>
#include <m_pd.h>

/*--------------------------------------------------------------------
 * DEBUG
 *--------------------------------------------------------------------*/
//#define PDFSM_DEBUG 1

/*=====================================================================
 * Structures and Types
 *=====================================================================*/
//(none)

/*=====================================================================
 * pd_gfsm_automaton_obj: Utilities
 *=====================================================================*/
//(none)

/*=====================================================================
 * pd_gfsm_automaton_obj: Algebra
 *=====================================================================*/

/*--------------------------------------------------------------------
 * complement
 */
static void pd_gfsm_automaton_complement(t_pd_gfsm_automaton_obj *x)
{
  gfsm_automaton_complement(x->x_automaton_pd->x_automaton);
  pd_gfsm_automaton_obj_outlet_bang(x, gensym("complement"));
}

/*--------------------------------------------------------------------
 * closure
 */
static void pd_gfsm_automaton_closure(t_pd_gfsm_automaton_obj *x, t_float nf)
{
  gfsm_automaton_n_closure(x->x_automaton_pd->x_automaton, (guint)nf);
  pd_gfsm_automaton_obj_outlet_float(x, gensym("closure"), nf);
}

/*--------------------------------------------------------------------
 * compose
 */
static void pd_gfsm_automaton_compose(t_pd_gfsm_automaton_obj *x, t_symbol *fsm2_name)
{
  t_pd_gfsm_automaton_pd *fsm2_pd = pd_gfsm_automaton_pd_find(fsm2_name);
  if (!fsm2_pd) {
    error("pd_gfsm_automaton_compose(): no fsm named '%s'", fsm2_name->s_name);
    return;
  } else {
    ++fsm2_pd->x_refcnt;
    gfsm_automaton_compose(x->x_automaton_pd->x_automaton, fsm2_pd->x_automaton);
    pd_gfsm_automaton_pd_release(fsm2_pd);
  }
  pd_gfsm_automaton_obj_outlet_symbol(x, gensym("compose"), fsm2_name);
}

/*--------------------------------------------------------------------
 * concat
 */
static void pd_gfsm_automaton_concat(t_pd_gfsm_automaton_obj *x, t_symbol *fsm2_name)
{
  t_pd_gfsm_automaton_pd *fsm2_pd = pd_gfsm_automaton_pd_find(fsm2_name);
  if (!fsm2_pd) {
    error("pd_gfsm_automaton_concat(): no fsm named '%s'", fsm2_name->s_name);
    return;
  } else {
    ++fsm2_pd->x_refcnt;
    gfsm_automaton_concat(x->x_automaton_pd->x_automaton, fsm2_pd->x_automaton);
    pd_gfsm_automaton_pd_release(fsm2_pd);
  }
  pd_gfsm_automaton_obj_outlet_symbol(x, gensym("concat"), fsm2_name);
}

/*--------------------------------------------------------------------
 * determinize
 */
static void pd_gfsm_automaton_determinize(t_pd_gfsm_automaton_obj *x)
{
  gfsm_automaton_determinize(x->x_automaton_pd->x_automaton);
  pd_gfsm_automaton_obj_outlet_bang(x, gensym("determinize"));
}

/*--------------------------------------------------------------------
 * difference
 */
static void pd_gfsm_automaton_difference(t_pd_gfsm_automaton_obj *x, t_symbol *fsm2_name)
{
  t_pd_gfsm_automaton_pd *fsm2_pd = pd_gfsm_automaton_pd_find(fsm2_name);
  if (!fsm2_pd) {
    error("pd_gfsm_automaton_difference(): no fsm named '%s'", fsm2_name->s_name);
    return;
  } else {
    ++fsm2_pd->x_refcnt;
    gfsm_automaton_difference(x->x_automaton_pd->x_automaton, fsm2_pd->x_automaton);
    pd_gfsm_automaton_pd_release(fsm2_pd);
  }
  pd_gfsm_automaton_obj_outlet_symbol(x, gensym("difference"), fsm2_name);
}

/*--------------------------------------------------------------------
 * intersection
 */
static void pd_gfsm_automaton_intersect(t_pd_gfsm_automaton_obj *x, t_symbol *fsm2_name)
{
  t_pd_gfsm_automaton_pd *fsm2_pd = pd_gfsm_automaton_pd_find(fsm2_name);
  if (!fsm2_pd) {
    error("pd_gfsm_automaton_intersect(): no fsm named '%s'", fsm2_name->s_name);
    return;
  } else {
    ++fsm2_pd->x_refcnt;
    gfsm_automaton_intersect(x->x_automaton_pd->x_automaton, fsm2_pd->x_automaton);
    pd_gfsm_automaton_pd_release(fsm2_pd);
  }
  pd_gfsm_automaton_obj_outlet_symbol(x, gensym("intersect"), fsm2_name);
}

/*--------------------------------------------------------------------
 * invert
 */
static void pd_gfsm_automaton_invert(t_pd_gfsm_automaton_obj *x)
{
  gfsm_automaton_invert(x->x_automaton_pd->x_automaton);
  pd_gfsm_automaton_obj_outlet_bang(x, gensym("invert"));
}

/*--------------------------------------------------------------------
 * product
 */
static void pd_gfsm_automaton_product(t_pd_gfsm_automaton_obj *x, t_symbol *fsm2_name)
{
  t_pd_gfsm_automaton_pd *fsm2_pd = pd_gfsm_automaton_pd_find(fsm2_name);
  if (!fsm2_pd) {
    error("pd_gfsm_automaton_product(): no fsm named '%s'", fsm2_name->s_name);
    return;
  } else {
    ++fsm2_pd->x_refcnt;
    gfsm_automaton_product(x->x_automaton_pd->x_automaton, fsm2_pd->x_automaton);
    pd_gfsm_automaton_pd_release(fsm2_pd);
  }
  pd_gfsm_automaton_obj_outlet_symbol(x, gensym("intersect"), fsm2_name);
}

/*--------------------------------------------------------------------
 * project
 */
static void pd_gfsm_automaton_project(t_pd_gfsm_automaton_obj *x, t_float which)
{
  gfsm_automaton_project(x->x_automaton_pd->x_automaton,
			 (which==0 ? gfsmLSLower : gfsmLSUpper));
  pd_gfsm_automaton_obj_outlet_float(x, gensym("project"), which);
}

/*--------------------------------------------------------------------
 * prune
 */
static void pd_gfsm_automaton_connect(t_pd_gfsm_automaton_obj *x)
{
  gfsm_automaton_connect(x->x_automaton_pd->x_automaton);
  pd_gfsm_automaton_obj_outlet_bang(x, gensym("connect"));
}

/*--------------------------------------------------------------------
 * renumber
 */
static void pd_gfsm_automaton_renumber(t_pd_gfsm_automaton_obj *x)
{
  gfsm_automaton_renumber_states(x->x_automaton_pd->x_automaton);
  pd_gfsm_automaton_obj_outlet_bang(x, gensym("renumber"));
}


/*--------------------------------------------------------------------
 * reverse
 */
static void pd_gfsm_automaton_reverse(t_pd_gfsm_automaton_obj *x)
{
  gfsm_automaton_reverse(x->x_automaton_pd->x_automaton);
  pd_gfsm_automaton_obj_outlet_bang(x, gensym("reverse"));
}

/*--------------------------------------------------------------------
 * rmepsilon
 */
static void pd_gfsm_automaton_rmepsilon(t_pd_gfsm_automaton_obj *x)
{
  gfsm_automaton_rmepsilon(x->x_automaton_pd->x_automaton);
  pd_gfsm_automaton_obj_outlet_bang(x, gensym("rmepsilon"));
}

/*--------------------------------------------------------------------
 * union
 */
static void pd_gfsm_automaton_union(t_pd_gfsm_automaton_obj *x, t_symbol *fsm2_name)
{
  t_pd_gfsm_automaton_pd *fsm2_pd = pd_gfsm_automaton_pd_find(fsm2_name);
  if (!fsm2_pd) {
    error("pd_gfsm_automaton_union(): no fsm named '%s'", fsm2_name->s_name);
    return;
  }
  else if (fsm2_pd != x->x_automaton_pd) {
    ++fsm2_pd->x_refcnt;
    gfsm_automaton_union(x->x_automaton_pd->x_automaton, fsm2_pd->x_automaton);
    pd_gfsm_automaton_pd_release(fsm2_pd);
  }
  pd_gfsm_automaton_obj_outlet_symbol(x, gensym("union"), fsm2_name);
}


/*=====================================================================
 * setup
 *=====================================================================*/

/*--------------------------------------------------------------------
 * pd_gfsm_automaton_obj: setup()
 */
void pd_gfsm_algebra_setup(t_class *automaton_class)
{
  //-- methods: algebra
  class_addmethod(automaton_class, (t_method)pd_gfsm_automaton_complement,
		  gensym("complement"), A_NULL);

  class_addmethod(automaton_class, (t_method)pd_gfsm_automaton_closure,
		  gensym("closure"), A_DEFFLOAT, A_NULL);

  class_addmethod(automaton_class, (t_method)pd_gfsm_automaton_compose,
		  gensym("compose"), A_SYMBOL, A_NULL);

  class_addmethod(automaton_class, (t_method)pd_gfsm_automaton_concat,
		  gensym("concat"), A_SYMBOL, A_NULL);

  class_addmethod(automaton_class, (t_method)pd_gfsm_automaton_determinize,
		  gensym("determinize"), A_NULL);

  class_addmethod(automaton_class, (t_method)pd_gfsm_automaton_difference,
		  gensym("difference"), A_SYMBOL, A_NULL);

  class_addmethod(automaton_class, (t_method)pd_gfsm_automaton_intersect,
		  gensym("intersect"), A_SYMBOL, A_NULL);

  class_addmethod(automaton_class, (t_method)pd_gfsm_automaton_invert,
		  gensym("invert"), A_NULL);

  class_addmethod(automaton_class, (t_method)pd_gfsm_automaton_product,
		  gensym("product"), A_SYMBOL, A_NULL);

  class_addmethod(automaton_class, (t_method)pd_gfsm_automaton_project,
		  gensym("project"), A_DEFFLOAT, A_NULL);

  class_addmethod(automaton_class, (t_method)pd_gfsm_automaton_connect,
		  gensym("connect"), A_NULL);
  class_addmethod(automaton_class, (t_method)pd_gfsm_automaton_connect,
		  gensym("prune"), A_NULL); //-- backwards-compatible alias


  class_addmethod(automaton_class, (t_method)pd_gfsm_automaton_renumber,
		  gensym("renumber"), A_NULL);

  class_addmethod(automaton_class, (t_method)pd_gfsm_automaton_reverse,
		  gensym("reverse"), A_NULL);

  class_addmethod(automaton_class, (t_method)pd_gfsm_automaton_rmepsilon,
		  gensym("rmepsilon"), A_NULL);

  class_addmethod(automaton_class, (t_method)pd_gfsm_automaton_union,
		  gensym("union"), A_SYMBOL, A_NULL);
}
