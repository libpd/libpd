/*************************************************************************** 
 * File: ln~.c 
 * Auth: Iain Mott [iain.mott@bigpond.com] 
 * Maintainer: Iain Mott [iain.mott@bigpond.com] 
 * Version: Part of motex_1.1.2 
 * Date: January 2001
 * 
 * Description: Pd signal external. Finds natural log of signal. Optional argument '-1' finfs inverse. 
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

#include "math.h"
#include <m_pd.h>

/* ----------------------------- ln ----------------------------- */
static t_class *ln_class;


#define INVTWOPI 0.15915494f

typedef struct _ln
{
  t_object x_obj;
  int flag;
} t_ln;

/*  static void *ln_new(t_symbol *s, int argc, t_atom *argv) */
static void *ln_new(t_floatarg f)
{
/*      if (argc > 1)  */
/*        post("+~: extra arguments ignored"); */
/*      { */
	t_ln *x = (t_ln *)pd_new(ln_class);
	outlet_new(&x->x_obj, &s_signal);
	x->flag = f;
	return (x);
/*      } */
}

t_int *ln_perform(t_int *w)
{
    t_float *in1 = (t_float *)(w[1]);
    t_float *out = (t_float *)(w[2]);

    int n = (int)(w[3]);
    int flag = (int)(w[4]);
    if(flag != -1)
      while (n--) *out++ = (t_float) log(*in1++); 
    else
      while (n--) *out++ = (t_float) exp(*in1++); 
    return (w+5);
}

t_int *ln_perf8(t_int *w)
{
  t_float *in1 = (t_float *)(w[1]);
  t_float *out = (t_float *)(w[2]);
  int n = (int)(w[3]);
  int flag = (int)(w[4]);
  if(flag != -1)
    {
      for (; n; n -= 8, in1 += 8,  out += 8)
	{
	  float f0 = in1[0], f1 = in1[1], f2 = in1[2], f3 = in1[3];
	  float f4 = in1[4], f5 = in1[5], f6 = in1[6], f7 = in1[7];

	  out[0] = (t_float) log(f0); 
	  out[1] = (t_float) log(f1); 
	  out[2] = (t_float) log(f2); 
	  out[3] = (t_float) log(f3); 
	  out[4] = (t_float) log(f4); 
	  out[5] = (t_float) log(f5); 
	  out[6] = (t_float) log(f6); 
	  out[7] = (t_float) log(f7); 
	}
    }
  else
    {
      for (; n; n -= 8, in1 += 8,  out += 8)
	{
	  float f0 = in1[0], f1 = in1[1], f2 = in1[2], f3 = in1[3];
	  float f4 = in1[4], f5 = in1[5], f6 = in1[6], f7 = in1[7];

	  out[0] = (t_float) exp(f0); 
	  out[1] = (t_float) exp(f1); 
	  out[2] = (t_float) exp(f2); 
	  out[3] = (t_float) exp(f3); 
	  out[4] = (t_float) exp(f4); 
	  out[5] = (t_float) exp(f5); 
	  out[6] = (t_float) exp(f6); 
	  out[7] = (t_float) exp(f7); 
	}
    }

  return (w+5);
}

void dsp_add_ln(t_sample *in1, t_sample *out, int n, int flag)
{
    if (n&7)
    	dsp_add(ln_perform, 4, in1, out, n, flag);
    else	
    	dsp_add(ln_perf8, 4, in1, out, n, flag);
}

static void ln_dsp(t_ln *x, t_signal **sp)
{
    dsp_add_ln(sp[0]->s_vec, sp[1]->s_vec, sp[0]->s_n, x->flag);
}

void ln_tilde_setup(void)
{
    ln_class = class_new(gensym("ln~"), (t_newmethod)ln_new, 0,
    	sizeof(t_ln), 0, A_DEFFLOAT, 0);
    class_addmethod(ln_class, nullfn, gensym("signal"), 0);
    class_addmethod(ln_class, (t_method)ln_dsp, gensym("dsp"), 0);
}
