#include "m_pd.h"

t_class *cup_class;

typedef struct _cup
{
  t_object x_obj;
  t_int f_count, fa;
  t_outlet *count;
} t_cup;

void cup_float(t_cup *y, t_floatarg f)
{
  y->f_count = f;
}

void cup_bang(t_cup *y)
{
  outlet_float(y->count, y->f_count);
  y->f_count += 1;
}

void cup_setbang(t_cup *y, t_floatarg f)
{
  y->f_count = f;
  outlet_float(y->count, y->f_count);
  y->f_count += 1;
}

void *cup_new(t_floatarg f)
{
  t_cup *y = (t_cup *)pd_new(cup_class);
  y->fa = f;
  y->f_count = 0;
  y->count = outlet_new(&y->x_obj, gensym("float"));
  return(void *)y;
}

void cup_setup(void) 
{
  cup_class = class_new(gensym("cup"),
  (t_newmethod)cup_new,
  0, sizeof(t_cup),
  0, A_DEFFLOAT, 0);
  post("cup counts up ^_^");

  class_addbang(cup_class, cup_bang);
  class_addfloat(cup_class, cup_float);
  class_addmethod(cup_class, (t_method)cup_setbang, gensym("setbang"), A_DEFFLOAT, 0);}
