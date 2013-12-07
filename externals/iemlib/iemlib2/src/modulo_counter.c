/* For information on usage and redistribution, and for a DISCLAIMER OF ALL
* WARRANTIES, see the file, "LICENSE.txt," in this distribution.

iemlib2 written by Thomas Musil, Copyright (c) IEM KUG Graz Austria 2000 - 2006 */

#include "m_pd.h"
#include "iemlib.h"

/* ---------------- modulo-counter. ----------------- */
/* -------- counter increments if input a bang ------ */
/* -------- output is a modulo function. ------------ */

static t_class *modulo_counter_class;

typedef struct _modulo_counter
{
  t_object x_obj;
  int      x_max;
  int      x_cur;
} t_modulo_counter;

static void modulo_counter_bang(t_modulo_counter *x)
{
  outlet_float(x->x_obj.ob_outlet, x->x_cur++);
  x->x_cur = x->x_cur % x->x_max;
}

static void modulo_counter_float(t_modulo_counter *x, t_floatarg max)
{
  int i = (int)max;
  
  if(i < 1)
    i = 1;
  if(x->x_cur >= i)
    x->x_cur = i - 1;
  x->x_max = i;
}

static void modulo_counter_ft1(t_modulo_counter *x, t_floatarg set_init)
{
  int i = (int)set_init;
  
  if(i < 0)
    i = 0;
  if(i >= x->x_max)
    i = x->x_max - 1;
  x->x_cur = i;
}

static void *modulo_counter_new(t_symbol *s, int ac, t_atom *av)
{
  t_modulo_counter *x = (t_modulo_counter *)pd_new(modulo_counter_class);
  int max = 1, cur = 0;
  
  if((ac > 0) && IS_A_FLOAT(av, 0))
    max = atom_getintarg(0, ac, av);
  if((ac > 1) && IS_A_FLOAT(av, 1))
    cur = atom_getintarg(1, ac, av);
  if(max < 1)
    x->x_max = 1;
  else
    x->x_max = max;
  if(cur < 0)
    cur = 0;
  if(cur >= x->x_max)
    cur = x->x_max - 1;
  x->x_cur = cur;
  outlet_new(&x->x_obj, &s_float);
  inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_float, gensym("ft1"));
  return (x);
}

void modulo_counter_setup(void)
{
  modulo_counter_class = class_new(gensym("modulo_counter"),
    (t_newmethod)modulo_counter_new, 0,
    sizeof(t_modulo_counter), 0, A_GIMME, 0);
  class_addbang(modulo_counter_class, (t_method)modulo_counter_bang);
  class_addfloat(modulo_counter_class, (t_method)modulo_counter_float);
  class_addmethod(modulo_counter_class, (t_method)modulo_counter_ft1, gensym("ft1"), A_FLOAT, 0);
//  class_sethelpsymbol(modulo_counter_class, gensym("iemhelp/help-modulo_counter"));
}
