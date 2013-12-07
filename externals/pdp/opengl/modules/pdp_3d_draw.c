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


//#include "GL/gl.h"
//#include <GL/glut.h>
#include <GL/glu.h>
#include <math.h>

#include "pdp_opengl.h"
#include "pdp_3dp_base.h"

typedef struct _drawcommand
{
    t_pdp_dpd_command x_head;
    int x_context_packet;
    int x_texture_packet;
    float x_p0;
    float x_p1;
    float x_p2;
    float x_p3;
    t_pdp_method x_method;
    GLUquadric* x_quadric; 
    int x_have_texture; /* is there a valid texture ? */
    
} t_drawcommand;


typedef struct _pdp_3d_draw
{
    t_pdp_3dp_base x_base;
    t_pdp_dpd_commandfactory x_clist;

    int x_inlets;
    float x_p0;
    float x_p1;
    float x_p2;
    float x_p3;

    t_pdp_method x_method;

    int x_tex_in;       /* the number of texture inlets */
    GLUquadric* x_quadric; 
} t_pdp_3d_draw;


void pdp_3d_draw_delete_texture(t_pdp_3d_draw *x)
{
    pdp_base_move_packet(x, 1);
}

/* return a new command object */
void *pdp_3d_draw_get_command_object(t_pdp_3d_draw *x)
{
    t_drawcommand *c = (t_drawcommand *)pdp_dpd_commandfactory_get_new_command(&x->x_clist);
    c->x_p0 = x->x_p0;
    c->x_p1 = x->x_p1;
    c->x_p2 = x->x_p2;
    c->x_p3 = x->x_p3;
    c->x_context_packet = pdp_3dp_base_get_context_packet(x);
    c->x_texture_packet = pdp_packet_copy_ro(pdp_base_get_packet(x, 1));

    c->x_quadric = x->x_quadric; /* $$$TODO: this assumes quadric doesn't change */

    c->x_method = x->x_method;
    //post("o: %x, vc %x, n %d, u %d", x, c, x->x_clist.nb_commands, c->x_head.used);
    return c;
}

/* object drawing methods */

static void draw_clear(t_drawcommand *x)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

}

static void draw_square(t_drawcommand *x)
{
    float f = x->x_p0 * 0.5f;
    float z = x->x_p1 * 0.5f;
    /* draw a square */
    glBegin(GL_QUADS);
        glNormal3f(0.0f, 0.0f, 1.0f);
        glTexCoord2f(1, 0);
        glVertex3f(f,-f, z);
	glTexCoord2f(1, 1);
	glVertex3f(f, f, z);
	glTexCoord2f(0, 1);
	glVertex3f(-f, f, z);
	glTexCoord2f(0, 0);
	glVertex3f(-f,-f, z);
    glEnd();
}

static void draw_wsquare(t_drawcommand *x)
{
    float f = x->x_p0;
    float z = x->x_p1;
    /* draw a square */
    glBegin(GL_LINE_LOOP);
        glVertex3f(f,-f, z);
	glVertex3f(f, f, z);
	glVertex3f(-f, f, z);
	glVertex3f(-f,-f, z);
    glEnd();
}

static void draw_triangle(t_drawcommand *x)
{
    float f = x->x_p0 * 0.5f;
    float f2 = f * 0.5f;
    float f3 = f * (sqrt(3.0f) / 2.0f);
    float z = x->x_p1 * 0.5f;
    /* draw a triangle */
    glBegin(GL_TRIANGLES);
        glNormal3f(0.0f, 0.0f, 1.0f);

        glTexCoord2f(0.5f, 1.0f);
        glVertex3f(0, f, z);

	glTexCoord2f(0.5f * (1.0f - sqrt(3.0f)/2.0f), 0.25f);
	glVertex3f(-f3, -f2, z);

	glTexCoord2f(0.5f * (1.0f + sqrt(3.0f)/2.0f), 0.25f);
	glVertex3f(f3, -f2, z);
    glEnd();
}

static void draw_wtriangle(t_drawcommand *x)
{
    float f = x->x_p0 * 0.5f;
    float f2 = f * 0.5f;
    float f3 = f * (sqrt(3.0f) / 2.0f);
    float z = x->x_p1 * 0.5f;

    /* draw a wire triangle */
    glBegin(GL_LINE_LOOP);
        glNormal3f(0.0f, 0.0f, 1.0f);
        glVertex3f(0, f, z);
	glVertex3f(-f3, -f2, z);
	glVertex3f(f3, -f2, z);
    glEnd();
}


static void draw_wcube(t_drawcommand *x) 
{
    x->x_p1 = x->x_p0; // set square z coord;
    glPushMatrix();
        draw_wsquare(x);
	glRotatef(90, 0,1,0);
	draw_wsquare(x);
	glRotatef(90, 0,1,0);
	draw_wsquare(x);
	glRotatef(90, 0,1,0);
	draw_wsquare(x);
    glPopMatrix();

}

static void draw_cube(t_drawcommand *x) 
{
    x->x_p1 = x->x_p0; // set square z coord;

    glPushMatrix();
        draw_square(x);
	glRotatef(90, 0,1,0);
	draw_square(x);
	glRotatef(90, 0,1,0);
	draw_square(x);
	glRotatef(90, 0,1,0);
	draw_square(x);
    glPopMatrix();

    glPushMatrix();
        glRotatef(90, 1, 0, 0);
	draw_square(x);
	glRotatef(180, 1, 0, 0);
	draw_square(x);
    glPopMatrix();

}

static void draw_wtorus(t_drawcommand *x)
{
    float ri = x->x_p0;
    float ro = x->x_p1;
    int n = (int)x->x_p2;
    int m = (int)x->x_p3;

    if (n < 1) n = 20;
    if (m < 1) m = n;

    // glutWireTorus(ri, ro, n, m);

}

static void draw_torus(t_drawcommand *x)
{
    float ri = x->x_p0;
    float ro = x->x_p1;
    int n = (int)x->x_p2;
    int m = (int)x->x_p3;

    if (n < 1) n = 20;
    if (m < 1) m = n;

    // glutSolidTorus(ri, ro, n, m);

}

static void draw_cone(t_drawcommand *x)
{
    float base = x->x_p0;
    float height = x->x_p1;
    int n = (int)x->x_p2;
    int m = (int)x->x_p3;

    if (n < 1) n = 20;
    if (m < 1) m = n;

    // glutSolidCone(base, height, n, m);

}

static void draw_wcone(t_drawcommand *x)
{
    float base = x->x_p0;
    float height = x->x_p1;
    int n = (int)x->x_p2;
    int m = (int)x->x_p3;

    if (n < 1) n = 20;
    if (m < 1) m = n;

    // glutWireCone(base, height, n, m);

}

static void draw_wteapot(t_drawcommand *x)
{
    float f = x->x_p0;
    // glutWireTeapot(f);

}

static void draw_teapot(t_drawcommand *x)
{
    float f = x->x_p0;
    // glutSolidTeapot(f);

}

static void draw_wsphere(t_drawcommand *x)
{
    float f = x->x_p0;
    int n = (int)x->x_p1;
    int m = (int)x->x_p2;

    if (n < 1) n = 20;
    if (m < 1) m = n;

    // glutWireSphere(f, n, m);

}

static void draw_sphere(t_drawcommand *x)
{
    float f = x->x_p0;
    int n = (int)x->x_p1;
    int m = (int)x->x_p2;

    if (n < 1) n = 20;
    if (m < 1) m = n;

    gluSphere(x->x_quadric, f, n, m);

    //glutSolidSphere(f, n, m);

}

static void draw_dodeca(t_drawcommand *x){ /* glutSolidDodecahedron(); */ }
static void draw_octa(t_drawcommand *x)  { /* glutSolidOctahedron(); */ }
static void draw_tetra(t_drawcommand *x) { /* glutSolidTetrahedron(); */ }
static void draw_icosa(t_drawcommand *x) { /* glutSolidIcosahedron(); */ } 

static void draw_wdodeca(t_drawcommand *x){ /* glutWireDodecahedron(); */ }
static void draw_wocta(t_drawcommand *x)  { /* glutWireOctahedron(); */ }
static void draw_wtetra(t_drawcommand *x) { /* glutWireTetrahedron(); */ }
static void draw_wicosa(t_drawcommand *x) { /* glutWireIcosahedron(); */ }






/* the actual (registered) draw method */
/* when this is finished, the drawcommand object should commit suicide */

static void draw_process(t_drawcommand *x)
{
    int p = x->x_context_packet;
    int pt = x->x_texture_packet;
    float fx=1;
    float fy=1;
    x->x_have_texture = pdp_packet_texture_isvalid(pt);

    //post("pdp_3d_draw: context = %d, texture = %d", p, pt);
 
    /* check if it's a valid buffer we can draw in */
    if (pdp_packet_3Dcontext_isvalid(p)){ 
	

	/* setup rendering context */
	pdp_packet_3Dcontext_set_rendering_context(p);

	/* enable texture */
	if (x->x_have_texture){
	    fx = pdp_packet_texture_fracx(pt);
	    fy = pdp_packet_texture_fracy(pt);
	    glEnable(GL_TEXTURE_2D);
	    pdp_packet_texture_make_current(pt);
	    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
	    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
	    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);	   

	    /* scale texture matrix to reflect subtexture's coords */
	    glMatrixMode(GL_TEXTURE);
	    //glLoadIdentity();
	    glPushMatrix();
	    glScalef(fx, fy, 1);
	    glMatrixMode(GL_MODELVIEW);

	    gluQuadricTexture(x->x_quadric, 1);
	}

	/* call the generating method */
	if (x->x_method) (*x->x_method)(x);

	/* disable texture */
	if (x->x_have_texture){
	    glMatrixMode(GL_TEXTURE);
	    glPopMatrix();
	    glMatrixMode(GL_MODELVIEW);
	    glDisable(GL_TEXTURE_2D);
	    gluQuadricTexture(x->x_quadric, 0);
	}

    }

    /* you know the drill: command done, sword in belly. */
    pdp_packet_mark_unused(x->x_texture_packet);
    pdp_dpd_command_suicide(x);

}

static void pdp_3d_draw_p0(t_pdp_3d_draw *x, t_floatarg f){x->x_p0 = f;}
static void pdp_3d_draw_p1(t_pdp_3d_draw *x, t_floatarg f){x->x_p1 = f;}
static void pdp_3d_draw_p2(t_pdp_3d_draw *x, t_floatarg f){x->x_p2 = f;}
static void pdp_3d_draw_p3(t_pdp_3d_draw *x, t_floatarg f){x->x_p3 = f;}


t_class *pdp_3d_draw_class;



void pdp_3d_draw_free(t_pdp_3d_draw *x)
{
    pdp_3dp_base_free(x);
    gluDeleteQuadric(x->x_quadric);
    pdp_dpd_commandfactory_free(&x->x_clist);
}

void pdp_3d_draw_object(t_pdp_3d_draw *x, t_symbol *s)
{
    /* find out if it is a buffer operation */
    if      (s == gensym("clear"))   {x->x_method = (t_pdp_method)draw_clear;   x->x_inlets = 0;}

    /* if not, find out which object we need to draw */
    else if (s == gensym("triangle"))  {x->x_method = (t_pdp_method)draw_triangle;  x->x_inlets = 1;}
    else if (s == gensym("wtriangle")) {x->x_method = (t_pdp_method)draw_wtriangle; x->x_inlets = 1;}
    else if (s == gensym("square"))    {x->x_method = (t_pdp_method)draw_square;  x->x_inlets = 1;}
    else if (s == gensym("wsquare"))   {x->x_method = (t_pdp_method)draw_wsquare; x->x_inlets = 1;}
    else if (s == gensym("cube"))      {x->x_method = (t_pdp_method)draw_cube;    x->x_inlets = 1;}
    else if (s == gensym("wcube"))     {x->x_method = (t_pdp_method)draw_wcube;   x->x_inlets = 1;}
    else if (s == gensym("sphere"))    {x->x_method = (t_pdp_method)draw_sphere;  x->x_inlets = 3;}
    else if (s == gensym("wsphere"))   {x->x_method = (t_pdp_method)draw_wsphere; x->x_inlets = 3;}
    else if (s == gensym("torus"))     {x->x_method = (t_pdp_method)draw_torus;   x->x_inlets = 4;}
    else if (s == gensym("wtorus"))    {x->x_method = (t_pdp_method)draw_wtorus;  x->x_inlets = 4;}
    else if (s == gensym("cone"))      {x->x_method = (t_pdp_method)draw_cone;    x->x_inlets = 4;}
    else if (s == gensym("wcone"))     {x->x_method = (t_pdp_method)draw_wcone;   x->x_inlets = 4;}
    else if (s == gensym("teapot"))    {x->x_method = (t_pdp_method)draw_teapot;  x->x_inlets = 1;}
    else if (s == gensym("wteapot"))   {x->x_method = (t_pdp_method)draw_wteapot; x->x_inlets = 1;}

    else if (s == gensym("dodeca"))  {x->x_method = (t_pdp_method)draw_dodeca;  x->x_inlets = 0;}
    else if (s == gensym("icosa"))   {x->x_method = (t_pdp_method)draw_icosa;   x->x_inlets = 0;}
    else if (s == gensym("octa"))    {x->x_method = (t_pdp_method)draw_octa;    x->x_inlets = 0;}
    else if (s == gensym("tetra"))   {x->x_method = (t_pdp_method)draw_tetra;   x->x_inlets = 0;}
    else if (s == gensym("wdodeca")) {x->x_method = (t_pdp_method)draw_wdodeca;  x->x_inlets = 0;}
    else if (s == gensym("wicosa"))  {x->x_method = (t_pdp_method)draw_wicosa;   x->x_inlets = 0;}
    else if (s == gensym("wocta"))   {x->x_method = (t_pdp_method)draw_wocta;    x->x_inlets = 0;}
    else if (s == gensym("wtetra"))  {x->x_method = (t_pdp_method)draw_wtetra;   x->x_inlets = 0;}

    else {
	post("pdp_3d_draw: object %s not found", s->s_name);
	x->x_method = 0;
	x->x_inlets = 0;
    }

    // the number of texture inlets
    x->x_tex_in = 1;
}


void *pdp_3d_draw_new(t_symbol *s, t_floatarg p0, t_floatarg p1, t_floatarg p2, t_floatarg p3)
{
    t_pdp_3d_draw *x = (t_pdp_3d_draw *)pd_new(pdp_3d_draw_class);
    char param[] = "p0";
    int i;

    /* super init */
    pdp_3dp_base_init(x);

    x->x_p0 = p0;
    x->x_p1 = p1;
    x->x_p2 = p2;
    x->x_p3 = p3;

    /* set the object & number of inlets */
    pdp_3d_draw_object(x, s);

    /* create texture inlets */
    for(i=0; i<x->x_tex_in; i++){
	pdp_base_add_pdp_inlet(x);
    }

    /* create additional inlets */
    for(i=0; i<x->x_inlets; i++){
	pdp_base_add_gen_inlet(x, gensym("float"), gensym(param));
	param[1]++;
    }

    /* create dpd outlet */
    pdp_3dp_base_add_outlet(x, (t_pdp_method)draw_process, 0);

    /* setup quadric */
    x->x_quadric = gluNewQuadric();

    /* init command list */
    pdp_dpd_commandfactory_init(&x->x_clist, sizeof(t_drawcommand));

    /* register command factory method */
    pdp_dpd_base_register_command_factory_method(x, (t_pdp_newmethod)pdp_3d_draw_get_command_object);
       



    return (void *)x;
}


#ifdef __cplusplus
extern "C"
{
#endif


void pdp_3d_draw_setup(void)
{


    pdp_3d_draw_class = class_new(gensym("3dp_draw"), (t_newmethod)pdp_3d_draw_new,
    	(t_method)pdp_3d_draw_free, sizeof(t_pdp_3d_draw), 0, A_SYMBOL, 
				  A_DEFFLOAT, A_DEFFLOAT, A_DEFFLOAT, A_DEFFLOAT, A_NULL);
    pdp_3dp_base_setup(pdp_3d_draw_class);

    class_addmethod(pdp_3d_draw_class, (t_method)pdp_3d_draw_p0, gensym("p0"),  A_DEFFLOAT, A_NULL);  
    class_addmethod(pdp_3d_draw_class, (t_method)pdp_3d_draw_p1, gensym("p1"),  A_DEFFLOAT, A_NULL);  
    class_addmethod(pdp_3d_draw_class, (t_method)pdp_3d_draw_p2, gensym("p2"),  A_DEFFLOAT, A_NULL);  
    class_addmethod(pdp_3d_draw_class, (t_method)pdp_3d_draw_p3, gensym("p3"),  A_DEFFLOAT, A_NULL);  

    class_addmethod(pdp_3d_draw_class, (t_method)pdp_3d_draw_delete_texture, gensym("delete_texture"),  A_DEFFLOAT, A_NULL);  

}

#ifdef __cplusplus
}
#endif
