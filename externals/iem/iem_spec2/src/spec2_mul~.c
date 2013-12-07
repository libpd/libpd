/* For information on usage and redistribution, and for a DISCLAIMER OF ALL
* WARRANTIES, see the file, "LICENSE.txt," in this distribution.

iem_spec2 written by Thomas Musil, Copyright (c) IEM KUG Graz Austria 2000 - 2006 */

#include "m_pd.h"
#include "iemlib.h"

/* -------------------------- spec2_mul~ ------------------------------ */
static t_class *spec2_mul_tilde_class;

typedef struct _spec2_mul_tilde
{
  t_object  x_obj;
  t_float   x_msi;
} t_spec2_mul_tilde;

static t_int *spec2_mul_tilde_perform(t_int *w)
{
  t_float *in1 = (t_float *)(w[1]);
  t_float *in2 = (t_float *)(w[2]);
  t_float *out = (t_float *)(w[3]);
  int i, n = (t_int)(w[4]);
  
  for(i=0; i<=n; i++)
  {
    out[i] = in1[i] * in2[i];
  }
  return(w+5);
}

static t_int *spec2_mul_tilde_perf16(t_int *w)
{
  t_float *in1 = (t_float *)(w[1]);
  t_float *in2 = (t_float *)(w[2]);
  t_float *out = (t_float *)(w[3]);
  int n = (t_int)(w[4]);
  
  while(n)
  {
    out[0] = in1[0] * in2[0];
    out[1] = in1[1] * in2[1];
    out[2] = in1[2] * in2[2];
    out[3] = in1[3] * in2[3];
    out[4] = in1[4] * in2[4];
    out[5] = in1[5] * in2[5];
    out[6] = in1[6] * in2[6];
    out[7] = in1[7] * in2[7];
    out[8] = in1[8] * in2[8];
    out[9] = in1[9] * in2[9];
    out[10] = in1[10] * in2[10];
    out[11] = in1[11] * in2[11];
    out[12] = in1[12] * in2[12];
    out[13] = in1[13] * in2[13];
    out[14] = in1[14] * in2[14];
    out[15] = in1[15] * in2[15];
    
    
    in1 += 16;
    in2 += 16;
    out += 16;
    n -= 16;
  }
  out[0] = in1[0] * in2[0];
  return(w+5);
}

static void spec2_mul_tilde_dsp(t_spec2_mul_tilde *x, t_signal **sp)
{
  int n = (sp[0]->s_n)/2;
  
  if(n&15)
    dsp_add(spec2_mul_tilde_perform, 4, sp[0]->s_vec, sp[1]->s_vec, sp[2]->s_vec, n);
  else
    dsp_add(spec2_mul_tilde_perf16, 4, sp[0]->s_vec, sp[1]->s_vec, sp[2]->s_vec, n);
}

static void *spec2_mul_tilde_new(void)
{
  t_spec2_mul_tilde *x = (t_spec2_mul_tilde *)pd_new(spec2_mul_tilde_class);
  
  inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_signal, &s_signal);
  outlet_new(&x->x_obj, &s_signal);
  x->x_msi = 0.0f;
  return (x);
}

static void spec2_mul_tilde_free(t_spec2_mul_tilde *x)
{
}

void spec2_mul_tilde_setup(void)
{
  spec2_mul_tilde_class = class_new(gensym("spec2_mul~"), (t_newmethod)spec2_mul_tilde_new, (t_method)spec2_mul_tilde_free,
    sizeof(t_spec2_mul_tilde), 0, 0);
  class_addcreator((t_newmethod)spec2_mul_tilde_new, gensym("spec2*~"), 0);
  CLASS_MAINSIGNALIN(spec2_mul_tilde_class, t_spec2_mul_tilde, x_msi);
  class_addmethod(spec2_mul_tilde_class, (t_method)spec2_mul_tilde_dsp, gensym("dsp"), 0);
//  class_sethelpsymbol(spec2_mul_tilde_class, gensym("iemhelp2/spec2_mul~-help"));
}
