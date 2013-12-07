/*
 *   matrix.c  - applies a matrix transform to a signal block
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

#include "m_pd.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>  

#define MAXORDER 1024

typedef struct matrixctl
{
  t_float *c_A; /* matrix */
  t_float *c_x; /* vector */
  t_int c_order;

} t_matrixctl;

typedef struct matrix
{
  t_object x_obj;
  t_float x_f;
  t_matrixctl x_ctl;
} t_matrix;

static void matrix_load(t_matrix *x, t_symbol *s)
{
  FILE *matrix;
  
  if(s && s->s_name)
    {
      post("matrix: loading %s",s->s_name);
      if(matrix = sys_fopen(s->s_name, "r"))
	{
	  int n = x->x_ctl.c_order;
	  fread(x->x_ctl.c_A, sizeof(t_float), n*n, matrix);
	}
      else post("matrix: error, cant open file.");
    }
}


static t_int *matrix_perform(t_int *w)
{


  t_float *in       = (t_float *)(w[3]);
  t_float *out      = (t_float *)(w[4]);
  t_matrixctl *ctl  = (t_matrixctl *)(w[1]);
  t_int n           = (t_int)(w[2]);

  t_int i,j;
  t_float *A = ctl->c_A;
  t_float *x = ctl->c_x;

  if (in == out) /* store input if ness. */
    {
      memcpy(x, in, sizeof(t_float)*n);
      in = x;
    }
  bzero(out, sizeof(t_float)*n); /* init output */

  for (j=0; j<n; j++)
      for (i=0; i<n; i++) 
	out[i] += A[i+j*n] * in[j];

  return (w+5);
}

static void matrix_dsp(t_matrix *x, t_signal **sp)
{
  int n = sp[0]->s_n;
  int k,i;

  if (x->x_ctl.c_order != n)
    {
      if (x->x_ctl.c_A) free (x->x_ctl.c_A);

      x->x_ctl.c_A = (t_float *)calloc(n*n,sizeof(t_float));
      x->x_ctl.c_x = (t_float *)calloc(n,sizeof(t_float));
      x->x_ctl.c_order = n;
    }
  
  for (i=0;i<n;i++) x->x_ctl.c_A[i] = 1;

  dsp_add(matrix_perform, 4, &x->x_ctl, sp[0]->s_n, sp[0]->s_vec, sp[1]->s_vec);


}                                  
static void matrix_free(t_matrix *x)
{

  if (x->x_ctl.c_A) free (x->x_ctl.c_A);
  if (x->x_ctl.c_x) free (x->x_ctl.c_x);

}

t_class *matrix_class;

static void *matrix_new(t_floatarg order)
{
    t_matrix *x = (t_matrix *)pd_new(matrix_class);
    int iorder = (int)order;
    int i, n=64, k;


    /* out 1 */
    outlet_new(&x->x_obj, gensym("signal")); 



    /* init data */

    x->x_ctl.c_A = (t_float *)calloc(n*n,sizeof(t_float));
    x->x_ctl.c_x = (t_float *)calloc(n,sizeof(t_float));


    for (i=0;i<n;i++) x->x_ctl.c_A[i] = 1;
    x->x_ctl.c_order = n;

    return (void *)x;
}

void bmatrix_tilde_setup(void)
{
  //post("matrix~ v0.1");
    matrix_class = class_new(gensym("bmatrix~"), (t_newmethod)matrix_new,
    	(t_method)matrix_free, sizeof(t_matrix), 0, A_DEFFLOAT, 0);
    CLASS_MAINSIGNALIN(matrix_class, t_matrix, x_f);
    class_addmethod(matrix_class, (t_method)matrix_dsp, gensym("dsp"), 0); 
    class_addmethod(matrix_class, (t_method)matrix_load, gensym("load"), A_SYMBOL,0); 


}

