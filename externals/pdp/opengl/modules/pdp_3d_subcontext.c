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

typedef struct pdp_3d_subcontext_struct
{
    t_pdp_3dp_base x_base;
    int x_width;
    int x_height;

} t_pdp_3d_subcontext;



static void pdp_3d_subcontext_process_right(t_pdp_3d_subcontext *x)
{
    int p = pdp_3dp_base_get_context_packet(x);
    if (-1 != p){

	/* set subdims */
	pdp_packet_3Dcontext_set_subwidth(p, x->x_width);
	pdp_packet_3Dcontext_set_subheight(p, x->x_height);

	/* reinit everything */
	pdp_packet_3Dcontext_set_rendering_context(p);
	pdp_packet_3Dcontext_setup_3d_context(p);

    }
}
static void pdp_3d_subcontext_process_left(t_pdp_3d_subcontext *x)
{
    int p = pdp_3dp_base_get_context_packet(x);
    if (-1 != p){
      
	/* restore subdims */
	pdp_packet_3Dcontext_set_subwidth(p, pdp_packet_3Dcontext_width(p));
	pdp_packet_3Dcontext_set_subheight(p, pdp_packet_3Dcontext_height(p));

	/* re-init everything */
	pdp_packet_3Dcontext_set_rendering_context(p);
	pdp_packet_3Dcontext_setup_3d_context(p);

    }

}

t_class *pdp_3d_subcontext_class;



void pdp_3d_subcontext_free(t_pdp_3d_subcontext *x)
{
    pdp_3dp_base_free(x);
}

void *pdp_3d_subcontext_new(t_floatarg w, t_floatarg h)
{
    t_pdp_3d_subcontext *x = (t_pdp_3d_subcontext *)pd_new(pdp_3d_subcontext_class);

    /* super init */
    pdp_3dp_base_init(x);


    /* create dpd outlets */
    pdp_3dp_base_add_outlet(x, (t_pdp_method)pdp_3d_subcontext_process_left, 0);
    pdp_3dp_base_add_outlet(x, (t_pdp_method)pdp_3d_subcontext_process_right, 0);

    x->x_width = (w < 0) ? 64 : w;
    x->x_height = (h < 0) ? 64 : h;

    return (void *)x;
}


#ifdef __cplusplus
extern "C"
{
#endif


void pdp_3d_subcontext_setup(void)
{


    pdp_3d_subcontext_class = class_new(gensym("3dp_subcontext"), (t_newmethod)pdp_3d_subcontext_new,
    	(t_method)pdp_3d_subcontext_free, sizeof(t_pdp_3d_subcontext), 0, A_FLOAT, A_FLOAT, A_NULL);

    pdp_3dp_base_setup(pdp_3d_subcontext_class);

}

#ifdef __cplusplus
}
#endif
