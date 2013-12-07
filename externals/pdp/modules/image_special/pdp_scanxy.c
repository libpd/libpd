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
#include <math.h>

typedef struct pdp_scanxy_struct
{
    t_object x_obj;
    t_float x_f;

    t_outlet *x_outlet0;
    t_outlet *x_outlet1;

    int x_packet0;
    int x_packet1;

    int x_interpolate;


} t_pdp_scanxy;


static t_int *pdp_scanxy_perform(t_int *w)
{

  t_pdp_scanxy *x  = (t_pdp_scanxy *)(w[1]);
  t_int n = (t_int)(w[2]);
  t_float *inx    = (float *)(w[3]);
  t_float *iny    = (float *)(w[4]);
  t_float *out   = (float *)(w[5]);


  /* check if valid image */
  if (-1 == x->x_packet0){
      while (n--) *out++ = 0;
      return (w+6);
  }
  else{

      t_pdp *header0      = pdp_packet_header(x->x_packet0);
      short int  *data0   = (short int *)pdp_packet_data  (x->x_packet0);
      short int  *data1   = (short int *)pdp_packet_data  (x->x_packet1);
      int   width    = (float)header0->info.image.width;
      int   height   = (float)header0->info.image.height;
      int i;

      float scale = 1.0f / 32767.0f;
      float scalein = 0x10000;

      if (x->x_interpolate && (-1 != x->x_packet1)){
	  float a_old = 1.0f;
	  float a_new = 0.0f;
	  float a_inc = 1.0f / (float)n;
	  float old, new;

	  while(n--){
	      int xxx = ((((int)(scalein * *inx++)) & 0xffff) * width) >> 16;
	      int yyy = ((((int)(scalein * *iny++)) & 0xffff) * height) >> 16;
	      int offset = yyy*width+xxx;
	      new = ((float)(data0[offset])) * scale;
	      old = ((float)(data1[offset])) * scale;
	      *out++ = a_old * old + a_new * new;
	      a_new += a_inc;
	      a_old -= a_inc;
	  }

	  pdp_packet_mark_unused(x->x_packet1);
	  x->x_packet1 = -1;
      }
      else{
	  while(n--){
	      int xxx = ((((int)(scalein * *inx++)) & 0xffff) * width) >> 16;
	      int yyy = ((((int)(scalein * *iny++)) & 0xffff) * height) >> 16;
	      int offset = yyy*width+xxx;
	      *out++ = ((float)(data0[offset])) * scale;
	  }
      }

      return (w+6);

  }
}
  



static void pdp_scanxy_input_0(t_pdp_scanxy *x, t_symbol *s, t_floatarg f)
{
    int packet = (int)f;

    /* register */
    if (s== gensym("register_ro")){
	t_pdp *header = pdp_packet_header(packet);
	if (!header) return;
	if (PDP_IMAGE != header->type) return;
	if ((header->info.image.encoding != PDP_IMAGE_YV12) && (header->info.image.encoding != PDP_IMAGE_GREY)) return;

	/* replace if not compatible or we are not interpolating */
	if (!x->x_interpolate || (!pdp_packet_image_compat(x->x_packet0, packet))){
	    pdp_packet_mark_unused(x->x_packet0);
	    x->x_packet0 = pdp_packet_copy_ro(packet);
	}
	/* otherwize keep the old one */
	else{
	    pdp_packet_mark_unused(x->x_packet1);
	    x->x_packet1 = x->x_packet0;
	    x->x_packet0 = pdp_packet_copy_ro(packet);
	}
    }

    /*  pass packet */
    if (s== gensym("process")){
	//if (-1 != x->x_packet0) outlet_pdp (x->x_outlet0, x->x_packet0);
    }


}




static void pdp_scanxy_dsp (t_pdp_scanxy *x, t_signal **sp)
{
    dsp_add(pdp_scanxy_perform, 5, x, sp[0]->s_n, sp[0]->s_vec, sp[1]->s_vec, sp[2]->s_vec);

}


static void pdp_scanxy_interpolate(t_pdp_scanxy *x, t_floatarg f)
{
    if (0.0 == f){
	x->x_interpolate = 0;
	pdp_packet_mark_unused(x->x_packet1);
    }
    if (1.0 == f) x->x_interpolate = 1;
}

static void pdp_scanxy_free(t_pdp_scanxy *x)
{
    pdp_packet_mark_unused(x->x_packet0);
}


t_class *pdp_scanxy_class;



void *pdp_scanxy_new(void)
{
    t_pdp_scanxy *x = (t_pdp_scanxy *)pd_new(pdp_scanxy_class);


    inlet_new(&x->x_obj, &x->x_obj.ob_pd, gensym("signal"), gensym("signal"));
    x->x_outlet1 = outlet_new(&x->x_obj, &s_signal); 
    x->x_packet0 = -1;
    x->x_packet1 = -1;

    pdp_scanxy_interpolate(x, 0);

    return (void *)x;
}


#ifdef __cplusplus
extern "C"
{
#endif


void pdp_scanxy_setup(void)
{

    pdp_scanxy_class = class_new(gensym("pdp_scanxy~"), (t_newmethod)pdp_scanxy_new,
    	(t_method)pdp_scanxy_free, sizeof(t_pdp_scanxy), 0, A_NULL);

    CLASS_MAINSIGNALIN(pdp_scanxy_class, t_pdp_scanxy, x_f);

    class_addmethod(pdp_scanxy_class, (t_method)pdp_scanxy_interpolate, gensym("interpolate"), A_FLOAT, A_NULL);
    class_addmethod(pdp_scanxy_class, (t_method)pdp_scanxy_input_0, gensym("pdp"),  A_SYMBOL, A_DEFFLOAT, A_NULL);
    class_addmethod(pdp_scanxy_class, (t_method)pdp_scanxy_dsp, gensym("dsp"),  A_NULL);


}

#ifdef __cplusplus
}
#endif
