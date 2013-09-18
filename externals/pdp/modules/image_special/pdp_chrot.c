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



#include "pdp.h"
#include "pdp_base.h"
#include <math.h>


typedef struct pdp_chrot_struct
{
    t_pdp_base x_base;

    float x_matrix[4];
    void *x_crot2d;

} t_pdp_chrot;



static void pdp_chrot_process(t_pdp_chrot *x)
{
    int packet;
    t_pdp *header;
    void  *data;
    unsigned int w,h,size,v_offset;
    short int *idata;


    /* get packet & info */
    packet = pdp_base_get_packet(x, 0);
    header = pdp_packet_header(packet);
    data   = pdp_packet_data  (packet);

    /* only process if we have a vlid yv12 image */
    if ((header) && (PDP_IMAGE == header->type) && (PDP_IMAGE_YV12 == header->info.image.encoding)){

	w = header->info.image.width;
	h = header->info.image.height;

	size = w*h;
	v_offset = size;
	
	idata = (short int *)data;

    
	/* color rotation for 2 colour planes */
	pdp_imageproc_crot2d_process(x->x_crot2d, idata + v_offset, w>>1, h>>1);

    }
    return;
}


static void pdp_chrot_setelement(t_pdp_chrot *x, int element, float f)
{
    x->x_matrix[element] = f;
	
}

static void pdp_chrot_angle_radians(t_pdp_chrot *x, t_floatarg angle)
{
    float c = cos(angle);
    float s = sin(angle);

    pdp_chrot_setelement(x, 0, c);
    pdp_chrot_setelement(x, 1, s);
    pdp_chrot_setelement(x, 2, -s);
    pdp_chrot_setelement(x, 3, c);

    pdp_imageproc_crot2d_setmatrix(x->x_crot2d, x->x_matrix);
}

static void pdp_chrot_angle_degrees(t_pdp_chrot *x, t_floatarg angle)
{
    pdp_chrot_angle_radians(x, (angle * (M_PI / 180.f)));

}

static void pdp_chrot_free(t_pdp_chrot *x)
{
    pdp_base_free(x);
    pdp_imageproc_crot2d_delete(x->x_crot2d);
}

t_class *pdp_chrot_class;



void *pdp_chrot_new(t_floatarg f)
{
    t_pdp_chrot *x = (t_pdp_chrot *)pd_new(pdp_chrot_class);

    pdp_base_init(x);
    pdp_base_add_gen_inlet(x, gensym("float"), gensym("angle"));
    pdp_base_add_pdp_outlet(x);
    pdp_base_set_process_method(x, (t_pdp_method)pdp_chrot_process);

    x->x_crot2d = pdp_imageproc_crot2d_new();
    pdp_chrot_angle_radians(x, 0.0f);

    return (void *)x;
}


#ifdef __cplusplus
extern "C"
{
#endif


void pdp_chrot_setup(void)
{


    pdp_chrot_class = class_new(gensym("pdp_chrot"), (t_newmethod)pdp_chrot_new,
    	(t_method)pdp_chrot_free, sizeof(t_pdp_chrot), 0, A_DEFFLOAT, A_NULL);

    pdp_base_setup(pdp_chrot_class);
    class_addmethod(pdp_chrot_class, (t_method)pdp_chrot_angle_degrees, gensym("angle"),  A_DEFFLOAT, A_NULL);   

}

#ifdef __cplusplus
}
#endif
