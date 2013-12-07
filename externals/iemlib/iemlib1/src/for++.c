/* For information on usage and redistribution, and for a DISCLAIMER OF ALL
* WARRANTIES, see the file, "LICENSE.txt," in this distribution.

iemlib1 written by Thomas Musil, Copyright (c) IEM KUG Graz Austria 2000 - 2006 */


#include "m_pd.h"
#include "iemlib.h"

/* ----------------------------- for++ -------------------------------- */
/* -- an internal timed counter (start-, stop-number and metro-time) -- */

typedef struct _forpp
{
  t_object  x_obj;
  int       x_beg;
  int       x_end;
  t_float   x_delay;
  int       x_cur;
  int       x_incr;
  void      *x_out_end;
  void      *x_clock;
  void      *x_clock2;
} t_forpp;

static t_class *forpp_class;

static void forpp_tick2(t_forpp *x)
{
  outlet_bang(x->x_out_end);
  clock_unset(x->x_clock2);
}

static void forpp_tick(t_forpp *x)
{
  outlet_float(x->x_obj.ob_outlet, x->x_cur);
  x->x_cur += x->x_incr;
  if(x->x_incr > 0)
  {
    if(x->x_cur <= x->x_end)
      clock_delay(x->x_clock, x->x_delay);
    else
    {
      clock_unset(x->x_clock);
      clock_delay(x->x_clock2, x->x_delay);
    }
  }
  else
  {
    if(x->x_cur >= x->x_end)
      clock_delay(x->x_clock, x->x_delay);
    else
    {
      clock_unset(x->x_clock);
      clock_delay(x->x_clock2, x->x_delay);
    }
  }
}

static void forpp_bang(t_forpp *x)
{
  x->x_cur = x->x_beg;
  outlet_float(x->x_obj.ob_outlet, x->x_cur);
  x->x_cur += x->x_incr;
  if(x->x_incr > 0)
  {
    if(x->x_cur <= x->x_end)
      clock_delay(x->x_clock, x->x_delay);
    else
    {
      clock_unset(x->x_clock);
      clock_delay(x->x_clock2, x->x_delay);
    }
  }
  else
  {
    if(x->x_cur >= x->x_end)
      clock_delay(x->x_clock, x->x_delay);
    else
    {
      clock_unset(x->x_clock);
      clock_delay(x->x_clock2, x->x_delay);
    }
  }
  
}

static void forpp_start(t_forpp *x)
{
  forpp_bang(x);
}

static void forpp_stop(t_forpp *x)
{
  if(x->x_incr > 0)
    x->x_cur = x->x_end + 1;
  else
    x->x_cur = x->x_end - 1;
  clock_unset(x->x_clock);
  clock_unset(x->x_clock2);
}

static void forpp_float(t_forpp *x, t_floatarg beg)
{
  x->x_beg = (int)beg;
  if(x->x_end < x->x_beg)
    x->x_incr = -1;
  else
    x->x_incr = 1;
}

static void forpp_ft1(t_forpp *x, t_floatarg end)
{
  x->x_end = (int)end;
  if(x->x_end < x->x_beg)
    x->x_incr = -1;
  else
    x->x_incr = 1;
}

static void forpp_ft2(t_forpp *x, t_floatarg delay)
{
  if(delay < 0.0)
    delay = 0.0;
  x->x_delay = delay;
}

static void forpp_list(t_forpp *x, t_symbol *s, int argc, t_atom *argv)
{
  if(argc == 2)
  {
    forpp_float(x, atom_getfloatarg(0, argc, argv));
    forpp_ft1(x, atom_getfloatarg(1, argc, argv));
  }
  else if(argc == 3)
  {
    forpp_float(x, atom_getfloatarg(0, argc, argv));
    forpp_ft1(x, atom_getfloatarg(1, argc, argv));
    forpp_ft2(x, atom_getfloatarg(2, argc, argv));
  }
}

static void *forpp_new(t_floatarg beg, t_floatarg end, t_floatarg delay)
{
  t_forpp *x = (t_forpp *)pd_new(forpp_class);
  
  inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_float, gensym("ft1"));
  inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_float, gensym("ft2"));
  outlet_new(&x->x_obj, &s_float);
  x->x_out_end = outlet_new(&x->x_obj, &s_bang);
  x->x_clock = clock_new(x, (t_method)forpp_tick);
  x->x_clock2 = clock_new(x, (t_method)forpp_tick2);
  x->x_beg = (int)beg;
  x->x_end = (int)end;
  if(x->x_end < x->x_beg)
    x->x_incr = -1;
  else
    x->x_incr = 1;
  if(delay < 0.0)
    delay = 0.0;
  x->x_delay = delay;
  x->x_cur = x->x_beg;
  return(x);
}

static void forpp_ff(t_forpp *x)
{
  clock_free(x->x_clock);
  clock_free(x->x_clock2);
}

void forpp_setup(void)
{
  forpp_class = class_new(gensym("for++"), (t_newmethod)forpp_new,
    (t_method)forpp_ff, sizeof(t_forpp),
    0, A_DEFFLOAT, A_DEFFLOAT, A_DEFFLOAT, 0);
  class_addcreator((t_newmethod)forpp_new, gensym("for_pp"), A_DEFFLOAT, A_DEFFLOAT, A_DEFFLOAT, 0);

  class_addbang(forpp_class, forpp_bang);
  class_addfloat(forpp_class, forpp_float);
  class_addlist(forpp_class, forpp_list);
  class_addmethod(forpp_class, (t_method)forpp_start, gensym("start"), 0);
  class_addmethod(forpp_class, (t_method)forpp_stop, gensym("stop"), 0);
  class_addmethod(forpp_class, (t_method)forpp_ft1, gensym("ft1"), A_FLOAT, 0);
  class_addmethod(forpp_class, (t_method)forpp_ft2, gensym("ft2"), A_FLOAT, 0);
//  class_sethelpsymbol(forpp_class, gensym("iemhelp/help-for++"));
}

void setup_for0x2b0x2b(void)
{
    forpp_setup();
}
