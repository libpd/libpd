#include "m_pd.h"
#include <math.h>

typedef struct _steady
{
  t_object x_obj;
  t_float f_min;
  t_float f_max;
  t_float f_prev;
  t_int resetflag;
  t_float f_in, f_maxjump;
  t_outlet *max, *min, *smooth;
} t_steady;

t_class *steady_class;

void steady_float (t_steady *x, t_floatarg fin)
{
  if (x->resetflag == 0)
    {
      x->f_max = fin > x->f_max ? fin : x->f_max;
      x->f_min = fin < x->f_min ? fin : x->f_min;
      outlet_float(x->smooth, fin);
      outlet_float(x->min, x->f_min);
      outlet_float(x->max, x->f_max);
      x->f_prev = fin;
      x->resetflag=1;
    }
  else
    {
      float min = fin >= x->f_prev ? x->f_prev : fin;
      float max = fin <= x->f_prev ? x->f_prev : fin;
      fin = (max - min) > x->f_maxjump ? x->f_prev : fin;
      x->f_max = fin > x->f_max ? fin : x->f_max;
      x->f_min = fin < x->f_min ? fin : x->f_min;
      float sm_fin = fabs(fin - x->f_prev) > x->f_maxjump ? x->f_prev : fin;
      outlet_float(x->smooth, sm_fin);
      outlet_float(x->min, x->f_min);
      outlet_float(x->max, x->f_max);
      x->f_prev = fin;
    }
}

void steady_bang (t_steady *x)
{
  outlet_float(x->min, x->f_min);
  outlet_float(x->max, x->f_max);
  x->f_min = 1e08;
  x->f_max = -1e08;
  x->resetflag=0;
}

void *steady_new(t_floatarg f) 
{
  t_steady *x = (t_steady *)pd_new(steady_class);
  x->f_min = 1e08;
  x->f_max = -1e08;
  x->f_maxjump = f;
  x->resetflag = 0;
  x->f_prev = 0;
  floatinlet_new(&x->x_obj, &x->f_maxjump);
  x->max = outlet_new(&x->x_obj, gensym("float"));
  x->min = outlet_new(&x->x_obj, gensym("float"));
  x->smooth = outlet_new(&x->x_obj, gensym("float"));
  return (void *)x;
}

void steady_setup(void) {
  steady_class = class_new(gensym("steady"),
  (t_newmethod)steady_new,
  0, sizeof(t_steady),
  0, A_DEFFLOAT, 0);
  post("|+++++++++++++++++>steady<------------------|");
  post("|+>max, min and through must not jump more<-|");
  post("|+++++++++>than a specified amount<---------|");
  post("|+++>edward<------->kelly<++++++++>2005<----|");

  class_addbang(steady_class, steady_bang);
  class_addfloat(steady_class, steady_float);
}
