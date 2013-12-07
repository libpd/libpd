/*
 *   scrollgrid1D.c  -  1D scroll grid attractor
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


/*  1D scroll grid attractor
    for more information see:

    Yalcin M., Ozoguz S., Suykens J.A.K., Vandewalle J., ``Families of
    Scroll Grid Attractors'', International Journal of Bifurcation and
    Chaos, vol. 12, no. 1, Jan. 2002, pp. 23-41.

    this file implements a digital variant of the method introduced in
    the paper, so that it can be used as a parametrizable, bounded
    chatotic oscillator.  in short it is a switched linear system,
    with some added hard limiting to convert unstable oscillations
    into stable ones.
    
*/

#include "m_pd.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>  
#include "filters.h"


typedef struct scrollgrid1Dctl
{
    t_float c_x, c_y, c_z; /* state */

} t_scrollgrid1Dctl;

typedef struct scrollgrid1D
{
    t_object x_obj;
    t_float x_f;
    t_scrollgrid1Dctl x_ctl;
} t_scrollgrid1D;


static inline t_float _fixedpoint(t_float x, int n)
{
  int ix = (x + 0.5);
  if (ix < 0) ix = 0;
  else if (ix >= n) ix = n-1;
  return (t_float)ix;
}

static inline t_float _sat(t_float x, t_float upper)
{
    t_float lower = -1.0;
    if (x < lower) x = lower;
    else if (x > upper) x = upper;
    return x;
}

static t_int *scrollgrid1D_perform(t_int *w)
{


  t_float *freq    = (t_float *)(w[3]);
  t_float *t1      = (t_float *)(w[4]);
  t_float *t2      = (t_float *)(w[5]);
  t_float *order   = (t_float *)(w[6]);
  t_float *outx    = (t_float *)(w[7]);
  t_float *outy    = (t_float *)(w[8]);
  t_float *outz    = (t_float *)(w[9]);
  t_scrollgrid1Dctl *ctl    = (t_scrollgrid1Dctl *)(w[1]);
  t_int n          = (t_int)(w[2]);
  
  t_int i;
  t_float inv_sr = 1.0 /sys_getsr();
  t_float state[3] = {ctl->c_x, ctl->c_y, ctl->c_z};
  t_float c,f;
  t_float pole[2], r1, r2;
  t_int o;
  t_float x,y,z;


  for (i=0; i<n; i++){

      /* get params */
      r1 = exp(1000.0 * inv_sr / (0.01 + fabs(*t1++)));
      r2 = exp(-1000.0 * inv_sr / (0.01 + fabs(*t2++)));
      f = *freq++;
      o = (int)(*order++);
      if (o < 2) o = 2;
      pole[0] = r1 * cos(2.0 * M_PI * inv_sr * f);
      pole[1] = r1 * sin(2.0 * M_PI * inv_sr * f);

      /* debug */
      //post("%f", r1);

      /* base transform + clipping to prevent blowup */
      /* projection onto axis containing fixed */
      x = _sat(0.5 * (state[0] - state[2]), (t_float)o); 
      /* the "pure" oscillation axis */
      y = _sat(0.5 * state[1], 1.0);
      /* orthogonal complement of x */
      z = _sat(0.5 * (state[0] + state[2]), 1.0);

      /* output */
      *outx++ = x;
      *outy++ = y;
      *outz++ = z;


      /* calculate fixed point location (c, 0, -c) */
      c = _fixedpoint(x, o);

      /* inverse base transform */
      state[0] = x + z;
      state[1] = 2.0 * y;
      state[2] = -x + z;


      /* update transformed linear system around unstable fixed point */
      state[0] -= c;
      state[2] += c;
      vcmul2(state, pole);
      state[2] *= r2;
      state[0] += c;
      state[2] -= c;

  }
  


  ctl->c_x = state[0];
  ctl->c_y = state[1];
  ctl->c_z = state[2];

  return (w+10);
}

static void scrollgrid1D_dsp(t_scrollgrid1D *x, t_signal **sp)
{
  int n = sp[0]->s_n;
  int k;


  dsp_add(scrollgrid1D_perform, 
	  9, 
	  &x->x_ctl, 
	  sp[0]->s_n, 
	  sp[0]->s_vec, 
	  sp[1]->s_vec, 
	  sp[2]->s_vec,
	  sp[3]->s_vec,
	  sp[4]->s_vec,
	  sp[5]->s_vec,
	  sp[6]->s_vec);


}                                  
static void scrollgrid1D_free(t_scrollgrid1D *x)
{


}




static void scrollgrid1D_reset(t_scrollgrid1D *x)
{
    x->x_ctl.c_x = 1;
    x->x_ctl.c_y = 1;
    x->x_ctl.c_z = 1;
}


t_class *scrollgrid1D_class;

static void *scrollgrid1D_new(t_floatarg algotype)
{
    t_scrollgrid1D *x = (t_scrollgrid1D *)pd_new(scrollgrid1D_class);

    /* ins */
    inlet_new(&x->x_obj, &x->x_obj.ob_pd, gensym("signal"), gensym("signal"));  
    inlet_new(&x->x_obj, &x->x_obj.ob_pd, gensym("signal"), gensym("signal"));  
    inlet_new(&x->x_obj, &x->x_obj.ob_pd, gensym("signal"), gensym("signal"));  

    /* outs */
    outlet_new(&x->x_obj, gensym("signal")); 
    outlet_new(&x->x_obj, gensym("signal")); 
    outlet_new(&x->x_obj, gensym("signal")); 


    /* init data */
    scrollgrid1D_reset(x);

    return (void *)x;
}

void scrollgrid1D_tilde_setup(void)
{
    //post("scrollgrid1D~ v0.1");
    scrollgrid1D_class = class_new(gensym("scrollgrid1D~"),
				   (t_newmethod)scrollgrid1D_new,
    	(t_method)scrollgrid1D_free, sizeof(t_scrollgrid1D), 0, A_DEFFLOAT, 0);
    CLASS_MAINSIGNALIN(scrollgrid1D_class, t_scrollgrid1D, x_f);
    class_addmethod(scrollgrid1D_class, (t_method)scrollgrid1D_dsp,
		    gensym("dsp"), 0); 
    class_addmethod(scrollgrid1D_class, (t_method)scrollgrid1D_reset,
		    gensym("reset"), 0); 

}

