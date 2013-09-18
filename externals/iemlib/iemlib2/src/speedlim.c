/* For information on usage and redistribution, and for a DISCLAIMER OF ALL
* WARRANTIES, see the file, "LICENSE.txt," in this distribution.

iemlib2 written by Thomas Musil, Copyright (c) IEM KUG Graz Austria 2000 - 2008 */


#include "m_pd.h"
#include "iemlib.h"

/* ----------------------- speedlim -------------------------- */
/* -- reduces the flow of float-messages to one message per -- */
/* ----------------- initial argument time in ms ------------- */

static t_class *speedlim_class;

typedef struct _speedlim
{
  t_object  x_obj;
  t_clock   *x_clock;
  float     x_delay;
  int       x_output_is_locked;
  int       x_there_was_n_event;
  t_float   x_curval;
} t_speedlim;

static void speedlim_stop(t_speedlim *x)
{
  x->x_output_is_locked = 0;
  x->x_there_was_n_event = 0;
  clock_unset(x->x_clock);
}

static void speedlim_tick(t_speedlim *x)
{
  if(x->x_there_was_n_event)
  {
    x->x_output_is_locked = 1;
    x->x_there_was_n_event = 0;
    outlet_float(x->x_obj.ob_outlet, x->x_curval);
    clock_delay(x->x_clock, x->x_delay);
  }
  else
  {
    x->x_output_is_locked = 0;
    x->x_there_was_n_event = 0;
  }
}

static void speedlim_float(t_speedlim *x, t_floatarg val)
{
  x->x_curval = val;
  if(!x->x_output_is_locked)
  {
    x->x_output_is_locked = 1;
    x->x_there_was_n_event = 0;
    outlet_float(x->x_obj.ob_outlet, x->x_curval);
    clock_delay(x->x_clock, x->x_delay);
  }
  else
    x->x_there_was_n_event = 1;
}

static void speedlim_ft1(t_speedlim *x, t_floatarg delay)
{
  if(delay < 0.0)
    delay = 0.0;
  x->x_delay = delay;
}

static void speedlim_free(t_speedlim *x)
{
  clock_free(x->x_clock);
}

static void *speedlim_new(t_floatarg delay)
{
  t_speedlim *x = (t_speedlim *)pd_new(speedlim_class);
  
  if(delay < 0.0)
    delay = 0.0;
  x->x_delay = delay;
  x->x_curval = 0.0f;
  x->x_output_is_locked = 0;
  x->x_there_was_n_event = 0;
  x->x_clock = clock_new(x, (t_method)speedlim_tick);
  outlet_new(&x->x_obj, &s_float);
  inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_float, gensym("ft1"));
  return (x);
}

void speedlim_setup(void)
{
  speedlim_class = class_new(gensym("speedlim"), (t_newmethod)speedlim_new,
    (t_method)speedlim_free, sizeof(t_speedlim), 0, A_DEFFLOAT, 0);
  class_addmethod(speedlim_class, (t_method)speedlim_stop, gensym("stop"), 0);
  class_addfloat(speedlim_class, (t_method)speedlim_float);
  class_addmethod(speedlim_class, (t_method)speedlim_ft1, gensym("ft1"), A_FLOAT, 0);
//  class_sethelpsymbol(speedlim_class, gensym("iemhelp/help-speedlim"));
}
