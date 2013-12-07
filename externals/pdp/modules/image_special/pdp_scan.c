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
#include "pdp_mmx.h"
#include <math.h>

#define PDP_SCAN_COSTABLE_SIZE 1024
static float pdp_cos[PDP_SCAN_COSTABLE_SIZE];

typedef struct pdp_scan_struct
{
    t_object x_obj;
    t_float x_f;

    t_outlet *x_outlet0;
    t_outlet *x_outlet1;

    float x_centerx;
    float x_centery;
    float x_sizeh;
    float x_sizev;

    int x_packet0;
    int x_packet1;

    int x_interpolate;


} t_pdp_scan;


static t_int *pdp_scan_perform(t_int *w)
{

  t_pdp_scan *x  = (t_pdp_scan *)(w[1]);
  t_int n = (t_int)(w[2]);
  t_float *in    = (float *)(w[3]);
  t_float *out   = (float *)(w[4]);


  /* check if valid image */
  if (-1 == x->x_packet0){
      while (n--) *out++ = 0;
      return (w+5);
  }
  else{

      t_pdp *header0      = pdp_packet_header(x->x_packet0);
      short int  *data0   = (short int *)pdp_packet_data  (x->x_packet0);
      short int  *data1   = (short int *)pdp_packet_data  (x->x_packet1);
      int   width    = (float)header0->info.image.width;
      float widthm1  = (float)header0->info.image.width - 1;
      float heightm1 = (float)header0->info.image.height - 1;
      int i;

      float scale = 1.0f / 32767.0f;

      if (x->x_interpolate && (-1 != x->x_packet1)){
	  float a_old = 1.0f;
	  float a_new = 0.0f;
	  float a_inc = 1.0f / (float)n;
	  float old, new;

	  while(n--){
	      float phase = *in++;
	      int iphase = (int)(phase * PDP_SCAN_COSTABLE_SIZE);
	      float c = pdp_cos[iphase & (PDP_SCAN_COSTABLE_SIZE - 1)];
	      float s = pdp_cos[(iphase - (PDP_SCAN_COSTABLE_SIZE>>1))  & (PDP_SCAN_COSTABLE_SIZE - 1)];
	      int xxx = (int)((x->x_centerx + x->x_sizeh * c) * widthm1);
	      int yyy = (int)((x->x_centery + x->x_sizev * c) * heightm1);
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
	      float phase = *in++;
	      int iphase = (int)(phase * PDP_SCAN_COSTABLE_SIZE);
	      float c = pdp_cos[iphase & (PDP_SCAN_COSTABLE_SIZE - 1)];
	      float s = pdp_cos[(iphase - (PDP_SCAN_COSTABLE_SIZE>>1))  & (PDP_SCAN_COSTABLE_SIZE - 1)];
	      int xxx = (int)((x->x_centerx + x->x_sizeh * c) * widthm1);
	      int yyy = (int)((x->x_centery + x->x_sizev * c) * heightm1);
	      *out++ = ((float)(data0[yyy*width+xxx])) * scale;
	  }
      }

      return (w+5);

  }
}
  



static void pdp_scan_input_0(t_pdp_scan *x, t_symbol *s, t_floatarg f)
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




static void pdp_scan_dsp (t_pdp_scan *x, t_signal **sp)
{
    dsp_add(pdp_scan_perform, 4, x, sp[0]->s_n, sp[0]->s_vec, sp[1]->s_vec);

}


static void pdp_scan_interpolate(t_pdp_scan *x, t_floatarg f)
{
    if (0.0 == f){
	x->x_interpolate = 0;
	pdp_packet_mark_unused(x->x_packet1);
    }
    if (1.0 == f) x->x_interpolate = 1;
}

static void pdp_scan_free(t_pdp_scan *x)
{
    pdp_packet_mark_unused(x->x_packet0);
}


t_class *pdp_scan_class;



void *pdp_scan_new(void)
{
    t_pdp_scan *x = (t_pdp_scan *)pd_new(pdp_scan_class);


    //x->x_outlet0 = outlet_new(&x->x_obj, &s_anything);
    x->x_outlet1 = outlet_new(&x->x_obj, &s_signal); 
    x->x_packet0 = -1;
    x->x_packet1 = -1;

    x->x_centerx = 0.5f;
    x->x_centery = 0.5f;
    x->x_sizeh = 0.3;
    x->x_sizev = 0.3;

    pdp_scan_interpolate(x, 0);

    return (void *)x;
}


#ifdef __cplusplus
extern "C"
{
#endif


void pdp_scan_setup(void)
{
    int i;
    for (i=0; i<PDP_SCAN_COSTABLE_SIZE; i++) 
	pdp_cos[i] = cos((double)(i) * 2 * M_PI / PDP_SCAN_COSTABLE_SIZE);


    pdp_scan_class = class_new(gensym("pdp_scan~"), (t_newmethod)pdp_scan_new,
    	(t_method)pdp_scan_free, sizeof(t_pdp_scan), 0, A_NULL);

    CLASS_MAINSIGNALIN(pdp_scan_class, t_pdp_scan, x_f);

    class_addmethod(pdp_scan_class, (t_method)pdp_scan_interpolate, gensym("interpolate"), A_FLOAT, A_NULL);
    class_addmethod(pdp_scan_class, (t_method)pdp_scan_input_0, gensym("pdp"),  A_SYMBOL, A_DEFFLOAT, A_NULL);
    class_addmethod(pdp_scan_class, (t_method)pdp_scan_dsp, gensym("dsp"),  A_NULL);


}

#ifdef __cplusplus
}
#endif
