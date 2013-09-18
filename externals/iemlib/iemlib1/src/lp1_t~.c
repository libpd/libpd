/* For information on usage and redistribution, and for a DISCLAIMER OF ALL
* WARRANTIES, see the file, "LICENSE.txt," in this distribution.

iemlib1 written by Thomas Musil, Copyright (c) IEM KUG Graz Austria 2000 - 2006 */

#include "m_pd.h"
#include "iemlib.h"
#include <math.h>

/* -- lp1_t~ - slow dynamic lowpass-filter 1. order with tau input --- */

typedef struct _lp1_t_tilde
{
  t_object  x_obj;
  t_float   yn1;
  t_float   c0;
  t_float   c1;
  t_float   sr;
  t_float   cur_t;
  t_float   delta_t;
  t_float   end_t;
  t_float   ticks_per_interpol_time;
  t_float   rcp_ticks;
  t_float   interpol_time;
  int       ticks;
  int       counter_t;
  t_float   x_msi;
} t_lp1_t_tilde;

static t_class *lp1_t_tilde_class;

static void lp1_t_tilde_dsp_tick(t_lp1_t_tilde *x)
{
  if(x->counter_t)
  {
    if(x->counter_t <= 1)
    {
      x->cur_t = x->end_t;
      x->counter_t = 0;
    }
    else
    {
      x->counter_t--;
      x->cur_t += x->delta_t;
    }
    if(x->cur_t == 0.0f)
      x->c1 = 0.0f;
    else
      x->c1 = exp((x->sr)/x->cur_t);
    x->c0 = 1.0f - x->c1;
  }
}

static t_int *lp1_t_tilde_perform(t_int *w)
{
  t_float *in = (t_float *)(w[1]);
  t_float *out = (t_float *)(w[2]);
  t_lp1_t_tilde *x = (t_lp1_t_tilde *)(w[3]);
  int i, n = (t_int)(w[4]);
  t_float yn0, yn1=x->yn1;
  t_float c0=x->c0, c1=x->c1;
  
  lp1_t_tilde_dsp_tick(x);
  for(i=0; i<n; i++)
  {
    yn0 = (*in++)*c0 + yn1*c1;
    *out++ = yn0;
    yn1 = yn0;
  }
  /* NAN protect */
  if(IEM_DENORMAL(yn1))
    yn1 = 0.0f;
  x->yn1 = yn1;
  return(w+5);
}

static t_int *lp1_t_tilde_perf8(t_int *w)
{
  t_float *in = (t_float *)(w[1]);
  t_float *out = (t_float *)(w[2]);
  t_lp1_t_tilde *x = (t_lp1_t_tilde *)(w[3]);
  int i, n = (t_int)(w[4]);
  t_float ynn[9];
  t_float c0=x->c0, c1=x->c1;
  
  lp1_t_tilde_dsp_tick(x);
  ynn[0] = x->yn1;
  for(i=0; i<n; i+=8, in+=8, out+=8)
  {
    ynn[1] = in[0]*c0 + ynn[0]*c1;
    out[0] = ynn[1];
    ynn[2] = in[1]*c0 + ynn[1]*c1;
    out[1] = ynn[2];
    ynn[3] = in[2]*c0 + ynn[2]*c1;
    out[2] = ynn[3];
    ynn[4] = in[3]*c0 + ynn[3]*c1;
    out[3] = ynn[4];
    ynn[5] = in[4]*c0 + ynn[4]*c1;
    out[4] = ynn[5];
    ynn[6] = in[5]*c0 + ynn[5]*c1;
    out[5] = ynn[6];
    ynn[7] = in[6]*c0 + ynn[6]*c1;
    out[6] = ynn[7];
    ynn[8] = in[7]*c0 + ynn[7]*c1;
    out[7] = ynn[8];
    ynn[0] = ynn[8];
  }
  /* NAN protect */
  if(IEM_DENORMAL(ynn[0]))
    ynn[0] = 0.0f;
  
  x->yn1 = ynn[0];
  return(w+5);
}

static void lp1_t_tilde_ft2(t_lp1_t_tilde *x, t_floatarg t)
{
  int i = (int)((x->ticks_per_interpol_time)*t);
  
  x->interpol_time = t;
  if(i <= 0)
    i = 1;
  x->ticks = i;
  x->rcp_ticks = 1.0f / (t_float)i;
}

static void lp1_t_tilde_ft1(t_lp1_t_tilde *x, t_floatarg time_const)
{
  if(time_const < 0.0f)
    time_const = 0.0f;
  if(time_const != x->cur_t)
  {
    x->end_t = time_const;
    x->counter_t = x->ticks;
    x->delta_t = (time_const - x->cur_t) * x->rcp_ticks;
  }
}

static void lp1_t_tilde_dsp(t_lp1_t_tilde *x, t_signal **sp)
{
  int i, n=(int)sp[0]->s_n;
  
  x->sr = -1000.0f / (t_float)(sp[0]->s_sr);
  x->ticks_per_interpol_time = 0.001f * (t_float)(sp[0]->s_sr) / (t_float)n;
  i = (int)((x->ticks_per_interpol_time)*(x->interpol_time));
  if(i <= 0)
    i = 1;
  x->ticks = i;
  x->rcp_ticks = 1.0f / (t_float)i;
  if(x->cur_t == 0.0f)
    x->c1 = 0.0f;
  else
    x->c1 = exp((x->sr)/x->cur_t);
  x->c0 = 1.0f - x->c1;
  if(n&7)
    dsp_add(lp1_t_tilde_perform, 4, sp[0]->s_vec, sp[1]->s_vec, x, n);
  else
    dsp_add(lp1_t_tilde_perf8, 4, sp[0]->s_vec, sp[1]->s_vec, x, n);
}

static void *lp1_t_tilde_new(t_symbol *s, int argc, t_atom *argv)
{
  t_lp1_t_tilde *x = (t_lp1_t_tilde *)pd_new(lp1_t_tilde_class);
  int i;
  t_float time_const=0.0f, interpol=0.0f;
  
  inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_float, gensym("ft1"));
  inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_float, gensym("ft2"));
  outlet_new(&x->x_obj, &s_signal);
  x->x_msi = 0;
  x->counter_t = 1;
  x->delta_t = 0.0f;
  x->interpol_time = 0.0f;
  x->yn1 = 0.0f;
  x->sr = -1.0f / 44.1f;
  if((argc >= 1)&&IS_A_FLOAT(argv,0))
    time_const = (t_float)atom_getfloatarg(0, argc, argv);
  if((argc >= 2)&&IS_A_FLOAT(argv,1))
    interpol = (t_float)atom_getfloatarg(1, argc, argv);
  if(time_const < 0.0f)
    time_const = 0.0f;
  x->cur_t = time_const;
  if(time_const == 0.0f)
    x->c1 = 0.0f;
  else
    x->c1 = exp((x->sr)/time_const);
  x->c0 = 1.0f - x->c1;
  if(interpol < 0.0f)
    interpol = 0.0f;
  x->interpol_time = interpol;
  x->ticks_per_interpol_time = 0.5f;
  i = (int)((x->ticks_per_interpol_time)*(x->interpol_time));
  if(i <= 0)
    i = 1;
  x->ticks = i;
  x->rcp_ticks = 1.0f / (t_float)i;
  x->end_t = x->cur_t;
  return (x);
}

void lp1_t_tilde_setup(void)
{
  lp1_t_tilde_class = class_new(gensym("lp1_t~"), (t_newmethod)lp1_t_tilde_new,
        0, sizeof(t_lp1_t_tilde), 0, A_GIMME, 0);
  CLASS_MAINSIGNALIN(lp1_t_tilde_class, t_lp1_t_tilde, x_msi);
  class_addmethod(lp1_t_tilde_class, (t_method)lp1_t_tilde_dsp, gensym("dsp"), 0);
  class_addmethod(lp1_t_tilde_class, (t_method)lp1_t_tilde_ft1, gensym("ft1"), A_FLOAT, 0);
  class_addmethod(lp1_t_tilde_class, (t_method)lp1_t_tilde_ft2, gensym("ft2"), A_FLOAT, 0);
//  class_sethelpsymbol(lp1_t_tilde_class, gensym("iemhelp/help-lp1_t~"));
}
