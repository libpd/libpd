/*************************************************************************** 
 * File: pol2rec~.c 
 * Auth: Iain Mott [iain.mott@bigpond.com] 
 * Maintainer: Iain Mott [iain.mott@bigpond.com] 
 * Version: Part of motex_1.1.2 
 * Date: January 2001
 * 
 * Description: Pd signal external. Converts polar coordinates into rectangular coordinates.  
 * Used in conjuction with rec2pol~ (also in motex) and rfft~
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

static t_class *pol2rec_class;

#define HALFPI 1.570796327
#define PI 3.141592654
#define PIHALF 4.71238898
typedef struct _pol2rec
{
  t_object x_obj;
  float x_f;
  float pol2rec;
  float left;
  float right;
} t_pol2rec;

static void *pol2rec_new(t_symbol *s, int argc, t_atom *argv)
{
  t_pol2rec *x = (t_pol2rec *)pd_new(pol2rec_class);
  inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_signal, &s_signal);
  outlet_new(&x->x_obj, gensym("signal"));
  outlet_new(&x->x_obj, gensym("signal"));
  x->x_f = 0;
  return (x);
}

/*  static void pol2rec_doit(t_float phase, t_float mag, double *x, double *y) */
/*  { */
/*  	*x = mag*cos(phase); */
/*  	*y = mag*sin(phase);     */
/*  } */

static t_int *pol2rec_perform8(t_int *w)
{
  t_float *in1 = (t_float *)(w[1]);
  t_float *in2 = (t_float *)(w[2]);
  t_float *out1 = (t_float *)(w[3]);
  t_float *out2 = (t_float *)(w[4]);
  int n = (int)(w[5]);
  for (; n; n -= 8, in1 += 8,  out1 += 8, in2 += 8,  out2 += 8)
    {
      t_float p0 = in1[0], p1 = in1[1], p2 = in1[2], p3 = in1[3];
      t_float p4 = in1[4], p5 = in1[5], p6 = in1[6], p7 = in1[7];
      t_float m0 = in2[0], m1 = in2[1], m2 = in2[2], m3 = in2[3];
      t_float m4 = in2[4], m5 = in2[5], m6 = in2[6], m7 = in2[7];
      out1[0] = m0*cos(p0); 
      out1[1] = m1*cos(p1);
      out1[2] = m2*cos(p2);
      out1[3] = m3*cos(p3);
      out1[4] = m4*cos(p4); 
      out1[5] = m5*cos(p5); 
      out1[6] = m6*cos(p6);
      out1[7] = m7*cos(p7); 
      out2[0] = m0*sin(p0); 
      out2[1] = m1*sin(p1);
      out2[2] = m2*sin(p2);
      out2[3] = m3*sin(p3);
      out2[4] = m4*sin(p4); 
      out2[5] = m5*sin(p5); 
      out2[6] = m6*sin(p6);
      out2[7] = m7*sin(p7); 
    }
  return (w+6);
}

static t_int *pol2rec_perform(t_int *w)
{
  float *in1 = (t_float *)(w[1]);
  float *in2 = (t_float *)(w[2]);
  float *out1 = (t_float *)(w[3]);
  float *out2 = (t_float *)(w[4]);
  int n = (int)(w[5]);
  double phase, mag, x, y;
  while  (n--) 
    {
      phase = *in1++; mag = *in2++;
      *out1++ = mag*cos(phase);
      *out2++ = mag*sin(phase);;      
    }
  return (w+6);
}

static void pol2rec_dsp(t_pol2rec *x, t_signal **sp)
{
  int n = sp[0]->s_n;
  float *in1 = sp[0]->s_vec;
  float *in2 = sp[1]->s_vec;
  float *out1 = sp[2]->s_vec;
  float *out2 = sp[3]->s_vec;

    if (n&7)
      {
      dsp_add(pol2rec_perform, 5, in1, in2, out1, out2, n);
      post("it's a seven");
      }
    else
      dsp_add(pol2rec_perform8, 5, in1, in2, out1, out2, n);
}



void pol2rec_tilde_setup(void)
{
  pol2rec_class = class_new(gensym("pol2rec~"), (t_newmethod)pol2rec_new, 0,
			    sizeof(t_pol2rec), 0, A_GIMME, 0);
  class_addmethod(pol2rec_class, nullfn, gensym("signal"), 0);

  class_addmethod(pol2rec_class, (t_method)pol2rec_dsp, gensym("dsp"), 0);
  CLASS_MAINSIGNALIN(pol2rec_class, t_pol2rec, x_f);

}
