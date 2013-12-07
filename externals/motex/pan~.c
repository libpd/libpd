/*************************************************************************** 
 * File: pan~.c 
 * Auth: Iain Mott [iain.mott@bigpond.com] 
 * Maintainer: Iain Mott [iain.mott@bigpond.com] 
 * Version: Part of motex_1.1.2 
 * Date: January 2001
 * 
 * Description: Pd signal external. Equal-power stereo panning
 * Angle input specified in degrees. -45 left, 0 centre, 45 right. 
 * See supporting Pd patch: pan~.pd
 * 
 * Copyright (C) 2001 by Iain Mott [iain.mott@bigpond.com] 
 * 
 * This program is free software; you can redistribute it and/or modify 
 * it under the terms of the GNU General Public License as published by 
 * the Free Software Foundation; either version 2, or (at your option) 
 * any later version. 
 * 
 * This program is distributed in the hope that it will be useful, 
 * but WITHOUT ANY WARRANTY; without even the implied warranty of 
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the 
 * GNU General Public License, which should be included with this 
 * program, for more details. 
 * 
 ****************************************************************************/ 


#include "m_pd.h"
#include <math.h>

static t_class *pan_class;
#define RADCONST 0.017453293
#define ROOT2DIV2 0.707106781

typedef struct _pan
{
  t_object x_obj;
  float x_f;
  float pan;
  float left;
  float right;
} t_pan;

static void *pan_new(t_symbol *s, int argc, t_atom *argv)
{
  t_pan *x = (t_pan *)pd_new(pan_class);
  outlet_new(&x->x_obj, gensym("signal"));
  outlet_new(&x->x_obj, gensym("signal"));
  inlet_new(&x->x_obj, &x->x_obj.ob_pd, gensym("float"), gensym("panf"));

  x->x_f = 0;
  x->left = ROOT2DIV2;
  x->right = ROOT2DIV2;
  return (x);
}

static t_int *pan_perform(t_int *w)
{
  float *in1 = (t_float *)(w[1]);
  float *out1 = (t_float *)(w[2]);
  float *out2 = (t_float *)(w[3]);
  int n = (int)(w[4]);
  t_pan *x = (t_pan *)(w[5]);
  float value;
  while  (n--) 
    {
      value = *in1++;
      *out1++ = value * x->left;
      *out2++ = value * x->right; 
    }
  return (w+6);
}

static void pan_dsp(t_pan *x, t_signal **sp)
{
  int n = sp[0]->s_n;
  float *in1 = sp[0]->s_vec;
  float *out1 = sp[1]->s_vec;
  float *out2 = sp[2]->s_vec;

  dsp_add(pan_perform, 5,
	  in1, out1, out2, n, x);
}

void pan_f(t_pan *x, t_floatarg f)
{
  double angle;
  f = f < -45 ? -45 : f;
  f = f > 45 ? 45 : f;
  angle = f * RADCONST; // convert degrees to radians
  x->right  = ROOT2DIV2 * (cos(angle) + sin(angle));
  x->left  = ROOT2DIV2 * (cos(angle) - sin(angle));
/*    post("left = %f : right = %f", x->left, x->right); */
}

void pan_tilde_setup(void)
{
  pan_class = class_new(gensym("pan~"), (t_newmethod)pan_new, 0,
			sizeof(t_pan), 0, A_GIMME, 0);
  class_addmethod(pan_class, nullfn, gensym("signal"), 0);

  class_addmethod(pan_class, (t_method)pan_dsp, gensym("dsp"), 0);
  class_addmethod(pan_class, (t_method)pan_f, gensym("panf"), A_FLOAT, 0);  
}
