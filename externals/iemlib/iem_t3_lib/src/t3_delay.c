/* For information on usage and redistribution, and for a DISCLAIMER OF ALL
* WARRANTIES, see the file, "LICENSE.txt," in this distribution.

iem_t3_lib written by Gerhard Eckel, Thomas Musil, Copyright (c) IEM KUG Graz Austria 2000 - 2006 */

#include "m_pd.h"
#include "iemlib.h"

/* ------------------------ t3_delay ---------------------------- */
static t_class *t3_delay_class;

typedef struct _t3_delay
{
  t_object x_obj;
  t_clock  *x_clock;
  double   x_deltime;
  double   x_ticks2ms;
  double   x_t3_bang;
} t_t3_delay;

static void t3_delay_tick(t_t3_delay *x)
{
  outlet_float(x->x_obj.ob_outlet, x->x_t3_bang);
}

static void t3_delay_stop(t_t3_delay *x)
{
  clock_unset(x->x_clock);
}

static void t3_delay_float(t_t3_delay *x, t_floatarg t3_bang)
{
  double dticks;
  int iticks;
  
  if(t3_bang < 0)
    t3_bang = 0;
  dticks = (x->x_deltime + (double)t3_bang)/x->x_ticks2ms;
  iticks = (int)dticks;
  x->x_t3_bang = (dticks - (double)iticks)*x->x_ticks2ms;
  clock_delay(x->x_clock, (double)iticks*x->x_ticks2ms);
}

static void t3_delay_ft1(t_t3_delay *x, t_floatarg f)
{
  if(f < 0)
    f = 0;
  x->x_deltime = f;
}

static void t3_delay_list(t_t3_delay *x, t_symbol *s, int ac, t_atom *av)
{
  if((ac == 2)&&IS_A_FLOAT(av,0)&&IS_A_FLOAT(av,1))
  {
    t3_delay_ft1(x, atom_getfloatarg(1, ac, av));
    t3_delay_float(x, atom_getfloatarg(0, ac, av));
  }
}

static void t3_delay_free(t_t3_delay *x)
{
  clock_free(x->x_clock);
}

static void *t3_delay_new(t_floatarg f)
{
  t_t3_delay *x = (t_t3_delay *)pd_new(t3_delay_class);
  
  x->x_ticks2ms = 1000.0*(double)sys_getblksize()/(double)sys_getsr();
  t3_delay_ft1(x, f);
  x->x_clock = clock_new(x, (t_method)t3_delay_tick);
  outlet_new(&x->x_obj, &s_float);
  inlet_new(&x->x_obj, &x->x_obj.ob_pd, gensym("float"), gensym("ft1"));
  return (x);
}

void t3_delay_setup(void)
{
  t3_delay_class = class_new(gensym("t3_delay"), (t_newmethod)t3_delay_new,
    (t_method)t3_delay_free, sizeof(t_t3_delay), 0, A_DEFFLOAT, 0);
  class_addcreator((t_newmethod)t3_delay_new, gensym("t3_del"), A_DEFFLOAT, 0);
  class_addmethod(t3_delay_class, (t_method)t3_delay_stop, gensym("stop"), 0);
  class_addmethod(t3_delay_class, (t_method)t3_delay_ft1,
    gensym("ft1"), A_FLOAT, 0);
  class_addfloat(t3_delay_class, (t_method)t3_delay_float);
  class_addlist(t3_delay_class, (t_method)t3_delay_list);
//  class_sethelpsymbol(t3_delay_class, gensym("iemhelp/help-t3_delay"));
}
