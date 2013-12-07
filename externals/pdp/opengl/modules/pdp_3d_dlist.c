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

/* gl display list compilation & execution */

typedef struct pdp_3d_dlist_struct
{
    t_pdp_3dp_base x_base;

    GLuint x_dlist;
    int x_compile;

} t_pdp_3d_dlist;



static void pdp_3d_dlist_complete_notify(t_pdp_3d_dlist *x)
{
    /* disable the second outlet */
    pdp_3dp_base_enable_outlet(x, 0, 0);
}

static void pdp_3d_dlist_compile(t_pdp_3d_dlist *x)
{
    //x->x_compile = 1;
    /* enable the second outlet */
    pdp_3dp_base_enable_outlet(x, 0, 1);
}

static void pdp_3d_dlist_process_start(t_pdp_3d_dlist *x)
{
    int p = pdp_3dp_base_get_context_packet(x);
    if (-1 != p){

	/* check if pbuf */
	if (pdp_packet_3Dcontext_isvalid(p)){

	    /* set context */
	    //pdp_pbuf_set_rendering_context(p);

	    /* display list needs to be created in the correct context
	       if we don't have one yet, create it */
	    if (!x->x_dlist) x->x_dlist = glGenLists(1);

	    

	    /* start the list */ /* $$$TODO: error checking for recursion */
	    x->x_compile = 1;
	    glNewList(x->x_dlist, GL_COMPILE_AND_EXECUTE);
	    //glNewList(x->x_dlist, GL_COMPILE);

	    //post("compiling");

	    
	}
    }
}

static void pdp_3d_dlist_process_cleanup(t_pdp_3d_dlist *x)
{
    int p = pdp_3dp_base_get_context_packet(x);
    if (-1 != p){

	/* check if pbuf */
	if (pdp_packet_3Dcontext_isvalid(p)){

	    /* end list if we're compiling */
	    if (x->x_compile){
		
		/* end the list */
		glEndList();
		
		/* use the list next time */
		x->x_compile = 0;

		//post("ending compile");

	    }

	    /* or execute the old one */
	    else {
		if (x->x_dlist) {
		    //post("calling dlist %d", x->x_dlist);
		    glCallList(x->x_dlist);
		}
		
	    }


	}
    }
}




t_class *pdp_3d_dlist_class;



void pdp_3d_dlist_free(t_pdp_3d_dlist *x)
{
    pdp_3dp_base_free(x);
    if (x->x_dlist) glDeleteLists(x->x_dlist, 1);
}

void *pdp_3d_dlist_new(t_symbol *s)
{
    t_pdp_3d_dlist *x = (t_pdp_3d_dlist *)pd_new(pdp_3d_dlist_class);

    /* super init */
    pdp_3dp_base_init(x);


    /* io & callbacks */
    pdp_3dp_base_add_outlet(x, (t_pdp_method)pdp_3d_dlist_process_start, 0);
    pdp_3dp_base_add_cleanup(x, (t_pdp_method)pdp_3d_dlist_process_cleanup, 0);
    pdp_3dp_base_register_complete_notify(x, (t_pdp_method)pdp_3d_dlist_complete_notify);

    /* disable the second outlet */
    pdp_3dp_base_enable_outlet(x, 1, 0);


    /* create dlist */
    x->x_dlist = 0;
    x->x_compile = 0;

    /* compile the first packet */
    pdp_3d_dlist_compile(x);

    return (void *)x;
}


#ifdef __cplusplus
extern "C"
{
#endif


void pdp_3d_dlist_setup(void)
{


    pdp_3d_dlist_class = class_new(gensym("pdp_3d_dlist"), (t_newmethod)pdp_3d_dlist_new,
    	(t_method)pdp_3d_dlist_free, sizeof(t_pdp_3d_dlist), 0, A_DEFSYMBOL, A_NULL);

    class_addcreator((t_newmethod)pdp_3d_dlist_new, gensym("3dp_dlist"), A_DEFSYMBOL, A_NULL);

    pdp_3dp_base_setup(pdp_3d_dlist_class);

    class_addmethod(pdp_3d_dlist_class, (t_method)pdp_3d_dlist_compile, gensym("compile"), A_NULL);



}

#ifdef __cplusplus
}
#endif
