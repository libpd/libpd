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


/* this module converts a greyscale image or the luma channel of a colour image
   to a colour image intensity mask, usable for multiplication */

#include "pdp.h"
#include "pdp_resample.h"

typedef struct pdp_grey2mask_struct
{
    t_object x_obj;
    t_float x_f;

    t_outlet *x_outlet0;

    int x_packet0;
    int x_dropped;
    int x_queue_id;


} t_pdp_grey2mask;



static void pdp_grey2mask_process_grey(t_pdp_grey2mask *x)
{
    t_pdp     *header = pdp_packet_header(x->x_packet0);
    short int *data   = (short int *)pdp_packet_data  (x->x_packet0);
    t_pdp     *newheader = 0;
    short int *newdata = 0;
    int       newpacket = -1;

    unsigned int w = header->info.image.width;
    unsigned int h = header->info.image.height;

    unsigned int size = w*h;
    unsigned int totalnbpixels = size;
    unsigned int u_offset = size;
    unsigned int v_offset = size + (size>>2);

    unsigned int row, col;

    newpacket = pdp_packet_new_image_YCrCb(w, h);
    newheader = pdp_packet_header(newpacket);
    newdata = (short int *)pdp_packet_data(newpacket);

    /* copy luma channel */
    memcpy(newdata, data, size * sizeof(s16));
    
    /* subsample luma -> chroma channel */
    pdp_resample_halve(data, newdata+u_offset, w, h);

    /* copy this to the other chroma channel */
    memcpy(newdata+v_offset,  newdata+u_offset, (size>>2)*sizeof(s16));

    /* delete source packet and replace with new packet */
    pdp_packet_mark_unused(x->x_packet0);
    x->x_packet0 = newpacket;
    return;
}

static void pdp_grey2mask_process_yv12(t_pdp_grey2mask *x)
{
    /* process only the luminance channel */
    pdp_grey2mask_process_grey(x);
}



static void pdp_grey2mask_process(t_pdp_grey2mask *x)
{
   int encoding;
   t_pdp *header = 0;

   /* check if image data packets are compatible */
   if ( (header = pdp_packet_header(x->x_packet0))
	&& (PDP_IMAGE == header->type)){
    
	/* pdp_grey2mask_process inputs and write into active inlet */
	switch(pdp_packet_header(x->x_packet0)->info.image.encoding){

	case PDP_IMAGE_YV12:
	    pdp_grey2mask_process_yv12(x);
	    break;

	case PDP_IMAGE_GREY:
	    pdp_grey2mask_process_grey(x);
	    break;

	default:
	    /* don't know the type, so dont pdp_grey2mask_process */
	    
	    break;
	}
    }
}

static void pdp_grey2mask_sendpacket(t_pdp_grey2mask *x)
{
    /* unregister and propagate if valid packet */
    pdp_packet_pass_if_valid(x->x_outlet0, &x->x_packet0);
}

static void pdp_grey2mask_input_0(t_pdp_grey2mask *x, t_symbol *s, t_floatarg f)
{

    int p = (int)f;

    if (s== gensym("register_ro"))  x->x_dropped = pdp_packet_copy_ro_or_drop(&x->x_packet0, p);


    if ((s == gensym("process")) && (-1 != x->x_packet0) && (!x->x_dropped)){


	/* add the process method and callback to the process queue */

	//pdp_queue_add(x, pdp_grey2mask_process, pdp_grey2mask_sendpacket, &x->x_queue_id);
	// since the process method creates a packet, this is not processed in the thread
	// $$$TODO: fix this
	pdp_grey2mask_process(x);
	pdp_grey2mask_sendpacket(x);
    }

}



static void pdp_grey2mask_free(t_pdp_grey2mask *x)
{
    t_pdp_procqueue *q = pdp_queue_get_queue();
    pdp_procqueue_finish(q, x->x_queue_id);
    pdp_packet_mark_unused(x->x_packet0);

}

t_class *pdp_grey2mask_class;



void *pdp_grey2mask_new(void)
{
    int i;

    t_pdp_grey2mask *x = (t_pdp_grey2mask *)pd_new(pdp_grey2mask_class);

    x->x_outlet0 = outlet_new(&x->x_obj, &s_anything); 
    x->x_packet0 = -1;
    x->x_queue_id = -1;


    return (void *)x;
}


#ifdef __cplusplus
extern "C"
{
#endif


void pdp_grey2mask_setup(void)
{


    pdp_grey2mask_class = class_new(gensym("pdp_grey2mask"), (t_newmethod)pdp_grey2mask_new,
    	(t_method)pdp_grey2mask_free, sizeof(t_pdp_grey2mask), 0, A_NULL);


    class_addmethod(pdp_grey2mask_class, (t_method)pdp_grey2mask_input_0, gensym("pdp"),  A_SYMBOL, A_DEFFLOAT, A_NULL);


}

#ifdef __cplusplus
}
#endif
