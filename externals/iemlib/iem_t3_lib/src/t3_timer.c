/* For information on usage and redistribution, and for a DISCLAIMER OF ALL
* WARRANTIES, see the file, "LICENSE.txt," in this distribution.

iem_t3_lib written by Gerhard Eckel, Thomas Musil, Copyright (c) IEM KUG Graz Austria 2000 - 2006 */


#include "m_pd.h"
#include "iemlib.h"

/* -------------------------- t3_timer ------------------------------ */
static t_class *t3_timer_class;

typedef struct _t3_timer
{
  t_object x_obj;
  double   x_settime;
  double   x_t3_off;
} t_t3_timer;

static void t3_timer_float(t_t3_timer *x, t_floatarg t3_bang)
{
  x->x_settime = (double)clock_getsystime();
  x->x_t3_off = (double)t3_bang;
}

static void t3_timer_ft1(t_t3_timer *x, t_floatarg t3_bang)
{
  outlet_float(x->x_obj.ob_outlet, clock_gettimesince(x->x_settime)
    + (double)t3_bang - x->x_t3_off);
}

static void *t3_timer_new(void)
{
  t_t3_timer *x = (t_t3_timer *)pd_new(t3_timer_class);
  t3_timer_float(x, 0.0f);
  outlet_new(&x->x_obj, &s_float);
  inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_float, gensym("ft1"));
  return (x);
}

void t3_timer_setup(void)
{
  t3_timer_class = class_new(gensym("t3_timer"), (t_newmethod)t3_timer_new, 0,
    sizeof(t_t3_timer), 0, 0);
  class_addfloat(t3_timer_class, t3_timer_float);
  class_addmethod(t3_timer_class, (t_method)t3_timer_ft1, gensym("ft1"), A_FLOAT, 0);
//  class_sethelpsymbol(t3_timer_class, gensym("iemhelp/help-t3_timer"));
}
