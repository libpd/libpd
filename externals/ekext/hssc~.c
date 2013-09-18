/*
 *  hssc~ : Highest Significant Spectral Component, according to amplitude ratio to
 *  Strongest Significant Spectral Component. 
 *  Copyright (C) 2005 Edward Kelly <morph_2016@yahoo.co.uk>
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
#include <math.h>

static t_class *hssc_tilde_class;

typedef struct _hssc_tilde 
{
  t_object x_obj;
  t_float f;
  t_float f_maxbin, f_minbin, f_ratio;
  t_outlet *f_hssc, *f_sssc;
} t_hssc_tilde;

t_int *hssc_tilde_perform(t_int *w)
{
  t_hssc_tilde *x = (t_hssc_tilde *)(w[1]);
  t_sample  *real =     (t_sample *)(w[2]);
  t_sample  *imag =     (t_sample *)(w[3]);
  int           n =            (int)(w[4]);
  int incr = 0;
  double vectorr, vectori;
  double max = 0;
  double alpha;
  x->f_maxbin = x->f_minbin = 0;
  x->f_ratio = x->f_ratio > 0 ? x->f_ratio : 100;

  while (n--)
    {
      vectorr = (*real++);
	  vectori = (*imag++);
	  alpha = sqrt((vectorr * vectorr) + (vectori * vectori));
      x->f_maxbin = alpha > max ? incr : x->f_maxbin;
      max = alpha > max ? alpha : max;
      x->f_minbin = alpha > (max / x->f_ratio) ? incr : x->f_minbin;
      incr++;
    }
  outlet_float(x->f_sssc, x->f_maxbin);	
  outlet_float(x->f_hssc, x->f_minbin);
  
  return(w+5);
}

void hssc_tilde_dsp(t_hssc_tilde *x, t_signal **sp)
{
  dsp_add(hssc_tilde_perform, 4, x,
	  sp[0]->s_vec, sp[1]->s_vec, sp[0]->s_n);
}

void *hssc_tilde_new(t_floatarg f)
{
  t_hssc_tilde *x = (t_hssc_tilde *)pd_new(hssc_tilde_class);

  x->f_ratio = f;

  inlet_new (&x->x_obj, &x->x_obj.ob_pd, &s_signal, &s_signal);
  floatinlet_new (&x->x_obj, &x->f_ratio);
  x->f_hssc = outlet_new(&x->x_obj, gensym("float"));
  x->f_sssc = outlet_new(&x->x_obj, gensym("float"));

  return (void *)x;
}


void hssc_tilde_setup(void)
{
  hssc_tilde_class = class_new(gensym("hssc~"),
				     (t_newmethod)hssc_tilde_new,
				     0, sizeof(t_hssc_tilde),
				     CLASS_DEFAULT, A_DEFFLOAT, 0);

  post("|=================hssc~==================|");
  post("|=highest significant spectral component=|");
  post("|======edward=======kelly=======2005=====|");

  class_addmethod(hssc_tilde_class, (t_method)hssc_tilde_dsp,
		  gensym("dsp"), 0);

  CLASS_MAINSIGNALIN(hssc_tilde_class, t_hssc_tilde, f);
}
