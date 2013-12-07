/*=============================================================================*\
 * File: pd_state.c
 * Author: Bryan Jurish <moocow@ling.uni-potsdam.de>
 * Description: finite state automata for Pd
 *
 * Copyright (c) 2004-2007 Bryan Jurish.
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

#include <pd_state.h>

/*=====================================================================
 * Structures and Types
 *=====================================================================*/
static t_class *pd_gfsm_state_class;


/*=====================================================================
 * pd_gfsm_state: Methods
 *=====================================================================*/

/*=====================================================================
 * Constructors, etc.
 */

/*--------------------------------------------------------------------
 * new()
 */
static void *pd_gfsm_state_new(GFSM_UNUSED t_symbol *sel, int argc, t_atom *argv)
{
  t_symbol *name = &s_;
  t_pd_gfsm_state *x = (t_pd_gfsm_state *)pd_new(pd_gfsm_state_class);

  //-- defaults
  x->x_id = gfsmNoState;
  gfsm_arciter_close(&x->x_arci);
  x->x_open = FALSE;

  //-- args
  if (argc > 0) {
    name = atom_getsymbolarg(0,argc,argv);
    if (argc > 1) x->x_id = (gfsmStateId)atom_getfloatarg(1,argc,argv);
  }

  //-- bindings
  x->x_automaton_pd = pd_gfsm_automaton_pd_get(name);
  x->x_automaton_pd->x_refcnt++;

  //-- outlets
  x->x_valout = outlet_new(&x->x_obj, &s_anything);  //-- value outlet

  return (void *)x;
}

/*--------------------------------------------------------------------
 * free
 */
static void pd_gfsm_state_free(t_pd_gfsm_state *x)
{
  gfsm_arciter_close(&x->x_arci);
  pd_gfsm_automaton_pd_release(x->x_automaton_pd);

  //-- do we need to do this?
  outlet_free(x->x_valout);
}

/*=====================================================================
 * Basic Accessors
 */

/*--------------------------------------------------------------------
 * automaton
 */
static void pd_gfsm_state_automaton(t_pd_gfsm_state *x, t_symbol *name)
{
  gfsm_arciter_close(&x->x_arci);
  x->x_open = FALSE;

  if (name == x->x_automaton_pd->x_name) return;
  pd_gfsm_automaton_pd_release(x->x_automaton_pd);

  x->x_automaton_pd = pd_gfsm_automaton_pd_get(name);
  ++x->x_automaton_pd->x_refcnt;
}


/*--------------------------------------------------------------------
 * id
 */
static void pd_gfsm_state_id(t_pd_gfsm_state *x)
{
  SETSYMBOL(x->x_argv, gensym("id"));
  if (x->x_id==gfsmNoState) SETSYMBOL(x->x_argv, &s_bang);
  else                      SETFLOAT(x->x_argv, (t_float)(x->x_id));
  outlet_anything(x->x_valout, gensym("id"), 1, x->x_argv);
}

/*--------------------------------------------------------------------
 * set
 */
static void pd_gfsm_state_set(t_pd_gfsm_state *x, t_floatarg qf)
{
  if (qf<0) qf = -qf;
  gfsm_arciter_close(&x->x_arci);
  x->x_open = FALSE;
  x->x_id   = gfsm_automaton_ensure_state(x->x_automaton_pd->x_automaton, (gfsmStateId)qf);
}


/*=====================================================================
 * Navigation
 */

/*--------------------------------------------------------------------
 * utility: outlet_arc()
 */
static void pd_gfsm_state_outlet_arc(t_pd_gfsm_state *x, t_symbol *sel)
{
  //outlet_anything(x->x_opout, op, 0, NULL);
  if (gfsm_arciter_ok(&x->x_arci)) {
    gfsmArc *a = gfsm_arciter_arc(&x->x_arci);
    SETFLOAT(x->x_argv,   (t_float)(a->target));
    SETFLOAT(x->x_argv+1, (t_float)(a->lower));
    SETFLOAT(x->x_argv+2, (t_float)(a->upper));
    SETFLOAT(x->x_argv+3, (t_float)(a->weight));
    outlet_anything(x->x_valout, sel, 4, x->x_argv);
  }
  else {
    SETSYMBOL(x->x_argv, &s_bang);
    outlet_anything(x->x_valout, sel, 1, x->x_argv);
  }
}

/*--------------------------------------------------------------------
 * arc_first
 */
static void pd_gfsm_state_arc_first(t_pd_gfsm_state *x)
{
  gfsm_arciter_close(&x->x_arci);
  gfsm_arciter_open(&x->x_arci, x->x_automaton_pd->x_automaton, x->x_id);
  x->x_open = TRUE;
  pd_gfsm_state_outlet_arc(x, gensym("arc_first"));
}

/*--------------------------------------------------------------------
 * arc_next
 */
static void pd_gfsm_state_arc_next(t_pd_gfsm_state *x)
{
  gfsmState *s = gfsm_automaton_find_state(x->x_automaton_pd->x_automaton, x->x_id);
  if (s) {
    if (x->x_open && gfsm_arciter_ok(&x->x_arci)) {
      gfsm_arciter_next(&x->x_arci);
    } else if (!x->x_open) {
      gfsm_arciter_open(&x->x_arci, x->x_automaton_pd->x_automaton, x->x_id);
      x->x_open = TRUE;
    }
  }
  pd_gfsm_state_outlet_arc(x, gensym("arc_next"));
}

/*--------------------------------------------------------------------
 * arc_seek
 */
static void pd_gfsm_state_arc_seek(t_pd_gfsm_state *x, t_float flo, t_float fhi)
{
  gfsmState *s = gfsm_automaton_find_state(x->x_automaton_pd->x_automaton, x->x_id);
  if (s) {
    if (x->x_open && gfsm_arciter_ok(&x->x_arci)) {
      gfsm_arciter_next(&x->x_arci);
    } else if (!x->x_open) {
      gfsm_arciter_open(&x->x_arci, x->x_automaton_pd->x_automaton, x->x_id);
      x->x_open = TRUE;
    }
  }
  gfsm_arciter_seek_both(&x->x_arci,
			 (flo < 0 ? gfsmNoLabel : ((gfsmLabelVal)flo)),
			 (fhi < 0 ? gfsmNoLabel : ((gfsmLabelVal)fhi)));
  pd_gfsm_state_outlet_arc(x, gensym("arc_seek"));
}

/*--------------------------------------------------------------------
 * arc_nth(n)
 */
static void pd_gfsm_state_arc_nth(t_pd_gfsm_state *x, t_float n)
{
  int i = n;
  gfsmState *s = gfsm_automaton_find_state(x->x_automaton_pd->x_automaton, x->x_id);

  if (s) {
    gfsm_arciter_close(&x->x_arci);
    for (gfsm_arciter_open(&x->x_arci, x->x_automaton_pd->x_automaton, x->x_id);
	 i > 0 && gfsm_arciter_ok(&x->x_arci);
	 gfsm_arciter_next(&x->x_arci))
      ;
  }

  pd_gfsm_state_outlet_arc(x, gensym("arc_nth"));
}

/*--------------------------------------------------------------------
 * get_total_weight(bool use_semiring)
 *  + low-level
 */
static t_float pd_gfsm_state_get_total_weight(t_pd_gfsm_state *x, int use_semiring)
{
  gfsmAutomaton *fsm = x->x_automaton_pd->x_automaton;
  gfsmState     *s   = gfsm_automaton_find_state(fsm, x->x_id);
  gfsmSemiring  *sr  = fsm->sr;
  gfsmWeight     w   = use_semiring ? sr->zero : 0.0;

  if (s) {
    /*w = gfsm_sr_plus(sr,w,gfsm_automaton_get_final_weight(fsm,x->x_id));*/ //--ignore final weights!
    gfsm_arciter_close(&x->x_arci);
    for (gfsm_arciter_open(&x->x_arci, x->x_automaton_pd->x_automaton, x->x_id);
	 gfsm_arciter_ok(&x->x_arci);
	 gfsm_arciter_next(&x->x_arci))
      {
	gfsmArc *a = gfsm_arciter_arc(&x->x_arci);
	w = use_semiring ? gfsm_sr_plus(sr,w,a->weight) : (w+a->weight);
      }
  }
  return w;
}

/*--------------------------------------------------------------------
 * total_weight()
 *  + pd level
 */
static void pd_gfsm_state_total_weight(t_pd_gfsm_state *x, t_float use_semiring)
{
  t_float w = pd_gfsm_state_get_total_weight(x, (int)use_semiring);
  SETFLOAT(x->x_argv, (t_float)(w));
  outlet_anything(x->x_valout, gensym("total_weight"), 1, x->x_argv);
}


/*--------------------------------------------------------------------
 * arc_gen(weight_hint)
 */
static void pd_gfsm_state_arc_gen(t_pd_gfsm_state *x, t_float weight_hint, t_float use_semiring_f)
{
  gfsmState *s = gfsm_automaton_find_state(x->x_automaton_pd->x_automaton, x->x_id);
  int use_semiring = (int)use_semiring_f;
  if (s) {
    gfsmSemiring *sr = x->x_automaton_pd->x_automaton->sr;
    gfsmWeight     w = use_semiring ? sr->zero : 0;

    for (gfsm_arciter_open(&x->x_arci, x->x_automaton_pd->x_automaton, x->x_id);
	 gfsm_arciter_ok(&x->x_arci);
	 gfsm_arciter_next(&x->x_arci))
      {
	gfsmArc *a = gfsm_arciter_arc(&x->x_arci);
	if (use_semiring) {
	  w = gfsm_sr_plus(sr,w,a->weight);
	  if (!gfsm_sr_less(sr,w,weight_hint)) break;
	} else {
	  w += a->weight;
	  if (w >= weight_hint) break;
	}
      }
  }
  pd_gfsm_state_outlet_arc(x, gensym("arc_gen"));
}



/*--------------------------------------------------------------------
 * arc_reset
 */
static void pd_gfsm_state_arc_reset(t_pd_gfsm_state *x)
{
  gfsm_arciter_close(&x->x_arci);
  x->x_open = FALSE;
}

/*--------------------------------------------------------------------
 * add_weight
 */
static void pd_gfsm_state_add_weight(t_pd_gfsm_state *x,
				     t_float fto,
				     t_float flo,
				     t_float fhi,
				     t_float w)
{
  gfsmStateId  to = (fto < 0 ? (-fto)      : ((gfsmStateId)fto));
  gfsmLabelVal lo = (flo < 0 ? gfsmNoLabel : ((gfsmLabelVal)flo));
  gfsmLabelVal hi = (fhi < 0 ? gfsmNoLabel : ((gfsmLabelVal)fhi));

  if (x->x_open) pd_gfsm_state_arc_reset(x);

  gfsm_arciter_open(&x->x_arci, x->x_automaton_pd->x_automaton, x->x_id);
  gfsm_arciter_seek_both(&x->x_arci, lo, hi);

  if (gfsm_arciter_ok(&x->x_arci)) {
    gfsm_arciter_arc(&x->x_arci)->weight += w;
  } else {
    if (lo==gfsmNoLabel) lo = gfsmEpsilon;
    if (hi==gfsmNoLabel) hi = gfsmEpsilon;
    gfsm_automaton_add_arc(x->x_automaton_pd->x_automaton, x->x_id, to, lo, hi, w);
  }

  pd_gfsm_state_arc_reset(x);
}

/*=====================================================================
 * Properties
 */

/*--------------------------------------------------------------------
 * degree
 */
static void pd_gfsm_state_degree(t_pd_gfsm_state *x)
{
  gfsmState *s = gfsm_automaton_find_state(x->x_automaton_pd->x_automaton, x->x_id);
  SETFLOAT(x->x_argv, (s ? (t_float)gfsm_state_out_degree(s) : 0));
  outlet_anything(x->x_valout, gensym("degree"), 1, x->x_argv);
}

/*--------------------------------------------------------------------
 * cyclic()
 */
static void pd_gfsm_state_cyclic(t_pd_gfsm_state *x)
{
  gfsmAutomaton *fsm = x->x_automaton_pd->x_automaton;
  gfsmBitVector *visited, *completed;
  gboolean rc = FALSE;

  if (gfsm_automaton_has_state(fsm,x->x_id)) {
    visited   = gfsm_bitvector_sized_new(fsm->states->len);
    completed = gfsm_bitvector_sized_new(fsm->states->len);
    rc        = gfsm_automaton_is_cyclic_state(fsm, x->x_id, visited, completed);
    gfsm_bitvector_free(visited);
    gfsm_bitvector_free(completed);
  }
  SETFLOAT(x->x_argv, (t_float)rc);
  outlet_anything(x->x_valout, gensym("cyclic"), 1, x->x_argv);
}


/*=====================================================================
 * Setup
 */

/*--------------------------------------------------------------------
 * setup()
 */
void pd_gfsm_state_setup(void)
{
  //-- class
  pd_gfsm_state_class = class_new(gensym("gfsm_state"),
				  (t_newmethod)pd_gfsm_state_new,
				  (t_method)pd_gfsm_state_free,
				  sizeof(t_pd_gfsm_state),
				  CLASS_DEFAULT,
				  A_GIMME, A_NULL);

  //-- methods: automaton
  class_addmethod(pd_gfsm_state_class,
		  (t_method)pd_gfsm_state_automaton,
		  gensym("automaton"),
		  A_DEFSYM, A_NULL);

  //-- methods: id
  class_addmethod(pd_gfsm_state_class, (t_method)pd_gfsm_state_id,
		  gensym("id"), A_NULL);
  class_addmethod(pd_gfsm_state_class, (t_method)pd_gfsm_state_set,
		  gensym("set"), A_DEFFLOAT, A_NULL);


  //-- methods: navigation
  class_addmethod(pd_gfsm_state_class, (t_method)pd_gfsm_state_arc_first,
		  gensym("arc_first"), A_NULL);
  class_addmethod(pd_gfsm_state_class, (t_method)pd_gfsm_state_arc_next,
		  gensym("arc_next"), A_NULL);
  class_addmethod(pd_gfsm_state_class, (t_method)pd_gfsm_state_arc_seek,
		  gensym("arc_seek"), A_FLOAT, A_FLOAT, A_NULL);
  class_addmethod(pd_gfsm_state_class, (t_method)pd_gfsm_state_arc_reset,
		  gensym("arc_reset"), A_NULL);

  //-- new arc methods
  class_addmethod(pd_gfsm_state_class, (t_method)pd_gfsm_state_arc_nth,
		  gensym("arc_nth"), A_DEFFLOAT, A_NULL);
  class_addmethod(pd_gfsm_state_class, (t_method)pd_gfsm_state_total_weight,
		  gensym("total_weight"), A_DEFFLOAT, A_DEFFLOAT, A_NULL);
  class_addmethod(pd_gfsm_state_class, (t_method)pd_gfsm_state_arc_gen,
		  gensym("arc_gen"), A_DEFFLOAT, A_DEFFLOAT, A_NULL);

  //-- methods: manipulation
  class_addmethod(pd_gfsm_state_class, (t_method)pd_gfsm_state_add_weight,
		  gensym("add_weight"), A_FLOAT, A_FLOAT, A_FLOAT, A_FLOAT, A_NULL);

  //-- methods: properties
  class_addmethod(pd_gfsm_state_class, (t_method)pd_gfsm_state_degree,
		  gensym("degree"), A_NULL);
  class_addmethod(pd_gfsm_state_class, (t_method)pd_gfsm_state_cyclic,
		  gensym("cyclic"), A_NULL);

  //-- help symbol
  class_sethelpsymbol(pd_gfsm_state_class, gensym("gfsm_state-help.pd"));
}
