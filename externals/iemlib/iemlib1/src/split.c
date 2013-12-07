/* For information on usage and redistribution, and for a DISCLAIMER OF ALL
* WARRANTIES, see the file, "LICENSE.txt," in this distribution.

iemlib1 written by Thomas Musil, Copyright (c) IEM KUG Graz Austria 2000 - 2006 */


#include "m_pd.h"
#include "iemlib.h"


/* --------- split is like moses ----------- */

typedef struct _split
{
  t_object  x_obj;
  t_outlet  *x_out_less;
  t_outlet  *x_out_greater_equal;
  float     x_threshold;
} t_split;

static t_class *split_class;

static void split_float(t_split *x, t_float f)
{
  if(f < x->x_threshold)
    outlet_float(x->x_out_less, f);
  else
    outlet_float(x->x_out_greater_equal, f);
}

static void *split_new(t_floatarg f)
{
  t_split *x = (t_split *)pd_new(split_class);
  floatinlet_new(&x->x_obj, &x->x_threshold);
  x->x_out_less = outlet_new(&x->x_obj, &s_float);
  x->x_out_greater_equal = outlet_new(&x->x_obj, &s_float);
  x->x_threshold = f;
  return (x);
}

void split_setup(void)
{
  split_class = class_new(gensym("split"), (t_newmethod)split_new, 0,
    sizeof(t_split), 0, A_DEFFLOAT, 0);
  class_addfloat(split_class, split_float);
//  class_sethelpsymbol(split_class, gensym("iemhelp/help-split"));
}
