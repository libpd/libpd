/*
 *   junction.c  - computes a lossless circulant junction 
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

#include "m_pd.h"
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>


typedef struct junctionctl
{
  t_int c_channels;
  t_float **c_in;
  t_float **c_out;
  t_float *c_buffer;
  t_float *c_coef;
  t_float c_norm;
} t_junctionctl;

typedef struct junction
{
  t_object x_obj;
  t_float x_f;
  t_junctionctl x_ctl;
} t_junction;

void junction_bang(t_junction *x)
{
  int i, n       = x->x_ctl.c_channels;
  t_float *coef  = x->x_ctl.c_coef;
  t_float r;

  for (i=1; i<n/2; i++)
    {
      r = rand();
      r *= ((2 * M_PI)/RAND_MAX);
      coef[i]= cos(r);
      coef[n-i] = sin(r);
    }

  coef[0] = (rand() & 1) ? 1 : -1;
  coef[n/2] = (rand() & 1) ? 1 : -1;

  /*  mayer_realfft(n, coef); */
  

}

void junction_random(t_junction *x, t_floatarg f)
{
  srand((int)f);
  junction_bang(x);
}

static t_int *junction_perform(t_int *w)
{



  t_junctionctl *ctl  = (t_junctionctl *)(w[1]);
  t_int n             = (t_int)(w[2]);
  t_int i,j;
  t_float x,y;

  t_int c             = ctl->c_channels;
  t_float **in        = ctl->c_in;
  t_float **out       = ctl->c_out;
  t_float *buf        = ctl->c_buffer;
  t_float *coef       = ctl->c_coef;

  t_float norm        = ctl->c_norm;

 
  for (i=0;i<n;i++)
    {

      /* read input */
      for (j=0; j<c; j++)
	{
	  buf[j] = in[j][i];
	}
      
      /* transform */
      mayer_realfft(c, buf);
      for (j=1; j<c/2; j++)
	{
	  t_float x,y,a,b;
	  x = buf[j];
	  y = buf[c-j];
	  a = coef[j];
	  b = coef[c-j];
	  buf[j]   = a * x - b * y;
	  buf[c-j] = a * y + b * x;
	}
      buf[0] *= coef[0];
      buf[c/2] *= coef[c/2];


      mayer_realifft(c, buf);
      

      /* write output */
      for (j=0; j<c; j++) 
	{ 
	  out[j][i] = buf[j] * norm;
	}  
    }

   
  return (w+3);
}



static void junction_dsp(t_junction *x, t_signal **sp)
{
  int i, c = x->x_ctl.c_channels;
  t_float norm;

  for (i=0;i<c;i++)
    {
      x->x_ctl.c_in[i] = sp[i]->s_vec;
      x->x_ctl.c_out[i] = sp[i+c]->s_vec;
    }

  norm = c;
  norm = 1. / (norm);
  x->x_ctl.c_norm =  norm; 


  dsp_add(junction_perform, 2, &x->x_ctl, sp[0]->s_n);

    /*    dsp_add(junction_perform, 4, &x->x_ctl, sp[0]->s_n, sp[0]->s_vec, sp[1]->s_vec);*/

}                                  


void junction_free(t_junction *x)
{

  if (x->x_ctl.c_in) free (x->x_ctl.c_in);
  if (x->x_ctl.c_out) free (x->x_ctl.c_out);
  if (x->x_ctl.c_buffer) free (x->x_ctl.c_buffer);
  if (x->x_ctl.c_coef) free (x->x_ctl.c_coef);

}

t_class *junction_class;

void *junction_new(t_floatarg channels)
{

  int l = ilog2(channels);
  int i,n;

  t_junction *x = (t_junction *)pd_new(junction_class);


  if (l<2) l = 2;
  if (l>4) l = 4;

  n=1;
  while (l--) n *= 2;

  for (i=1;i<n;i++) inlet_new(&x->x_obj, &x->x_obj.ob_pd, gensym("signal"), gensym("signal")); 
  for (i=0;i<n;i++) outlet_new(&x->x_obj, gensym("signal")); 

  x->x_ctl.c_in = (t_float **)malloc(n*sizeof(t_float *));
  x->x_ctl.c_out = (t_float **)malloc(n*sizeof(t_float *));
  x->x_ctl.c_buffer = (t_float *)malloc(n*sizeof(t_float));
  x->x_ctl.c_coef = (t_float *)malloc(n*sizeof(t_float));
  x->x_ctl.c_channels = n;

  junction_bang(x);

  return (void *)x;
}

void junction_tilde_setup(void)
{
  //post("junction~ v0.1");
    junction_class = class_new(gensym("junction~"), (t_newmethod)junction_new,
    	(t_method)junction_free, sizeof(t_junction), 0, A_DEFFLOAT, 0);
    CLASS_MAINSIGNALIN(junction_class, t_junction, x_f); 
    class_addmethod(junction_class, (t_method)junction_bang, gensym("bang"), 0);
    class_addmethod(junction_class, (t_method)junction_random, gensym("random"), A_FLOAT, 0);
    class_addmethod(junction_class, (t_method)junction_dsp, gensym("dsp"), 0); 

}

