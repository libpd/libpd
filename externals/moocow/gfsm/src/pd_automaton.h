/*=============================================================================*\
 * File: pd_automaton.h
 * Author: Bryan Jurish <moocow@ling.uni-potsdam.de>
 * Description: finite state automata for Pd
 *
 * Copyright (c) 2004-2006 Bryan Jurish.
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

/*=====================================================================
 * Protos
 *=====================================================================*/
#ifndef PD_GFSM_AUTOMATON_H
#define PD_GFSM_AUTOMATON_H

/*----------------------------------------------------------------------
 * includes
 */
#include <m_pd.h>
#include <gfsm.h>
#include <pd_alphabet.h>

/*--------------------------------------------------------------
 * pd_fsm_automaton_pd
 */
typedef struct _pd_gfsm_automaton_pd
{
  t_pd             x_pd;
  t_symbol        *x_name;
  size_t           x_refcnt;
  gfsmAutomaton   *x_automaton;
} t_pd_gfsm_automaton_pd;

typedef struct _pd_gfsm_automaton_obj
{
  t_object                 x_obj;

  //-- underlying automaton
  t_pd_gfsm_automaton_pd  *x_automaton_pd;

  //-- for lookup()
  gfsmLabelVector         *x_labels;

  //-- for paths()
  gfsmSet                 *x_paths_s;
  GPtrArray               *x_paths_a;
  guint                    x_paths_i;

  //-- output-related stuff
  t_int                    x_argc;
  t_atom                  *x_argv;
  t_outlet                *x_valout;
} t_pd_gfsm_automaton_obj;

/*--------------------------------------------------------------------
 * utility macros
 */
#define atom_getboolarg(which,argc,argv) (atom_getintarg(which,argc,argv)==0 ? FALSE : TRUE)
#define GIMME_ARGS t_symbol *sel, int argc, t_atom *argv
#define GIMME_ARGS_NOCV t_symbol *sel, GFSM_UNUSED int argc, GFSM_UNUSED t_atom *argv

/*----------------------------------------------------------------------
 * utilities
 */
t_pd_gfsm_automaton_pd *pd_gfsm_automaton_pd_find(t_symbol *name);
t_pd_gfsm_automaton_pd *pd_gfsm_automaton_pd_get(t_symbol *name);
void pd_gfsm_automaton_pd_release(t_pd_gfsm_automaton_pd *x);

void pd_gfsm_automaton_obj_outlet_symbol(t_pd_gfsm_automaton_obj *x, t_symbol *sel, t_symbol *val);
void pd_gfsm_automaton_obj_outlet_float(t_pd_gfsm_automaton_obj *x, t_symbol *sel, t_float f);
void pd_gfsm_automaton_obj_outlet_float_2(t_pd_gfsm_automaton_obj *x, t_symbol *sel, t_float f1, t_float f2);
void pd_gfsm_automaton_obj_outlet_bang(t_pd_gfsm_automaton_obj *x, t_symbol *sel);

/*----------------------------------------------------------------------
 * setup routines
 */
extern void pd_gfsm_algebra_setup(t_class *automaton_class);

void pd_gfsm_automaton_setup(void);

#endif /* PD_GFSM_AUTOMATON_H */
