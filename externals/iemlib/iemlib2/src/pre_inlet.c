/* For information on usage and redistribution, and for a DISCLAIMER OF ALL
* WARRANTIES, see the file, "LICENSE.txt," in this distribution.

iemlib2 written by Thomas Musil, Copyright (c) IEM KUG Graz Austria 2000 - 2006 */

#include "m_pd.h"
#include "iemlib.h"


/* ------------------------ pre_inlet ---------------------------- */
/* --------- any incoming message produce additionally a --------- */
/* ----- special anything message at output, followed by the ----- */
/* --- original message. this pre message contains a selector ---- */
/* ---- symbol made of one character of the m-th entry of the ---- */
/* --- ASCII-table, followed by one float atom list entry with --- */
/* ---- with value n. "m" and "n" are the 2 initial arguments ---- */

static t_class *pre_inlet_class;

typedef struct _pre_inlet
{
  t_object    x_obj;
  t_atom      x_at;
  t_symbol    *x_sym;
} t_pre_inlet;

static void pre_inlet_bang(t_pre_inlet *x)
{
  outlet_anything(x->x_obj.ob_outlet, x->x_sym, 1, &x->x_at);
  outlet_bang(x->x_obj.ob_outlet);
}

static void pre_inlet_float(t_pre_inlet *x, t_floatarg f)
{
  outlet_anything(x->x_obj.ob_outlet, x->x_sym, 1, &x->x_at);
  outlet_float(x->x_obj.ob_outlet, f);
}

static void pre_inlet_symbol(t_pre_inlet *x, t_symbol *s)
{
  outlet_anything(x->x_obj.ob_outlet, x->x_sym, 1, &x->x_at);
  outlet_symbol(x->x_obj.ob_outlet, s);
}

static void pre_inlet_pointer(t_pre_inlet *x, t_gpointer *gp)
{
  outlet_anything(x->x_obj.ob_outlet, x->x_sym, 1, &x->x_at);
  outlet_pointer(x->x_obj.ob_outlet, gp);
}

static void pre_inlet_list(t_pre_inlet *x, t_symbol *s, int ac, t_atom *av)
{
  outlet_anything(x->x_obj.ob_outlet, x->x_sym, 1, &x->x_at);
  outlet_list(x->x_obj.ob_outlet, s, ac, av);
}

static void pre_inlet_anything(t_pre_inlet *x, t_symbol *s, int ac, t_atom *av)
{
  outlet_anything(x->x_obj.ob_outlet, x->x_sym, 1, &x->x_at);
  outlet_anything(x->x_obj.ob_outlet, s, ac, av);
}

static void pre_inlet_free(t_pre_inlet *x)
{
}

static void *pre_inlet_new(t_floatarg fsym, t_floatarg finlet)
{
  t_pre_inlet *x = (t_pre_inlet *)pd_new(pre_inlet_class);
  char str[2];
  
  SETFLOAT(&x->x_at, finlet);
  str[0] = (char)((int)(fsym)&0xff);
  str[1] = 0;
  x->x_sym = gensym(str);
  outlet_new(&x->x_obj, &s_list);
  return (x);
}

void pre_inlet_setup(void)
{
  pre_inlet_class = class_new(gensym("pre_inlet"), (t_newmethod)pre_inlet_new,
    (t_method)pre_inlet_free, sizeof(t_pre_inlet), 0, A_DEFFLOAT, A_DEFFLOAT, 0);
  class_addbang(pre_inlet_class, (t_method)pre_inlet_bang);
  class_addfloat(pre_inlet_class, (t_method)pre_inlet_float);
  class_addsymbol(pre_inlet_class, pre_inlet_symbol);
  class_addpointer(pre_inlet_class, pre_inlet_pointer);
  class_addlist(pre_inlet_class, pre_inlet_list);
  class_addanything(pre_inlet_class, pre_inlet_anything);
//  class_sethelpsymbol(pre_inlet_class, gensym("iemhelp/help-pre_inlet"));
}
