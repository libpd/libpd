/*
 *   diag.c  - diagonal state space system.
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

typedef struct diagctl
{
  t_float *c_state;
  t_float *c_eigen;
  t_int c_order;
} t_diagctl;

typedef struct diag
{
  t_object x_obj;
  t_float x_f;
  t_diagctl x_ctl;
} t_diag;


static t_float randfloat(void){
  t_float r = rand ();
  r /= (RAND_MAX/2);
  r -= 1;
  return r;

}

static void diag_eigen(t_diag *x, t_floatarg index, t_floatarg val)
{
  int i = (int)index;
  if (i<0) return;
  if (i>=x->x_ctl.c_order) return;
  x->x_ctl.c_eigen[i] = val;
}

/* set decay time of pole at index */
static void diag_time(t_diag *x, t_floatarg index, t_floatarg time)
{
  t_float r;

  /* time in ms */
  time *= 0.001;

  if (time < 0.0) time = 0.0;
  r = pow(0.001, (t_float)x->x_ctl.c_order / (time * sys_getsr()));
  if (r < 0.0) r = 0.0;
  if (r > 1.0) r = 1.0;

  diag_eigen(x, index, r);
}



static void diag_reset(t_diag *x)
{
  int i;

  for (i=0; i<x->x_ctl.c_order; i++)
    { 
      x->x_ctl.c_state[i] = 0;
    }
  
}

static void diag_random(t_diag *x)
{
  int i;

  for (i=0; i<x->x_ctl.c_order; i++)
    { 
      x->x_ctl.c_state[i] = randfloat();
    }
  
}



static t_int *diag_perform(t_int *w)
{


  t_float *in     = (t_float *)(w[3]);
  t_float *out    = (t_float *)(w[4]);
  t_diagctl *ctl  = (t_diagctl *)(w[1]);

  t_float *eigen = ctl->c_eigen;
  t_float *state = ctl->c_state;
  t_int n = (t_int)(w[2]);
  t_float newstate;

  int i;

  for (i=0; i<n; i++)
    {
      newstate = (*state + *in) * (*eigen);
      newstate = IS_DENORMAL(newstate) ? 0 : newstate;
      *state   = newstate;
      *out     = newstate;

      in++;
      out++;
      state++;
      eigen++;
    }



  return (w+5);
}


static void diag_dsp(t_diag *x, t_signal **sp)
{

  int n = sp[0]->s_n;
  int i;

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



  dsp_add(diag_perform, 4, &x->x_ctl, sp[0]->s_n, sp[0]->s_vec, sp[1]->s_vec);

}
                                  
static void diag_free(t_diag *x)
{

  if (x->x_ctl.c_state) free(x->x_ctl.c_state);
  if (x->x_ctl.c_eigen) free(x->x_ctl.c_eigen);


}

t_class *diag_class;


static void *diag_new(t_floatarg permute)
{
    t_diag *x = (t_diag *)pd_new(diag_class);
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


void diag_tilde_setup(void)
{
  //post("diag~ v0.1");
    diag_class = class_new(gensym("diag~"), (t_newmethod)diag_new,
    	(t_method)diag_free, sizeof(t_diag), 0, A_DEFFLOAT, 0);
    CLASS_MAINSIGNALIN(diag_class, t_diag, x_f);
    class_addmethod(diag_class, (t_method)diag_dsp, gensym("dsp"), 0); 
    class_addmethod(diag_class, (t_method)diag_reset, gensym("reset"), 0); 
    class_addmethod(diag_class, (t_method)diag_random, gensym("random"), 0); 
    class_addmethod(diag_class, (t_method)diag_random, gensym("bang"), 0); 
    class_addmethod(diag_class, (t_method)diag_eigen, gensym("eigen"), A_DEFFLOAT, A_DEFFLOAT, 0); 
    class_addmethod(diag_class, (t_method)diag_time, gensym("time"), A_DEFFLOAT, A_DEFFLOAT, 0); 

}

