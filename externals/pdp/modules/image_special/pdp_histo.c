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



#include "pdp.h"
#include "pdp_base.h"
#include <math.h>

struct _pdp_histo;
typedef void (*t_histo_proc)(struct _pdp_histo *);


typedef struct _pdp_histo
{
    t_object x_obj;
    t_int x_logN;
    t_symbol *x_array_sym;
    t_float x_scale;
    t_int x_debug;
    t_int x_sample_size;            /* pointcloud size */
    t_histo_proc x_process_method;  /* what to do with the histogram */
    t_outlet *x_outlet0;
    int x_matrix_output;

    /* the packet */
    int x_packet0;

    /* packet data */
    short int *x_data;
    int x_width;
    int x_height;
    int x_nb_pixels;

    /* histo data for processor: these are stored on the stack */
    int *x_histo;
 
} t_pdp_histo;


static int round_up_2log(int i)
{
    int l = 0;
    i--;
    while (i) {
	i >>= 1;
	l++;
    }
    //post("log is %d, 2^n is %d", l, 1 << l);

    l = (l < 16) ? l : 15;
    return l;
}


static void dump_to_array(t_pdp_histo *x)
{
    float *vec;
    int nbpoints;
    t_garray *a;
    int i;
    int *histo = x->x_histo;
    int N = 1 << (x->x_logN);
    float scale = 1.0f / (float)(x->x_nb_pixels);
    

    /* dump to array if possible */
    if (!x->x_array_sym){
    }

    /* check if array is valid */
    else if (!(a = (t_garray *)pd_findbyclass(x->x_array_sym, garray_class))){
        post("pdp_histo: %s: no such array", x->x_array_sym->s_name);
    }
    /* get data */
    else if (!garray_getfloatarray(a, &nbpoints, &vec)){
        post("pdp_histo: %s: bad template", x->x_array_sym->s_name);
    }
    /* scale and dump in array */
    else{

	N = (nbpoints < N) ? nbpoints : N;
	for (i=0; i<N; i++) vec[i] = (float)(histo[i]) * scale * x->x_scale;
	//garray_redraw(a);
    }

}

static void get_sampleset(t_pdp_histo *x, int log_tmp_size, int threshold)
{
    int N = 1 << log_tmp_size;
    int mask = N-1;
    int index, nbpoints, i;
    t_atom a[2];
    double scalex = 1.0f / (double)(x->x_width);
    double scaley = 1.0f / (double)(x->x_height);
    t_symbol *s = gensym("list");
    int matrix_packet;
    double *mat_data;

    /* store the offsets of the points in a in an oversized array
       the oversizing is to eliminate a division and to limit the
       searching for a free location after a random index is generated */

    int offset[N]; 

    /* reset the array */
    memset(offset, -1, N * sizeof(int));

    /* get the coordinates of the tempsize brightest points
       and store them in a random location in the hash */
    for (i=0; i<x->x_nb_pixels; i++){
	if (x->x_data[i] >= threshold){
	    /* get a random index */
	    int ri = random();
	    //int ri = 0;
	    /* find an empty spot to store it */
	    while (-1 != offset[ri & mask]) ri++;
	    offset[ri & mask] = i;
	}
    }


    /* repack the array to get the requested
       sample size at the start */
    index = 0;
    nbpoints = 0;
    while (nbpoints < x->x_sample_size){
	while (-1 == offset[index]) index++;   // ffwd to next nonepty slot
	offset[nbpoints++] = offset[index++];  // move slot
    }
	

    /* MATRIX OUTPUT */
    if (x->x_matrix_output){

	matrix_packet = pdp_packet_new_matrix(x->x_sample_size, 2, PDP_MATRIX_TYPE_RDOUBLE);
	mat_data = pdp_packet_data(matrix_packet);
	if (mat_data){
	    
	    /* build the cluster data struct */
	    for (i=0; i<x->x_sample_size; i++){
		mat_data[2*i]   = ((double)(offset[i] % x->x_width)) * scalex;
		mat_data[2*i+1] = ((double)(offset[i] / x->x_width)) * scaley;
	    }

	    pdp_pass_if_valid(x->x_outlet0, &matrix_packet);
	    pdp_packet_mark_unused(x->x_packet0);
	    x->x_packet0 = -1;
	}
	
    }

    /* IMAGE OUTPUT */
    else {

	/* get rw copy */
	pdp_packet_replace_with_writable(&x->x_packet0);
	x->x_data = pdp_packet_data(x->x_packet0);

	/* mark output packet samples */
	if (x->x_data){
	    memset(x->x_data, 0, 2*x->x_nb_pixels);
	    for (i=0; i<x->x_sample_size; i++){
		x->x_data[offset[i]] = 0x7fff;
	    }
	}

	/* send packet to left outlet */
	pdp_pass_if_valid(x->x_outlet0, &x->x_packet0);
    }


}

static void get_brightest(t_pdp_histo *x)
{
    int i;
    int *histo = x->x_histo;
    int N = 1 << (x->x_logN);

    int index, nsamps;

    /* check requested size */
    if (x->x_sample_size > x->x_nb_pixels){
	post("WARNING: more samples requested than pixels in image");
	x->x_sample_size = x->x_nb_pixels;
    }
    

    /* find limiting index */
    index = N;
    nsamps = 0;
    while (nsamps < x->x_sample_size){
	index--;
	nsamps += histo[index];
    }

    /* status report */
    if (x->x_debug){
	post("found %d samples between h[%d] and h[%d]", nsamps, index, N-1);
    }

    /* get a representative set from the candidates 
       the tempbuf is the rounded log of the nb of samples + 1
       so it is at least 50% sparse */
    get_sampleset(x, round_up_2log(nsamps) + 1, index << (15-x->x_logN));

}


static void _pdp_histo_perform(t_pdp_histo *x)
{
    short int *pp;
    int N = 1 << x->x_logN;
    int nbpixels = x->x_width * x->x_height, i;

    int histo[N];

    /* init */
    for (i=0; i<N; i++) histo[i] = 0;

    /* build histo */
    for (i=0; i<nbpixels; i++){
	int index = x->x_data[i] >> (15 - x->x_logN);
	if (index < 0) index = 0; /* negative -> zero */
	histo[index]++;
    }

    /* save the histo stack location  */
    x->x_histo = histo;

    /* print it */
    if (x->x_debug){
	post("histogram:");
	for (i=0; i<N; i++){
	    fprintf(stderr, "%d\t", histo[i]);
	    if (!(i % 10)) post("");
	}
	post("");
    }

    /* call the processor */
    x->x_process_method(x);


}
  

// packet is an image/*/* packet or invalid */
static void pdp_histo_perform(t_pdp_histo *x)
{
    t_pdp *header0 = pdp_packet_header(x->x_packet0);
    void *data0 = pdp_packet_data(x->x_packet0);
    if (!header0 || !data0) return;

    x->x_width = header0->info.image.width;
    x->x_height = header0->info.image.height;
    x->x_nb_pixels = x->x_width * x->x_height;
    x->x_data = data0;

    _pdp_histo_perform(x);
}



static void pdp_histo_input_0(t_pdp_histo *x, t_symbol *s, t_floatarg f)
{
    int packet = (int)f;

    /* register */
    if (s == gensym("register_rw")){
	/* replace if not compatible or we are not interpolating */
	pdp_packet_mark_unused(x->x_packet0);
	x->x_packet0 = pdp_packet_convert_ro(packet, pdp_gensym("image/grey/*"));
	
    }

    if (s == gensym("process")){
	pdp_histo_perform(x);
    }

}



static void pdp_histo_samplesize(t_pdp_histo *x, t_floatarg f)
{
    int i = (int)f;
    if (i > 0) x->x_sample_size = i;
}


static void pdp_histo_scale(t_pdp_histo *x, t_floatarg f){x->x_scale = f;}



static void pdp_histo_size(t_pdp_histo *x, t_floatarg f)
{
    int i = (int)f;
    if (i < 1) return;
    x->x_logN = round_up_2log(i);
}


static void pdp_histo_array(t_pdp_histo *x, t_symbol *s)
{
    //post("setting symbol %x", s);
    x->x_array_sym = s;
}


static void pdp_histo_free(t_pdp_histo *x)
{
    pdp_packet_mark_unused(x->x_packet0);
}


t_class *pdp_histo_class;



void *pdp_histo_new(t_floatarg f)
{

    t_pdp_histo *x = (t_pdp_histo *)pd_new(pdp_histo_class);
    if (f == 0.0f) f = 64;
    pdp_histo_size(x, f);
    x->x_packet0 = -1;
    x->x_debug = 0;
    x->x_sample_size = 16;
    return (void *)x;
}


void *pdp_histo_array_new(t_symbol *s, t_float f, t_float f2)
{
    t_pdp_histo *x = (t_pdp_histo *)pdp_histo_new(f);
    if (f2 == 0.0f) f2 = 1.0f;
    pdp_histo_scale(x, f2);
    pdp_histo_array(x, s);
    x->x_process_method = dump_to_array;
    return (void *)x;
}    

void *pdp_histo_sample_new(t_float nbsamples, t_float histosize)
{
    t_pdp_histo *x;
    if (histosize == 0.0f) histosize = 256.0f;
    x = (t_pdp_histo *)pdp_histo_new(histosize);
    if (nbsamples == 0.0f) nbsamples = 16.0f;
    pdp_histo_samplesize(x, nbsamples);
    x->x_process_method = get_brightest;
    x->x_outlet0 = outlet_new(&x->x_obj, gensym("anything"));
    x->x_matrix_output = 0;

    inlet_new((t_object *)x, (t_pd *)&x->x_obj, gensym("float"), gensym("nbpoints"));

    return (void *)x;
}    

void *pdp_histo_sample_matrix_new(t_float nbsamples, t_float histosize)
{
    t_pdp_histo *x = pdp_histo_sample_new(nbsamples, histosize);
    if (x) x->x_matrix_output = 1;
    return (void *)x;
}


#ifdef __cplusplus
extern "C"
{
#endif


void pdp_histo_setup(void)
{

    pdp_histo_class = class_new(gensym("pdp_histo"), (t_newmethod)pdp_histo_array_new,
    	(t_method)pdp_histo_free, sizeof(t_pdp_histo), 0, A_DEFSYMBOL, A_DEFFLOAT, A_DEFFLOAT, A_NULL);

    class_addcreator((t_newmethod)pdp_histo_sample_new, gensym("pdp_pointcloud"), A_DEFFLOAT, A_DEFFLOAT, A_NULL);
    class_addcreator((t_newmethod)pdp_histo_sample_matrix_new, gensym("pdp_pointcloud_matrix"), A_DEFFLOAT, A_DEFFLOAT, A_NULL);
     
    class_addmethod(pdp_histo_class, (t_method)pdp_histo_input_0, gensym("pdp"),  A_SYMBOL, A_DEFFLOAT, A_NULL);

    class_addmethod(pdp_histo_class, (t_method)pdp_histo_size, gensym("size"), A_FLOAT, A_NULL);
    class_addmethod(pdp_histo_class, (t_method)pdp_histo_size, gensym("scale"), A_FLOAT, A_NULL);
    class_addmethod(pdp_histo_class, (t_method)pdp_histo_array, gensym("array"), A_SYMBOL, A_NULL);
    class_addmethod(pdp_histo_class, (t_method)pdp_histo_samplesize, gensym("nbpoints"), A_FLOAT, A_NULL);
}

#ifdef __cplusplus
}
#endif



