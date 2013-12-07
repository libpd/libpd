/* For information on usage and redistribution, and for a DISCLAIMER OF ALL
* WARRANTIES, see the file, "LICENSE.txt," in this distribution.

iem_spec2 written by Thomas Musil, Copyright (c) IEM KUG Graz Austria 2000 - 2006 */

#include "m_pd.h"
#include "iemlib.h"

/* ------------------------ spec2_sum_tilde~ ------------------------- */

static t_class *spec2_sum_tilde_class;

typedef struct _spec2_sum_tilde
{
  t_object  x_obj;
  t_float   x_msi;
} t_spec2_sum_tilde;

static t_int *spec2_sum_tilde_perform(t_int *w)
{
  t_float *in = (t_float *)(w[1]);
  t_float *out = (t_float *)(w[2]);
  t_float sum = 0.0f;
  int n, hn;
  
  n = hn = w[3];
  sum = *in++;
  while(n--)
    sum += (*in++)*2.0f;
  while(hn--)
    *out++ = sum;
  *out++ = sum;
  return(w+4);
}

static t_int *spec2_sum_tilde_perf16(t_int *w)
{
  t_float *in = (t_float *)(w[1]);
  t_float *out = (t_float *)(w[2]);
  t_float sum=0.0f;
  int n, hn;
  
  n = hn = w[3];
  sum = *in++;
  while(n)
  { 
    sum += 2.0f*in[0];
    sum += 2.0f*in[1];
    sum += 2.0f*in[2];
    sum += 2.0f*in[3];
    sum += 2.0f*in[4];
    sum += 2.0f*in[5];
    sum += 2.0f*in[6];
    sum += 2.0f*in[7];
    sum += 2.0f*in[8];
    sum += 2.0f*in[9];
    sum += 2.0f*in[10];
    sum += 2.0f*in[11];
    sum += 2.0f*in[12];
    sum += 2.0f*in[13];
    sum += 2.0f*in[14];
    sum += 2.0f*in[15];
    
    in += 16;
    n -= 16;
  }
  
  while(hn)
  { 
    out[0] = sum;
    out[1] = sum;
    out[2] = sum;
    out[3] = sum;
    out[4] = sum;
    out[5] = sum;
    out[6] = sum;
    out[7] = sum;
    out[8] = sum;
    out[9] = sum;
    out[10] = sum;
    out[11] = sum;
    out[12] = sum;
    out[13] = sum;
    out[14] = sum;
    out[15] = sum;
    
    out += 16;
    hn -= 16;
  }
  out[0] = sum;
  return(w+4);
}

static void spec2_sum_tilde_dsp(t_spec2_sum_tilde *x, t_signal **sp)
{
  int n = (sp[0]->s_n)/2;
  
  if(n&15)
    dsp_add(spec2_sum_tilde_perform, 3, sp[0]->s_vec, sp[0]->s_vec, n);
  else
    dsp_add(spec2_sum_tilde_perf16, 3, sp[0]->s_vec, sp[0]->s_vec, n);
}

static void *spec2_sum_tilde_new(void)
{
  t_spec2_sum_tilde *x = (t_spec2_sum_tilde *)pd_new(spec2_sum_tilde_class);
  
  outlet_new(&x->x_obj, &s_signal);
  x->x_msi = 0.0f;
  return (x);
}

void spec2_sum_tilde_setup(void)
{
  spec2_sum_tilde_class = class_new(gensym("spec2_sum~"), (t_newmethod)spec2_sum_tilde_new,
    0, sizeof(t_spec2_sum_tilde), 0, 0);
  CLASS_MAINSIGNALIN(spec2_sum_tilde_class, t_spec2_sum_tilde, x_msi);
  class_addmethod(spec2_sum_tilde_class, (t_method)spec2_sum_tilde_dsp, gensym("dsp"), 0);
//  class_sethelpsymbol(spec2_sum_tilde_class, gensym("iemhelp2/spec2_sum~-help"));
}
