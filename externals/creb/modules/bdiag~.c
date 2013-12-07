/*
 *   bdiag.c  - block diagonal state space system
 *   treats input dsp block as n parallel signals
 *
 *     s1 = (a * s1) + (b * s2) + u1;
 *     s2 = (a * s2) - (b * s1) + u2;
 *
 *   Copyright (c) 2000-2003 by Tom Schouten
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include "extlib_util.h"
#include <stdio.h>
#include <stdlib.h>

#define MAXORDER 64

typedef struct bdiagctl
{
  t_float *c_state;
  t_float *c_eigen;
  t_int c_order;
} t_bdiagctl;

typedef struct bdiag
{
  t_object x_obj;
  t_float x_f;
  t_bdiagctl x_ctl;
} t_bdiag;


static t_float randfloat(void){
  t_float r = rand ();
  r /= (RAND_MAX/2);
  r -= 1;
  return r;

}

static void bdiag_random(t_bdiag *x)
{
  int i;

  for (i=0; i<x->x_ctl.c_order; i++)
    { 
      x->x_ctl.c_state[i] = randfloat();
    }
  
}


static void bdiag_reset(t_bdiag *x)
{
  int i;

  for (i=0; i<x->x_ctl.c_order; i++)
    { 
      x->x_ctl.c_state[i] = 0;
    }
  
}






static void bdiag_eigen(t_bdiag *x, t_floatarg index, 
			t_floatarg aval, t_floatarg bval)
{
  int i = (int)index;
  if (i<0) return;
  if (i>=x->x_ctl.c_order/2) return;
  x->x_ctl.c_eigen[2*i+0] = aval;
  x->x_ctl.c_eigen[2*i+1] = bval;
  
}

/* set decay time and frequency of pole at index */
static void bdiag_timefreq(t_bdiag *x, t_floatarg index, 
			   t_floatarg time, t_floatarg freq)
{
  t_float r,a,b,n;
  t_float sr = sys_getsr() / (t_float)x->x_ctl.c_order;

  /* time in ms */
  time *= 0.001;

  if (time < 0.0) time = 0.0;
  r = pow(0.001, 1.0 / (time * sr));
  if (r < 0.0) r = 0.0;
  if (r > 1.0) r = 1.0;

  a = cos(2*M_PI*freq/sr);
  b = sin(2*M_PI*freq/sr);
 
  /* normalize to be sure */
  n = 1.0 / sqrt(a*a + b*b);
  a *= n;
  b *= n;

  bdiag_eigen(x, index, r*a, r*b);
}

static void bdiag_preset(t_bdiag *x, t_floatarg preset)
{
  int p = preset;
  int i;
  t_float a, b, w, r;

  switch(p){
  case 0:
    post("preset 0");
    for (i=0; i<x->x_ctl.c_order/2; i++){
      w = randfloat() * .001;
      r = 1. - (((t_float)i + 1.)/1000.);
      a = cos(w) * r;
      b = sin(w) * r;
      post("%f %f %f %f", w, r, a, b);
      bdiag_eigen(x,i,a,b);
    }
    break;
  case 1:
  default:
    break;
  
  }
}

static t_int *bdiag_perform(t_int *w)
{


  t_float *in     = (t_float *)(w[3]);
  t_float *out    = (t_float *)(w[4]);
  t_bdiagctl *ctl  = (t_bdiagctl *)(w[1]);

  t_float *eigen = ctl->c_eigen;
  t_float *state = ctl->c_state;
  t_int n = (t_int)(w[2]);

  t_float u1,u2,a,b,s1,s2,s1new,s2new;

  int i;

  for (i=0; i<n; i+=2)
    {
      u1 = *in++;
      u2 = *in++;
      a = *eigen++; /* real part */
      b = *eigen++; /* imag part */
      s1 = state[0];
      s2 = state[1];


      s1new = (a * s1) - (b * s2) + u1; /* update state */
      s2new = (a * s2) + (b * s1) + u2;

      s1new = IS_DENORMAL(s1new) ? 0 : s1new; /* clear denormals */
      s2new = IS_DENORMAL(s2new) ? 0 : s2new;

      *state++ = s1new; /* store state */
      *state++ = s2new;

      *out++ = s1new; /* output state */
      *out++ = s2new;
    }

  return (w+5);
}


static void bdiag_dsp(t_bdiag *x, t_signal **sp)
{

  int n = sp[0]->s_n;
  int i;

  if (n == 1)
    {
      post("bdiag: doesnt work with blocksize == 1");
      dsp_add_copy(sp[0]->s_vec, sp[1]->s_vec, sp[0]->s_n);
    }
  else
    {
      if (x->x_ctl.c_order != n)
	{
	  if (x->x_ctl.c_state) free(x->x_ctl.c_state);
	  if (x->x_ctl.c_eigen) free(x->x_ctl.c_eigen);
	  
	  x->x_ctl.c_state = (t_float *)malloc(n*sizeof(t_float));
	  x->x_ctl.c_eigen = (t_float *)malloc(n*sizeof(t_float));
	  
	  for(i=0;i<n;i++)
	    {
	      x->x_ctl.c_state[i] = 0;
	      x->x_ctl.c_eigen[i] = 0;
	    }
	  
	  x->x_ctl.c_order = n;
	}
      
           
      dsp_add(bdiag_perform, 4, &x->x_ctl, sp[0]->s_n, 
	      sp[0]->s_vec, sp[1]->s_vec);
    }

}
                                  
static void bdiag_free(t_bdiag *x)
{

  if (x->x_ctl.c_state) free(x->x_ctl.c_state);
  if (x->x_ctl.c_eigen) free(x->x_ctl.c_eigen);


}

t_class *bdiag_class;


static void *bdiag_new(t_floatarg permute)
{
    t_bdiag *x = (t_bdiag *)pd_new(bdiag_class);
    int i, n=64;

    outlet_new(&x->x_obj, gensym("signal")); 

    x->x_ctl.c_state = (t_float *)malloc(n*sizeof(t_float));
    x->x_ctl.c_eigen = (t_float *)malloc(n*sizeof(t_float));

    for(i=0;i<n;i++)
      {
	x->x_ctl.c_state[i] = 0;
	x->x_ctl.c_eigen[i] = 0;
      }
    
    x->x_ctl.c_order = n;


    return (void *)x;


}


void bdiag_tilde_setup(void)
{
  //post("bdiag~ v0.1");
    bdiag_class = class_new(gensym("bdiag~"), (t_newmethod)bdiag_new,
    	(t_method)bdiag_free, sizeof(t_bdiag), 0, A_DEFFLOAT, 0);
    CLASS_MAINSIGNALIN(bdiag_class, t_bdiag, x_f);
    class_addmethod(bdiag_class, (t_method)bdiag_random, gensym("random"), 0);
    class_addmethod(bdiag_class, (t_method)bdiag_random, gensym("bang"), 0);
    class_addmethod(bdiag_class, (t_method)bdiag_reset, gensym("reset"), 0);
    class_addmethod(bdiag_class, (t_method)bdiag_dsp, gensym("dsp"), 0); 

    class_addmethod(bdiag_class, (t_method)bdiag_eigen, gensym("eigen"), 
		    A_DEFFLOAT, A_DEFFLOAT, A_DEFFLOAT, 0); 
    class_addmethod(bdiag_class, (t_method)bdiag_timefreq, gensym("timefreq"), 
		    A_DEFFLOAT, A_DEFFLOAT, A_DEFFLOAT, 0); 
    class_addmethod(bdiag_class, (t_method)bdiag_preset, gensym("preset"), 
		    A_DEFFLOAT, 0); 
}

