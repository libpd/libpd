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
#include "pdp_imagebase.h"



typedef struct pdp_zoom_struct
{
    t_pdp_imagebase x_base;

    int x_packet1;
    t_outlet *x_outlet0;
    void *x_zoom;

    int x_quality; //not used

    
} t_pdp_zoom;


static void pdp_zoom_process(t_pdp_zoom *x)
{
    int p0 = pdp_base_get_packet(x, 0);
    u32 mask = pdp_imagebase_get_chanmask(x);
    pdp_imageproc_dispatch_2buf(&pdp_imageproc_resample_affinemap_process, x->x_zoom, mask, p0, x->x_packet1);
}

static void pdp_zoom_postproc(t_pdp_zoom *x)
{
    /* delete source packet */
    pdp_packet_mark_unused(pdp_base_move_packet(x, 0));

    /* unregister and propagate if valid dest packet */
    pdp_packet_pass_if_valid(x->x_outlet0, &x->x_packet1);
}

static void pdp_zoom_preproc(t_pdp_zoom *x)
{
    int p = pdp_base_get_packet(x, 0);
    t_pdp *header0 = pdp_packet_header(p);
    if ((header0) && (PDP_IMAGE == header0->type)){
	x->x_packet1 = pdp_packet_clone_rw(p);
    }

}



static void pdp_zoom_zoom_x(t_pdp_zoom *x, t_floatarg f)
{
    pdp_imageproc_resample_affinemap_setzoomx(x->x_zoom, f);
}

static void pdp_zoom_angle(t_pdp_zoom *x, t_floatarg f)
{
    pdp_imageproc_resample_affinemap_setangle(x->x_zoom, f);
}

static void pdp_zoom_zoom_y(t_pdp_zoom *x, t_floatarg f)
{
    pdp_imageproc_resample_affinemap_setzoomy(x->x_zoom, f);
}

static void pdp_zoom_zoom(t_pdp_zoom *x, t_floatarg f)
{
    pdp_zoom_zoom_x(x, f);
    pdp_zoom_zoom_y(x, f);
}

static void pdp_zoom_center_x(t_pdp_zoom *x, t_floatarg f)
{
    pdp_imageproc_resample_affinemap_setcenterx(x->x_zoom, f);
}

static void pdp_zoom_center_y(t_pdp_zoom *x, t_floatarg f)
{
    pdp_imageproc_resample_affinemap_setcentery(x->x_zoom, f);
}
static void pdp_zoom_center(t_pdp_zoom *x, t_floatarg fx, t_floatarg fy)
{
    pdp_zoom_center_x(x, fx);
    pdp_zoom_center_y(x, fy);
}

// not used
static void pdp_zoom_quality(t_pdp_zoom *x, t_floatarg f)
{
    if (f==0) x->x_quality = 0;
    if (f==1) x->x_quality = 1;
}


t_class *pdp_zoom_class;



void pdp_zoom_free(t_pdp_zoom *x)
{
    pdp_imagebase_free(x);
    pdp_imageproc_resample_affinemap_delete(x->x_zoom);
    pdp_packet_mark_unused(x->x_packet1);
}


void pdp_zoom_init_common(t_pdp_zoom *x)
{
    pdp_imagebase_init(x);
    pdp_base_set_process_method(x, (t_pdp_method)pdp_zoom_process);
    pdp_base_set_postproc_method(x, (t_pdp_method)pdp_zoom_postproc);
    pdp_base_set_preproc_method(x, (t_pdp_method)pdp_zoom_preproc);

    x->x_outlet0 = pdp_base_add_pdp_outlet(x); 
    x->x_packet1 = -1;
    x->x_zoom = pdp_imageproc_resample_affinemap_new();

    //quality is not used: all routines are "high quality" bilinear
    //pdp_zoom_quality(x, 1);
    pdp_zoom_center_x(x, 0.5f);
    pdp_zoom_center_y(x, 0.5f);
  
}


void *pdp_zoom_new(t_floatarg zoom)
{
    t_pdp_zoom *x = (t_pdp_zoom *)pd_new(pdp_zoom_class);

    pdp_zoom_init_common(x);
    pdp_base_add_gen_inlet(x, gensym("float"), gensym("zoom"));

    if (zoom == 0.0f) zoom = 1.0f;
    pdp_zoom_zoom(x, zoom);
    pdp_zoom_angle(x, 0.0f);

    return (void *)x;
}

void *pdp_zrot_new(t_floatarg zoom, t_floatarg angle)
{
    t_pdp_zoom *x = (t_pdp_zoom *)pd_new(pdp_zoom_class);

    pdp_zoom_init_common(x);
    pdp_base_add_gen_inlet(x, gensym("float"), gensym("zoom"));
    pdp_base_add_gen_inlet(x, gensym("float"), gensym("angle"));


    if (zoom == 0.0f) zoom = 1.0f;
    pdp_zoom_zoom(x, zoom);
    pdp_zoom_angle(x, angle);

    return (void *)x;
}

void *pdp_rotate_new(t_floatarg angle)
{
    t_pdp_zoom *x = (t_pdp_zoom *)pd_new(pdp_zoom_class);

    pdp_zoom_init_common(x);

    pdp_base_add_gen_inlet(x, gensym("float"), gensym("angle"));

    pdp_zoom_zoom(x, 1.0f);
    pdp_zoom_angle(x, angle);

    return (void *)x;
}


#ifdef __cplusplus
extern "C"
{
#endif


void pdp_zoom_setup(void)
{

    pdp_zoom_class = class_new(gensym("pdp_zoom"), (t_newmethod)pdp_zoom_new,
    	(t_method)pdp_zoom_free, sizeof(t_pdp_zoom), 0, A_DEFFLOAT, A_NULL);

    class_addcreator((t_newmethod)pdp_zrot_new, gensym("pdp_zrot"), A_DEFFLOAT, A_DEFFLOAT, A_NULL);
    class_addcreator((t_newmethod)pdp_rotate_new, gensym("pdp_rotate"), A_DEFFLOAT, A_NULL);

    pdp_imagebase_setup(pdp_zoom_class);

    class_addmethod(pdp_zoom_class, (t_method)pdp_zoom_quality, gensym("quality"),  A_FLOAT, A_NULL);   
    class_addmethod(pdp_zoom_class, (t_method)pdp_zoom_center_x, gensym("centerx"),  A_FLOAT, A_NULL);   
    class_addmethod(pdp_zoom_class, (t_method)pdp_zoom_center_y, gensym("centery"),  A_FLOAT, A_NULL);   
    class_addmethod(pdp_zoom_class, (t_method)pdp_zoom_center, gensym("center"),  A_FLOAT, A_FLOAT, A_NULL);   
    class_addmethod(pdp_zoom_class, (t_method)pdp_zoom_zoom_x, gensym("zoomx"),  A_FLOAT, A_NULL);   
    class_addmethod(pdp_zoom_class, (t_method)pdp_zoom_zoom_y, gensym("zoomy"),  A_FLOAT, A_NULL);   
    class_addmethod(pdp_zoom_class, (t_method)pdp_zoom_zoom, gensym("zoom"),  A_FLOAT, A_NULL);   
    class_addmethod(pdp_zoom_class, (t_method)pdp_zoom_angle, gensym("angle"),  A_FLOAT, A_NULL);   


}

#ifdef __cplusplus
}
#endif
