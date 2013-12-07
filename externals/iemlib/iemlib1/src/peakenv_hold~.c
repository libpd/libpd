/* For information on usage and redistribution, and for a DISCLAIMER OF ALL
* WARRANTIES, see the file, "LICENSE.txt," in this distribution.

iemlib1 written by Thomas Musil, Copyright (c) IEM KUG Graz Austria 2000 - 2008 */


#include "m_pd.h"
#include "iemlib.h"
#include <math.h>

/* ---------------- peakenv_hold~ - simple peak-envelope-converter with peak hold time and release time. ----------------- */

typedef struct _peakenv_hold_tilde
{
  t_object x_obj;
  t_float  x_sr;
  t_float  x_old_peak;
  t_float  x_c1;
  t_float  x_releasetime;
  t_float  x_holdtime;
  t_int    x_n_hold;
  t_int    x_counter;
  t_float  x_msi;
} t_peakenv_hold_tilde;

static t_class *peakenv_hold_tilde_class;

static void peakenv_hold_tilde_reset(t_peakenv_hold_tilde *x)
{
  x->x_old_peak = 0.0f;
}

static void peakenv_hold_tilde_ft1(t_peakenv_hold_tilde *x, t_float t_hold)/* hold-time in ms */
{
  double dhold;
  
  if(t_hold < 0.0f)
    t_hold = 0.0f;
  x->x_holdtime = t_hold;
  dhold = (double)x->x_sr*(double)0.001*(double)x->x_holdtime;
  if(dhold > 2147483647.0)
    dhold = 2147483647.0;
  x->x_n_hold = (t_int)(dhold + (double)0.5);
}

static void peakenv_hold_tilde_ft2(t_peakenv_hold_tilde *x, t_float t_rel)/* release-time in ms */
{
  if(t_rel < 0.0f)
    t_rel = 0.0f;
  x->x_releasetime = t_rel;
  x->x_c1 = exp(-1.0/(x->x_sr*0.001*x->x_releasetime));
}

static t_int *peakenv_hold_tilde_perform(t_int *w)
{
  t_float *in = (t_float *)(w[1]);
  t_float *out = (t_float *)(w[2]);
  t_peakenv_hold_tilde *x = (t_peakenv_hold_tilde *)(w[3]);
  int n = (int)(w[4]);
  t_float peak = x->x_old_peak;
  t_float c1 = x->x_c1;
  t_float absolute;
  t_int i, counter;
  
  counter = x->x_counter;
  for(i=0; i<n; i++)
  {
    absolute = fabs(*in++);
    if(counter > 0)
      counter--;// hold peride
    else
      peak *= c1;// release periode
    if(absolute > peak)
    {
      peak = absolute;
      counter = x->x_n_hold;// new hold initialisation
    }
    *out++ = peak;
  }
  /* NAN protect */
  if(IEM_DENORMAL(peak))
    peak = 0.0f;
  x->x_old_peak = peak;
  x->x_counter = counter;
  return(w+5);
}

static void peakenv_hold_tilde_dsp(t_peakenv_hold_tilde *x, t_signal **sp)
{
  x->x_sr = (t_float)sp[0]->s_sr;
  peakenv_hold_tilde_ft1(x, x->x_holdtime);
  peakenv_hold_tilde_ft2(x, x->x_releasetime);
  dsp_add(peakenv_hold_tilde_perform, 4, sp[0]->s_vec, sp[1]->s_vec, x, sp[0]->s_n);
}

static void *peakenv_hold_tilde_new(t_float t_hold, t_float t_rel)
{
  t_peakenv_hold_tilde *x = (t_peakenv_hold_tilde *)pd_new(peakenv_hold_tilde_class);
  
  x->x_sr = 44100.0f;
  peakenv_hold_tilde_ft1(x, t_hold);
  peakenv_hold_tilde_ft2(x, t_rel);
  x->x_old_peak = 0.0f;
  x->x_counter = 0;
  inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_float, gensym("ft1"));
  inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_float, gensym("ft2"));
  outlet_new(&x->x_obj, &s_signal);
  x->x_msi = 0;
  return(x);
}

void peakenv_hold_tilde_setup(void)
{
  peakenv_hold_tilde_class = class_new(gensym("peakenv_hold~"), (t_newmethod)peakenv_hold_tilde_new,
    0, sizeof(t_peakenv_hold_tilde), 0, A_DEFFLOAT, A_DEFFLOAT, 0);
  CLASS_MAINSIGNALIN(peakenv_hold_tilde_class, t_peakenv_hold_tilde, x_msi);
  class_addmethod(peakenv_hold_tilde_class, (t_method)peakenv_hold_tilde_dsp, gensym("dsp"), 0);
  class_addmethod(peakenv_hold_tilde_class, (t_method)peakenv_hold_tilde_ft1, gensym("ft1"), A_FLOAT, 0);
   class_addmethod(peakenv_hold_tilde_class, (t_method)peakenv_hold_tilde_ft2, gensym("ft2"), A_FLOAT, 0);
  class_addmethod(peakenv_hold_tilde_class, (t_method)peakenv_hold_tilde_reset, gensym("reset"), 0);
}
