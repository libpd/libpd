/* For information on usage and redistribution, and for a DISCLAIMER OF ALL
* WARRANTIES, see the file, "LICENSE.txt," in this distribution.

iemlib1 written by Thomas Musil, Copyright (c) IEM KUG Graz Austria 2000 - 2006 */

#include "m_pd.h"
#include "iemlib.h"
#include <math.h>

/* ---------------- pvu~ - simple peak-vu-meter. ----------------- */

typedef struct _pvu_tilde
{
  t_object  x_obj;
  void      *x_outlet_meter;
  void      *x_outlet_over;
  void      *x_clock;
  t_float   x_cur_peak;
  t_float   x_old_peak;
  t_float   x_threshold_over;
  t_float   x_c1;
  t_float   x_metro_time;
  t_float   x_release_time;
  int       x_overflow_counter;
  int       x_started;
  t_float   x_msi;
} t_pvu_tilde;

static t_class *pvu_tilde_class;
static void pvu_tilde_tick(t_pvu_tilde *x);

static void pvu_tilde_reset(t_pvu_tilde *x)
{
  outlet_float(x->x_outlet_over, 0.0f);
  outlet_float(x->x_outlet_meter, -199.9f);
  x->x_overflow_counter = 0;
  x->x_cur_peak = 0.0f;
  x->x_old_peak = 0.0f;
  clock_delay(x->x_clock, x->x_metro_time);
}

static void pvu_tilde_stop(t_pvu_tilde *x)
{
  clock_unset(x->x_clock);
  x->x_started = 0;
}

static void pvu_tilde_start(t_pvu_tilde *x)
{
  clock_delay(x->x_clock, x->x_metro_time);
  x->x_started = 1;
}

static void pvu_tilde_float(t_pvu_tilde *x, t_floatarg f)
{
  if(f == 0.0)
  {
    clock_unset(x->x_clock);
    x->x_started = 0;
  }
  else
  {
    clock_delay(x->x_clock, x->x_metro_time);
    x->x_started = 1;
  }
}

static void pvu_tilde_t_release(t_pvu_tilde *x, t_floatarg release_time)
{
  if(release_time <= 5.0f)
    release_time = 5.0f;
  x->x_release_time = release_time;
  x->x_c1 = exp(-x->x_metro_time/release_time);
}

static void pvu_tilde_t_metro(t_pvu_tilde *x, t_floatarg metro_time)
{
  if(metro_time <= 5.0f)
    metro_time = 5.0f;
  x->x_metro_time = (int)metro_time;
  x->x_c1 = exp(-metro_time/x->x_release_time);
}

static void pvu_tilde_threshold(t_pvu_tilde *x, t_floatarg thresh)
{
  x->x_threshold_over = thresh;
}

static t_int *pvu_tilde_perform(t_int *w)
{
  t_float *in = (t_float *)(w[1]);
  t_pvu_tilde *x = (t_pvu_tilde *)(w[2]);
  int n = (int)(w[3]);
  t_float peak = x->x_cur_peak;
  t_float absolute;
  int i;
  
  if(x->x_started)
  {
    for(i=0; i<n; i++)
    {
      absolute = fabs(*in++);
      if(absolute > peak)
        peak = absolute;
    }
    x->x_cur_peak = peak;
  }
  return(w+4);
}

static void pvu_tilde_dsp(t_pvu_tilde *x, t_signal **sp)
{
  dsp_add(pvu_tilde_perform, 3, sp[0]->s_vec, x, sp[0]->s_n);
  clock_delay(x->x_clock, x->x_metro_time);
}

static void pvu_tilde_tick(t_pvu_tilde *x)
{
  t_float db;
  int i;
  
  x->x_old_peak *= x->x_c1;
  /* NAN protect */
  if(IEM_DENORMAL(x->x_old_peak))
    x->x_old_peak = 0.0f;
  
  if(x->x_cur_peak > x->x_old_peak)
    x->x_old_peak = x->x_cur_peak;
  if(x->x_old_peak <= 0.0000000001f)
    db = -199.9f;
  else if(x->x_old_peak > 1000000.0f)
  {
    db = 120.0f;
    x->x_old_peak = 1000000.0f;
  }
  else
    db = 8.6858896381f*log(x->x_old_peak);
  if(db >= x->x_threshold_over)
  {
    x->x_overflow_counter++;
    outlet_float(x->x_outlet_over, (t_float)x->x_overflow_counter);
  }
  outlet_float(x->x_outlet_meter, db);
  x->x_cur_peak = 0.0f;
  clock_delay(x->x_clock, x->x_metro_time);
}

static void *pvu_tilde_new(t_floatarg metro_time, t_floatarg release_time, t_floatarg threshold)
{
  t_pvu_tilde *x;
  t_float t;
  
  x = (t_pvu_tilde *)pd_new(pvu_tilde_class);
  if(metro_time <= 0.0f)
    metro_time = 300.0f;
  if(metro_time <= 5.0f)
    metro_time = 5.0f;
  if(release_time <= 0.0f)
    release_time = 300.0f;
  if(release_time <= 5.0f)
    release_time = 5.0f;
  if(threshold == 0.0f)
    threshold = -0.01f;
  x->x_threshold_over = threshold;
  x->x_overflow_counter = 0;
  x->x_metro_time = metro_time;
  x->x_release_time = release_time;
  x->x_c1 = exp(-metro_time/release_time);
  x->x_cur_peak = 0.0f;
  x->x_old_peak = 0.0f;
  x->x_clock = clock_new(x, (t_method)pvu_tilde_tick);
  x->x_outlet_meter = outlet_new(&x->x_obj, &s_float);/* left */
  x->x_outlet_over = outlet_new(&x->x_obj, &s_float); /* right */
  x->x_started = 1;
  x->x_msi = 0;
  return(x);
}

static void pvu_tilde_ff(t_pvu_tilde *x)
{
  clock_free(x->x_clock);
}

void pvu_tilde_setup(void )
{
  pvu_tilde_class = class_new(gensym("pvu~"), (t_newmethod)pvu_tilde_new,
    (t_method)pvu_tilde_ff, sizeof(t_pvu_tilde), 0, A_DEFFLOAT, A_DEFFLOAT, A_DEFFLOAT, 0);
  CLASS_MAINSIGNALIN(pvu_tilde_class, t_pvu_tilde, x_msi);
  class_addmethod(pvu_tilde_class, (t_method)pvu_tilde_dsp, gensym("dsp"), 0);
  class_addfloat(pvu_tilde_class, pvu_tilde_float);
  class_addmethod(pvu_tilde_class, (t_method)pvu_tilde_reset, gensym("reset"), 0);
  class_addmethod(pvu_tilde_class, (t_method)pvu_tilde_start, gensym("start"), 0);
  class_addmethod(pvu_tilde_class, (t_method)pvu_tilde_stop, gensym("stop"), 0);
  class_addmethod(pvu_tilde_class, (t_method)pvu_tilde_t_release, gensym("t_release"), A_FLOAT, 0);
  class_addmethod(pvu_tilde_class, (t_method)pvu_tilde_t_metro, gensym("t_metro"), A_FLOAT, 0);
  class_addmethod(pvu_tilde_class, (t_method)pvu_tilde_threshold, gensym("threshold"), A_FLOAT, 0);
//  class_sethelpsymbol(pvu_tilde_class, gensym("iemhelp/help-pvu~"));
}
