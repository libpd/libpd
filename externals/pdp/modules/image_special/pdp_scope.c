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
#include <string.h>

#define BUFSIZE 2048

typedef struct pdp_scope_data
{
    short int random_seed[4];

}t_pdp_scope_data;

typedef struct pdp_scope_struct
{
    t_object x_obj;
    t_float x_f;

    t_outlet *x_outlet0;

    t_pdp_scope_data *x_data;
    int x_packet0;
    int x_queue_id;

    int x_pdp_image_type;

    unsigned int x_width;
    unsigned int x_height;

    float *x_buffer;
    int x_needle;
    
 
} t_pdp_scope;



void pdp_scope_type(t_pdp_scope *x, t_symbol *s)
{
    if (gensym("yv12") == s) {x->x_pdp_image_type = PDP_IMAGE_YV12; return;}
    if (gensym("grey") == s) {x->x_pdp_image_type = PDP_IMAGE_GREY; return;}

    x->x_pdp_image_type = -1;
    
}





static void pdp_scope_createpacket_yv12(t_pdp_scope *x)
{
    t_pdp *header;

    unsigned int w = x->x_width;
    unsigned int h = x->x_height;

    unsigned int size = w*h;
    unsigned int totalnbpixels = size + (size >> 1);
    unsigned int packet_size = totalnbpixels << 1;

    x->x_packet0 = pdp_packet_new_image_YCrCb(w, h);
    if(x->x_packet0 == -1){
	post("pdp_scope: can't allocate packet");
	return;
    }
    header = pdp_packet_header(x->x_packet0);
    memset(pdp_packet_data(x->x_packet0), 0, packet_size);

}

static void pdp_scope_generate_yv12(t_pdp_scope *x)
{
    unsigned int w = x->x_width;
    unsigned int h = x->x_height;
    unsigned int size = w*h;
    unsigned int totalnbpixels = size + (size >> 1);
    short int *data = (short int *) pdp_packet_data(x->x_packet0);

    unsigned int i;
    int offset = x->x_needle;
    int val;
    unsigned int y;
    float fh2 = (float)(h/2);

    if (!data) return;


    for (i=0; i<w; i++){
	y = (h/2) + (int)(fh2 * -x->x_buffer[(offset - w + i) & (BUFSIZE - 1)]);
	if (y>=h) y = h-1;
    
	data[i + y*w] = 0x7fff;
    }

    return;

}

static void pdp_scope_createpacket_grey(t_pdp_scope *x)
{
    t_pdp *header;
    short int *data;

    unsigned int w = x->x_width;
    unsigned int h = x->x_height;

    unsigned int size = w*h;
    unsigned int totalnbpixels = size;
    unsigned int packet_size = totalnbpixels << 1;

    /* create new packet */
    x->x_packet0 = pdp_packet_new_image_grey(w,h);
    if(x->x_packet0 == -1){
	post("pdp_scope: can't allocate packet");
	return;
    }


    header = pdp_packet_header(x->x_packet0);
    data = (short int *) pdp_packet_data(x->x_packet0);

    memset(pdp_packet_data(x->x_packet0), 0, packet_size);

}

static void pdp_scope_generate_grey(t_pdp_scope *x)
{
    unsigned int w = x->x_width;
    unsigned int h = x->x_height;
    unsigned int totalnbpixels = x->x_width * x->x_height;
    short int *data = (short int *) pdp_packet_data(x->x_packet0);
    
    unsigned int i;
    int offset = x->x_needle;
    int val;
    unsigned int y;
    float fh2 = (float)(h/2);

    if (!data) return;

    for (i=0; i<w; i++){
	y = (h/2) + (int)(fh2 * -x->x_buffer[(offset - w + i) & (BUFSIZE - 1)]);
	if (y>=h) y = h-1;
    
	data[i + y*w] = 0x7fff;
    }

    return;
}

static void pdp_scope_sendpacket(t_pdp_scope *x)
{
    /* propagate if valid */
    pdp_packet_pass_if_valid(x->x_outlet0, &x->x_packet0);
}


static void pdp_scope_bang(t_pdp_scope *x)
{

    int encoding;

    /* if we have an active packet, don't do anything */
    if (-1 != x->x_packet0) return;

    switch(x->x_pdp_image_type){

    case PDP_IMAGE_YV12:
	pdp_scope_createpacket_yv12(x); // don't create inside thread!!!
	pdp_scope_generate_yv12(x);
	pdp_scope_sendpacket(x);
	//pdp_queue_add(x, pdp_scope_generate_yv12, pdp_scope_sendpacket, &x->x_queue_id);
	break;
      
    case PDP_IMAGE_GREY:
	pdp_scope_createpacket_grey(x); // don't create inside thread!!!
	pdp_scope_generate_grey(x);
	pdp_scope_sendpacket(x);
	//pdp_queue_add(x, pdp_scope_generate_grey, pdp_scope_sendpacket, &x->x_queue_id);
	break;

    default:
	break;
	
    }


    /* release the packet */

}


static void pdp_scope_dim(t_pdp_scope *x, t_floatarg w, t_floatarg h)
{
    if (w<32.0f) w = 32.0f;
    if (h<32.0f) h = 32.0f;

    x->x_width = (unsigned int)w;
    x->x_height = (unsigned int)h;
}


static void pdp_scope_free(t_pdp_scope *x)
{

    /* remove callback from process queue */
    t_pdp_procqueue *q = pdp_queue_get_queue();
    pdp_procqueue_finish(q, x->x_queue_id);


    /* tidy up */
    pdp_packet_mark_unused(x->x_packet0);
    pdp_dealloc(x->x_data);

}
static t_int *pdp_scope_perform(t_int *w)
{


  t_float *in    = (float *)(w[3]);
  t_pdp_scope *x  = (t_pdp_scope *)(w[1]);
  t_int n = (t_int)(w[2]);
  t_int i;

  t_int offset = x->x_needle;

  for (i=0; i<n; i++)
      x->x_buffer[(offset+i)&(BUFSIZE-1)] = in[i];

  x->x_needle =  (offset + n ) & (BUFSIZE - 1);

  return (w+4);

}
static void pdp_scope_dsp(t_pdp_scope *x, t_signal **sp)
{
    dsp_add(pdp_scope_perform, 3, x, sp[0]->s_n, sp[0]->s_vec);

}
  
t_class *pdp_scope_class;




void *pdp_scope_new(void)
{
    int i;

    t_pdp_scope *x = (t_pdp_scope *)pd_new(pdp_scope_class);

    x->x_outlet0 = outlet_new(&x->x_obj, &s_anything); 

    x->x_packet0 = -1;
    x->x_queue_id = -1;
    x->x_width = 320;
    x->x_height = 240;
    x->x_f = 0.0;

    x->x_data = (t_pdp_scope_data *)pdp_alloc(sizeof(t_pdp_scope_data));

    pdp_scope_type(x, gensym("yv12"));

    x->x_buffer = (float *)pdp_alloc(sizeof(float) * BUFSIZE);
    x->x_needle = 0;

    return (void *)x;
}



#ifdef __cplusplus
extern "C"
{
#endif



void pdp_scope_setup(void)
{


    pdp_scope_class = class_new(gensym("pdp_scope~"), (t_newmethod)pdp_scope_new,
    	(t_method)pdp_scope_free, sizeof(t_pdp_scope), 0, A_NULL);

    CLASS_MAINSIGNALIN(pdp_scope_class, t_pdp_scope, x_f);

    class_addmethod(pdp_scope_class, (t_method)pdp_scope_type, gensym("type"), A_SYMBOL, A_NULL);
    class_addmethod(pdp_scope_class, (t_method)pdp_scope_dim, gensym("dim"), A_FLOAT, A_FLOAT, A_NULL);
    class_addmethod(pdp_scope_class, (t_method)pdp_scope_bang, gensym("bang"), A_NULL);
    class_addmethod(pdp_scope_class, (t_method)pdp_scope_dsp, gensym("dsp"), 0); 
}

#ifdef __cplusplus
}
#endif
