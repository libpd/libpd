/* For information on usage and redistribution, and for a DISCLAIMER OF ALL
* WARRANTIES, see the file, "LICENSE.txt," in this distribution.

iem_spec2 written by Thomas Musil, Copyright (c) IEM KUG Graz Austria 2000 - 2006 */

#include "m_pd.h"
#include "iemlib.h"
#include <math.h>

#define SPEC2LOGTEN 2.302585092994f

/* ------------------------ spec2_dbtopow_tilde~ ------------------------- */

static t_class *spec2_dbtopow_tilde_class;

typedef struct _spec2_dbtopow_tilde
{
  t_object  x_obj;
  t_float   x_msi;
} t_spec2_dbtopow_tilde;

static t_int *spec2_dbtopow_tilde_perform(t_int *w)
{
  t_float *in = (t_float *)(w[1]);
  t_float *out = (t_float *)(w[2]);
  int n = w[3]+1;
  
  for (; n--; in++, out++)
  { 
    t_float f = *in;
    
    if(f <= 0.0f)
      *out = 0.0f;
    else
    {
      if(f > 870.0f)
        f = 870.0f;
      *out = exp((SPEC2LOGTEN * 0.1f) * (f-100.0f));
    }
  }
  return(w+4);
}

static void spec2_dbtopow_tilde_dsp(t_spec2_dbtopow_tilde *x, t_signal **sp)
{
  int n = (sp[0]->s_n)/2;
  
  dsp_add(spec2_dbtopow_tilde_perform, 3, sp[0]->s_vec, sp[0]->s_vec, n);
}

static void *spec2_dbtopow_tilde_new(void)
{
  t_spec2_dbtopow_tilde *x = (t_spec2_dbtopow_tilde *)pd_new(spec2_dbtopow_tilde_class);
  
  outlet_new(&x->x_obj, &s_signal);
  x->x_msi = 0.0f;
  return (x);
}

void spec2_dbtopow_tilde_setup(void)
{
  spec2_dbtopow_tilde_class = class_new(gensym("spec2_dbtopow~"), (t_newmethod)spec2_dbtopow_tilde_new,
    0, sizeof(t_spec2_dbtopow_tilde), 0, 0);
  CLASS_MAINSIGNALIN(spec2_dbtopow_tilde_class, t_spec2_dbtopow_tilde, x_msi);
  class_addmethod(spec2_dbtopow_tilde_class, (t_method)spec2_dbtopow_tilde_dsp, gensym("dsp"), 0);
//  class_sethelpsymbol(spec2_dbtopow_tilde_class, gensym("iemhelp2/spec2_dbtopow~-help"));
}
