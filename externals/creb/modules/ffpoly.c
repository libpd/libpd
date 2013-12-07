/*
 *   ffpoly.c - compute a finite field polynomial
 *   Copyright (c) by Tom Schouten <tom@ziwzwa.be>
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
 *
 */


#include "m_pd.h"
#include <stdlib.h>




typedef struct ffpoly_struct
{
    t_object x_obj;
    t_float x_f;

    t_outlet *x_outlet;
    t_int *x_coef;
    t_int x_poly_order;
    t_int x_field_order;

    t_int x_lastpackedcoef;



} t_ffpoly;




static void ffpoly_compute(t_ffpoly *x, t_floatarg fcoef)
{
    int in = (int)fcoef;
    int fo = x->x_field_order;
    int po = x->x_poly_order;
    t_int* c = x->x_coef;
    int i, out;

    in %= fo;
    if (in < 0) in += fo;

    out = c[po];
    for (i=po-1; i>=0; i--){
	out *= in;
	out += c[i];
	out %= fo;
    }
    

    outlet_float(x->x_outlet, (t_float)out);
}


/* this sets all coefficients given one float */
static void ffpoly_coefficients(t_ffpoly *x, t_floatarg fcoef)
{
    int coef = (int)fcoef;
    int i;
    if (coef < 0) coef = -coef;

    x->x_lastpackedcoef = coef;

    for (i=0; i<x->x_poly_order + 1; i++){
	x->x_coef[i] = coef % x->x_field_order;
	coef = coef / x->x_field_order;
    }


}

/* this sets one coefficient */
static void ffpoly_coef(t_ffpoly *x, t_floatarg index, t_floatarg val)
{
  int i = (int)index;
  int v = (int)val;
  if (i<0) return;
  if (i>x->x_poly_order) return;

  v %= x->x_field_order;
  if (v<0) v += x->x_field_order;

  x->x_coef[i] = v;


}

static void ffpoly_fieldorder(t_ffpoly *x, t_floatarg ffieldorder)
{
    int i;
    int order = (int)ffieldorder;
    if (order < 2) order = 2;
    x->x_field_order = order;

    for (i=0; i<x->x_poly_order+1; i++)
      x->x_coef[i] %= x->x_field_order;

    //ffpoly_coefficients(x, x->x_lastpackedcoef);
}

static void ffpoly_free(t_ffpoly *x)
{
    free (x->x_coef);
}

t_class *ffpoly_class;



static void *ffpoly_new(t_floatarg fpolyorder, t_floatarg ffieldorder)
{
    t_int polyorder = (int)fpolyorder;
    t_int fieldorder = (int)ffieldorder;

    t_ffpoly *x = (t_ffpoly *)pd_new(ffpoly_class);

    if (polyorder < 1) polyorder = 1;
    if (fieldorder < 2) fieldorder = 2;

    x->x_poly_order = polyorder;
    x->x_field_order = fieldorder;

    x->x_coef = (t_int *)malloc((x->x_poly_order  + 1) * sizeof(int));

    /* set poly to f(x) = x */
    ffpoly_coefficients(x, x->x_field_order);

    x->x_outlet = outlet_new(&x->x_obj, &s_float); 

    return (void *)x;
}


#ifdef __cplusplus
extern "C"
{
#endif


void ffpoly_setup(void)
{


    ffpoly_class = class_new(gensym("ffpoly"), (t_newmethod)ffpoly_new,
    	(t_method)ffpoly_free, sizeof(t_ffpoly), 0, A_DEFFLOAT, A_DEFFLOAT, A_NULL);
    
    class_addmethod(ffpoly_class, (t_method)ffpoly_coefficients, gensym("coefficients"),  A_FLOAT, A_NULL);

    class_addmethod(ffpoly_class, (t_method)ffpoly_coef, gensym("coef"),  A_FLOAT, A_FLOAT, A_NULL);
    class_addmethod(ffpoly_class, (t_method)ffpoly_fieldorder, gensym("order"),  A_FLOAT, A_NULL);
    class_addfloat(ffpoly_class, (t_method)ffpoly_compute);


}

#ifdef __cplusplus
}
#endif
