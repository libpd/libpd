/*
 *   Pure Data Packet module.
 *   Copyright (c) 2003 by Johannes Taelman <johannes.taelman@rug.ac.be>
 *   API updates by Tom Schouten <tom@zwizwa.be>
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

typedef struct pdp_cog_struct
{
    t_object x_obj;
    t_float x_f;

    t_outlet *x_outlet0;
    t_outlet *x_outlet1;
    t_outlet *x_outlet2;
    t_outlet *x_outlet3;
    t_outlet *x_outlet4;
 
    int x_packet0;
    int x_threshold; 
    int x_do_thresholding; 
} t_pdp_cog;


static void _pdp_cog_perform(t_pdp_cog *x, int width, int height, short int *data0)
{
    short int *pp;
    int nbpixels;

    int y;
    int h,v;
	
    int rowsums[width];
    int columnsums[height];

    float vsum,vvar,vcog,vstd;
    float hsum,hvar,hcog,hstd;

    
    pp=data0;

    nbpixels=width*height;

    for (h=0;h<width;h++)
	columnsums[h]=0;

    /* create column & row sums from thresholded data */
    if (x->x_do_thresholding){
	for (v=0;v<height;v++){
	    int rs=0;
	    for (h=0;h<width;h++){
		int d=*pp++;
		
		d=(d>x->x_threshold) ? d : ((d<-x->x_threshold)?(-d):0);
		columnsums[h]+= d;
		rs+=d;
	    }
	    rowsums[v]=rs;
	}
    }

    /* don't perform thresholding */
    else{
	for (v=0;v<height;v++){
	    int rs=0;
	    for (h=0;h<width;h++){
		int d=*pp++;
		columnsums[h]+= d;
		rs+=d;
	    }
	    rowsums[v]=rs;
	}
    }


    /* compute vertical mean and standard dev */
    vsum=1;
    vvar=height*height/4;
    vcog=height/2;
    for (v=0;v<height;v++){
	float d=rowsums[v];
	vsum+=d;
	vcog+=d*v;
    }
    vcog/=vsum;
    for (v=0;v<height;v++){
	float d=rowsums[v];
	float f=v-vcog;
	vvar+=d*f*f;
    }
    vstd=sqrt(vvar/vsum)/height;

    /* compute horizontal meaan and standard dev */
    hsum=1;
    hvar=width*width/4;
    hcog=width/2;
    for (h=0;h<width;h++){
	float d=columnsums[h];
	hsum+=d;
	hcog+=d*h;
    }
    hcog/=hsum;
    for (h=0;h<width;h++){
	float d=columnsums[h];
	float f=h-hcog;
	hvar+=d*f*f;
    }
    hstd=sqrt(hvar/hsum)/width;
   
    /* pass it on */
    outlet_float(x->x_outlet4,vstd);
    outlet_float(x->x_outlet3,hstd);
    outlet_float(x->x_outlet2,vcog/height);
    outlet_float(x->x_outlet1,hcog/width);
    outlet_float(x->x_outlet0,(1.0f / (float)(0x7fff)) * hsum/(height*width));
}
  

// packet is an image/*/* packet or invalid */
static void pdp_cog_perform(t_pdp_cog *x)
{
    t_pdp *header0 = pdp_packet_header(x->x_packet0);
    void *data0 = pdp_packet_data(x->x_packet0);
    if (!header0 || !data0) return;

    _pdp_cog_perform(x,
		     header0->info.image.width,
		     header0->info.image.height,
		     data0);
}



static void pdp_cog_input_0(t_pdp_cog *x, t_symbol *s, t_floatarg f)
{
    int packet = (int)f;

    /* register */
    if (s == gensym("register_ro")){
	/* replace if not compatible or we are not interpolating */
	pdp_packet_mark_unused(x->x_packet0);
	x->x_packet0 = pdp_packet_convert_ro(packet, pdp_gensym("image/*/*"));
	
    }

    if (s == gensym("process")){
	pdp_cog_perform(x);
    }

}


static void pdp_cog_threshold(t_pdp_cog *x, t_floatarg f)
{
    x->x_threshold=(int) (f * ((float) 0x7fff));
}



static void pdp_cog_free(t_pdp_cog *x)
{
    pdp_packet_mark_unused(x->x_packet0);
}


t_class *pdp_cog_class;



void *pdp_cog_new(void)
{

    t_pdp_cog *x = (t_pdp_cog *)pd_new(pdp_cog_class);


    x->x_outlet0 = outlet_new(&x->x_obj, &s_float); 
    x->x_outlet1 = outlet_new(&x->x_obj, &s_float);
    x->x_outlet2 = outlet_new(&x->x_obj, &s_float); 
    x->x_outlet3 = outlet_new(&x->x_obj, &s_float); 
    x->x_outlet4 = outlet_new(&x->x_obj, &s_float); 

    x->x_packet0 = -1;
    x->x_do_thresholding = 0;
    
    return (void *)x;
}

void *pdp_cog_abs_thresh_new(t_floatarg f)
{
    t_pdp_cog *x = (t_pdp_cog *)pdp_cog_new();
    inlet_new((void *)x, &x->x_obj.ob_pd, gensym("float"),gensym("threshold"));
    pdp_cog_threshold(x, f);
    x->x_do_thresholding = 1;
    return (void *)x;
}



#ifdef __cplusplus
extern "C"
{
#endif


void pdp_cog_setup(void)
{

    pdp_cog_class = class_new(gensym("pdp_cog"), (t_newmethod)pdp_cog_new,
    	(t_method)pdp_cog_free, sizeof(t_pdp_cog), 0,A_NULL);

    class_addcreator((t_newmethod)pdp_cog_abs_thresh_new, gensym("pdp_cog_abs_thresh"), A_DEFFLOAT, A_NULL);

    class_addmethod(pdp_cog_class, (t_method)pdp_cog_input_0, gensym("pdp"),  A_SYMBOL, A_DEFFLOAT, A_NULL);

    class_addmethod(pdp_cog_class, (t_method)pdp_cog_threshold, gensym("threshold"),A_DEFFLOAT, A_NULL);
}

#ifdef __cplusplus
}
#endif



