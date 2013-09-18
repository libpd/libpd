/*=============================================================================*\
 * File: pd_alphabet.c
 * Author: Bryan Jurish <moocow@ling.uni-potsdam.de>
 * Description: finite state automata for Pd: alphabet
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

#include <pd_alphabet.h>
#include <pd_io.h>
#include <string.h>

/*=====================================================================
 * DEBUG
 *=====================================================================*/
//#define ALPHABET_DEBUG

/*=====================================================================
 * Pd Constants
 *=====================================================================*/
static t_class *pd_gfsm_alphabet_pd_class;
static t_class *pd_gfsm_alphabet_obj_class;

/*=====================================================================
 * pd_gfsm_alphabet_pd: Methods
 *=====================================================================*/

/*--------------------------------------------------------------------
 * pd_gfsm_alphabet_pd: new
 */
static void *pd_gfsm_alphabet_pd_new(t_symbol *name)
{
    t_pd_gfsm_alphabet_pd *x = (t_pd_gfsm_alphabet_pd *)pd_new(pd_gfsm_alphabet_pd_class);

#ifdef ALPHABET_DEBUG
    post("pd_gfsm_alphabet_pd_new() called ; name=%s ; x=%p",
	 (name ? name->s_name : "(null)"),
	 x);
#endif

    //-- defaults
    x->x_refcnt   = 0;
    x->x_name     = name;
    x->x_alphabet = (gfsmAlphabet*)gfsm_pd_atom_alphabet_new();
    
    //-- bindings
    if (name != &s_) pd_bind((t_pd*)x, name);
    
    return (void *)x;
}


/*--------------------------------------------------------------------
 * pd_gfsm_alphabet_pd: free
 */
static void pd_gfsm_alphabet_pd_free(t_pd_gfsm_alphabet_pd *x)
{
#ifdef ALPHABET_DEBUG
  post("pd_gfsm_alphabet_pd_free() called ; x=%p", x);
#endif

  if (x->x_alphabet) gfsm_pd_atom_alphabet_free((gfsmPdAtomAlphabet*)(x->x_alphabet));

  /* unbind the symbol of the name of hashtable in pd's global namespace */
  if (x->x_name != &s_) pd_unbind((t_pd*)x, x->x_name);
}

/*--------------------------------------------------------------------
 * pd_gfsm_alphabet_pd: find
 */
t_pd_gfsm_alphabet_pd *pd_gfsm_alphabet_pd_find(t_symbol *name)
{
  if (name != &s_)
    return (t_pd_gfsm_alphabet_pd*)pd_findbyclass(name,pd_gfsm_alphabet_pd_class);
  return NULL;
}

/*--------------------------------------------------------------------
 * pd_gfsm_alphabet_pd: get
 */
t_pd_gfsm_alphabet_pd *pd_gfsm_alphabet_pd_get(t_symbol *name)
{
  t_pd_gfsm_alphabet_pd *x = pd_gfsm_alphabet_pd_find(name);
  if (!x) {
    x = (t_pd_gfsm_alphabet_pd*)pd_gfsm_alphabet_pd_new(name);
    x->x_refcnt = 0;
  }
  return x;
}

/*--------------------------------------------------------------------
 * pd_gfsm_alphabet_pd: release
 */
void pd_gfsm_alphabet_pd_release(t_pd_gfsm_alphabet_pd *x)
{
  if (x) {
    if (x->x_refcnt) --x->x_refcnt;
    if (!x->x_refcnt) pd_gfsm_alphabet_pd_free(x);
  }
}

/*--------------------------------------------------------------------
 * pd_gfsm_alphabet_pd: setup
 */
void pd_gfsm_alphabet_pd_setup(void)
{
  //-- class
  pd_gfsm_alphabet_pd_class = class_new(gensym("gfsm_alphabet_pd"),
					(t_newmethod)pd_gfsm_alphabet_pd_new,
					(t_method)pd_gfsm_alphabet_pd_free,
					sizeof(t_pd_gfsm_alphabet_pd),
					CLASS_PD,
					A_DEFSYM,
					A_NULL);
}


/*=====================================================================
 * pd_gfsm_alphabet: Methods
 *=====================================================================*/

/*--------------------------------------------------------------------
 * pd_gfsm_alphabet_obj: new
 */
static void *pd_gfsm_alphabet_obj_new(t_symbol *name)
{
  t_pd_gfsm_alphabet_obj *x = (t_pd_gfsm_alphabet_obj *)pd_new(pd_gfsm_alphabet_obj_class);

#ifdef ALPHABET_DEBUG
  post("pd_gfsm_alphabet_obj_new() called: name=%s ; x=%p",
       (name ? name->s_name : "(null)"),
       x);
#endif

    //-- defaults
    x->x_alphabet_pd = NULL;

    //-- bindings
    x->x_alphabet_pd = pd_gfsm_alphabet_pd_get(name);
    x->x_alphabet_pd->x_refcnt++;

    //-- outlets
    x->x_labout = outlet_new(&x->x_obj, &s_float);     //-- label / "char" outlet
    x->x_keyout = outlet_new(&x->x_obj, &s_anything);  //-- atom outlet

    return (void *)x;
}


/*--------------------------------------------------------------------
 * pd_gfsm_alphabet_obj: free
 */
static void pd_gfsm_alphabet_obj_free(t_pd_gfsm_alphabet_obj *x)
{
#ifdef ALPHABET_DEBUG
  post("pd_gfsm_alphabet_obj_free() called: x=%p", x);
#endif

  pd_gfsm_alphabet_pd_release(x->x_alphabet_pd);

  //-- do we need to do this?
  outlet_free(x->x_labout);
  outlet_free(x->x_keyout);
}

/*--------------------------------------------------------------------
 * pd_gfsm_alphabet_obj: set
 */
static void pd_gfsm_alphabet_obj_alphabet(t_pd_gfsm_alphabet_obj *x, t_symbol *name)
{
#ifdef ALPHABET_DEBUG
  post("pd_gfsm_alphabet_obj_alphabet() called ; oldname=%p=%s ; newname=%p=%s",
       x->x_alphabet_pd->x_name, x->x_alphabet_pd->x_name->s_name, name, name->s_name);
#endif
  if (name == x->x_alphabet_pd->x_name) return;
  pd_gfsm_alphabet_pd_release(x->x_alphabet_pd);

  x->x_alphabet_pd = pd_gfsm_alphabet_pd_get(name);
  ++x->x_alphabet_pd->x_refcnt;
}

/*--------------------------------------------------------------------
 * pd_gfsm_alphabet_obj: load
 */
static void pd_gfsm_alphabet_obj_load(t_pd_gfsm_alphabet_obj *x, t_symbol *s)
{
  gfsmError *errp = NULL;
  gfsm_alphabet_load_filename(x->x_alphabet_pd->x_alphabet, s->s_name, &errp);
  if (errp != NULL) {
    error("gfsm_alphabet: load %s: %s", s->s_name, errp->message);
    g_error_free(errp);
  }
}

/*--------------------------------------------------------------------
 * pd_gfsm_alphabet_obj: print
 */
static void pd_gfsm_alphabet_obj_print(t_pd_gfsm_alphabet_obj *x)
{
  gfsmError    *errp = NULL;
  gfsmIOHandle *ioh  = pd_gfsm_console_handle_new();
  gfsm_alphabet_save_handle(x->x_alphabet_pd->x_alphabet, ioh, &errp);
  if (errp != NULL) {
    error("gfsm_alphabet: print: %s", errp->message);
    g_error_free(errp);
  }
  pd_gfsm_console_handle_free(ioh);
}

/*--------------------------------------------------------------------
 * pd_gfsm_alphabet_obj: save
 */
static void pd_gfsm_alphabet_obj_save(t_pd_gfsm_alphabet_obj *x, t_symbol *s)
{
  gfsmError *errp = NULL;
  gfsm_alphabet_save_filename(x->x_alphabet_pd->x_alphabet, s->s_name, &errp);
  if (errp != NULL) {
    error("gfsm_alphabet: save %s: %s", s->s_name, errp->message);
    g_error_free(errp);
  }
}

/*--------------------------------------------------------------------
 * pd_gfsm_alphabet_obj: clear
 */
static void pd_gfsm_alphabet_obj_clear(t_pd_gfsm_alphabet_obj *x)
{
  gfsm_alphabet_clear(x->x_alphabet_pd->x_alphabet);
}


/*--------------------------------------------------------------------
 * pd_gfsm_alphabet_obj: outlet_pair
 */
static void pd_gfsm_alphabet_obj_outlet_pair(t_pd_gfsm_alphabet_obj *x, t_float lab, t_atom *a)
{
  if (a) {
    switch (a->a_type) {
    case A_SYMBOL:
      outlet_symbol(x->x_keyout, a->a_w.w_symbol);
      break;
    case A_FLOAT:
      outlet_float(x->x_keyout, a->a_w.w_float);
      break;
    default:
      outlet_symbol(x->x_keyout, atom_getsymbol(a));
      break;
    }
  } else {
    outlet_bang(x->x_keyout);
  }

  if (lab != gfsmNoLabel) outlet_float(x->x_obj.ob_outlet, lab);
  else                    outlet_bang(x->x_obj.ob_outlet);
}

/*--------------------------------------------------------------------
 * pd_gfsm_alphabet_obj: insert
 */
static void pd_gfsm_alphabet_obj_insert(t_pd_gfsm_alphabet_obj *x, GFSM_UNUSED t_symbol *s, int argc, t_atom *argv)
{
  if (argc < 1) {
    error("pd_gfsm_alphabet_obj_insert(): no atom to insert?");
    return;
  }
  t_float labf = argc > 1 ? atom_getfloat(argv+1) : (t_float)gfsmNoLabel;

  gfsm_alphabet_get_full(x->x_alphabet_pd->x_alphabet, argv, (gfsmLabelVal)labf);
}

/*--------------------------------------------------------------------
 * pd_gfsm_alphabet_obj: atom2label
 */
static void pd_gfsm_alphabet_obj_atom2label(t_pd_gfsm_alphabet_obj *x, GFSM_UNUSED t_symbol *s, int argc, t_atom *argv)
{
  if (argc < 1) {
    error("pd_gfsm_alphabet_obj_atom2label(): no arguments?");
    return;
  }
  t_float labf = (t_float)(gfsm_alphabet_find_label(x->x_alphabet_pd->x_alphabet, argv));

  pd_gfsm_alphabet_obj_outlet_pair(x, labf, argv);
}

/*--------------------------------------------------------------------
 * pd_gfsm_alphabet_obj: atom2label_force
 */
static void pd_gfsm_alphabet_obj_atom2label_force(t_pd_gfsm_alphabet_obj *x, GFSM_UNUSED t_symbol *s, int argc, t_atom *argv)
{
  if (argc < 1) {
    error("pd_gfsm_alphabet_obj_atom2label_force(): no arguments?");
    return;
  }
  t_float labf = (t_float)(gfsm_alphabet_get_label(x->x_alphabet_pd->x_alphabet, argv));

  pd_gfsm_alphabet_obj_outlet_pair(x, labf, argv);
}

/*--------------------------------------------------------------------
 * pd_gfsm_alphabet_obj: label2atom
 */
static void pd_gfsm_alphabet_obj_label2atom(t_pd_gfsm_alphabet_obj *x, t_float labf)
{
  t_atom *key = (t_atom*)gfsm_alphabet_find_key(x->x_alphabet_pd->x_alphabet, (gfsmLabelVal)labf);

  if (key==NULL) pd_gfsm_alphabet_obj_outlet_pair(x, labf, NULL);
  else           pd_gfsm_alphabet_obj_outlet_pair(x, labf, key);
}

/*--------------------------------------------------------------------
 * pd_gfsm_alphabet_obj: label2atom_force
 */
static void pd_gfsm_alphabet_obj_label2atom_force(t_pd_gfsm_alphabet_obj *x, t_float labf)
{
  t_atom *key = (t_atom*)gfsm_alphabet_find_key(x->x_alphabet_pd->x_alphabet,
						(gfsmLabelVal)labf);
  if (key==NULL) {
    t_atom labatom;
    SETFLOAT(&labatom, labf);
    gfsm_alphabet_insert(x->x_alphabet_pd->x_alphabet, &labatom, (gfsmLabelVal)labf);
    pd_gfsm_alphabet_obj_outlet_pair(x, labf, &labatom);
  }
  else {
    pd_gfsm_alphabet_obj_outlet_pair(x, labf, key);
  }
}

/*--------------------------------------------------------------------
 * pd_gfsm_alphabet_obj: rmatom()
 */
static void pd_gfsm_alphabet_obj_rmatom(t_pd_gfsm_alphabet_obj *x, GFSM_UNUSED t_symbol *sel, int argc, t_atom *argv)
{
  if (argc < 1) return;
  gfsm_alphabet_remove_key(x->x_alphabet_pd->x_alphabet, argv);
}

/*--------------------------------------------------------------------
 * pd_gfsm_alphabet_obj: rmlabel()
 */
static void pd_gfsm_alphabet_obj_rmlabel(t_pd_gfsm_alphabet_obj *x, t_float labf)
{
  gfsm_alphabet_remove_label(x->x_alphabet_pd->x_alphabet, (gfsmLabelVal)labf);
}


/*--------------------------------------------------------------------
 * pd_gfsm_alphabet: setup
 */
void pd_gfsm_alphabet_setup(void)
{
  //-- pd_gfsm_alphabet_pd
  pd_gfsm_alphabet_pd_setup();

  //-- class
  pd_gfsm_alphabet_obj_class = class_new(gensym("gfsm_alphabet"),
			     (t_newmethod)pd_gfsm_alphabet_obj_new,
			     (t_method)pd_gfsm_alphabet_obj_free,
			     sizeof(t_pd_gfsm_alphabet_obj),
			     CLASS_DEFAULT,
			     A_DEFSYM,
			     A_NULL);

  //-- methods: I/O
  class_addmethod(pd_gfsm_alphabet_obj_class,
		  (t_method)pd_gfsm_alphabet_obj_print,
		  gensym("print"),
		  A_NULL);
  class_addmethod(pd_gfsm_alphabet_obj_class,
		  (t_method)pd_gfsm_alphabet_obj_load,
		  gensym("load"),
		  A_SYMBOL,
		  A_NULL);
  class_addmethod(pd_gfsm_alphabet_obj_class,
		  (t_method)pd_gfsm_alphabet_obj_save,
		  gensym("save"),
		  A_SYMBOL,
		  A_NULL);


  //-- methods: whole-object manipulation
  class_addmethod(pd_gfsm_alphabet_obj_class,
		  (t_method)pd_gfsm_alphabet_obj_clear,
		  gensym("clear"),
		  A_NULL);
  class_addmethod(pd_gfsm_alphabet_obj_class,
		  (t_method)pd_gfsm_alphabet_obj_alphabet,
		  gensym("alphabet"),
		  A_DEFSYM,
		  A_NULL);


  //-- methods: insert
  class_addmethod(pd_gfsm_alphabet_obj_class,
		  (t_method)pd_gfsm_alphabet_obj_insert,
		  gensym("insert"),
		  A_GIMME,
		  A_NULL);

  //-- methods: safe access: atom->label
  class_addmethod(pd_gfsm_alphabet_obj_class,
		  (t_method)pd_gfsm_alphabet_obj_atom2label,
		  gensym("atom2char"),
		  A_GIMME,
		  A_NULL);
  /*
  class_addmethod(pd_gfsm_alphabet_obj_class,
		  (t_method)pd_gfsm_alphabet_obj_atom2label,
		  gensym("a2c"),
		  A_GIMME,
		  A_NULL);
  */

  //-- methods: destructive access: atom->label
  class_addmethod(pd_gfsm_alphabet_obj_class,
		  (t_method)pd_gfsm_alphabet_obj_atom2label_force,
		  gensym("atom2char!"),
		  A_GIMME,
		  A_NULL);
  /*
  class_addmethod(pd_gfsm_alphabet_obj_class,
		  (t_method)pd_gfsm_alphabet_obj_atom2label_force,
		  gensym("a2c!"),
		  A_GIMME,
		  A_NULL);
  */


  //-- methods: safe access: label->atom
  class_addmethod(pd_gfsm_alphabet_obj_class,
		  (t_method)pd_gfsm_alphabet_obj_label2atom,
		  gensym("char2atom"),
		  A_FLOAT,
		  A_NULL);
  /*
  class_addmethod(pd_gfsm_alphabet_obj_class,
		  (t_method)pd_gfsm_alphabet_obj_label2atom,
		  gensym("c2a"),
		  A_FLOAT,
		  A_NULL);
  */

  //-- methods: destructive access: label->atom
  class_addmethod(pd_gfsm_alphabet_obj_class,
		  (t_method)pd_gfsm_alphabet_obj_label2atom_force,
		  gensym("char2atom!"),
		  A_FLOAT,
		  A_NULL);
  /*
  class_addmethod(pd_gfsm_alphabet_obj_class,
		  (t_method)pd_gfsm_alphabet_obj_label2atom_force,
		  gensym("c2a!"),
		  A_FLOAT,
		  A_NULL);
  */

  //-- methods: removal
  class_addmethod(pd_gfsm_alphabet_obj_class,
		  (t_method)pd_gfsm_alphabet_obj_rmatom,
		  gensym("rmatom"),
		  A_GIMME,
		  A_NULL);
  class_addmethod(pd_gfsm_alphabet_obj_class,
		  (t_method)pd_gfsm_alphabet_obj_rmlabel,
		  gensym("rmchar"),
		  A_FLOAT,
		  A_NULL);
  

  //-- help symbol
  class_sethelpsymbol(pd_gfsm_alphabet_obj_class, gensym("gfsm_alphabet-help.pd"));
}
