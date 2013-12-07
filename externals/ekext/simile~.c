/*
 *  simile~ : windowed similarity comparison for signals
 *  Copyright (C) 2005 edward kelly <morph_2016@yahoo.co.uk>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include "m_pd.h"

static t_class *simile_tilde_class;

typedef struct _simile_tilde {
  t_object x_obj;
  t_sample f_win;
  t_float x_dummy;
  t_outlet *f_frame;
} t_simile_tilde;

t_int *simile_tilde_perform(t_int *w) {
  t_simile_tilde *x = (t_simile_tilde *)(w[1]);
  t_sample     *in1 =       (t_sample *)(w[2]);
  t_sample     *in2 =       (t_sample *)(w[3]);
  t_sample     *out =       (t_sample *)(w[4]);
  t_sample    *sign =       (t_sample *)(w[5]);
  int             n =              (int)(w[6]);

  t_sample f_win    = x->f_win > 0 ? x->f_win : 0.01;
  t_float f_accum   = 0;
  while (n--) {
    float i1=(*in1++);
    float i2=(*in2++);
    float win = x->f_win > 0 ? x->f_win : 0.001;
    float min = i1 > i2 ? i1 - i2 : i2 - i1;
    f_accum += *out++ = 1/((min/win)+1);
    *sign++ = i1 >= i2 ? 1 : -1;
  }
  outlet_float(x->f_frame, f_accum);
  return (w+7);
}

void simile_tilde_dsp(t_simile_tilde *x, t_signal **sp) {
  dsp_add(simile_tilde_perform, 6, x, 
	  sp[0]->s_vec, sp[1]->s_vec, sp[2]->s_vec, sp[3]->s_vec, sp[0]->s_n);
}

void *simile_tilde_new(t_floatarg f) {
  t_simile_tilde *x = (t_simile_tilde *)pd_new(simile_tilde_class);

  x->f_win = f > 0 ? f : 0.01; /* window defaults to 0.01 if no argument given */

  inlet_new (&x->x_obj, &x->x_obj.ob_pd, &s_signal, &s_signal);
  floatinlet_new (&x->x_obj, &x->f_win);
  outlet_new(&x->x_obj, &s_signal);
  outlet_new(&x->x_obj, &s_signal);
  x->f_frame = outlet_new(&x->x_obj, gensym("float"));
  return (void *)x;
}

void simile_tilde_setup(void) {
  simile_tilde_class = class_new(gensym("simile~"), 
  (t_newmethod)simile_tilde_new, 
  0, sizeof(t_simile_tilde),
  CLASS_DEFAULT, A_DEFFLOAT, 0);

  post("|~~~~~~~~~~~~&simile~&~~~~~~~~~~~~~~|");
  post("|~&weighted similarity measurement&~|");
  post("|~~&edward&~~~~~&kelly&~~~~~&2005&~~|");


  class_addmethod(simile_tilde_class,
  (t_method)simile_tilde_dsp, gensym("dsp"), 0);
  CLASS_MAINSIGNALIN(simile_tilde_class, t_simile_tilde, x_dummy);
}
