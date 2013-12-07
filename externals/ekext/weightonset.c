#include "m_pd.h"
#include <math.h>

typedef struct _datachunk
{
  t_atom data[1024];
} t_datachunk;

typedef struct _weightonset
{
  t_object x_obj;
  t_float multiplier, accumulator, divider, divaccum, weighted, increment;
  t_float length, attack;
  t_datachunk x_datachunk;
  t_outlet *waverage, *datachunk;
} t_weightonset;

t_class *weightonset_class;

void weightonset_float (t_weightonset *x, t_floatarg fin)
{
  if(x->length < 1024)
    {
      int len = (int)x->length;
      SETFLOAT(&x->x_datachunk.data[len], fin);
      if(x->length < x->attack)
	{
	  int i;
	  float weight = 1;
	  x->increment = x->length - x->attack;
	  for(i=0;i<x->increment;i++)
	    {
	      weight = weight*x->multiplier;
	    }
	  x->accumulator += fin*weight; 
	  x->divaccum += weight;
	  x->weighted = x->accumulator / x->divaccum;
	  outlet_float(x->waverage, x->weighted);
	}
      else if(x->length == x->attack)
	{
	  x->divider = 1;
	  x->divaccum += 1;
	  x->accumulator += fin*x->divider;
	  x->weighted = x->accumulator / x->divaccum;
	  x->divider *= x->multiplier;
	  x->divaccum += x->divider;
	  outlet_float(x->waverage, x->weighted);
	}
      else
	{
	  x->accumulator += fin*x->divider;
	  x->weighted = x->accumulator / x->divaccum;
	  x->divider *= x->multiplier;
	  x->divaccum += x->divider;
	  outlet_float(x->waverage, x->weighted);
	}
      x->length += 1;
    }
}

void weightonset_bang (t_weightonset *x)
{
  int (len) = (int)x->length;
  outlet_list(x->datachunk, gensym("list"), len, x->x_datachunk.data);
  x->accumulator = 0;
  x->length = 0;
  if(x->attack > 0)
    {
      x->divider = 1;
      x->divaccum = 1;
    }
  else if (x->attack == 0)
    {
      x->divider = 0;
      x->divaccum = 0;
    }
}

void *weightonset_new(t_floatarg f1, t_floatarg f2) 
{
  t_weightonset *x = (t_weightonset *)pd_new(weightonset_class);
  x->multiplier = f1 != 0 ? f1 : 0.5;
  x->accumulator = 0;
  x->divider = f2 > 0 ? 1 : 0;
  x->divaccum = f2 > 0 ? 1 : 0;
  x->length = 0;
  x->attack = f2;
  floatinlet_new(&x->x_obj, &x->multiplier);
  floatinlet_new(&x->x_obj, &x->attack);
  x->waverage = outlet_new(&x->x_obj, &s_float);
  x->datachunk = outlet_new(&x->x_obj, &s_list);
  return (void *)x;
}

void weightonset_setup(void) {
  weightonset_class = class_new(gensym("weightonset"),
  (t_newmethod)weightonset_new,
  0, sizeof(t_weightonset),
  0, A_DEFFLOAT, A_DEFFLOAT, 0);
  post("input values become less and less important");
  post("as entropy brings alternative rewards");
  class_sethelpsymbol(weightonset_class, gensym("help-weightonset"));
  class_addbang(weightonset_class, weightonset_bang);
  class_addfloat(weightonset_class, weightonset_float);
}
