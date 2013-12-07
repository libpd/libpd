/*************************************************************************** 
 * File: rec2pol~.c 
 * Auth: Iain Mott [iain.mott@bigpond.com] 
 * Maintainer: Iain Mott [iain.mott@bigpond.com] 
 * Version: Part of motex_1.1.2 
 * Date: January 2001
 * 
 * Description: Pd signal external. Converts rectangular coordinates to polar. 
 * Used to convert sine & cosine output of 'rfft~' object to phase and magnitude values.
 * Used in conjunction with pol2rec~ external in motex
 * See supporting Pd patch: rec2pol~.pd & polcoc~.pd
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

static t_class *rec2pol_class;

typedef struct _rec2pol
{
  t_object x_obj;
  float x_f;
} t_rec2pol;

static void *rec2pol_new(t_symbol *s, int argc, t_atom *argv)
{
  t_rec2pol *x = (t_rec2pol *)pd_new(rec2pol_class);
  inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_signal, &s_signal);
  outlet_new(&x->x_obj, gensym("signal"));
  outlet_new(&x->x_obj, gensym("signal"));
  x->x_f = 0;
  return (x);
}

/* t_int *sigsqrt_perform(t_int *w); */


static t_int *rec2pol_perform8(t_int *w)
{
  t_float *in1 = (t_float *)(w[1]);
  t_float *in2 = (t_float *)(w[2]);
  t_float *out1 = (t_float *)(w[3]);
  t_float *out2 = (t_float *)(w[4]);
  int n = (int)(w[5]);
  for (; n; n -= 8, in1 += 8,  out1 += 8, in2 += 8,  out2 += 8)
    {
      t_float X0 = in1[0], X1 = in1[1], X2 = in1[2], X3 = in1[3];
      t_float X4 = in1[4], X5 = in1[5], X6 = in1[6], X7 = in1[7];
      t_float Y0 = in2[0], Y1 = in2[1], Y2 = in2[2], Y3 = in2[3];
      t_float Y4 = in2[4], Y5 = in2[5], Y6 = in2[6], Y7 = in2[7];
      out1[0] = atan2(Y0, X0); 
      out1[1] = atan2(Y1, X1);
      out1[2] = atan2(Y2, X2); 
      out1[3] = atan2(Y3, X3); 
      out1[4] = atan2(Y4, X4); 
      out1[5] = atan2(Y5, X5); 
      out1[6] = atan2(Y6, X6); 
      out1[7] = atan2(Y7, X7); 
      out2[0] = X0*X0 + Y0*Y0; 
      out2[1] = X1*X1 + Y1*Y1;
      out2[2] = X2*X2 + Y2*Y2; 
      out2[3] = X3*X3 + Y3*Y3; 
      out2[4] = X4*X4 + Y4*Y4; 
      out2[5] = X5*X5 + Y5*Y5; 
      out2[6] = X6*X6 + Y6*Y6; 
      out2[7] = X7*X7 + Y7*Y7; 
    }
  return (w+6);
}

static t_int *rec2pol_perform(t_int *w)
{
  float *in1 = (t_float *)(w[1]);
  float *in2 = (t_float *)(w[2]);
  float *out1 = (t_float *)(w[3]);
  float *out2 = (t_float *)(w[4]);
  int n = (int)(w[5]);
  float x, y;
  //  double angle;
  while  (n--) 
    {
      x = *in1++;
      y = *in2++;
      *out1++ = atan2(y, x);
      *out2++ = x*x + y*y;
    }
  return (w+6);
}

static void rec2pol_dsp(t_rec2pol *x, t_signal **sp)
{
  int n = sp[0]->s_n;
  /*    int n2 = (n>>1); */
  float *in1 = sp[0]->s_vec;
  float *in2 = sp[1]->s_vec;
  float *out1 = sp[2]->s_vec;
  float *out2 = sp[3]->s_vec;
  if (n&7)
    dsp_add(rec2pol_perform, 5, in1, in2, out1, out2, n);
  else 
    dsp_add(rec2pol_perform8, 5, in1, in2, out1, out2, n);
/*   dsp_add(sigsqrt_perform, 3, out2, out2, n); */
}



void rec2pol_tilde_setup(void)
{
  rec2pol_class = class_new(gensym("rec2pol~"), (t_newmethod)rec2pol_new, 0,
			    sizeof(t_rec2pol), 0, A_GIMME, 0);
  class_addmethod(rec2pol_class, nullfn, gensym("signal"), 0);

  class_addmethod(rec2pol_class, (t_method)rec2pol_dsp, gensym("dsp"), 0);
  CLASS_MAINSIGNALIN(rec2pol_class, t_rec2pol, x_f);
}





