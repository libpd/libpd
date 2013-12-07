/* For information on usage and redistribution, and for a DISCLAIMER OF ALL
* WARRANTIES, see the file, "LICENSE.txt," in this distribution.

iem_spec2 written by Thomas Musil, Copyright (c) IEM KUG Graz Austria 2000 - 2006 */


#include "m_pd.h"
#include "iemlib.h"
#include <math.h>

/* ------------------------ spec2_abs_tilde~ ------------------------- */
static t_class *spec2_abs_tilde_class;

typedef struct _spec2_abs_tilde
{
  t_object  x_obj;
  t_float   x_msi;
} t_spec2_abs_tilde;

static t_int *spec2_abs_tilde_perform(t_int *w)
{
  t_float *in = (t_float *)(w[1]);
  t_float *out = (t_float *)(w[2]);
  int n = w[3]+1;
  
  while(n--)
  { 
    *in++ = fabs(*out++);
  }
  return(w+4);
}

static t_int *spec2_abs_tilde_perf16(t_int *w)
{
  t_float *in = (t_float *)(w[1]);
  t_float *out = (t_float *)(w[2]);
  int n = w[3];
  
  while(n)
  { 
    in[0] = fabs(out[0]);
    in[1] = fabs(out[1]);
    in[2] = fabs(out[2]);
    in[3] = fabs(out[3]);
    in[4] = fabs(out[4]);
    in[5] = fabs(out[5]);
    in[6] = fabs(out[6]);
    in[7] = fabs(out[7]);
    in[8] = fabs(out[8]);
    in[9] = fabs(out[9]);
    in[10] = fabs(out[10]);
    in[11] = fabs(out[11]);
    in[12] = fabs(out[12]);
    in[13] = fabs(out[13]);
    in[14] = fabs(out[14]);
    in[15] = fabs(out[15]);
    
    in += 16;
    out += 16;
    n -= 16;
  }
  in[0] = fabs(out[0]);
  return(w+4);
}

static void spec2_abs_tilde_dsp(t_spec2_abs_tilde *x, t_signal **sp)
{
  int n = (sp[0]->s_n)/2;
  
  if(n&15)
    dsp_add(spec2_abs_tilde_perform, 3, sp[0]->s_vec, sp[0]->s_vec, n);
  else
    dsp_add(spec2_abs_tilde_perf16, 3, sp[0]->s_vec, sp[0]->s_vec, n);
}

static void *spec2_abs_tilde_new(void)
{
  t_spec2_abs_tilde *x = (t_spec2_abs_tilde *)pd_new(spec2_abs_tilde_class);
  
  outlet_new(&x->x_obj, &s_signal);
  x->x_msi = 0.0f;
  return (x);
}

void spec2_abs_tilde_setup(void)
{
  spec2_abs_tilde_class = class_new(gensym("spec2_abs~"), (t_newmethod)spec2_abs_tilde_new,
    0, sizeof(t_spec2_abs_tilde), 0, 0);
  CLASS_MAINSIGNALIN(spec2_abs_tilde_class, t_spec2_abs_tilde, x_msi);
  class_addmethod(spec2_abs_tilde_class, (t_method)spec2_abs_tilde_dsp, gensym("dsp"), 0);
//  class_sethelpsymbol(spec2_abs_tilde_class, gensym("iemhelp2/spec2_abs~-help"));
}
