#include "m_pd.h"

t_class *doubledelta_class;

typedef struct _doubledelta
{
  t_object x_obj;
  t_float f_now, f_prev, f_delta, f_delta_prev, f_doubledelta, fa;
  t_outlet *delta, *doubledelta;
} t_doubledelta;

void doubledelta_float(t_doubledelta *y, t_floatarg f)
{
  y->f_delta_prev = y->f_delta;
  y->f_prev = y->f_now;
  y->f_now = f;
  y->f_delta = y->f_now - y->f_prev;
  y->f_doubledelta = y->f_delta - y->f_delta_prev;
  outlet_float(y->doubledelta, y->f_doubledelta);
  outlet_float(y->delta, y->f_delta);
}

void doubledelta_bang(t_doubledelta *y)
{
  outlet_float(y->doubledelta, y->f_doubledelta);
  outlet_float(y->delta, y->f_delta);
}

void *doubledelta_new(t_floatarg f)
{
  t_doubledelta *y = (t_doubledelta *)pd_new(doubledelta_class);
  y->fa = f;
  y->delta = outlet_new(&y->x_obj, gensym("float"));
  y->doubledelta = outlet_new(&y->x_obj, gensym("float"));
  return(void *)y;
}

void doubledelta_setup(void) 
{
  doubledelta_class = class_new(gensym("doubledelta"),
  (t_newmethod)doubledelta_new,
  0, sizeof(t_doubledelta),
  0, A_DEFFLOAT, 0);
  post("delta & delta-delta values, <morph_2016@yahoo.co.uk>");

  class_addbang(doubledelta_class, doubledelta_bang);
  class_addfloat(doubledelta_class, doubledelta_float);
}
