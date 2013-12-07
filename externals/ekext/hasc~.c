/*
 *  hasc~ : Highest Apparent Spectral Component, according to amplitude threshold 
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

#ifdef _WIN32
#pragma warning( disable : 4244 )
#pragma warning( disable : 4305 )
#endif

static t_class *hasc_tilde_class;

typedef struct _hasc_tilde 
{
  t_object x_obj;
  t_float f;
  t_float f_topbin, f_thresh;
  t_outlet *f_hasc, *f_levs;
} t_hasc_tilde;

t_int *hasc_tilde_perform(t_int *w)
{
  t_hasc_tilde *x = (t_hasc_tilde *)(w[1]);
  t_sample  *real =     (t_sample *)(w[2]);
  t_sample  *imag =     (t_sample *)(w[3]);
  int           n =            (int)(w[4]);
  int incr = 0;
  double max = 0;
  double vectorr, vectori;
  double alpha;
  x->f_topbin = 0;
  x->f_thresh = x->f_thresh > 0 ? x->f_thresh : 1;

  while (n--)
    {
      vectorr = (*real++);
      vectori = (*imag++);
      alpha = sqrt((vectorr * vectorr) + (vectori * vectori));
      max = alpha > max ? alpha : max;
      x->f_topbin = alpha > x->f_thresh ? incr : x->f_topbin;
      incr++;
    }
  outlet_float(x->f_levs, (float)max);
  outlet_float(x->f_hasc, x->f_topbin);

  return(w+5);
}

void hasc_tilde_dsp(t_hasc_tilde *x, t_signal **sp)
{
  dsp_add(hasc_tilde_perform, 4, x,
	  sp[0]->s_vec, sp[1]->s_vec, sp[0]->s_n);
}

void *hasc_tilde_new(t_floatarg f)
{
  t_hasc_tilde *x = (t_hasc_tilde *)pd_new(hasc_tilde_class);

  x->f_thresh = f;

  inlet_new (&x->x_obj, &x->x_obj.ob_pd, &s_signal, &s_signal);
  floatinlet_new (&x->x_obj, &x->f_thresh);
  x->f_hasc = outlet_new(&x->x_obj, gensym("float"));
  x->f_levs = outlet_new(&x->x_obj, gensym("float"));
  return (void *)x;
}


void hasc_tilde_setup(void)
{
  hasc_tilde_class = class_new(gensym("hasc~"),
				     (t_newmethod)hasc_tilde_new,
				     0, sizeof(t_hasc_tilde),
				     CLASS_DEFAULT, A_DEFFLOAT, 0);

  post("|===============hasc~=================|");
  post("|=highest apparent spectral component=|");
  post("|=====edward======kelly======2005=====|");

  class_addmethod(hasc_tilde_class, (t_method)hasc_tilde_dsp,
		  gensym("dsp"), 0);

  CLASS_MAINSIGNALIN(hasc_tilde_class, t_hasc_tilde, f);
}
