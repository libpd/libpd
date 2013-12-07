/* For information on usage and redistribution, and for a DISCLAIMER OF ALL
* WARRANTIES, see the file, "LICENSE.txt," in this distribution.

iemlib2 written by Thomas Musil, Copyright (c) IEM KUG Graz Austria 2000 - 2006 */

#include "m_pd.h"
#include "iemlib.h"


/* ------------------------ exp_inc ---------------------------- */
/* ------- exponetial/linear-incremental-bang-counter ---------- */


static t_class *exp_inc_class;

typedef struct _exp_inc
{
  t_object x_obj;
  t_float  x_initialval;
  t_float  x_currentval;
  t_float  x_mulfactor;
  t_float  x_addincrement;
  t_float  x_min;
  t_float  x_max;
} t_exp_inc;

static void exp_inc_bang(t_exp_inc *x)
{                                                   
  outlet_float(x->x_obj.ob_outlet, x->x_currentval);
  x->x_currentval = x->x_currentval * x->x_mulfactor + x->x_addincrement;
  if(x->x_currentval < x->x_min)
    x->x_currentval = x->x_min;
  if(x->x_currentval > x->x_max)
    x->x_currentval = x->x_max;
}

static void exp_inc_reset(t_exp_inc *x)
{
  x->x_currentval = x->x_initialval;
  if(x->x_currentval < x->x_min)
    x->x_currentval = x->x_min;
  if(x->x_currentval > x->x_max)
    x->x_currentval = x->x_max;
}

static void exp_inc_float(t_exp_inc *x, t_floatarg f)
{
  x->x_initialval = (t_float)f;
  x->x_currentval = x->x_initialval;
  if(x->x_currentval < x->x_min)
    x->x_currentval = x->x_min;
  if(x->x_currentval > x->x_max)
    x->x_currentval = x->x_max;
}

static void exp_inc_ft1(t_exp_inc *x, t_floatarg f)
{
  x->x_mulfactor = 1.0 + 0.01*(t_float)f;
}

static void exp_inc_ft2(t_exp_inc *x, t_floatarg f)
{
  x->x_addincrement = (t_float)f;
}

static void exp_inc_ft3(t_exp_inc *x, t_floatarg f)
{
  x->x_min = (t_float)f;
  if(x->x_currentval < x->x_min)
    x->x_currentval = x->x_min;
}

static void exp_inc_ft4(t_exp_inc *x, t_floatarg f)
{
  x->x_max = (t_float)f;
  if(x->x_currentval > x->x_max)
    x->x_currentval = x->x_max;
}

static void exp_inc_list(t_exp_inc *x, t_symbol *s, int ac, t_atom *av)
{
  if((ac == 5)&&IS_A_FLOAT(av,0)&&IS_A_FLOAT(av,1)&&IS_A_FLOAT(av,2)
    &&IS_A_FLOAT(av,3)&&IS_A_FLOAT(av,4))
  {
    exp_inc_ft4(x, atom_getfloatarg(4, ac, av));
    exp_inc_ft3(x, atom_getfloatarg(3, ac, av));
    exp_inc_ft2(x, atom_getfloatarg(2, ac, av));
    exp_inc_ft1(x, atom_getfloatarg(1, ac, av));
    exp_inc_float(x, atom_getfloatarg(0, ac, av));
  }
}

static void *exp_inc_new(t_symbol *s, int ac, t_atom *av)
{
  t_exp_inc *x = (t_exp_inc *)pd_new(exp_inc_class);
  
  x->x_currentval = 10.0;
  x->x_mulfactor = 1.0;
  x->x_addincrement = 0.0;
  x->x_min = 0.0;
  x->x_max = 1000.0;
  if((ac == 5)&&IS_A_FLOAT(av,0)&&IS_A_FLOAT(av,1)&&IS_A_FLOAT(av,2)
    &&IS_A_FLOAT(av,3)&&IS_A_FLOAT(av,4))
  {
    exp_inc_ft4(x, atom_getfloatarg(4, ac, av));
    exp_inc_ft3(x, atom_getfloatarg(3, ac, av));
    exp_inc_ft2(x, atom_getfloatarg(2, ac, av));
    exp_inc_ft1(x, atom_getfloatarg(1, ac, av));
    exp_inc_float(x, atom_getfloatarg(0, ac, av));
  }
  outlet_new(&x->x_obj, &s_float);
  inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_float, gensym("ft1"));
  inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_float, gensym("ft2"));
  inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_float, gensym("ft3"));
  inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_float, gensym("ft4"));
  return (x);
}

void exp_inc_setup(void)
{
  exp_inc_class = class_new(gensym("exp_inc"), (t_newmethod)exp_inc_new,
    0, sizeof(t_exp_inc), 0, A_GIMME, 0);
  class_addbang(exp_inc_class, exp_inc_bang);
  class_addlist(exp_inc_class, (t_method)exp_inc_list);
  class_addmethod(exp_inc_class, (t_method)exp_inc_reset, gensym("reset"), 0);
  class_addfloat(exp_inc_class, (t_method)exp_inc_float);
  class_addmethod(exp_inc_class, (t_method)exp_inc_ft1, gensym("ft1"), A_FLOAT, 0);
  class_addmethod(exp_inc_class, (t_method)exp_inc_ft2, gensym("ft2"), A_FLOAT, 0);
  class_addmethod(exp_inc_class, (t_method)exp_inc_ft3, gensym("ft3"), A_FLOAT, 0);
  class_addmethod(exp_inc_class, (t_method)exp_inc_ft4, gensym("ft4"), A_FLOAT, 0);
//  class_sethelpsymbol(exp_inc_class, gensym("iemhelp/help-exp_inc"));
}
