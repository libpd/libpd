/* For information on usage and redistribution, and for a DISCLAIMER OF ALL
* WARRANTIES, see the file, "LICENSE.txt," in this distribution.

iemlib1 written by Thomas Musil, Copyright (c) IEM KUG Graz Austria 2000 - 2006 */


#include "m_pd.h"
#include "iemlib.h"
#include <math.h>

/* ---------------- rvu~ - simple peak&rms-vu-meter. ----------------- */

typedef struct _rvu_tilde
{
  t_object  x_obj;
  void      *x_clock_metro;
  t_float   x_metro_time;
  t_float   x_sum_rms;
  t_float   x_old_rms;
  t_float   x_rcp;
  t_float   x_sr;
  t_float   x_release_time;
  t_float   x_c1;
  int       x_started;
  t_float   x_msi;
} t_rvu_tilde;

static t_class *rvu_tilde_class;
static void rvu_tilde_tick_metro(t_rvu_tilde *x);

static void rvu_tilde_reset(t_rvu_tilde *x)
{
  outlet_float(x->x_obj.ob_outlet, -99.9f);
  x->x_sum_rms = 0.0f;
  x->x_old_rms = 0.0f;
  clock_delay(x->x_clock_metro, x->x_metro_time);
}

static void rvu_tilde_stop(t_rvu_tilde *x)
{
  clock_unset(x->x_clock_metro);
  x->x_started = 0;
}

static void rvu_tilde_start(t_rvu_tilde *x)
{
  clock_delay(x->x_clock_metro, x->x_metro_time);
  x->x_started = 1;
}

static void rvu_tilde_float(t_rvu_tilde *x, t_floatarg f)
{
  if(f == 0.0f)
  {
    clock_unset(x->x_clock_metro);
    x->x_started = 0;
  }
  else
  {
    clock_delay(x->x_clock_metro, x->x_metro_time);
    x->x_started = 1;
  }
}

static void rvu_tilde_t_release(t_rvu_tilde *x, t_floatarg release_time)
{
  if(release_time <= 5.0f)
    release_time = 5.0f;
  x->x_release_time = release_time;
  x->x_c1 = exp(-2.0f*x->x_metro_time/x->x_release_time);
}

static void rvu_tilde_t_metro(t_rvu_tilde *x, t_floatarg metro_time)
{
  if(metro_time <= 5.0f)
    metro_time = 5.0f;
  x->x_metro_time = metro_time;
  x->x_c1 = exp(-2.0f*x->x_metro_time/x->x_release_time);
  x->x_rcp = 1.0f/(x->x_sr*x->x_metro_time);
}

static t_int *rvu_tilde_perform(t_int *w)
{
  t_float *in = (t_float *)(w[1]);
  t_rvu_tilde *x = (t_rvu_tilde *)(w[2]);
  int n = (int)(w[3]);
  t_float sum=x->x_sum_rms;
  int i;
  
  if(x->x_started)
  {
    for(i=0; i<n; i++)
    {
      sum += in[i]*in[i];
    }
    x->x_sum_rms = sum;
  }
  return(w+4);
}

static void rvu_tilde_dsp(t_rvu_tilde *x, t_signal **sp)
{
  x->x_sr = 0.001*(t_float)sp[0]->s_sr;
  x->x_rcp = 1.0f/(x->x_sr*x->x_metro_time);
  dsp_add(rvu_tilde_perform, 3, sp[0]->s_vec, x, sp[0]->s_n);
  clock_delay(x->x_clock_metro, x->x_metro_time);
}

static void rvu_tilde_tick_metro(t_rvu_tilde *x)
{
  t_float dbr, cur_rms, c1=x->x_c1;
  
  cur_rms = (1.0f - c1)*x->x_sum_rms*x->x_rcp + c1*x->x_old_rms;
  /* NAN protect */
  if(IEM_DENORMAL(cur_rms))
    cur_rms = 0.0f;
  
  if(cur_rms <= 0.0000000001f)
    dbr = -99.9f;
  else if(cur_rms > 1000000.0f)
  {
    dbr = 60.0f;
    x->x_old_rms = 1000000.0f;
  }
  else
    dbr = 4.3429448195f*log(cur_rms);
  x->x_sum_rms = 0.0f;
  x->x_old_rms = cur_rms;
  outlet_float(x->x_obj.ob_outlet, dbr);
  clock_delay(x->x_clock_metro, x->x_metro_time);
}

static void rvu_tilde_ff(t_rvu_tilde *x)
{
  clock_free(x->x_clock_metro);
}

static void *rvu_tilde_new(t_floatarg metro_time, t_floatarg release_time)
{
  t_rvu_tilde *x=(t_rvu_tilde *)pd_new(rvu_tilde_class);
  
  if(metro_time <= 0.0f)
    metro_time = 300.0f;
  if(metro_time <= 5.0f)
    metro_time = 5.0f;
  if(release_time <= 0.0f)
    release_time = 300.0f;
  if(release_time <= 5.0f)
    release_time = 5.0f;
  x->x_metro_time = metro_time;
  x->x_release_time = release_time;
  x->x_c1 = exp(-2.0f*x->x_metro_time/x->x_release_time);
  x->x_sum_rms = 0.0f;
  x->x_old_rms = 0.0f;
  x->x_sr = 44.1f;
  x->x_rcp = 1.0f/(x->x_sr*x->x_metro_time);
  x->x_clock_metro = clock_new(x, (t_method)rvu_tilde_tick_metro);
  x->x_started = 1;
  outlet_new(&x->x_obj, &s_float);
  x->x_msi = 0.0f;
  return(x);
}

void rvu_tilde_setup(void)
{
  rvu_tilde_class = class_new(gensym("rvu~"), (t_newmethod)rvu_tilde_new,
    (t_method)rvu_tilde_ff, sizeof(t_rvu_tilde), 0,
    A_DEFFLOAT, A_DEFFLOAT, 0);
  CLASS_MAINSIGNALIN(rvu_tilde_class, t_rvu_tilde, x_msi);
  class_addmethod(rvu_tilde_class, (t_method)rvu_tilde_dsp, gensym("dsp"), 0);
  class_addfloat(rvu_tilde_class, rvu_tilde_float);
  class_addmethod(rvu_tilde_class, (t_method)rvu_tilde_reset, gensym("reset"), 0);
  class_addmethod(rvu_tilde_class, (t_method)rvu_tilde_start, gensym("start"), 0);
  class_addmethod(rvu_tilde_class, (t_method)rvu_tilde_stop, gensym("stop"), 0);
  class_addmethod(rvu_tilde_class, (t_method)rvu_tilde_t_release, gensym("t_release"), A_FLOAT, 0);
  class_addmethod(rvu_tilde_class, (t_method)rvu_tilde_t_metro, gensym("t_metro"), A_FLOAT, 0);
//  class_sethelpsymbol(rvu_tilde_class, gensym("iemhelp/help-rvu~"));
}
