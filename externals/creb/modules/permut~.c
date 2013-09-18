/*
 *   permut.c  - applies a (random) permutation on a signal block
 *   intended for spectral processing, dynwav
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


#include <math.h>
#include <stdlib.h>
//#include "m_pd.h"
#include "extlib_util.h"
                               
typedef union
{
    float f;
    unsigned int i;
}t_permutflint;


typedef struct permutctl
{
  char c_type;
  t_int *c_permutationtable;
  int c_blocksize;
} t_permutctl;


typedef struct permut
{
  t_object x_obj;
  t_float x_f;
  t_permutctl x_ctl;
} t_permut;


static inline void permut_perform_permutation(t_float *S, int n, t_int *f)
{
  t_int k,l;
  t_float swap;
  for(k=0; k<n; k++)
    {
      l = f[k];
      while (l<k) l = f[l];
      swap = S[k];
      S[k] = S[l];
      S[l] = swap;
    }
}


static void permut_random(t_permut *x, t_floatarg seed)
{
  int i,j;
  int N = x->x_ctl.c_blocksize;
  int mask = N-1;
  t_int *p = x->x_ctl.c_permutationtable;
  int r, last = 0;
  t_permutflint flintseed;
  
  flintseed.f = (float)seed;
  srand(flintseed.i);

  if(p)
    {
      p[0] = rand() & mask;
      for (i=1;i<N;i++)
	{
	  r = rand() & mask;
	  j = 0;
	  while(j<i)
	    {
	      if (p[j] == r)
		{
		  r = (r + 1) & mask;
		  j = 0;
		}
	      else j++;
	    }
	  p[i] = r;
	  
	}
    }
}

static void permut_bang(t_permut *x)
{
    t_permutflint seed;
    seed.i = rand();
    t_float floatseed = (t_float)seed.f;
    permut_random(x, floatseed);
}

static void permut_resize_table(t_permut *x, int size)
{
  if (x->x_ctl.c_blocksize != size)
    {
      if (x->x_ctl.c_permutationtable)
	free(x->x_ctl.c_permutationtable);
      x->x_ctl.c_permutationtable = (t_int *)malloc(sizeof(int)*size);
      x->x_ctl.c_blocksize = size;

      /* make sure it's initialized */
      permut_bang(x);


    } 
}




static t_int *permut_perform(t_int *w)
{


  t_float *in    = (t_float *)(w[3]);
  t_float *out    = (t_float *)(w[4]);
  t_permutctl *ctl  = (t_permutctl *)(w[1]);
  t_int i;
  t_int n = (t_int)(w[2]);
  t_float x,y;
  t_int *p =  ctl->c_permutationtable;


  if (in != out)
    for (i=0; i<n; i++) out[i] = in[i];


  permut_perform_permutation(out, n, p);

  return (w+5);
}

static void permut_dsp(t_permut *x, t_signal **sp)
{
  permut_resize_table(x, sp[0]->s_n);
  dsp_add(permut_perform, 4, &x->x_ctl, sp[0]->s_n, sp[0]->s_vec, sp[1]->s_vec);

}
                                  
static void permut_free(t_permut *x)
{
  if (x->x_ctl.c_permutationtable) free(x->x_ctl.c_permutationtable);

}

t_class *permut_class;

static void *permut_new(void)
{
    t_permut *x = (t_permut *)pd_new(permut_class);
    outlet_new(&x->x_obj, gensym("signal")); 

    x->x_ctl.c_permutationtable = 0;
    x->x_ctl.c_blocksize = 0;
    permut_resize_table(x, 64);
    permut_random(x, 0);
    
    return (void *)x;
}

void permut_tilde_setup(void)
{
  //post("permut~ v0.1");
    permut_class = class_new(gensym("permut~"), (t_newmethod)permut_new,
    	(t_method)permut_free, sizeof(t_permut), 0, A_DEFFLOAT, 0);
    CLASS_MAINSIGNALIN(permut_class, t_permut, x_f); 
    class_addmethod(permut_class, (t_method)permut_random, gensym("random"), A_FLOAT, 0);
    class_addmethod(permut_class, (t_method)permut_bang, gensym("bang"), 0);
    class_addmethod(permut_class, (t_method)permut_dsp, gensym("dsp"), 0); 

}

