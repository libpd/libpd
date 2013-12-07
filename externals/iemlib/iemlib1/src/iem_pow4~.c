/* For information on usage and redistribution, and for a DISCLAIMER OF ALL
* WARRANTIES, see the file, "LICENSE.txt," in this distribution.

iemlib1 written by Thomas Musil, Copyright (c) IEM KUG Graz Austria 2000 - 2006 */

#include "m_pd.h"
#include "iemlib.h"
#include <math.h>

/* ------------------------ iem_pow4~ ----------------------------- */

static t_class *iem_pow4_tilde_class;

typedef struct _iem_pow4_tilde
{
  t_object  x_obj;
  t_float   x_exp;
  t_float   x_msi;
} t_iem_pow4_tilde;

static void iem_pow4_tilde_ft1(t_iem_pow4_tilde *x, t_floatarg f)
{
  x->x_exp = f;
}

static t_int *iem_pow4_tilde_perform(t_int *w)
{
  t_float *in = (t_float *)(w[1]);
  t_float *out = (t_float *)(w[2]);
  t_iem_pow4_tilde *x = (t_iem_pow4_tilde *)(w[3]);
  t_float y=x->x_exp;
  t_float f, g;
  int n = (int)(w[4])/4;
  
  while (n--)
  {
    f = *in;
    if(f < 0.01f)
      f = 0.01f;
    else if(f > 1000.0f)
      f = 1000.0f;
    g = log(f);
    f = exp(g*y);
    *out++ = f;
    *out++ = f;
    *out++ = f;
    *out++ = f;
    in += 4;
  }
  return (w+5);
}

static void iem_pow4_tilde_dsp(t_iem_pow4_tilde *x, t_signal **sp)
{
  dsp_add(iem_pow4_tilde_perform, 4, sp[0]->s_vec, sp[1]->s_vec, x, sp[0]->s_n);
}

static void *iem_pow4_tilde_new(t_floatarg f)
{
  t_iem_pow4_tilde *x = (t_iem_pow4_tilde *)pd_new(iem_pow4_tilde_class);
  
  x->x_exp = f;
  inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_float, gensym("ft1"));
  outlet_new(&x->x_obj, gensym("signal"));
  x->x_msi = 0;
  return (x);
}

void iem_pow4_tilde_setup(void)
{
  iem_pow4_tilde_class = class_new(gensym("iem_pow4~"), (t_newmethod)iem_pow4_tilde_new, 0,
    sizeof(t_iem_pow4_tilde), 0, A_DEFFLOAT, 0);
  class_addcreator((t_newmethod)iem_pow4_tilde_new, gensym("icot~"), 0);
  CLASS_MAINSIGNALIN(iem_pow4_tilde_class, t_iem_pow4_tilde, x_msi);
  class_addmethod(iem_pow4_tilde_class, (t_method)iem_pow4_tilde_dsp, gensym("dsp"), 0);
  class_addmethod(iem_pow4_tilde_class, (t_method)iem_pow4_tilde_ft1, gensym("ft1"), A_FLOAT, 0);
//  class_sethelpsymbol(iem_pow4_tilde_class, gensym("iemhelp/help-iem_pow4~"));
}
