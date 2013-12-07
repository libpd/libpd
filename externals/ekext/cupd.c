#include "m_pd.h"

t_class *cupd_class;

typedef struct _cupd
{
  t_object x_obj;
  t_int f_count;
  t_float f_dir, f_prevdir, firstbang, floatset;
  t_outlet *count;
} t_cupd;

void cupd_float(t_cupd *y, t_floatarg f)
{
  y->f_count = f;
  y->floatset = 1;
}

void cupd_bang(t_cupd *y)
{
  int flag = y->f_dir != y->f_prevdir ? 1 : 0;
  if (flag==1&&y->firstbang==0&&y->floatset==0)
    {
      y->f_count += y->f_dir == 0 ? 2 : -2;
      outlet_float(y->count, y->f_count);
      y->f_count += y->f_dir == 0 ? 1 : -1;
    }
  else
    {
      outlet_float(y->count, y->f_count);
      y->f_count += y->f_dir == 0 ? 1 : -1;
      y->firstbang = y->floatset = 0;
    }
  y->f_prevdir = y->f_dir;
}

void cupd_setbang(t_cupd *y, t_floatarg f)
{
  y->f_count = f;
  outlet_float(y->count, y->f_count);
  y->f_count += y->f_dir == 0 ? 1 : -1;
  y->firstbang = y->floatset = 0;
  y->f_prevdir = y->f_dir;
}

void *cupd_new(t_floatarg f)
{
  t_cupd *y = (t_cupd *)pd_new(cupd_class);
  y->f_dir = f;
  y->f_count = 0;
  y->firstbang = 1;
  y->floatset = 0;
  floatinlet_new(&y->x_obj, &y->f_dir);
  y->count = outlet_new(&y->x_obj, gensym("float"));
  return(void *)y;
}

void cupd_setup(void) 
{
  cupd_class = class_new(gensym("cupd"),
  (t_newmethod)cupd_new,
  0, sizeof(t_cupd),
  0, A_DEFFLOAT, 0);
  post("cupd counts up ^_^ and down _^_");

  class_addbang(cupd_class, cupd_bang);
  class_addfloat(cupd_class, cupd_float);
  class_addmethod(cupd_class, (t_method)cupd_setbang, gensym("setbang"), A_DEFFLOAT, 0);}
