/*=============================================================================*\
 * File: pd_automaton.c
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

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <pd_automaton.h>
#include <pd_io.h>
#include <m_pd.h>

/*--------------------------------------------------------------------
 * DEBUG
 *--------------------------------------------------------------------*/
//#define PDFSM_DEBUG 1

/*=====================================================================
 * Structures and Types
 *=====================================================================*/
static t_class *pd_gfsm_automaton_pd_class;
static t_class *pd_gfsm_automaton_obj_class;

#define PD_GFSM_AUTOMATON_OBJ_DEFAULT_LABELS_LENGTH 32

/*=====================================================================
 * pd_gfsm_automaton_pd: Methods
 *=====================================================================*/

/*--------------------------------------------------------------------
 * pd_gfsm_automaton_pd: new()
 */
static void *pd_gfsm_automaton_pd_new(t_symbol *name)
{
    t_pd_gfsm_automaton_pd *x = (t_pd_gfsm_automaton_pd *)pd_new(pd_gfsm_automaton_pd_class);

    //-- debug
#ifdef PDFSM_DEBUG
    post("pd_gfsm_automaton_pd_new() called; name=%s ; x=%p\n", name->s_name, x);
#endif

    //-- defaults
    x->x_refcnt    = 0;
    x->x_name      = name;
    x->x_automaton = gfsm_automaton_new();

    //-- bindings
    if (name != &s_) pd_bind((t_pd*)x, name);

    return (void *)x;
}

/*--------------------------------------------------------------------
 * pd_gfsm_automaton_pd: free()
 */
static void pd_gfsm_automaton_pd_free(t_pd_gfsm_automaton_pd *x)
{
#ifdef PDFSM_DEBUG
  post("pd_gfsm_automaton_pd_free() called ; x=%p", x);
#endif

  if (x->x_automaton) gfsm_automaton_free(x->x_automaton);
  x->x_automaton = NULL;

  /* unbind the symbol of the name of hashtable in pd's global namespace */
  if (x->x_name != &s_) pd_unbind((t_pd*)x, x->x_name);
}

/*--------------------------------------------------------------------
 * pd_gfsm_automaton_pd: setup()
 */
void pd_gfsm_automaton_pd_setup(void)
{
  //-- class
  pd_gfsm_automaton_pd_class = class_new(gensym("pd_gfsm_automaton_pd"),
					 (t_newmethod)pd_gfsm_automaton_pd_new,
					 (t_method)pd_gfsm_automaton_pd_free,
					 sizeof(t_pd_gfsm_automaton_pd),
					 CLASS_PD,
					 A_DEFSYM,
					 A_NULL);
}

/*=====================================================================
 * Utilities
 *=====================================================================*/

/*--------------------------------------------------------------------
 * Utilties: pd_gfsm_automaton_pd_find()
 */
t_pd_gfsm_automaton_pd *pd_gfsm_automaton_pd_find(t_symbol *name)
{
  if (name != &s_)
    return (t_pd_gfsm_automaton_pd*)pd_findbyclass(name,pd_gfsm_automaton_pd_class);
  return NULL;
}

/*--------------------------------------------------------------------
 * Utilties: pd_gfsm_automaton_pd_get()
 */
t_pd_gfsm_automaton_pd *pd_gfsm_automaton_pd_get(t_symbol *name)
{
  t_pd_gfsm_automaton_pd *x = pd_gfsm_automaton_pd_find(name);
  if (!x) {
    x = (t_pd_gfsm_automaton_pd*)pd_gfsm_automaton_pd_new(name);
    x->x_refcnt = 0;
  }
  return x;
}

/*--------------------------------------------------------------------
 * Utilties: pd_gfsm_automaton_pd: release()
 */
void pd_gfsm_automaton_pd_release(t_pd_gfsm_automaton_pd *x)
{
  if (x) {
    if (x->x_refcnt) --x->x_refcnt;
    if (!x->x_refcnt) pd_gfsm_automaton_pd_free(x);
  }
}

/*--------------------------------------------------------------------
 * Utilties: pd_gfsm_automaton_obj: outlet_symbol()
 */
void pd_gfsm_automaton_obj_outlet_symbol(t_pd_gfsm_automaton_obj *x, t_symbol *sel, t_symbol *val)
{
  SETSYMBOL(x->x_argv, val);
  outlet_anything(x->x_valout, sel, 1, x->x_argv);
}

/*--------------------------------------------------------------------
 * Utilties: pd_gfsm_automaton_obj: outlet_symbol_float()
 */
void pd_gfsm_automaton_obj_outlet_symbol_float(t_pd_gfsm_automaton_obj *x, t_symbol *sel, t_symbol *sym, t_float f)
{
  SETSYMBOL(x->x_argv,  sym);
  SETFLOAT(x->x_argv+1, f);
  outlet_anything(x->x_valout, sel, 2, x->x_argv);
}

/*--------------------------------------------------------------------
 * Utilties: pd_gfsm_automaton_obj: outlet_float()
 */
void pd_gfsm_automaton_obj_outlet_float(t_pd_gfsm_automaton_obj *x, t_symbol *sel, t_float f)
{
  SETFLOAT(x->x_argv, f);
  outlet_anything(x->x_valout, sel, 1, x->x_argv);
}

/*--------------------------------------------------------------------
 * Utilties: pd_gfsm_automaton_obj: outlet_float_2()
 */
void pd_gfsm_automaton_obj_outlet_float_2(t_pd_gfsm_automaton_obj *x, t_symbol *sel, t_float f1, t_float f2)
{
  SETFLOAT(x->x_argv  , f1);
  SETFLOAT(x->x_argv+1, f2);
  outlet_anything(x->x_valout, sel, 2, x->x_argv);
}

/*--------------------------------------------------------------------
 * Utilties: pd_gfsm_automaton_obj: outlet_bang()
 */
void pd_gfsm_automaton_obj_outlet_bang(t_pd_gfsm_automaton_obj *x, t_symbol *sel)
{
  SETSYMBOL(x->x_argv, &s_bang);
  outlet_anything(x->x_valout, sel, 1, x->x_argv);
}

/*--------------------------------------------------------------------
 * Utilties: pd_gfsm_automaton_obj: outlet_labels()
 */
void pd_gfsm_automaton_obj_outlet_labels(t_pd_gfsm_automaton_obj *x, t_symbol *sel, gfsmLabelVector *labs)
{

  if (labs->len > 0) {
    int i;
    if (x->x_argc < (int)labs->len) {
      size_t newsize = labs->len * sizeof(t_atom);
      x->x_argv = resizebytes(x->x_argv, x->x_argc*sizeof(t_atom), newsize);
      x->x_argc = labs->len;
    }
    for (i=0; i < (int)labs->len; i++) {
      SETFLOAT(x->x_argv+i, (gfsmLabelVal)g_ptr_array_index(labs,i));
    }
    outlet_anything(x->x_valout, sel, labs->len, x->x_argv);
  } else {
    SETSYMBOL(x->x_argv, &s_bang);
    outlet_anything(x->x_valout, sel, 1, x->x_argv);
  }
}


/*=====================================================================
 * pd_gfsm_automaton_obj: Methods
 *=====================================================================*/

/*=====================================================================
 * Constructors, etc.
 */

/*--------------------------------------------------------------------
 * pd_gfsm_automaton_obj: new
 */
static void *pd_gfsm_automaton_obj_new(t_symbol *name)
{
  t_pd_gfsm_automaton_obj *x =
    (t_pd_gfsm_automaton_obj *)pd_new(pd_gfsm_automaton_obj_class);

#ifdef PDFSM_DEBUG
  post("pd_gfsm_automaton_obj_new() called: name=%s ; x=%p",
       (name ? name->s_name : "(null)"),
       x);
#endif

  //-- defaults
  //x->x_automaton_pd = NULL;
  x->x_labels       = NULL;
  x->x_paths_s      = NULL;
  x->x_paths_a      = NULL;

  //-- bindings
  x->x_automaton_pd = pd_gfsm_automaton_pd_get(name);
  x->x_automaton_pd->x_refcnt++;

  //-- output buffer
  x->x_argc = 5;
  x->x_argv = (t_atom*)getbytes(x->x_argc * sizeof(t_atom));

  //-- outlets
  x->x_valout = outlet_new(&x->x_obj, &s_anything);  //-- value outlet

  return (void *)x;
}


/*--------------------------------------------------------------------
 * pd_gfsm_automaton_obj: free
 */
static void pd_gfsm_automaton_obj_free(t_pd_gfsm_automaton_obj *x)
{
#ifdef PDFSM_DEBUG
    post("pd_gfsm_automaton_obj_free() called: x=%p", x);
#endif

  pd_gfsm_automaton_pd_release(x->x_automaton_pd);
  if (x->x_labels) g_ptr_array_free(x->x_labels,TRUE);
  if (x->x_paths_s) gfsm_set_free(x->x_paths_s);
  if (x->x_paths_a) g_ptr_array_free(x->x_paths_a,TRUE);

  freebytes(x->x_argv, x->x_argc*sizeof(t_atom));

  //-- do we need to do this?
  outlet_free(x->x_valout);
}

/*=====================================================================
 * automaton_obj: Basic Accessors
 */

/*--------------------------------------------------------------------
 * pd_gfsm_automaton_obj: automaton
 */
static void pd_gfsm_automaton_obj_automaton(t_pd_gfsm_automaton_obj *x, t_symbol *name)
{
  if (name != x->x_automaton_pd->x_name) {
    pd_gfsm_automaton_pd_release(x->x_automaton_pd);
    x->x_automaton_pd = pd_gfsm_automaton_pd_get(name);
    ++x->x_automaton_pd->x_refcnt;
  }
  pd_gfsm_automaton_obj_outlet_symbol(x, gensym("automaton"), name);
}

/*--------------------------------------------------------------------
 * pd_gfsm_automaton_obj: size()
 */
static void pd_gfsm_automaton_obj_size(t_pd_gfsm_automaton_obj *x, GIMME_ARGS)
{
  if (argc > 0) {
    gfsmStateId n = (gfsmStateId)atom_getfloatarg(0,argc,argv);
    gfsm_automaton_reserve(x->x_automaton_pd->x_automaton, n);
  }
  pd_gfsm_automaton_obj_outlet_float
    (x, sel, (t_float)gfsm_automaton_n_states(x->x_automaton_pd->x_automaton));
}

/*--------------------------------------------------------------------
 * pd_gfsm_automaton_obj: clear()
 */
static void pd_gfsm_automaton_obj_clear(t_pd_gfsm_automaton_obj *x)
{
  gfsm_automaton_clear(x->x_automaton_pd->x_automaton);
  pd_gfsm_automaton_obj_outlet_bang(x, gensym("clear"));
}

/*=====================================================================
 * automaton_obj: flags
 */

/*--------------------------------------------------------------------
 * pd_gfsm_automaton_obj: root()
 */
static void pd_gfsm_automaton_obj_root(t_pd_gfsm_automaton_obj *x, GIMME_ARGS)
{
  gfsmStateId q0;
  if (argc > 0) {
    q0 = (gfsmStateId)atom_getintarg(0,argc,argv);
    gfsm_automaton_set_root(x->x_automaton_pd->x_automaton, q0);
  } else {
    q0 = gfsm_automaton_get_root(x->x_automaton_pd->x_automaton);
  }

  if (q0==gfsmNoState) pd_gfsm_automaton_obj_outlet_bang(x, sel);
  else                 pd_gfsm_automaton_obj_outlet_float(x, sel, (t_float)q0);
}


/*--------------------------------------------------------------------
 * pd_gfsm_automaton_obj: weighted()
 */
static void pd_gfsm_automaton_obj_weighted(t_pd_gfsm_automaton_obj *x, GIMME_ARGS)
{
  if (argc > 0)
    x->x_automaton_pd->x_automaton->flags.is_weighted = atom_getboolarg(0,argc,argv);
  pd_gfsm_automaton_obj_outlet_float
    (x, sel, (t_float)(x->x_automaton_pd->x_automaton->flags.is_weighted));
}

/*--------------------------------------------------------------------
 * pd_gfsm_automaton_obj: transducer()
 */
static void pd_gfsm_automaton_obj_transducer(t_pd_gfsm_automaton_obj *x, GIMME_ARGS)
{
  if (argc > 0)
    x->x_automaton_pd->x_automaton->flags.is_transducer = atom_getboolarg(0,argc,argv);
  pd_gfsm_automaton_obj_outlet_float
    (x, sel, (t_float)(x->x_automaton_pd->x_automaton->flags.is_transducer));
}

/*--------------------------------------------------------------------
 * pd_gfsm_automaton_obj: semiring()
 */
static void pd_gfsm_automaton_obj_semiring(t_pd_gfsm_automaton_obj *x, GIMME_ARGS)
{
  t_symbol *srsym;
  if (argc > 0) {
    srsym = atom_getsymbolarg(0,argc,argv);
    gfsm_automaton_set_semiring_type(x->x_automaton_pd->x_automaton,
				     gfsm_sr_name_to_type(srsym->s_name));
  }
  srsym = gensym(gfsm_sr_type_to_name(x->x_automaton_pd->x_automaton->sr->type));
  pd_gfsm_automaton_obj_outlet_symbol(x, sel, srsym);
}

/*=====================================================================
 * automaton_obj: automaton properties
 */

/*--------------------------------------------------------------------
 * pd_gfsm_automaton_obj: cyclic()
 */
static void pd_gfsm_automaton_obj_cyclic(t_pd_gfsm_automaton_obj *x)
{
  gboolean rc = gfsm_automaton_is_cyclic(x->x_automaton_pd->x_automaton);
  pd_gfsm_automaton_obj_outlet_float(x, gensym("cyclic"), (t_float)rc);
}


/*=====================================================================
 * automaton_obj: state properties
 */

/*--------------------------------------------------------------------
 * pd_gfsm_automaton_obj: final()
 */
static void pd_gfsm_automaton_obj_final(t_pd_gfsm_automaton_obj *x, GIMME_ARGS)
{
  gboolean ans = FALSE;
  t_float   qf = atom_getfloatarg(0,argc,argv);
  if (qf<0) qf = -qf;
  if (argc > 1) {
    ans = atom_getboolarg(1,argc,argv);
    gfsm_automaton_set_final_state(x->x_automaton_pd->x_automaton, (gfsmStateId)qf, ans);
  } else {
    ans = gfsm_automaton_is_final_state(x->x_automaton_pd->x_automaton, (gfsmStateId)qf);
  }
  pd_gfsm_automaton_obj_outlet_float_2(x, sel, qf, (t_float)ans);
}


/*=====================================================================
 * automaton_obj: states & arcs
 */

/*--------------------------------------------------------------------
 * pd_gfsm_automaton_obj: add_state()
 */
static void pd_gfsm_automaton_obj_add_state(t_pd_gfsm_automaton_obj *x, GIMME_ARGS)
{
  gfsmStateId id;
  if (argc>0) id = (gfsmStateId)atom_getfloatarg(0,argc,argv);
  else        id = gfsmNoState;

  id = gfsm_automaton_ensure_state(x->x_automaton_pd->x_automaton, id);
  pd_gfsm_automaton_obj_outlet_float(x, sel, (t_float)id);
}

/*--------------------------------------------------------------------
 * pd_gfsm_automaton_obj: add_arc()
 */
static void pd_gfsm_automaton_obj_add_arc(t_pd_gfsm_automaton_obj *x, GIMME_ARGS)
{
  t_float qsrc=0, qdst=0;
  t_float clo=0, chi=0;
  t_float w=0;
  if (argc < 2) {
    error("gfsm_automaton: add_arc(): not enough arguments!");
    return;
  }
  switch (argc) {
  case 5: //-- +from, +to, +lo, +hi, +weight
    w = atom_getfloat(argv+4);
  case 4: //-- +from, +to, +lo, +hi, -weight
    chi  = atom_getfloat(argv+3);
  case 3: //-- +from, +to, +lo, -hi, -weight
    clo  = atom_getfloat(argv+2);
  case 2: //-- +from, +to, -lo, -hi, -weight
  default:
    qdst = atom_getfloat(argv+1);
    qsrc = atom_getfloat(argv);

    if (qdst<0) qdst = -qdst;
    if (qsrc<0) qsrc = -qsrc;
    break;
  }

  gfsm_automaton_add_arc(x->x_automaton_pd->x_automaton,
			 (gfsmStateId)qsrc,
			 (gfsmStateId)qdst,
			 (gfsmLabelId)clo,
			 (gfsmLabelId)chi,
			 (gfsmWeight)w);

#ifdef PDFSM_DEBUG
  post("pd_gfsm_automaton_obj_add_arc(): node(qsrc=%ld).out_degree=%u",
       qsrc,
       gfsm_automaton_out_degree(x->x_automaton_pd->x_automaton, qsrc));
#endif

  //-- output
  SETFLOAT(x->x_argv  , (t_float)qsrc);
  SETFLOAT(x->x_argv+1, (t_float)qdst);
  SETFLOAT(x->x_argv+2, (t_float)clo);
  SETFLOAT(x->x_argv+3, (t_float)chi);
  SETFLOAT(x->x_argv+4, (t_float)w);
  outlet_anything(x->x_valout, sel, 5, x->x_argv);
}

/*=====================================================================
 * automaton_obj: Text I/O
 */

/*--------------------------------------------------------------------
 * pd_gfsm_automaton_obj: print()
 */
static void pd_gfsm_automaton_obj_print(t_pd_gfsm_automaton_obj *x)
{
  gfsmError *errp = NULL;
  gfsmIOHandle *ioh  = pd_gfsm_console_handle_new();
  gfsm_automaton_print_handle(x->x_automaton_pd->x_automaton,
			      ioh, NULL,NULL,NULL, &errp);
  if (errp) {
    error("gfsm_automaton: print(): Error: %s\n", errp->message);
    g_error_free(errp);
  }
  pd_gfsm_console_handle_free(ioh);
  pd_gfsm_automaton_obj_outlet_bang(x, gensym("print"));
}

/*--------------------------------------------------------------------
 * pd_gfsm_automaton_obj: load_txt()
 */
static void pd_gfsm_automaton_obj_load_txt(t_pd_gfsm_automaton_obj *x, t_symbol *s)
{
  gfsmError *errp = NULL;
  gfsm_automaton_compile_filename(x->x_automaton_pd->x_automaton, s->s_name, &errp);
  if (errp) {
    error("gfsm_automaton: load_txt(%s): Error: %s\n", s->s_name, errp->message);
    g_error_free(errp);
  } 
  pd_gfsm_automaton_obj_outlet_symbol(x, gensym("load"), s);
}

/*--------------------------------------------------------------------
 * pd_gfsm_automaton_obj: save_txt()
 */
static void pd_gfsm_automaton_obj_save_txt(t_pd_gfsm_automaton_obj *x, t_symbol *s)
{
  gfsmError *errp = NULL;
  gfsm_automaton_print_filename(x->x_automaton_pd->x_automaton, s->s_name, &errp);
  if (errp) {
    error("gfsm_automaton: save_txt(%s): Error: %s\n", s->s_name, errp->message);
    g_error_free(errp);
  }
  pd_gfsm_automaton_obj_outlet_symbol(x, gensym("save"), s);
}


/*=====================================================================
 * automaton_obj: Binary I/O
 */

/*--------------------------------------------------------------------
 * pd_gfsm_automaton_obj: load_bin()
 */
static void pd_gfsm_automaton_obj_load_bin(t_pd_gfsm_automaton_obj *x, t_symbol *s)
{
  gfsmError *errp = NULL;
  gfsm_automaton_load_bin_filename(x->x_automaton_pd->x_automaton, s->s_name, &errp);
  if (errp) {
    error("gfsm_automaton: load_bin(%s): Error: %s\n", s->s_name, errp->message);
    g_error_free(errp);
  }
  pd_gfsm_automaton_obj_outlet_symbol(x, gensym("load_bin"), s);
}

/*--------------------------------------------------------------------
 * pd_gfsm_automaton_obj: save_bin()
 */
static void pd_gfsm_automaton_obj_save_bin(t_pd_gfsm_automaton_obj *x, t_symbol *s, t_float zlevel)
{
  gfsmError *errp = NULL;
  gfsm_automaton_save_bin_filename(x->x_automaton_pd->x_automaton, s->s_name, (int)zlevel, &errp);
  if (errp) {
    error("gfsm_automaton: save_bin(%s,%d): Error: %s\n", s->s_name, (int)zlevel, errp->message);
    g_error_free(errp);
  }
  pd_gfsm_automaton_obj_outlet_symbol_float(x, gensym("save_bin"), s, (int)zlevel);
}


/*=====================================================================
 * automaton_obj: info
 */

/*--------------------------------------------------------------------
 * pd_gfsm_automaton_obj: info()
 */
static void pd_gfsm_automaton_obj_info(t_pd_gfsm_automaton_obj *x)
{
  gfsmAutomaton *fsm = x->x_automaton_pd->x_automaton;
  guint n_eps_i, n_eps_o, n_eps_io;

#define bool2char(b) ((b) ? 'y' : 'n')

  post("-- GFSM Automaton Info --");
  post("%-24s: %s", "Name", x->x_automaton_pd->x_name->s_name);
  post("%-24s: %s", "Semiring", gfsm_sr_type_to_name(fsm->sr->type));
  post("%-24s: %c", "Transducer?", bool2char(gfsm_automaton_is_transducer(fsm)));
  post("%-24s: %c", "Weighted?", bool2char(gfsm_automaton_is_weighted(fsm)));
  post("%-24s: %c", "Deterministic?", bool2char(fsm->flags.is_deterministic));
  post("%-24s: %s", "Sort Mode", gfsm_arc_sortmode_to_name(gfsm_automaton_sortmode(fsm)));
  if (fsm->root_id == gfsmNoState)
    post("%-24s: %s", "Initial state", "(none)");
  else 
    post("%-24s: %u", "Initial state", fsm->root_id);
  post("%-24s: %u", "# of states", gfsm_automaton_n_states(fsm));
  post("%-24s: %u", "# of final states", gfsm_automaton_n_final_states(fsm));
  post("%-24s: %u", "# of arcs", gfsm_automaton_n_arcs_full(fsm, &n_eps_i, &n_eps_o, &n_eps_io));
  post("%-24s: %u", "# of i/o epsilon arcs", n_eps_io);
  post("%-24s: %u", "# of input epsilon arcs", n_eps_i);
  post("%-24s: %u", "# of output epsilon arcs", n_eps_o);
  post("%-24s: %c", "cyclic?", bool2char(gfsm_automaton_is_cyclic(fsm)));
  //...

#undef bool2char

  pd_gfsm_automaton_obj_outlet_bang(x, gensym("info"));
}


/*=====================================================================
 * automaton_obj: draw
 */

/*--------------------------------------------------------------------
 * automaton_obj: draw_dot()
 */
static void pd_gfsm_automaton_obj_draw_dot(t_pd_gfsm_automaton_obj *x, GFSM_UNUSED GIMME_ARGS)
{
  t_pd_gfsm_alphabet_pd *ialph=NULL, *oalph=NULL, *salph=NULL;
  t_symbol
    *filename = atom_getsymbolarg(0,argc,argv),
    *title    = x->x_automaton_pd->x_name,
    *fontname = NULL;
  float
    width=0,
    height=0,
    nodesep=0,
    ranksep=0;
  int fontsize=0;
  gboolean
    portrait=FALSE,
    vertical=FALSE;
  gfsmError *errp = NULL;
  int i;

  for (i=1; i < argc; i++) {
    t_symbol *opt = atom_getsymbolarg(i,argc,argv);

    //--debug
    //post("gfsm_automaton: draw_dot: checking option '%s'", opt->s_name);

    if (opt==gensym("ilabels")) {
      if (ialph) pd_gfsm_alphabet_pd_release(ialph);
      ialph = pd_gfsm_alphabet_pd_find(atom_getsymbolarg(++i,argc,argv));
      if (ialph) ialph->x_refcnt++;
    }
    else if (opt==gensym("olabels")) {
      if (oalph) pd_gfsm_alphabet_pd_release(oalph);
      oalph = pd_gfsm_alphabet_pd_find(atom_getsymbolarg(++i,argc,argv));
      if (oalph) oalph->x_refcnt++;
    }
    else if (opt==gensym("slabels")) {
      if (salph) pd_gfsm_alphabet_pd_release(salph);
      salph = pd_gfsm_alphabet_pd_find(atom_getsymbolarg(++i,argc,argv));
      if (salph) salph->x_refcnt++;
    }
    else if (opt==gensym("title")) title = atom_getsymbolarg(++i,argc,argv);
    else if (opt==gensym("width")) width = atom_getfloatarg(++i,argc,argv);
    else if (opt==gensym("height")) height = atom_getfloatarg(++i,argc,argv);
    else if (opt==gensym("fontsize")) fontsize = atom_getintarg(++i,argc,argv);
    else if (opt==gensym("fontname")) fontname = atom_getsymbolarg(++i,argc,argv);
    else if (opt==gensym("portrait")) portrait = atom_getboolarg(++i,argc,argv);
    else if (opt==gensym("vertical")) vertical = atom_getboolarg(++i,argc,argv);
    else if (opt==gensym("nodesep")) nodesep = atom_getfloatarg(++i,argc,argv);
    else if (opt==gensym("ranksep")) ranksep = atom_getfloatarg(++i,argc,argv);
    else {
      char buf[64];
      atom_string(argv+i, buf, 64);
      error("gfsm_automaton: draw_dot: unknown option '%s' ignored", buf);
    }
  }

  gfsm_automaton_draw_dot_filename_full(x->x_automaton_pd->x_automaton,
					filename->s_name,
					(ialph ? ialph->x_alphabet : NULL),
					(oalph ? oalph->x_alphabet : NULL),
					(salph ? salph->x_alphabet : NULL),
					title->s_name,
					width,
					height,
					fontsize,
					(fontname ? fontname->s_name : NULL),
					portrait,
					vertical,
					nodesep,
					ranksep,
					&errp);
  
  if (errp) {
    error("gfsm_automaton: draw_dot(%s): Error: %s\n", filename->s_name, errp->message);
    g_error_free(errp);
  }

  //-- cleanup
  if (ialph) pd_gfsm_alphabet_pd_release(ialph);
  if (oalph) pd_gfsm_alphabet_pd_release(oalph);
  if (salph) pd_gfsm_alphabet_pd_release(salph);

  pd_gfsm_automaton_obj_outlet_symbol(x, gensym("draw_dot"), filename);
}

/*=====================================================================
 * automaton_obj: lookup
 */

/*--------------------------------------------------------------------
 * lookup()
 *   lookup RESULT LABELS
 */
static void pd_gfsm_automaton_obj_lookup(t_pd_gfsm_automaton_obj *x, GIMME_ARGS)
{
  if (argc > 1) {
    guint i;
    t_pd_gfsm_automaton_pd *result_pd;

    //-- get result automaton
    result_pd = pd_gfsm_automaton_pd_get(atom_getsymbol(argv));
    result_pd->x_refcnt++;

    //-- ensure labels exists, is cleared, & is sufficiently allocated
    if (!x->x_labels) {
      x->x_labels = g_ptr_array_sized_new(argc-1);
    } else if (argc > (int)x->x_labels->len) {
      g_ptr_array_set_size(x->x_labels, argc);
    }
    x->x_labels->len = 0;

    //-- get labels
    for (i=1; (int)i < argc; i++) {
      gfsmLabelVal lab = atom_getfloat(argv+i);
      //if (lab==gfsmEpsilon) continue; //-- ignore epsilons (?)
      g_ptr_array_add(x->x_labels, (gpointer)lab);
    }

    //-- actual lookup
    gfsm_automaton_lookup(x->x_automaton_pd->x_automaton,
			  x->x_labels,
			  result_pd->x_automaton);
   

    //-- outlet
    pd_gfsm_automaton_obj_outlet_symbol(x, sel, result_pd->x_name);

    //-- cleanup
    pd_gfsm_automaton_pd_release(result_pd);
  }
  else {
    //-- no result automaton specified: complain
    error("gfsm_automaton: lookup: no result automaton specified!");
  }
}


/*=====================================================================
 * automaton_obj: paths
 */

/*--------------------------------------------------------------------
 * paths_unsafe()
 */
static void pd_gfsm_automaton_obj_paths_unsafe(t_pd_gfsm_automaton_obj *x, GIMME_ARGS_NOCV)
{
  //-- clear set first
  if (x->x_paths_s) gfsm_set_clear(x->x_paths_s);

  //-- get paths
  x->x_paths_s = gfsm_automaton_paths(x->x_automaton_pd->x_automaton, x->x_paths_s);

  //-- get array of paths
  if (!x->x_paths_a) {
    x->x_paths_a = g_ptr_array_sized_new(gfsm_set_size(x->x_paths_s));
  } else {
    g_ptr_array_set_size(x->x_paths_a,0);
  }
  gfsm_set_to_ptr_array(x->x_paths_s, x->x_paths_a);

  //-- set path index
  x->x_paths_i = 0;

  //-- report done
  pd_gfsm_automaton_obj_outlet_float(x, sel, x->x_paths_a->len);
}

/*--------------------------------------------------------------------
 * paths_safe()
 */
static void pd_gfsm_automaton_obj_paths_safe(t_pd_gfsm_automaton_obj *x, GIMME_ARGS_NOCV)
{
  gfsmAutomaton *fsm = x->x_automaton_pd->x_automaton;

  if (fsm && gfsm_automaton_is_cyclic(fsm)) {
    error("gfsm_automaton: paths: automaton is cyclic!");
    return;
  } else if (!fsm || !gfsm_automaton_has_state(fsm,fsm->root_id)) {
    pd_gfsm_automaton_obj_outlet_float(x, sel, 0);
  } else {
    pd_gfsm_automaton_obj_paths_unsafe(x,sel, 0,NULL);
  }
}

/*--------------------------------------------------------------------
 * path_first()
 */
static void pd_gfsm_automaton_obj_path_first(t_pd_gfsm_automaton_obj *x, GIMME_ARGS_NOCV)
{
  x->x_paths_i = 0;
  if (x->x_paths_a && x->x_paths_i < x->x_paths_a->len) {
    pd_gfsm_automaton_obj_outlet_float(x, sel, x->x_paths_i);
  } else {
    pd_gfsm_automaton_obj_outlet_float(x, sel, -1);
  }
}

/*--------------------------------------------------------------------
 * path_next()
 */
static void pd_gfsm_automaton_obj_path_next(t_pd_gfsm_automaton_obj *x, GIMME_ARGS_NOCV)
{
  if (x->x_paths_a && x->x_paths_i+1 < x->x_paths_a->len) { /*&& x->x_paths_i >= 0 : always true*/
    x->x_paths_i++;
    pd_gfsm_automaton_obj_outlet_float(x, sel, x->x_paths_i);
  } else {
    pd_gfsm_automaton_obj_outlet_float(x, sel, -1);
  }
}

/*--------------------------------------------------------------------
 * path_nth()
 */
static void pd_gfsm_automaton_obj_path_nth(t_pd_gfsm_automaton_obj *x, GIMME_ARGS)
{
  int ni = argc > 0 ? atom_getfloat(argv) : 0;
  if (x->x_paths_a && ni >= 0 && (guint)ni < x->x_paths_a->len) {
    x->x_paths_i = ni;
    pd_gfsm_automaton_obj_outlet_float(x, sel, x->x_paths_i);
  } else {
    pd_gfsm_automaton_obj_outlet_float(x, sel, -1);
  }
}

/*--------------------------------------------------------------------
 * path_lo()
 */
static void pd_gfsm_automaton_obj_path_lo(t_pd_gfsm_automaton_obj *x, GIMME_ARGS_NOCV)
{
  if (x->x_paths_a && x->x_paths_i < x->x_paths_a->len) { /*&& x->x_paths_i >= 0*/
    gfsmPath *p = (gfsmPath*)g_ptr_array_index(x->x_paths_a,x->x_paths_i);
    pd_gfsm_automaton_obj_outlet_labels(x, sel, p->lo);
  }
  else {
    pd_gfsm_automaton_obj_outlet_bang(x, sel);
  }
}

/*--------------------------------------------------------------------
 * path_hi()
 */
static void pd_gfsm_automaton_obj_path_hi(t_pd_gfsm_automaton_obj *x, GIMME_ARGS_NOCV)
{
  if (x->x_paths_a && x->x_paths_i < x->x_paths_a->len) { /*&& x->x_paths_i >= 0*/
    gfsmPath *p = (gfsmPath*)g_ptr_array_index(x->x_paths_a,x->x_paths_i);
    pd_gfsm_automaton_obj_outlet_labels(x, sel, p->hi);
  }
  else {
    pd_gfsm_automaton_obj_outlet_bang(x, sel);
  }
}

/*--------------------------------------------------------------------
 * path_w()
 */
static void pd_gfsm_automaton_obj_path_w(t_pd_gfsm_automaton_obj *x, GIMME_ARGS_NOCV)
{
  if (x->x_paths_a && x->x_paths_i < x->x_paths_a->len) { /*&& x->x_paths_i >= 0*/
    gfsmPath *p = (gfsmPath*)g_ptr_array_index(x->x_paths_a,x->x_paths_i);
    pd_gfsm_automaton_obj_outlet_float(x, sel, p->w);
  }
  else {
    pd_gfsm_automaton_obj_outlet_bang(x, sel);
  }
}


/*=====================================================================
 * automaton_obj: setup
 */

/*--------------------------------------------------------------------
 * pd_gfsm_automaton_obj: setup()
 */
static void pd_gfsm_automaton_obj_setup(void)
{
  //-- class
  pd_gfsm_automaton_obj_class = class_new(gensym("gfsm_automaton"),
					  (t_newmethod)pd_gfsm_automaton_obj_new,
					  (t_method)pd_gfsm_automaton_obj_free,
					  sizeof(t_pd_gfsm_automaton_obj),
					  CLASS_DEFAULT,
					  A_DEFSYM,
					  A_NULL);

  //-- methods: automaton
  class_addmethod(pd_gfsm_automaton_obj_class,
		  (t_method)pd_gfsm_automaton_obj_automaton,
		  gensym("automaton"),
		  A_DEFSYM, A_NULL);

  //-- methods: flags
  class_addmethod(pd_gfsm_automaton_obj_class,
		  (t_method)pd_gfsm_automaton_obj_root,
		  gensym("root"),
		  A_GIMME, A_NULL);
  class_addmethod(pd_gfsm_automaton_obj_class,
		  (t_method)pd_gfsm_automaton_obj_weighted,
		  gensym("weighted"),
		  A_GIMME, A_NULL);
  class_addmethod(pd_gfsm_automaton_obj_class,
		  (t_method)pd_gfsm_automaton_obj_transducer,
		  gensym("transducer"),
		  A_GIMME, A_NULL);
  class_addmethod(pd_gfsm_automaton_obj_class,
		  (t_method)pd_gfsm_automaton_obj_semiring,
		  gensym("semiring"),
		  A_GIMME, A_NULL);
  
  //-- methods: size
  class_addmethod(pd_gfsm_automaton_obj_class,
		  (t_method)pd_gfsm_automaton_obj_size,
		  gensym("size"),
		  A_GIMME, A_NULL);
  class_addmethod(pd_gfsm_automaton_obj_class,
		  (t_method)pd_gfsm_automaton_obj_clear,
		  gensym("clear"),
		  A_NULL);

  //-- methods: automaton properties
  class_addmethod(pd_gfsm_automaton_obj_class,
		  (t_method)pd_gfsm_automaton_obj_cyclic,
		  gensym("cyclic"),
		  A_NULL);

  //-- methods: state properties
  class_addmethod(pd_gfsm_automaton_obj_class,
		  (t_method)pd_gfsm_automaton_obj_final,
		  gensym("final"),
		  A_GIMME, A_NULL);

  //-- methods: states & arcs
  class_addmethod(pd_gfsm_automaton_obj_class,
		  (t_method)pd_gfsm_automaton_obj_add_state,
		  gensym("add_state"),
		  A_GIMME, A_NULL);
  class_addmethod(pd_gfsm_automaton_obj_class,
		  (t_method)pd_gfsm_automaton_obj_add_arc,
		  gensym("add_arc"),
		  A_GIMME,  //-- qsrc,qdst,clo,chi,weight
		  A_NULL);


  //-- methods: I/O: text
  class_addmethod(pd_gfsm_automaton_obj_class,
		  (t_method)pd_gfsm_automaton_obj_print,
		  gensym("print"),
		  A_NULL);
  class_addmethod(pd_gfsm_automaton_obj_class,
		  (t_method)pd_gfsm_automaton_obj_load_txt,
		  gensym("load"),
		  A_SYMBOL, A_NULL);
  class_addmethod(pd_gfsm_automaton_obj_class,
		  (t_method)pd_gfsm_automaton_obj_save_txt,
		  gensym("save"),
		  A_SYMBOL, A_NULL);

  //-- methods: I/O: binary
  class_addmethod(pd_gfsm_automaton_obj_class,
		  (t_method)pd_gfsm_automaton_obj_load_bin,
		  gensym("load_bin"),
		  A_SYMBOL, A_NULL);
  class_addmethod(pd_gfsm_automaton_obj_class,
		  (t_method)pd_gfsm_automaton_obj_save_bin,
		  gensym("save_bin"),
		  A_SYMBOL, A_DEFFLOAT, A_NULL);

  //-- methods: info
  class_addmethod(pd_gfsm_automaton_obj_class,
		  (t_method)pd_gfsm_automaton_obj_info,
		  gensym("info"),
		  A_NULL);

  //-- methods: draw
  class_addmethod(pd_gfsm_automaton_obj_class,
		  (t_method)pd_gfsm_automaton_obj_draw_dot,
		  gensym("draw_dot"),
		  A_GIMME, A_NULL);

  //-- methods: lookup
  class_addmethod(pd_gfsm_automaton_obj_class,
		  (t_method)pd_gfsm_automaton_obj_lookup,
		  gensym("lookup"),
		  A_GIMME, A_NULL);

  //-- methods: paths
  class_addmethod(pd_gfsm_automaton_obj_class,
		  (t_method)pd_gfsm_automaton_obj_paths_safe,
		  gensym("paths"),
		  A_GIMME, A_NULL);
  class_addmethod(pd_gfsm_automaton_obj_class,
		  (t_method)pd_gfsm_automaton_obj_paths_unsafe,
		  gensym("paths_fast"),
		  A_GIMME, A_NULL);
  class_addmethod(pd_gfsm_automaton_obj_class,
		  (t_method)pd_gfsm_automaton_obj_path_first,
		  gensym("path_first"),
		  A_GIMME, A_NULL);
  class_addmethod(pd_gfsm_automaton_obj_class,
		  (t_method)pd_gfsm_automaton_obj_path_next,
		  gensym("path_next"),
		  A_GIMME, A_NULL);
  class_addmethod(pd_gfsm_automaton_obj_class,
		  (t_method)pd_gfsm_automaton_obj_path_nth,
		  gensym("path_nth"),
		  A_GIMME, A_NULL);
  class_addmethod(pd_gfsm_automaton_obj_class,
		  (t_method)pd_gfsm_automaton_obj_path_lo,
		  gensym("path_lo"),
		  A_GIMME, A_NULL);
  class_addmethod(pd_gfsm_automaton_obj_class,
		  (t_method)pd_gfsm_automaton_obj_path_hi,
		  gensym("path_hi"),
		  A_GIMME, A_NULL);
  class_addmethod(pd_gfsm_automaton_obj_class,
		  (t_method)pd_gfsm_automaton_obj_path_w,
		  gensym("path_w"),
		  A_GIMME, A_NULL);

  //-- help symbol
  class_sethelpsymbol(pd_gfsm_automaton_obj_class, gensym("gfsm_automaton-help.pd"));
}


/*=====================================================================
 * automaton setup
 *=====================================================================*/

/*--------------------------------------------------------------------
 * pd_gfsm_automaton: setup()
 */
void pd_gfsm_automaton_setup(void)
{
  pd_gfsm_automaton_pd_setup();
  pd_gfsm_automaton_obj_setup();
  pd_gfsm_algebra_setup(pd_gfsm_automaton_obj_class);
}
