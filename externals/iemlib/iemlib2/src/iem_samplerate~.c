/* For information on usage and redistribution, and for a DISCLAIMER OF ALL
* WARRANTIES, see the file, "LICENSE.txt," in this distribution.

iemlib2 written by Thomas Musil, Copyright (c) IEM KUG Graz Austria 2000 - 2006 */


#include "m_pd.h"
#include "iemlib.h"

/* --------------- iem_samplerate~ ----------------- */
/* -- outputs the current samplerate of a window --- */

static t_class *iem_samplerate_tilde_class;

typedef struct _iem_samplerate_tilde
{
  t_object  x_obj;
  t_float   x_samplerate;
  t_clock   *x_clock;
  t_float   x_f;
} t_iem_samplerate_tilde;

static void iem_samplerate_tilde_out(t_iem_samplerate_tilde *x)
{
  outlet_float(x->x_obj.ob_outlet, x->x_samplerate);
}

static void iem_samplerate_tilde_free(t_iem_samplerate_tilde *x)
{
  clock_free(x->x_clock);
}

static void *iem_samplerate_tilde_new(t_symbol *s)
{
  t_iem_samplerate_tilde *x = (t_iem_samplerate_tilde *)pd_new(iem_samplerate_tilde_class);
  x->x_clock = clock_new(x, (t_method)iem_samplerate_tilde_out);
  outlet_new(&x->x_obj, &s_float);
  x->x_samplerate = 44100.0f;
  x->x_f = 0.0f;
  return (x);
}

static void iem_samplerate_tilde_dsp(t_iem_samplerate_tilde *x, t_signal **sp)
{
  x->x_samplerate = (t_float)(sp[0]->s_sr);
  clock_delay(x->x_clock, 0.0f);
}

void iem_samplerate_tilde_setup(void)
{
  iem_samplerate_tilde_class = class_new(gensym("iem_samplerate~"), (t_newmethod)iem_samplerate_tilde_new,
    (t_method)iem_samplerate_tilde_free, sizeof(t_iem_samplerate_tilde), 0, 0);
  CLASS_MAINSIGNALIN(iem_samplerate_tilde_class, t_iem_samplerate_tilde, x_f);
  class_addmethod(iem_samplerate_tilde_class, (t_method)iem_samplerate_tilde_dsp, gensym("dsp"), 0);
}
