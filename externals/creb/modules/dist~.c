/*
 *   dist.c  -  wave shaping extern 
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


#define     CLIP          0
#define     INVERSE       1
#define     INVERSESQ     2
#define     INVERSECUB    3
#define     RAT1          4
#define     RAT2          5
#define     FULLRECT      6
#define     HALFRECT      7
#define     PULSE         8
#define     NEWTON1       9
#define     UPPERCLIP    10
#define     COMPARATOR   11



typedef struct distctl
{
  t_float c_gain;
  t_float c_delay;
  char c_type;
} t_distctl;

typedef struct dist
{
  t_object x_obj;
  t_float x_f;
  t_distctl x_ctl;
} t_dist;

void dist_bang(t_dist *x)
{

}

void dist_gain(t_dist *x,  t_floatarg f)
{
  x->x_ctl.c_gain = f;

}


static t_int *dist_perform(t_int *w)
{


  t_float *in    = (t_float *)(w[3]);
  t_float *out    = (t_float *)(w[4]);
  t_distctl *ctl  = (t_distctl *)(w[1]);
  t_float gain  = ctl->c_gain;
  t_int i;
  t_int n = (t_int)(w[2]);
  t_float x,y,v;
  t_float z = ctl->c_delay;

  switch(ctl->c_type){
  case CLIP:	
    for (i = 0; i < n; i++)
      {
	x = *in++ * gain;
	x = (x >  1) ? ( 1.) : x;
	x = (x < -1) ? (-1.) : x;
	*out++ = 0.9999 * x;

      }
    break;

  case INVERSE:	
    for (i = 0; i < n; i++)
      {
	x = *in++ * gain;
	x = (x > 1) ? (2. - 1/x) : x;
	x = (x < -1) ? (-2. - 1/x) : x;
	*out++ = x/2.0001;

      }
    break;

  case INVERSESQ:	
    for (i = 0; i < n; i++)
      {
	x = *in++ * gain;
	x = (x > 1) ? (2. - 1/x) : x;
	x = (x < -1) ? (-2. - 1/x) : x;
	x /= 2;
	*out++ = 1.999*x*x-1;

      }
    break;

  case INVERSECUB:	
    for (i = 0; i < n; i++)
      {
	x = *in++ * gain;
	x = (x > 1) ? (2. - 1/x) : x;
	x = (x < -1) ? (-2. - 1/x) : x;
	x /= 2;
	*out++ = .9999 * x*x*x;

      }
    break;

  case RAT1: /*(2*d./((1+(d).^2)))*/   
    for (i = 0; i < n; i++)
      {
	x = *in++ * gain;
	y = (1. + x*x);
	x = 1.9999*x/y;
	*out++ = x;
      }
    break;

  case RAT2: /*(2*d./((1+(d).^16)))*/   
    for (i = 0; i < n; i++)
      {
	x = *in++ * gain;
	y = x*x;
	y *= y;
	y *= y;
	y *= y;
	y = (1. + y);
	x = 1.2*x/y;
	*out++ = x;
      }
    break;

  case FULLRECT:
    for (i = 0; i < n; i++)
      {
	x = *in++ * gain;
	x = (x>0) ? x : -x;
	x = (x>1) ? 1 : x;
	*out++ = 1.9999*(x-.5);
      }
    break;

  case HALFRECT:
    for (i = 0; i < n; i++)
      {
	x = *in++ * gain;
	x = (x>0) ? x : 0;
	x = (x>1) ? 1 : x;
	*out++ = 1.9999*(x-.5);
      }
    break;

  case PULSE:
    for (i = 0; i < n; i++)
      {
	x = *in++ * gain;
	y = (x>0) ? (1):(-1);
	x = (z*y > 0) ? (0) : (y);
	*out++ = .9999 * x;
	z = x;
	
      }
    ctl->c_delay = z;
    break;

  case NEWTON1:
    for (i = 0; i < n; i++)
      {
	x = *in++ * gain;
	y = 1./(1.+x*x);

	z = .5;
	z = .5*(y/z + z);
	z = .5*(y/z + z);
	z = .5*(y/z + z);

	/*	z = .5*(y/z + z);
	 *	z = .5*(y/z + z);
	 *	z = .5*(y/z + z);
	 */

	*out++ = x * z; 
	
      }
    ctl->c_delay = z;
    break;

  case UPPERCLIP:
    for (i = 0; i < n; i++)
      {
	x = *in++ * gain;

	x = (x < 0.0f) ? 0.0f : x;
	x = (x > 0.9999f) ? 0.9999f : x;

	*out++ = x; 
	
      }
    break;

  case COMPARATOR:
    for (i = 0; i < n; i++)
      {
	x = *in++ * gain;

	x = (x > 0.0f) ? 1.0f : -1.0f;

	*out++ = x; 
	
      }
    break;

  default:
    
    for (i = 0; i < n; i++) *out++ = *in++;
    break;

  }
    
  return (w+5);
}

static void dist_dsp(t_dist *x, t_signal **sp)
{
    dsp_add(dist_perform, 4, &x->x_ctl, sp[0]->s_n, 
	    sp[0]->s_vec, sp[1]->s_vec);

}                                  
void dist_free(void)
{

}

t_class *dist_class;

void *dist_new(t_floatarg type)
{
    t_dist *x = (t_dist *)pd_new(dist_class);
    inlet_new(&x->x_obj, &x->x_obj.ob_pd, gensym("float"), gensym("gain"));  
    outlet_new(&x->x_obj, gensym("signal")); 

    dist_gain(x, 1);
    x->x_ctl.c_type = (char)type;
    x->x_ctl.c_delay = 0;

    return (void *)x;
}

void dist_tilde_setup(void)
{
  //post("dist~ v0.1");
    dist_class = class_new(gensym("dist~"), (t_newmethod)dist_new,
    	(t_method)dist_free, sizeof(t_dist), 0, A_DEFFLOAT, 0);
    CLASS_MAINSIGNALIN(dist_class, t_dist, x_f); 
    class_addmethod(dist_class, (t_method)dist_bang, gensym("bang"), 0);
    class_addmethod(dist_class, (t_method)dist_dsp, gensym("dsp"), 0); 
    class_addmethod(dist_class, (t_method)dist_gain, gensym("gain"), A_FLOAT, 0); 

}

