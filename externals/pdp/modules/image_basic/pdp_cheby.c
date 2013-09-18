/*
 *   Pure Data Packet module.
 *   Copyright (c) by Tom Schouten <tom@zwizwa.be>
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


#include <stdio.h>
#include <gsl/gsl_math.h>
#include <gsl/gsl_chebyshev.h>
#include "pdp.h"
#include "pdp_imagebase.h"


typedef struct pdp_cheby_struct
{
    t_pdp_imagebase x_base;
    void *x_cheby;
    int x_iterations;
    int x_order;
    gsl_cheb_series *x_cs;
    gsl_function x_F;
    float *x_vec;
    int x_nbpoints;

} t_pdp_cheby;



static double pdp_cheby_mappingfunction(double f, void *params)
{
    t_pdp_cheby *x = (t_pdp_cheby *)params;
    int indx;

    /* if there's no array, return the identity function */
    if (!x->x_vec) return f;

    /* else interpolate the array */
    indx = ((f + 1) * 0.5) * (x->x_nbpoints - 1);
    return x->x_vec[indx];
    
}

static void pdp_cheby_coef(t_pdp_cheby *x, t_floatarg c, t_floatarg f)
{
    pdp_imageproc_cheby_setcoef(x->x_cheby, (int)c, f);
}

static void pdp_cheby_approx(t_pdp_cheby *x, t_symbol *s)
{
    int i;
    t_garray *a;

    /* check if array is valid */
    if (!(a = (t_garray *)pd_findbyclass(s, garray_class))){
        post("pdp_cheby: %s: no such array", s->s_name);
    }
    /* get data */
    else if (!garray_getfloatarray(a, &x->x_nbpoints, &x->x_vec)){
        post("pdp_cheby: %s: bad template", s->s_name);

    }

    else{

	/* calculate approximation */
	gsl_cheb_init (x->x_cs, &x->x_F, -1.0, 1.0);

	/* propagate coefficients */
	for (i=0; i<=x->x_order; i++){
	    pdp_cheby_coef(x, i, x->x_cs->c[i]);
	}
    }
    
    x->x_vec = 0;
    return;


}


static void pdp_cheby_process(t_pdp_cheby *x)
{
    int p0 = pdp_base_get_packet(x, 0);
    u32 mask = pdp_imagebase_get_chanmask(x);
    pdp_imageproc_cheby_setnbpasses(x->x_cheby, x->x_iterations);
    pdp_imageproc_dispatch_1buf(&pdp_imageproc_cheby_process, x->x_cheby, mask, p0);
}



static void pdp_cheby_reset(t_pdp_cheby *x)
{
    int i;
    for (i = 0; i <= x->x_order; i++)
	pdp_imageproc_cheby_setcoef(x->x_cheby, i, 0);
}


static void pdp_cheby_iterations(t_pdp_cheby *x, t_floatarg f)
{
  int i = (int)f;
  if (i<0) i = 0;
  x->x_iterations = i;

}
static void pdp_cheby_free(t_pdp_cheby *x)
{
    pdp_imagebase_free(x);
    pdp_imageproc_cheby_delete(x->x_cheby);
    gsl_cheb_free(x->x_cs); 

}

t_class *pdp_cheby_class;



void *pdp_cheby_new(t_floatarg f)
{
    t_pdp_cheby *x = (t_pdp_cheby *)pd_new(pdp_cheby_class);
    int order = (int)(f);

    /* super init */
    pdp_imagebase_init(x);

    /* create i/o */
    pdp_base_add_gen_inlet(x, gensym("float"), gensym("iterations"));
    pdp_base_add_pdp_outlet(x);

    /* setup callback */
    pdp_base_set_process_method(x, (t_pdp_method)pdp_cheby_process);

    /* data init */
    x->x_cheby = pdp_imageproc_cheby_new(order);
    x->x_iterations = 1;

    if (order < 2) order = 2;
    x->x_order = order;

    /* init gls chebychev series object */
    x->x_cs = gsl_cheb_alloc(order);
    x->x_F.function = pdp_cheby_mappingfunction;
    x->x_F.params = x;
    x->x_vec = 0;

    return (void *)x;
}


#ifdef __cplusplus
extern "C"
{
#endif


void pdp_cheby_setup(void)
{


    pdp_cheby_class = class_new(gensym("pdp_cheby"), (t_newmethod)pdp_cheby_new,
    	(t_method)pdp_cheby_free, sizeof(t_pdp_cheby), 0, A_DEFFLOAT, A_NULL);

    pdp_imagebase_setup(pdp_cheby_class);

    class_addmethod(pdp_cheby_class, (t_method)pdp_cheby_coef, gensym("coef"),  A_FLOAT, A_FLOAT, A_NULL);   
    class_addmethod(pdp_cheby_class, (t_method)pdp_cheby_iterations, gensym("iterations"),  A_FLOAT, A_NULL);   
    class_addmethod(pdp_cheby_class, (t_method)pdp_cheby_reset, gensym("reset"),  A_NULL);
    class_addmethod(pdp_cheby_class, (t_method)pdp_cheby_approx, gensym("approx"),  A_SYMBOL, A_NULL);

}

#ifdef __cplusplus
}
#endif
