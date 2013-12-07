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

/* a very naive approach to triangular meshes */


// $$TODO: some serious memory corruption in this file our the list implementation


#include "GL/gl.h"
#include <math.h>
//#include <GL/glut.h>

#include "pdp_opengl.h"
#include "pdp_3dp_base.h"
#include "pdp_mesh.h"


/* PD OBJECT */

typedef struct _pdp_3d_drawmesh
{
    t_pdp_3dp_base x_base;
    t_pdp_dpd_commandfactory x_clist;

    t_mesh *x_mesh; 
    int x_wireframe;
    int x_flatshading;

} t_pdp_3d_drawmesh;


/* MESHCOMMAND OBJECT */

typedef struct _meshcommand
{
    t_pdp_dpd_command x_head;
    int x_context_packet;
    int x_texture_packet;
    t_pdp_3d_drawmesh *x_mother;
    t_pdp_method x_method;

    int x_wireframe;
    int x_flatshading;
    float x_step;
    float x_d0;
    float x_r0;
    int x_normal_type;
    
} t_meshcommand;


/* MESHCOMMAND METHODS */

/* draw the mesh */
static void meshcommand_draw(t_meshcommand *x)
{
    int i = 0;
    t_pdp_atom *it;
    t_pdp_list *tl = x->x_mother->x_mesh->triangles;
    t_triangle *t;
    GLenum mode = (x->x_wireframe) ? GL_LINE_LOOP : GL_TRIANGLES;

    //glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_FALSE);

    glLineWidth(5);

    glBegin(mode);

    if (x->x_flatshading){
	PDP_POINTER_IN(tl, it, t){
	    glNormal3fv(t->n);
	    for (i=0; i<3; i++){
		glVertex3fv(t->v[i]->c);
	    }
	}
    }
    else{
	PDP_POINTER_IN(tl, it, t){
	    for (i=0; i<3; i++){
		glNormal3fv(t->v[i]->n);
		glVertex3fv(t->v[i]->c);
	    }
	}
    }
    glEnd();
}

static void meshcommand_relax(t_meshcommand *x)
{
    mesh_relax(x->x_mother->x_mesh, x->x_step, x->x_d0, x->x_r0);
}


/* the main subcommand dispatcher */
static void meshcommand_execute(t_meshcommand *x)
{
    int p = x->x_context_packet;
 
    /* check if it's a valid buffer we can draw in */
    if (pdp_packet_3Dcontext_isvalid(p)){ 
	

	/* setup rendering context */
	pdp_packet_3Dcontext_set_rendering_context(p);

	/* call the command method */
	if (x->x_method) (x->x_method)(x);

    }

    /* you know the drill: command done, sword in belly. */
    pdp_dpd_command_suicide(x);
}

static void meshcommand_split_all_four(t_meshcommand *x)
{
    mesh_split_all_four(x->x_mother->x_mesh);
}
static void meshcommand_split_all_three(t_meshcommand *x){
    mesh_split_all_three(x->x_mother->x_mesh);
}
static void meshcommand_split_random_three(t_meshcommand *x){
    mesh_split_random_three(x->x_mother->x_mesh);
}


static void meshcommand_reset(t_meshcommand *x)
{
    mesh_free(x->x_mother->x_mesh);
    x->x_mother->x_mesh = mesh_new_tetra();
}

static void meshcommand_debug(t_meshcommand *x)
{
    mesh_debug(x->x_mother->x_mesh);
}

static void meshcommand_calculate_normals(t_meshcommand *x)
{
    x->x_mother->x_mesh->normal_type = x->x_normal_type;
    mesh_calculate_normals(x->x_mother->x_mesh);
}




/* PD OBJECT METHODS */


/* return a new command object */
void *pdp_3d_drawmesh_get_command_object(t_pdp_3d_drawmesh *x)
{
    t_meshcommand *c = (t_meshcommand *)pdp_dpd_commandfactory_get_new_command(&x->x_clist);
    c->x_context_packet = pdp_3dp_base_get_context_packet(x);
    c->x_mother = x;
    c->x_method = (t_pdp_method)meshcommand_draw; //default command is draw
    c->x_wireframe = x->x_wireframe;
    c->x_flatshading = x->x_flatshading;

    return c;
}

/* schedule a command */
static void pdp_3d_drawmesh_queue_command(t_pdp_3d_drawmesh *x, t_meshcommand *c)
{
    pdp_3dp_base_queue_command(x, c, (t_pdp_method)meshcommand_execute, 0, 0);
}

static void pdp_3d_drawmesh_queue_simple_command(t_pdp_3d_drawmesh *x, t_pdp_method method)
{
    t_meshcommand *c = (t_meshcommand *)pdp_3d_drawmesh_get_command_object(x);
    c->x_method = method;
    pdp_3dp_base_queue_command(x, c, (t_pdp_method)meshcommand_execute, 0, 0);
}

//NOTE: only the meshcommands are entitled to use the mesh (thread issues)
//therefore all mesh manipulations must be queued as a command


static void pdp_3d_drawmesh_debug(t_pdp_3d_drawmesh *x)
{
    pdp_3d_drawmesh_queue_simple_command(x, (t_pdp_method)meshcommand_debug);
}

static void pdp_3d_drawmesh_relax(t_pdp_3d_drawmesh *x, t_floatarg step, 
				  t_floatarg d0, t_floatarg r0)
{
    t_meshcommand *c = (t_meshcommand *)pdp_3d_drawmesh_get_command_object(x);
    c->x_step = step;
    c->x_d0 = d0;
    c->x_r0 = r0;
    c->x_method = (t_pdp_method)meshcommand_relax;
    pdp_3d_drawmesh_queue_command(x, c);

}

void pdp_3d_drawmesh_normal(t_pdp_3d_drawmesh *x, t_symbol *s)
{
    t_meshcommand *c = (t_meshcommand *)pdp_3d_drawmesh_get_command_object(x);
    if (gensym("sphere") == s)       c->x_normal_type = MESH_NORMAL_SPHERE;
    else if (gensym("prism") == s)   c->x_normal_type = MESH_NORMAL_PRISM;
    else if (gensym("random") == s)  c->x_normal_type = MESH_NORMAL_RANDOM;
    else if (gensym("average") == s) c->x_normal_type = MESH_NORMAL_AVERAGE;
    c->x_method = (t_pdp_method)meshcommand_calculate_normals;
    pdp_3d_drawmesh_queue_command(x, c);

}

/* this is used by the standard drawing routine, so doesn't need to be scheduled */
void pdp_3d_drawmesh_wireframe(t_pdp_3d_drawmesh *x, t_float f)
{
    x->x_wireframe = (f != 0.0f);
}

void pdp_3d_drawmesh_flatshading(t_pdp_3d_drawmesh *x, t_float f)
{
    x->x_flatshading = (f != 0.0f);
}


static void pdp_3d_drawmesh_split_all_four(t_pdp_3d_drawmesh *x)
{
    pdp_3d_drawmesh_queue_simple_command(x, (t_pdp_method)meshcommand_split_all_four);
}

static void pdp_3d_drawmesh_split_all_three(t_pdp_3d_drawmesh *x)
{
    pdp_3d_drawmesh_queue_simple_command(x, (t_pdp_method)meshcommand_split_all_three);
}

static void pdp_3d_drawmesh_split_random_three(t_pdp_3d_drawmesh *x)
{
    pdp_3d_drawmesh_queue_simple_command(x, (t_pdp_method)meshcommand_split_random_three);
}


static void pdp_3d_drawmesh_reset(t_pdp_3d_drawmesh *x)
{
    pdp_3d_drawmesh_queue_simple_command(x, (t_pdp_method)meshcommand_reset);

}










t_class *pdp_3d_drawmesh_class;


void pdp_3d_drawmesh_free(t_pdp_3d_drawmesh *x)
{
    /* queue needs to finish before mesh is deleted */
    pdp_3dp_base_queue_wait(x);
    mesh_free(x->x_mesh);

    pdp_3dp_base_free(x);
    pdp_dpd_commandfactory_free(&x->x_clist);
}

void *pdp_3d_drawmesh_new(t_symbol *s, t_floatarg p0, t_floatarg p1, t_floatarg p2, t_floatarg p3)
{
    t_pdp_3d_drawmesh *x = (t_pdp_3d_drawmesh *)pd_new(pdp_3d_drawmesh_class);

    /* super init */
    pdp_3dp_base_init(x);

    /* create dpd outlet */
    pdp_3dp_base_add_outlet(x, (t_pdp_method)meshcommand_execute, 0);

    /* init command list */
    pdp_dpd_commandfactory_init(&x->x_clist, sizeof(t_meshcommand));

    /* register command factory method */
    pdp_dpd_base_register_command_factory_method(x, (t_pdp_newmethod)pdp_3d_drawmesh_get_command_object);
       

    /* initialize triangular mesh with a simply connected manifold */
    x->x_mesh = mesh_new_tetra();

    x->x_wireframe = 0;
    x->x_flatshading = 0;

    return (void *)x;
}


#ifdef __cplusplus
extern "C"
{
#endif


void pdp_3d_drawmesh_setup(void)
{

    pdp_3d_drawmesh_class = class_new(gensym("3dp_drawmesh"), (t_newmethod)pdp_3d_drawmesh_new,
    	(t_method)pdp_3d_drawmesh_free, sizeof(t_pdp_3d_drawmesh), 0, A_DEFSYMBOL, 
				  A_DEFFLOAT, A_DEFFLOAT, A_DEFFLOAT, A_DEFFLOAT, A_NULL);
    pdp_3dp_base_setup(pdp_3d_drawmesh_class);


    class_addmethod(pdp_3d_drawmesh_class, (t_method)pdp_3d_drawmesh_split_random_three, gensym("split3random"), A_NULL);
    class_addmethod(pdp_3d_drawmesh_class, (t_method)pdp_3d_drawmesh_split_all_three, gensym("split3"), A_NULL);
    class_addmethod(pdp_3d_drawmesh_class, (t_method)pdp_3d_drawmesh_split_all_four, gensym("split4"), A_NULL);
    class_addmethod(pdp_3d_drawmesh_class, (t_method)pdp_3d_drawmesh_reset, gensym("reset"), A_NULL);
    class_addmethod(pdp_3d_drawmesh_class, (t_method)pdp_3d_drawmesh_normal, gensym("normal"), A_SYMBOL, A_NULL);
    class_addmethod(pdp_3d_drawmesh_class, (t_method)pdp_3d_drawmesh_relax, gensym("springrelax"),
		    A_FLOAT, A_FLOAT, A_FLOAT, A_NULL);
    class_addmethod(pdp_3d_drawmesh_class, (t_method)pdp_3d_drawmesh_debug, gensym("info"), A_NULL);
    class_addmethod(pdp_3d_drawmesh_class, (t_method)pdp_3d_drawmesh_wireframe, gensym("wireframe"), A_FLOAT, A_NULL);
    class_addmethod(pdp_3d_drawmesh_class, (t_method)pdp_3d_drawmesh_flatshading, gensym("flatshading"), A_FLOAT, A_NULL);

}

#ifdef __cplusplus
}
#endif
