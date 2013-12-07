/*
 *   Pure Data Packet module.
 *   Copyright (c) 2003 by Tom Schouten <tom@zwizwa.be>
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


#include <math.h>

#include "pdp.h"
#include "pdp_base.h"


typedef struct _pdp_array
{
    t_object x_obj;
    t_symbol *x_array_sym;
    t_outlet *x_outlet0; // for array->pdp
    t_int x_rows; // transposed

    /* packet creation */
    t_int x_width;
    t_int x_height;

    /* only valid after "get array" */
    float *x_vec;
    int x_nbpoints;
    


    /* the packet */
    int x_packet0;

} t_pdp_array;

static void get_array(t_pdp_array *x){
    t_garray *a;

    x->x_vec = 0;

    /* dump to array if possible */
    if (!x->x_array_sym) return;

    /* check if array is valid */
    if (!(a = (t_garray *)pd_findbyclass(x->x_array_sym, garray_class))){
	post("pdp_array: %s: no such array", x->x_array_sym->s_name);
	return;
    }

    /* get data */
    if (!garray_getfloatarray(a, &x->x_nbpoints, &x->x_vec)){
	post("pdp_array: %s: bad template", x->x_array_sym->s_name);
	return;
    }    
}


static void pdp_array_bang(t_pdp_array *x)
{
    PDP_ASSERT(-1 == x->x_packet0);
    x->x_packet0 = pdp_packet_new_image_grey(x->x_width, x->x_height);
    if (-1 != x->x_packet0){
	t_pdp *header = pdp_packet_header(x->x_packet0);
	short int *data = (short int*)pdp_packet_data(x->x_packet0);
	get_array(x);
	if (x->x_vec){
	    int i;
	    int w = header->info.image.width;
	    int h = header->info.image.height;
	    int N = w*h;
	    N = (x->x_nbpoints < N) ? x->x_nbpoints : N;

	    /* scan rows */
	    if (1 || x->x_rows){
		// FIXME: saturation
		for (i=0; i<N; i++) {
		    float max = (float)0x8000;
		    float f = x->x_vec[i] * max;
		    int l = (int)f;
		    l = (l > 0x8000)  ?  0x7fff : l;
		    l = (l < -0x8000) ? -0x8000 : l;
		    data[i] = (short int)l;
		}
	    }
	}
	pdp_packet_pass_if_valid(x->x_outlet0, &x->x_packet0);
    }
}


static void pdp_array_input_0(t_pdp_array *x, t_symbol *s, t_floatarg f)
{
    int packet = (int)f;


    /* register */
    if (s == gensym("register_ro")){
	/* replace if not compatible or we are not interpolating */
	pdp_packet_mark_unused(x->x_packet0);
	x->x_packet0 = pdp_packet_convert_ro(packet, pdp_gensym("image/grey/*"));
    }

    /* process */
    if (s == gensym("process")){
	t_garray *a;
	t_pdp *header = pdp_packet_header(x->x_packet0);
	short int *data = pdp_packet_data(x->x_packet0);
	if (!header || !data) return;

	
	get_array(x);
	if (x->x_vec){
	    int i;
	    int w = header->info.image.width;
	    int h = header->info.image.height;
	    int N = w*h;
	    N = (x->x_nbpoints < N) ? x->x_nbpoints : N;

	    /* scan rows */
	    if (x->x_rows){
		for (i=0; i<N; i++) 
		    x->x_vec[i] = (float)data[i] * (1.0f / (float)0x8000);
	    }
	    /* scan columns */
	    else{
		for (i=0; i<N; i++) {
		    int xx = i / h;
		    int yy = i % h;
		    x->x_vec[i] = (float)data[xx+(h-yy-1)*w] * (1.0f / (float)0x8000);
		}
	    }
	}
    }
}

static void pdp_array_array(t_pdp_array *x, t_symbol *s)
{
    //post("setting symbol %x", s);
    x->x_array_sym = s;
    x->x_packet0 = -1;
}


static void pdp_array_free(t_pdp_array *x)
{
    pdp_packet_mark_unused(x->x_packet0);
}


t_class *pdp_array2grey_class;
t_class *pdp_grey2array_class;



void *pdp_array2grey_new(t_symbol *s, t_symbol *r)
{
    t_pdp_array *x = (t_pdp_array *)pd_new(pdp_array2grey_class);
    pdp_array_array(x, s);
    x->x_outlet0 = outlet_new(&x->x_obj, &s_anything);
    x->x_width = 320;
    x->x_height = 240;
    return (void *)x;
}

void *pdp_grey2array_new(t_symbol *s, t_symbol *r)
{
    t_pdp_array *x = (t_pdp_array *)pd_new(pdp_grey2array_class);
    pdp_array_array(x, s);
    if (r == gensym("rows")){
	x->x_rows = 1;
	post("pdp_grey2array: scanning rows");
    }
    else {
	x->x_rows = 0;
	post("pdp_grey2array: scanning columns");
    }
    return (void *)x;
}


#ifdef __cplusplus
extern "C"
{
#endif


void pdp_array_setup(void)
{

    pdp_array2grey_class = class_new(gensym("pdp_array2grey"), 
				     (t_newmethod)pdp_array2grey_new,
				     (t_method)pdp_array_free, 
				     sizeof(t_pdp_array), 
				     0, A_DEFSYMBOL, A_DEFSYMBOL, A_NULL);

    pdp_grey2array_class = class_new(gensym("pdp_grey2array"), 
				     (t_newmethod)pdp_grey2array_new,
				     (t_method)pdp_array_free, 
				     sizeof(t_pdp_array), 
				     0, A_DEFSYMBOL, A_DEFSYMBOL, A_NULL);


    /* packet input */
    class_addmethod(pdp_grey2array_class, 
		    (t_method)pdp_array_input_0, 
		    gensym("pdp"),  A_SYMBOL, A_DEFFLOAT, A_NULL);

    /* bang method */
    class_addmethod(pdp_array2grey_class, 
		    (t_method)pdp_array_bang, gensym("bang"), A_NULL);


    /* bookkeeping */
    class_addmethod(pdp_array2grey_class, (t_method)pdp_array_array, 
		    gensym("array"), A_SYMBOL, A_NULL);
    class_addmethod(pdp_grey2array_class, (t_method)pdp_array_array, 
		    gensym("array"), A_SYMBOL, A_NULL);

}

#ifdef __cplusplus
}
#endif



