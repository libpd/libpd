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
#include "pdp_3dp_base.h"
#include "pdp_opengl.h"


typedef struct _color_command
{
    t_pdp_dpd_command x_base;
    int x_context;
    float x_newcolor[4];
    float x_oldcolor[4];
} t_color_command;

typedef struct _pdp_3d_color
{
    t_pdp_3dp_base x_base;
    t_pdp_dpd_commandfactory x_cfact;

    float x_red;
    float x_green;
    float x_blue;
    float x_alpha;

} t_pdp_3d_color;



/* COMMAND METHODS */
static void pdp_3d_color_process_right(t_color_command *x)
{
    int p = x->x_context;
    if (pdp_packet_3Dcontext_isvalid(p)){
	pdp_packet_3Dcontext_set_rendering_context(p);

	/* save old color*/
	glGetFloatv(GL_CURRENT_COLOR, x->x_oldcolor);

	/* set new color */
	glColor4fv(x->x_newcolor);
    }

}

static void pdp_3d_color_process_left(t_color_command *x)
{
    int p = x->x_context;
    if (pdp_packet_3Dcontext_isvalid(p)){
	pdp_packet_3Dcontext_set_rendering_context(p);

	/* restore old color */
	glColor4fv(x->x_oldcolor);
	//glColor4f(1,1,1,1);
    }
    /* kill self */
    pdp_dpd_command_suicide(x);
}


/* PD OBJECT METHODS */
static void *pdp_3d_color_get_new_command(t_pdp_3d_color *x)
{
    t_color_command *c = (t_color_command *)pdp_dpd_commandfactory_get_new_command(&x->x_cfact);
    c->x_newcolor[0] = x->x_red;
    c->x_newcolor[1] = x->x_green;
    c->x_newcolor[2] = x->x_blue;
    c->x_newcolor[3] = x->x_alpha;
    c->x_context = pdp_3dp_base_get_context_packet(x);
    return (void *)c;
}


static void pdp_3d_color_set_r(t_pdp_3d_color *x, t_floatarg f) {x->x_red = f;}
static void pdp_3d_color_set_g(t_pdp_3d_color *x, t_floatarg f) {x->x_green = f;}
static void pdp_3d_color_set_b(t_pdp_3d_color *x, t_floatarg f) {x->x_blue = f;}
static void pdp_3d_color_set_a(t_pdp_3d_color *x, t_floatarg f) {x->x_alpha = f;}


t_class *pdp_3d_color_class;


void pdp_3d_color_free(t_pdp_3d_color *x)
{
    pdp_3dp_base_free(x);
}

void *pdp_3d_color_new(t_floatarg r, t_floatarg g, t_floatarg b, t_floatarg a)
{
    t_pdp_3d_color *x = (t_pdp_3d_color *)pd_new(pdp_3d_color_class);

    /* super init */
    pdp_3dp_base_init(x);


    /* input */
    pdp_base_add_gen_inlet(x, gensym("float"), gensym("r"));
    pdp_base_add_gen_inlet(x, gensym("float"), gensym("g"));
    pdp_base_add_gen_inlet(x, gensym("float"), gensym("b"));
    pdp_base_add_gen_inlet(x, gensym("float"), gensym("a"));

    /* output */
    pdp_3dp_base_add_outlet(x, (t_pdp_method)pdp_3d_color_process_left, 0);
    pdp_3dp_base_add_outlet(x, (t_pdp_method)pdp_3d_color_process_right, 0);

    x->x_red = r;
    x->x_green = g;
    x->x_blue = b;
    x->x_alpha = a;

    /* init factory */
    pdp_dpd_commandfactory_init(&x->x_cfact, sizeof(t_color_command));

    /* register command factory method */
    pdp_dpd_base_register_command_factory_method(x, (t_pdp_newmethod)pdp_3d_color_get_new_command);

    return (void *)x;
}


#ifdef __cplusplus
extern "C"
{
#endif


void pdp_3d_color_setup(void)
{


    pdp_3d_color_class = class_new(gensym("3dp_color"), (t_newmethod)pdp_3d_color_new,
    	(t_method)pdp_3d_color_free, sizeof(t_pdp_3d_color), 0, 
				   A_DEFFLOAT,  A_DEFFLOAT,  A_DEFFLOAT,  A_DEFFLOAT, A_NULL);


    pdp_3dp_base_setup(pdp_3d_color_class);

    class_addmethod(pdp_3d_color_class, (t_method)pdp_3d_color_set_r, gensym("r"),  A_FLOAT, A_NULL);  
    class_addmethod(pdp_3d_color_class, (t_method)pdp_3d_color_set_g, gensym("g"),  A_FLOAT, A_NULL);  
    class_addmethod(pdp_3d_color_class, (t_method)pdp_3d_color_set_b, gensym("b"),  A_FLOAT, A_NULL);  
    class_addmethod(pdp_3d_color_class, (t_method)pdp_3d_color_set_a, gensym("a"),  A_FLOAT, A_NULL);  


}

#ifdef __cplusplus
}
#endif
