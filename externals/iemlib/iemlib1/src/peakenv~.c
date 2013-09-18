/* For information on usage and redistribution, and for a DISCLAIMER OF ALL
* WARRANTIES, see the file, "LICENSE.txt," in this distribution.

iemlib1 written by Thomas Musil, Copyright (c) IEM KUG Graz Austria 2000 - 2006 */


#include "m_pd.h"
#include "iemlib.h"
#include <math.h>

/* ---------------- peakenv~ - simple peak-envelope-converter. ----------------- */

typedef struct _peakenv_tilde
{
  t_object x_obj;
  t_float  x_sr;
  t_float  x_old_peak;
  t_float  x_c1;
  t_float  x_releasetime;
  t_float  x_msi;
} t_peakenv_tilde;

static t_class *peakenv_tilde_class;

static void peakenv_tilde_reset(t_peakenv_tilde *x)
{
  x->x_old_peak = 0.0f;
}

static void peakenv_tilde_ft1(t_peakenv_tilde *x, t_floatarg f)/* release-time in ms */
{
  if(f < 0.0f)
    f = 0.0f;
  x->x_releasetime = f;
  x->x_c1 = exp(-1.0/(x->x_sr*0.001*x->x_releasetime));
}

static t_int *peakenv_tilde_perform(t_int *w)
{
  t_float *in = (t_float *)(w[1]);
  t_float *out = (t_float *)(w[2]);
  t_peakenv_tilde *x = (t_peakenv_tilde *)(w[3]);
  int n = (int)(w[4]);
  t_float peak = x->x_old_peak;
  t_float c1 = x->x_c1;
  t_float absolute;
  int i;
  
  for(i=0; i<n; i++)
  {
    absolute = fabs(*in++);
    peak *= c1;
    if(absolute > peak)
      peak = absolute;
    *out++ = peak;
  }
  /* NAN protect */
  if(IEM_DENORMAL(peak))
    peak = 0.0f;
  x->x_old_peak = peak;
  return(w+5);
}

static void peakenv_tilde_dsp(t_peakenv_tilde *x, t_signal **sp)
{
  x->x_sr = (t_float)sp[0]->s_sr;
  peakenv_tilde_ft1(x, x->x_releasetime);
  dsp_add(peakenv_tilde_perform, 4, sp[0]->s_vec, sp[1]->s_vec, x, sp[0]->s_n);
}

static void *peakenv_tilde_new(t_floatarg f)
{
  t_peakenv_tilde *x = (t_peakenv_tilde *)pd_new(peakenv_tilde_class);
  
  if(f <= 0.0f)
    f = 0.0f;
  x->x_sr = 44100.0f;
  peakenv_tilde_ft1(x, f);
  x->x_old_peak = 0.0f;
  inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_float, gensym("ft1"));
  outlet_new(&x->x_obj, &s_signal);
  x->x_msi = 0;
  return(x);
}

void peakenv_tilde_setup(void)
{
  peakenv_tilde_class = class_new(gensym("peakenv~"), (t_newmethod)peakenv_tilde_new,
    0, sizeof(t_peakenv_tilde), 0, A_DEFFLOAT, 0);
  CLASS_MAINSIGNALIN(peakenv_tilde_class, t_peakenv_tilde, x_msi);
  class_addmethod(peakenv_tilde_class, (t_method)peakenv_tilde_dsp, gensym("dsp"), 0);
  class_addmethod(peakenv_tilde_class, (t_method)peakenv_tilde_ft1, gensym("ft1"), A_FLOAT, 0);
  class_addmethod(peakenv_tilde_class, (t_method)peakenv_tilde_reset, gensym("reset"), 0);
//  class_sethelpsymbol(peakenv_tilde_class, gensym("iemhelp/help-peakenv~"));
}
