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

typedef struct pdp_3d_light_struct
{
    t_pdp_3dp_base x_base;

    //float x_centerx;
    //float x_centery;
    //float x_centerz;
    int x_index;

} t_pdp_3d_light;



static void pdp_3d_light_process(t_pdp_3d_light *x)
{
    int p = pdp_3dp_base_get_context_packet(x);
    int i;
    GLfloat ambient[] = {.7,.7,.7,1};
    GLfloat diffuse[] = {.6,.6,.6,1};
    GLfloat specular[] = {1, 1, 1, 1};
    GLfloat shininess[] = {50};
    GLfloat position[] = {0,0,1,1};
    GLfloat intensity[] = {1,1,1,0};

    int light = GL_LIGHT0 + x->x_index;

    /* check if it's a valid buffer we can draw in */
    if (pdp_packet_3Dcontext_isvalid(p)){

	position[0] = 0; //x->x_centerx;
	position[1] = 0; //x->x_centery;
	position[2] = 0; //x->x_centerz;
	
	/* set rendering context */
	//pdp_packet_3Dcontext_set_rendering_context(p);

	/* setup lighting */
	
	glEnable(GL_LIGHTING);
	glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);
	glColorMaterial(GL_FRONT_AND_BACK, GL_DIFFUSE);
	glEnable(GL_COLOR_MATERIAL);
	//glMaterialfv(GL_FRONT, GL_AMBIENT, ambient);
	glMaterialfv(GL_FRONT, GL_SPECULAR, specular);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, diffuse);
	glMaterialfv(GL_FRONT, GL_SHININESS, shininess);
	glLightfv(light, GL_POSITION, position);
	//glLightfv(light, GL_DIFFUSE, intensity);
	glEnable(light);


	/* ALPHA HACK */
	//glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_FALSE);
	//glEnable(GL_BLEND);
	//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	//glBlendFunc(GL_SRC_ALPHA, GL_ONE);
	//glEnable(GL_ALPHA_TEST);
	//glAlphaFunc(GL_GREATER, 0.f);

    }

}



//static void pdp_3d_light_centerx(t_pdp_3d_light *x, t_floatarg f){x->x_centerx = f;}
//static void pdp_3d_light_centery(t_pdp_3d_light *x, t_floatarg f){x->x_centery = f;}
//static void pdp_3d_light_centerz(t_pdp_3d_light *x, t_floatarg f){x->x_centerz = f;}


t_class *pdp_3d_light_class;



void pdp_3d_light_free(t_pdp_3d_light *x)
{
    pdp_3dp_base_free(x);
}

void *pdp_3d_light_new(t_floatarg fi, t_floatarg cx, t_floatarg cy, t_floatarg cz)
{
    t_pdp_3d_light *x = (t_pdp_3d_light *)pd_new(pdp_3d_light_class);

    /* super init */
    pdp_3dp_base_init(x);

    if (fi < 0) fi = 0;

    x->x_index = (int)fi;
    //x->x_centerx = cx;
    //x->x_centery = cy;
    //x->x_centerz = cz;

    /* io */
    //pdp_base_add_gen_inlet(x, gensym("float"), gensym("centerx"));
    //pdp_base_add_gen_inlet(x, gensym("float"), gensym("centery"));
    //pdp_base_add_gen_inlet(x, gensym("float"), gensym("centerz"));

    /* add dpd outlet */
    pdp_3dp_base_add_outlet(x,  (t_pdp_method)pdp_3d_light_process, 0);

    return (void *)x;
}


#ifdef __cplusplus
extern "C"
{
#endif


void pdp_3d_light_setup(void)
{


    pdp_3d_light_class = class_new(gensym("3dp_light"), (t_newmethod)pdp_3d_light_new,
    	(t_method)pdp_3d_light_free, sizeof(t_pdp_3d_light), 0, 
				   A_DEFFLOAT, A_DEFFLOAT, A_DEFFLOAT, A_DEFFLOAT, A_NULL);

    pdp_3dp_base_setup(pdp_3d_light_class);

    //class_addmethod(pdp_3d_light_class, (t_method)pdp_3d_light_centerx, gensym("centerx"),  A_DEFFLOAT, A_NULL);   
    //class_addmethod(pdp_3d_light_class, (t_method)pdp_3d_light_centery, gensym("centery"),  A_DEFFLOAT, A_NULL);   
    //class_addmethod(pdp_3d_light_class, (t_method)pdp_3d_light_centerz, gensym("centerz"),  A_DEFFLOAT, A_NULL);   

}

#ifdef __cplusplus
}
#endif
