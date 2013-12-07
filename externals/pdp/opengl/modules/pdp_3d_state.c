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


/* change binary opengl state variables. all defaults (flag = 0) should be set
   in the render context init.

   right outlet has the thing enabled (or disabled, depending on toggle)
   left outlet has the thing disabled (better: it should push it)

   simple version: does not permit reentry (yet) */


#include <GL/gl.h>
#include "pdp_opengl.h"
#include "pdp_3dp_base.h"

typedef struct pdp_3d_state_struct
{
    t_pdp_3dp_base x_base;
    GLboolean x_flag;
    GLboolean x_prev_flag;
    GLenum x_thing;
    void (*x_setup)(void);

} t_pdp_3d_state;


static void _setflag(GLenum thing, GLboolean flag)
{
    if (flag) glEnable(thing);
    else glDisable(thing);
}

static void pdp_3d_state_process_right(t_pdp_3d_state *x)
{
    int p;
    if (-1 != (p = pdp_3dp_base_get_context_packet(x))){
	/* store previous flag */
	pdp_packet_3Dcontext_set_rendering_context(p);
	glGetBooleanv(x->x_thing, &x->x_prev_flag);
	_setflag(x->x_thing, x->x_flag);
	if (x->x_setup) x->x_setup();
    }
}

static void pdp_3d_state_process_left(t_pdp_3d_state *x)
{
    int p;
    /* allways run left method (reset) */
    if (-1 != (p = pdp_3dp_base_get_context_packet(x))){
	pdp_packet_3Dcontext_set_rendering_context(p);
	_setflag(x->x_thing, x->x_prev_flag);
    }
}

static void pdp_3d_state_flag(t_pdp_3d_state *x, t_floatarg f)
{
    x->x_flag = (f == 0.0f) ? GL_FALSE : GL_TRUE;
}

static void _blend(void)     {glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);}
static void _blend_add(void) {glBlendFunc(GL_SRC_ALPHA, GL_ONE);}

t_class *pdp_3d_state_class;
void pdp_3d_state_free(t_pdp_3d_state *x){pdp_3dp_base_free(x);}
void *pdp_3d_state_new(t_symbol *s, t_floatarg f)
{
    t_pdp_3d_state *x = (t_pdp_3d_state *)pd_new(pdp_3d_state_class);

    /* super init */
    pdp_3dp_base_init(x);
    pdp_3d_state_flag(x,f);

    if      (s == gensym("blend_mix"))  {x->x_setup = _blend;     x->x_thing = GL_BLEND;}
    else if (s == gensym("blend_add"))  {x->x_setup = _blend_add; x->x_thing = GL_BLEND;}
    else if (s == gensym("depth_test")) {x->x_setup = 0;          x->x_thing = GL_DEPTH_TEST;}

    /* unkown command: do nothing */
    else {
	post ("3dp_state: unknown flag %s", s->s_name);
	pd_free((void *)x);
	return 0;
    }

    /* create additional inlet */
    pdp_base_add_gen_inlet(x, gensym("float"), gensym("flag"));

    /* create dpd outlets */
    pdp_3dp_base_add_outlet(x, (t_pdp_method)pdp_3d_state_process_left, 0);
    pdp_3dp_base_add_outlet(x, (t_pdp_method)pdp_3d_state_process_right, 0);


    return (void *)x;
}


#ifdef __cplusplus
extern "C"
{
#endif


void pdp_3d_state_setup(void)
{


    pdp_3d_state_class = class_new(gensym("3dp_toggle"), (t_newmethod)pdp_3d_state_new,
    	(t_method)pdp_3d_state_free, sizeof(t_pdp_3d_state), 0, A_SYMBOL, A_DEFFLOAT, A_NULL);

    pdp_3dp_base_setup(pdp_3d_state_class);
    class_addmethod(pdp_3d_state_class, (t_method)pdp_3d_state_flag, gensym("flag"), A_FLOAT, A_NULL);

}

#ifdef __cplusplus
}
#endif
