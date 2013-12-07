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
#include "pdp_resample.h"



typedef struct pdp_scale_struct
{
    t_object x_obj;
    t_float x_f;

    t_outlet *x_outlet0;


    int x_packet0;
    int x_packet1;
    int x_dropped;
    int x_queue_id;

    unsigned int x_width;
    unsigned int x_height;
    int x_quality;

    
} t_pdp_scale;


static void pdp_scale_process_yv12(t_pdp_scale *x)
{
    t_pdp *header0 = pdp_packet_header(x->x_packet0);
    t_pdp *header1 = pdp_packet_header(x->x_packet1);
    void  *data0   = pdp_packet_data  (x->x_packet0);
    void  *data1   = pdp_packet_data  (x->x_packet1);

    unsigned int src_w = header0->info.image.width;
    unsigned int src_h = header0->info.image.height;

    unsigned int dst_w = header1->info.image.width;
    unsigned int dst_h = header1->info.image.height;

    short int *src_image = (short int *)data0;
    short int *dst_image = (short int *)data1;

    unsigned int src_size = src_w*src_h;
    unsigned int src_voffset = src_size;
    unsigned int src_uoffset = src_size + (src_size>>2);

    unsigned int dst_size = dst_w*dst_h;
    unsigned int dst_voffset = dst_size;
    unsigned int dst_uoffset = dst_size + (dst_size>>2);

    if (x->x_quality){
	pdp_resample_scale_bilin(src_image, dst_image, src_w, src_h, dst_w, dst_h);
	pdp_resample_scale_bilin(src_image+src_voffset, dst_image+dst_voffset, src_w>>1, src_h>>1, dst_w>>1, dst_h>>1);
	pdp_resample_scale_bilin(src_image+src_uoffset, dst_image+dst_uoffset, src_w>>1, src_h>>1, dst_w>>1, dst_h>>1);
    }
    else{
	pdp_resample_scale_nn(src_image, dst_image, src_w, src_h, dst_w, dst_h);
	pdp_resample_scale_nn(src_image+src_voffset, dst_image+dst_voffset, src_w>>1, src_h>>1, dst_w>>1, dst_h>>1);
	pdp_resample_scale_nn(src_image+src_uoffset, dst_image+dst_uoffset, src_w>>1, src_h>>1, dst_w>>1, dst_h>>1);
    }

    return;
}

static void pdp_scale_process_grey(t_pdp_scale *x)
{

    t_pdp *header0 = pdp_packet_header(x->x_packet0);
    t_pdp *header1 = pdp_packet_header(x->x_packet1);
    void  *data0   = pdp_packet_data  (x->x_packet0);
    void  *data1   = pdp_packet_data  (x->x_packet1);

    unsigned int src_w = header0->info.image.width;
    unsigned int src_h = header0->info.image.height;

    unsigned int dst_w = header1->info.image.width;
    unsigned int dst_h = header1->info.image.height;

    short int *src_image = (short int *)data0;
    short int *dst_image = (short int *)data1;

    if (x->x_quality) pdp_resample_scale_bilin(src_image, dst_image, src_w, src_h, dst_w, dst_h);
    else              pdp_resample_scale_nn(src_image, dst_image, src_w, src_h, dst_w, dst_h);

    return;


}

static void pdp_scale_sendpacket(t_pdp_scale *x)
{
    /* delete source packet */
    pdp_packet_mark_unused(x->x_packet0);
    x->x_packet0 = -1;

    /* unregister and propagate if valid dest packet */
    pdp_packet_pass_if_valid(x->x_outlet0, &x->x_packet1);
}

static void pdp_scale_process(t_pdp_scale *x)
{
    t_pdp_procqueue *q = pdp_queue_get_queue();
    t_pdp *header0 = pdp_packet_header(x->x_packet0);

    /* check data packets */

    if ((header0) && (PDP_IMAGE == header0->type)){

	/* if dims are equal, just send the packet */
	if ((header0->info.image.width == x->x_width)
	    && (header0->info.image.height == x->x_height)){
	    x->x_packet1 = x->x_packet0;
	    x->x_packet0 = -1;
	    pdp_scale_sendpacket(x);
	    return;
	}

	/* type hub */
	switch(header0->info.image.encoding){

	case PDP_IMAGE_YV12:
	    x->x_packet1 = pdp_packet_new_image_YCrCb(x->x_width, x->x_height);
	    if(x->x_packet1 == -1){
		post("pdp_scale: can't allocate packet");
		return;
	    }
	    pdp_procqueue_add(q, x, pdp_scale_process_yv12, pdp_scale_sendpacket, &x->x_queue_id);
	    break;

	case PDP_IMAGE_GREY:
	    x->x_packet1 = pdp_packet_new_image_grey(x->x_width, x->x_height);
	    if(x->x_packet1 == -1){
		post("pdp_scale: can't allocate packet");
		return;
	    }
	    pdp_procqueue_add(q, x, pdp_scale_process_grey, pdp_scale_sendpacket, &x->x_queue_id);
	    break;

	default:
	    break;
	    /* don't know the type, so dont process */
	    
	}
    }

}




static void pdp_scale_input_0(t_pdp_scale *x, t_symbol *s, t_floatarg f)
{

    int p = (int)f;
    int passes, i;

    if (s== gensym("register_rw"))  x->x_dropped = pdp_packet_copy_ro_or_drop(&x->x_packet0, p);


    if ((s == gensym("process")) && (-1 != x->x_packet0) && (!x->x_dropped)){

	/* add the process method and callback to the process queue */
	pdp_scale_process(x);

    }

}




static void pdp_scale_width(t_pdp_scale *x, t_floatarg f)
{
    int i = (int)f;
    if (i < 32) i = 32;
    x->x_width = i;
}

static void pdp_scale_height(t_pdp_scale *x, t_floatarg f)
{
    int i = (int)f;
    if (i < 32) i = 32;
    x->x_height = i;
}


static void pdp_scale_dim(t_pdp_scale *x, t_floatarg w, t_floatarg h)
{
    pdp_scale_width(x, w);
    pdp_scale_height(x, h);
}

static void pdp_scale_quality(t_pdp_scale *x, t_floatarg f)
{
    if (f==0) x->x_quality = 0;
    if (f==1) x->x_quality = 1;
}


t_class *pdp_scale_class;



void pdp_scale_free(t_pdp_scale *x)
{
    t_pdp_procqueue *q = pdp_queue_get_queue();
    pdp_procqueue_finish(q, x->x_queue_id);
    pdp_packet_mark_unused(x->x_packet0);
    pdp_packet_mark_unused(x->x_packet1);
}

void *pdp_scale_new(t_floatarg fw, t_floatarg fh)
{
    t_pdp_scale *x = (t_pdp_scale *)pd_new(pdp_scale_class);

    x->x_outlet0 = outlet_new(&x->x_obj, &s_anything); 

    x->x_packet0 = -1;
    x->x_packet1 = -1;
    x->x_queue_id = -1;

    if ((fw != 0.0f) && (fh != 0.0f)) pdp_scale_dim(x, fw, fh);
    else pdp_scale_dim(x, 320, 240);

    pdp_scale_quality(x, 1);

    return (void *)x;
}


#ifdef __cplusplus
extern "C"
{
#endif


void pdp_scale_setup(void)
{


    pdp_scale_class = class_new(gensym("pdp_scale"), (t_newmethod)pdp_scale_new,
    	(t_method)pdp_scale_free, sizeof(t_pdp_scale), 0, A_DEFFLOAT, A_DEFFLOAT, A_NULL);


    class_addmethod(pdp_scale_class, (t_method)pdp_scale_quality, gensym("quality"),  A_FLOAT, A_NULL);   
    class_addmethod(pdp_scale_class, (t_method)pdp_scale_width, gensym("width"),  A_FLOAT, A_NULL);   
    class_addmethod(pdp_scale_class, (t_method)pdp_scale_height, gensym("height"),  A_FLOAT, A_NULL);   
    class_addmethod(pdp_scale_class, (t_method)pdp_scale_dim, gensym("dim"),  A_FLOAT, A_FLOAT, A_NULL);   
    class_addmethod(pdp_scale_class, (t_method)pdp_scale_input_0, gensym("pdp"),  A_SYMBOL, A_DEFFLOAT, A_NULL);

}

#ifdef __cplusplus
}
#endif
