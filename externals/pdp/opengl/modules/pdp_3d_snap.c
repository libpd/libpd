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


#include <GL/gl.h>
#include "pdp.h"
#include "pdp_3dp_base.h"
#include "pdp_opengl.h"

typedef struct _pdp_3d_snap
{
    t_pdp_3dp_base x_base;
    t_pdp_dpd_commandfactory x_cfact;
    t_outlet *x_result_outlet;
    t_pdp_symbol *x_dest_template;
    int x_is_texture;
    u32 x_width;
    u32 x_height;
    int x_auto_snap;
    int x_pending_snap;

} t_pdp_3d_snap;


typedef struct _snap_command
{
    t_pdp_dpd_command x_base;
    t_pdp_3d_snap *x_mother;
    int x_context_packet;
    int x_result_packet;
    int x_active;
} t_snap_command;




/* COMAND METHODS */

static void snap_texture_process(t_snap_command *x)
{
    int pt = -1;
    int p = x->x_context_packet;
    int i;
    u32 w,h;

    if (x->x_active && pdp_packet_3Dcontext_isvalid(p)){
	
	/* get dest texture sub dims */
	w = (x->x_mother->x_width)  ? x->x_mother->x_width : pdp_packet_3Dcontext_subwidth(p);
	h = (x->x_mother->x_height) ? x->x_mother->x_height : pdp_packet_3Dcontext_subheight(p);

	/* texture is a special case */
	if (x->x_mother->x_is_texture){

	    /* create a new texture packet */
	    pt = pdp_packet_new_texture(w,h,GL_RGB);
	    if (-1 != pt) {
		
		/* set rendering context */
		pdp_packet_3Dcontext_set_rendering_context(p);
		
		/* copy pbuf to new texture */
		pdp_packet_texture_make_current(pt);
		//glReadBuffer(GL_FRONT); //this is for weird feedback stuff..
		glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 0, 0, w, h);
		
		x->x_result_packet = pt;
	    }
	}

	/* other type: snap to bitmap first, then convert */
	else{

	    //nvidia driver 4191 bug workaround (w -> multiple of 4)
	    w &= -4;

	    pt = pdp_packet_3Dcontext_snap_to_bitmap(p, w, h);
	    //pt = pdp_packet_new_bitmap_rgb(w, h);
	    //pdp_packet_print_debug(pt);
	    x->x_result_packet = pdp_packet_convert_ro(pt, x->x_mother->x_dest_template);
	    pdp_packet_mark_unused(pt);
	}
    }
}

static void snap_callback(t_snap_command *x)
{
    /* send packet to outlet */
    pdp_packet_pass_if_valid(x->x_mother->x_result_outlet, &x->x_result_packet);
    pdp_dpd_command_suicide(x);
}


/* PD OBJECT METHODS */


static void pdp_3d_snap_snap(t_pdp_3d_snap *x)
{
    x->x_pending_snap = 1;
}

static void pdp_3d_snap_autosnap(t_pdp_3d_snap *x, t_floatarg f)
{
    if (f){
	x->x_auto_snap = 1;
	x->x_pending_snap = 1;
    }
    else{
	x->x_auto_snap = 0;
	x->x_pending_snap = 0;
    }
}

static void *pdp_3d_snap_get_new_command(t_pdp_3d_snap *x)
{
    t_snap_command *c = (t_snap_command *)pdp_dpd_commandfactory_get_new_command(&x->x_cfact);
    c->x_mother = x;
    c->x_context_packet = pdp_3dp_base_get_context_packet(x);
    c->x_result_packet = -1;
    c->x_active = x->x_pending_snap;
    if (!x->x_auto_snap) x->x_pending_snap = 0;
    return (void *)c;
}


t_class *pdp_3d_snap_class;



void pdp_3d_snap_free(t_pdp_3d_snap *x)
{
    //pdp_dpd_base_queue_wait(x);
    pdp_3dp_base_free(x);
}

void *pdp_3d_snap_new(t_symbol *s, t_floatarg w, t_floatarg h)
{
    t_pdp_3d_snap *x = (t_pdp_3d_snap *)pd_new(pdp_3d_snap_class);

    /* super init */
    pdp_3dp_base_init(x);

    /* get destination template */
    x->x_dest_template = (s == gensym("")) ?  pdp_gensym("texture/*/*") : pdp_gensym(s->s_name);
    x->x_is_texture = pdp_type_description_match(x->x_dest_template, pdp_gensym("texture/*/*"));
    w = (w < 0) ? 0 : w;
    h = (h < 0) ? 0 : h;
    x->x_width = w;
    x->x_height = h;

    x->x_auto_snap = 1;
    x->x_pending_snap = 1;

    /* issue warning */
    if (!x->x_is_texture && !(x->x_width && x->x_height)){
	//post("WARNING: 3dp_snap: target is not a texture and dimensions are not set.");
	//post("WARNING: using default image size 320x240.");
	//x->x_width = 320;
	//x->x_height = 240;
    }

    /* create outlets */
    pdp_3dp_base_add_outlet(x, (t_pdp_method)snap_texture_process, (t_pdp_method)snap_callback);
    x->x_result_outlet = outlet_new((t_object *)x, &s_anything);

    /* init command list */
    pdp_dpd_commandfactory_init(&x->x_cfact, sizeof(t_snap_command));

    /* register command factory method */
    pdp_dpd_base_register_command_factory_method(x, (t_pdp_newmethod)pdp_3d_snap_get_new_command);
       

    return (void *)x;
}


#ifdef __cplusplus
extern "C"
{
#endif


void pdp_3d_snap_setup(void)
{


    pdp_3d_snap_class = class_new(gensym("3dp_snap"), (t_newmethod)pdp_3d_snap_new,
    	(t_method)pdp_3d_snap_free, sizeof(t_pdp_3d_snap), 0, A_DEFSYMBOL, A_DEFFLOAT, A_DEFFLOAT, A_NULL);

    pdp_3dp_base_setup(pdp_3d_snap_class);

    class_addmethod(pdp_3d_snap_class, (t_method)pdp_3d_snap_snap, gensym("bang"), A_NULL);
    class_addmethod(pdp_3d_snap_class, (t_method)pdp_3d_snap_autosnap, gensym("autosnap"), A_FLOAT, A_NULL);

}

#ifdef __cplusplus
}
#endif
