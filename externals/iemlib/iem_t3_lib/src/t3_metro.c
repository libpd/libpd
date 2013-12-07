/* For information on usage and redistribution, and for a DISCLAIMER OF ALL
* WARRANTIES, see the file, "LICENSE.txt," in this distribution.

iem_t3_lib written by Gerhard Eckel, Thomas Musil, Copyright (c) IEM KUG Graz Austria 2000 - 2006 */


#include "m_pd.h"
#include "iemlib.h"

/* ------------------------ t3_metro ---------------------------- */
static t_class *t3_metro_class;

typedef struct _t3_metro
{
  t_object x_obj;
  t_clock  *x_clock;
  double   x_metrotime;
  double   x_ticks2ms;
  double   x_t3_bang;
  int      x_hit;
  void     *x_out_next;
} t_t3_metro;

static void t3_metro_stop(t_t3_metro *x)
{
  clock_unset(x->x_clock);
}

static void t3_metro_tick(t_t3_metro *x)
{
  double dticks;
  int iticks;
  
  x->x_hit = 0;
  outlet_float(x->x_out_next, x->x_metrotime);
  outlet_float(x->x_obj.ob_outlet, x->x_t3_bang);
  dticks = (x->x_metrotime + x->x_t3_bang)/x->x_ticks2ms;
  iticks = (int)dticks;
  x->x_t3_bang = (dticks - (double)iticks)*x->x_ticks2ms;
  if(!x->x_hit)
    clock_delay(x->x_clock, (double)iticks*x->x_ticks2ms);
}

static void t3_metro_float(t_t3_metro *x, t_floatarg t3_bang)
{
  double dticks;
  int iticks;
  
  if(t3_bang < 0)
    t3_bang = 0;
  dticks = (double)t3_bang/x->x_ticks2ms;
  iticks = (int)dticks;
  x->x_t3_bang = (dticks - (double)iticks)*x->x_ticks2ms;
  clock_delay(x->x_clock, (double)iticks*x->x_ticks2ms);
  x->x_hit = 1;
}

static void t3_metro_start(t_t3_metro *x, t_floatarg f)
{
  t3_metro_float(x, f);
  x->x_hit = 1;
}

static void t3_metro_ft1(t_t3_metro *x, t_floatarg f)
{
  if(f < 0.01) f = 0.01;
  x->x_metrotime = (double)f;
}

static void t3_metro_list(t_t3_metro *x, t_symbol *s, int ac, t_atom *av)
{
  if((ac == 2)&&IS_A_FLOAT(av,0)&&IS_A_FLOAT(av,1))
  {
    t3_metro_ft1(x, atom_getfloatarg(1, ac, av));
    t3_metro_float(x, atom_getfloatarg(0, ac, av));
  }
}

static void t3_metro_free(t_t3_metro *x)
{
  clock_free(x->x_clock);
}

static void *t3_metro_new(t_symbol *s, int ac, t_atom *av)
{
  t_t3_metro *x = (t_t3_metro *)pd_new(t3_metro_class);
  
  x->x_metrotime = 10.0;
  x->x_t3_bang = 0.0;
  x->x_hit = 0;
  if((ac == 1)&&IS_A_FLOAT(av,0))
  {
    t3_metro_ft1(x, atom_getfloatarg(0, ac, av));
  }
  x->x_ticks2ms = 1000.0*(double)sys_getblksize()/(double)sys_getsr();
  x->x_clock = clock_new(x, (t_method)t3_metro_tick);
  outlet_new(&x->x_obj, &s_float);
  x->x_out_next = outlet_new(&x->x_obj, &s_float);
  inlet_new(&x->x_obj, &x->x_obj.ob_pd, gensym("float"), gensym("ft1"));
  return (x);
}

void t3_metro_setup(void)
{
  t3_metro_class = class_new(gensym("t3_metro"), (t_newmethod)t3_metro_new,
    (t_method)t3_metro_free, sizeof(t_t3_metro), 0, A_GIMME, 0);
  class_addmethod(t3_metro_class, (t_method)t3_metro_stop, gensym("stop"), 0);
  class_addmethod(t3_metro_class, (t_method)t3_metro_start, gensym("start"), A_FLOAT, 0);
  class_addmethod(t3_metro_class, (t_method)t3_metro_ft1, gensym("ft1"), A_FLOAT, 0);
  class_addfloat(t3_metro_class, (t_method)t3_metro_float);
  class_addlist(t3_metro_class, (t_method)t3_metro_list);
//  class_sethelpsymbol(t3_metro_class, gensym("iemhelp/help-t3_metro"));
}
