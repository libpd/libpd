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
#include "pdp_opengl.h"
#include "pdp_3dp_base.h"



/* PD OBJECT */
typedef struct _pdp_3d_view
{
    t_pdp_3dp_base x_base;
    t_pdp_dpd_commandfactory x_clist;

    float x_p0;
    float x_p1;
    float x_p2;
    float x_p3;
    t_pdp_method x_method;


    int x_inlets;
} t_pdp_3d_view;


/* COMMAND OBJECT */
typedef struct _viewcommand
{
    t_pdp_dpd_command x_head; // viewcommand base
    t_pdp_3d_view *x_x;       // command owner
    int x_context_packet;
    float x_p0;
    float x_p1;
    float x_p2;
    float x_p3;
    
} t_viewcommand;




/* COMMAND OBJECT METHODS */

/* rotate about the negative z axis */
static void view_rot2d(t_viewcommand *x) {glRotatef(x->x_p0, 0, 0, -1);}

/* rotate about the positive x,y,z axis */
static void view_rotx(t_viewcommand *x) {glRotatef(x->x_p0, 1, 0, 0);}
static void view_roty(t_viewcommand *x) {glRotatef(x->x_p0, 0, 1, 0);}
static void view_rotz(t_viewcommand *x) {glRotatef(x->x_p0, 0, 0, 1);}
static void view_rota(t_viewcommand *x) {glRotatef(x->x_p3, x->x_p0, x->x_p1, x->x_p2);}

/* translate along an axis */
static void view_transx(t_viewcommand *x) {glTranslatef(x->x_p0, 0, 0);}
static void view_transy(t_viewcommand *x) {glTranslatef(0, x->x_p0, 0);}
static void view_transz(t_viewcommand *x) {glTranslatef(0, 0, x->x_p0);}
static void view_transxyz(t_viewcommand *x) {glTranslatef(x->x_p0, x->x_p1, x->x_p2);}

/* rotate about the positive x,y,z axis */
static void view_scalex(t_viewcommand *x) {glScalef(x->x_p0, 1, 1);}
static void view_scaley(t_viewcommand *x) {glScalef(1, x->x_p0, 1);}
static void view_scalez(t_viewcommand *x) {glScalef(1, 1, x->x_p0);}
static void view_scale(t_viewcommand *x) {glScalef(x->x_p0, x->x_p0, x->x_p0);}

/* specials */
static void view_reset_3d(t_viewcommand *x) {pdp_packet_3Dcontext_setup_3d_context(x->x_context_packet);}
static void view_scale_aspect(t_viewcommand *x) {glScalef(pdp_packet_3Dcontext_subaspect(x->x_context_packet),1,1);}


/* process command */
static void view_process(t_viewcommand *x)
{
    int p = x->x_context_packet;
 
    /* check if it's a valid context buffer we can draw in */
    if (pdp_packet_3Dcontext_isvalid(p)){ 

	/* setup rendering context */
	pdp_packet_3Dcontext_set_rendering_context(p);

	/* call the generating method */
	if (x->x_x->x_method) (*x->x_x->x_method)(x);
    }

    /* suicide */
    pdp_dpd_command_suicide(x);
}



/* command object factory method */
void *pdp_3d_view_get_command_object(t_pdp_3d_view *x)
{
    t_viewcommand *c = (t_viewcommand *)pdp_dpd_commandfactory_get_new_command(&x->x_clist);
    c->x_p0 = x->x_p0;
    c->x_p1 = x->x_p1;
    c->x_p2 = x->x_p2;
    c->x_p3 = x->x_p3;
    c->x_context_packet = pdp_3dp_base_get_context_packet(x);
    c->x_x = x;

    return c;
}



/* PD OBJECT METHODS */

static void pdp_3d_view_p0(t_pdp_3d_view *x, t_floatarg f){x->x_p0 = f;}
static void pdp_3d_view_p1(t_pdp_3d_view *x, t_floatarg f){x->x_p1 = f;}
static void pdp_3d_view_p2(t_pdp_3d_view *x, t_floatarg f){x->x_p2 = f;}
static void pdp_3d_view_p3(t_pdp_3d_view *x, t_floatarg f){x->x_p3 = f;}


t_class *pdp_3d_view_class;



void pdp_3d_view_free(t_pdp_3d_view *x)
{
    pdp_dpd_commandfactory_free(&x->x_clist);
    pdp_3dp_base_free(x);
}

void *pdp_3d_view_new(t_symbol *s, t_floatarg p0, t_floatarg p1, t_floatarg p2, t_floatarg p3)
{
    t_pdp_3d_view *x = (t_pdp_3d_view *)pd_new(pdp_3d_view_class);
    char param[] = "p0";
    int i;

    /* super init */
    pdp_3dp_base_init(x);

    x->x_p0 = p0;
    x->x_p1 = p1;
    x->x_p2 = p2;
    x->x_p3 = p3;

    /* find out which transform we need to apply */
    if      (s == gensym("rot2d")) {x->x_method = (t_pdp_method)view_rot2d; x->x_inlets = 1;}

    else if (s == gensym("rotx"))  {x->x_method = (t_pdp_method)view_rotx;  x->x_inlets = 1;}
    else if (s == gensym("roty"))  {x->x_method = (t_pdp_method)view_roty;  x->x_inlets = 1;}
    else if (s == gensym("rotz"))  {x->x_method = (t_pdp_method)view_rotz;  x->x_inlets = 1;}
    else if (s == gensym("rota"))  {x->x_method = (t_pdp_method)view_rota;  x->x_inlets = 4;}

    else if (s == gensym("transx"))    {x->x_method = (t_pdp_method)view_transx;  x->x_inlets = 1;}
    else if (s == gensym("transy"))    {x->x_method = (t_pdp_method)view_transy;  x->x_inlets = 1;}
    else if (s == gensym("transz"))    {x->x_method = (t_pdp_method)view_transz;  x->x_inlets = 1;}
    else if (s == gensym("transxyz"))  {x->x_method = (t_pdp_method)view_transxyz;  x->x_inlets = 3;}

    else if (s == gensym("scalex"))  {x->x_method = (t_pdp_method)view_scalex;  x->x_inlets = 1;}
    else if (s == gensym("scaley"))  {x->x_method = (t_pdp_method)view_scaley;  x->x_inlets = 1;}
    else if (s == gensym("scalez"))  {x->x_method = (t_pdp_method)view_scalez;  x->x_inlets = 1;}
    else if (s == gensym("scale"))   {x->x_method = (t_pdp_method)view_scale;  x->x_inlets = 1;}

    else if (s == gensym("scale_aspect"))   {x->x_method = (t_pdp_method)view_scale_aspect;  x->x_inlets = 0;}
    else if (s == gensym("reset"))   {x->x_method = (t_pdp_method)view_reset_3d;  x->x_inlets = 0;}

    else {
	post("pdp_view: view transformation %s not found", s->s_name);
	x->x_method = 0;
	x->x_inlets = 0;
    }

    /* create additional inlets */
    for(i=0; i<x->x_inlets; i++){
	pdp_base_add_gen_inlet(x, gensym("float"), gensym(param));
	param[1]++;
    }

    /* create dpd outlet */
    pdp_3dp_base_add_outlet(x, (t_pdp_method)view_process, 0);

    /* init command factory */
    pdp_dpd_commandfactory_init(&x->x_clist, sizeof(t_viewcommand));

    /* register command factory method */
    pdp_dpd_base_register_command_factory_method(x, (t_pdp_newmethod)pdp_3d_view_get_command_object);
       

    return (void *)x;
}


#ifdef __cplusplus
extern "C"
{
#endif


void pdp_3d_view_setup(void)
{


    pdp_3d_view_class = class_new(gensym("3dp_view"), (t_newmethod)pdp_3d_view_new,
    	(t_method)pdp_3d_view_free, sizeof(t_pdp_3d_view), 0, A_SYMBOL, 
				   A_DEFFLOAT,A_DEFFLOAT,A_DEFFLOAT,A_DEFFLOAT, A_NULL);

    pdp_3dp_base_setup(pdp_3d_view_class);

    class_addmethod(pdp_3d_view_class, (t_method)pdp_3d_view_p0, gensym("p0"),  A_DEFFLOAT, A_NULL);  
    class_addmethod(pdp_3d_view_class, (t_method)pdp_3d_view_p1, gensym("p1"),  A_DEFFLOAT, A_NULL);  
    class_addmethod(pdp_3d_view_class, (t_method)pdp_3d_view_p2, gensym("p2"),  A_DEFFLOAT, A_NULL);  
    class_addmethod(pdp_3d_view_class, (t_method)pdp_3d_view_p3, gensym("p3"),  A_DEFFLOAT, A_NULL);  

}

#ifdef __cplusplus
}
#endif
