/* For information on usage and redistribution, and for a DISCLAIMER OF ALL
* WARRANTIES, see the file, "LICENSE.txt," in this distribution.

iemlib2 written by Thomas Musil, Copyright (c) IEM KUG Graz Austria 2000 - 2006 */


#include "m_pd.h"
#include "iemlib.h"


/* -------------------- LFO_noise~ --------------------- */
/* ---- outputs a 2 point interpolated white noise ----- */
/* -- with lower cutoff frequency than 0.5 samplerate -- */

static t_class *LFO_noise_tilde_class;

typedef struct _LFO_noise_tilde
{
  t_object     x_obj;
  double       x_range;
  double       x_rcp_range;
  unsigned int x_state;
  t_float      x_fact;
  t_float      x_incr;
  t_float      x_y1;
  t_float      x_y2;
  t_float      x_phase;
} t_LFO_noise_tilde;

static int LFO_noise_tilde_makeseed(void)
{
  static unsigned int LFO_noise_tilde_nextseed = 1489853723;
  
  LFO_noise_tilde_nextseed = LFO_noise_tilde_nextseed * 435898247 + 938284287;
  return(LFO_noise_tilde_nextseed & 0x7fffffff);
}

static float LFO_noise_tilde_new_rand(t_LFO_noise_tilde *x)
{
  unsigned int state = x->x_state;
  double new_val, range = x->x_range;
  
  x->x_state = state = state * 472940017 + 832416023;
  new_val = range * ((double)state) * (1./4294967296.);
  if(new_val >= range)
    new_val = range-1;
  new_val -= 32767.0;
  return(new_val*(1.0/32767.0));
}

static void *LFO_noise_tilde_new(t_float freq)
{
  t_LFO_noise_tilde *x = (t_LFO_noise_tilde *)pd_new(LFO_noise_tilde_class);
  
  x->x_range = 65535.0;
  x->x_rcp_range =  (double)x->x_range * (1.0/4294967296.0);
  x->x_state = LFO_noise_tilde_makeseed();
  x->x_fact = 2.0f / 44100.0f;
  x->x_incr = freq * x->x_fact;
  if(x->x_incr < 0.0f)
    x->x_incr = 0.0f;
  else if(x->x_incr > 0.1f)
    x->x_incr = 0.1f;
  x->x_y1 = LFO_noise_tilde_new_rand(x);
  x->x_y2 = LFO_noise_tilde_new_rand(x);
  x->x_phase = 0.0f;
  outlet_new(&x->x_obj, gensym("signal"));
  return (x);
}

static t_int *LFO_noise_tilde_perform(t_int *w)
{
  t_float *out = (t_float *)(w[1]);
  t_LFO_noise_tilde *x = (t_LFO_noise_tilde *)(w[2]);
  int n = (int)(w[3]);
  t_float phase = x->x_phase;
  t_float x_y1 = x->x_y1;
  t_float x_y2 = x->x_y2;
  t_float incr = x->x_incr;
  
  while(n--)
  {
    if(phase > 1.0f)
    {
      x_y1 = x_y2;
      x_y2 = LFO_noise_tilde_new_rand(x);
      phase -= 1.0;
    }
    *out++ = (x_y2 - x_y1) * phase + x_y1;
    phase += incr;
  }
  x->x_phase = phase;
  x->x_y1 = x_y1;
  x->x_y2 = x_y2;
  return (w+4);
}

static void LFO_noise_tilde_float(t_LFO_noise_tilde *x, t_floatarg freq)
{
  x->x_incr = freq * x->x_fact;
  if(x->x_incr < 0.0f)
    x->x_incr = 0.0f;
  else if(x->x_incr > 0.1f)
    x->x_incr = 0.1f;
}

static void LFO_noise_tilde_dsp(t_LFO_noise_tilde *x, t_signal **sp)
{
  x->x_fact = 2.0f / sp[0]->s_sr;
  dsp_add(LFO_noise_tilde_perform, 3, sp[0]->s_vec, x, sp[0]->s_n);
}

void LFO_noise_tilde_setup(void)
{
  LFO_noise_tilde_class = class_new(gensym("LFO_noise~"),
    (t_newmethod)LFO_noise_tilde_new, 0,
    sizeof(t_LFO_noise_tilde), 0, A_DEFFLOAT, 0);
  class_addmethod(LFO_noise_tilde_class, (t_method)LFO_noise_tilde_dsp,
    gensym("dsp"), 0);
  class_addfloat(LFO_noise_tilde_class, (t_method)LFO_noise_tilde_float);
//  class_sethelpsymbol(LFO_noise_tilde_class, gensym("iemhelp/help-LFO_noise~"));
}
