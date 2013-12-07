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

// gl stuff
#include <GL/gl.h>
#include <GL/glx.h>
#include <GL/glu.h>
//#include <GL/glut.h>

// pdp stuff
#include "pdp.h"
#include "pdp_base.h"

// some x window glue code
#include "pdp_xwindow.h"

// pdp stuff
#include "pdp.h"
#include "pdp_llconv.h"
//#include "pdp_opengl.h"


/* initial image dimensions */
#define PDP_OGL_W 320
#define PDP_OGL_H 240

#define PDP_OGL_AUTOCREATE_RETRY 10


typedef struct pdp_glx_struct
{
    t_object x_obj;

    t_pdp_xwindow *x_xwin;

    t_outlet *x_outlet;

    int x_packet0;
    int x_queue_id;
    t_symbol *x_display;

    t_pdp_xdisplay *x_xdpy;

    XVisualInfo *x_vis_info;
    GLXContext x_glx_context;

    GLuint x_texture;
    u32 x_tex_width;
    u32 x_tex_height;

    unsigned char *x_data;
    unsigned int x_width;
    unsigned int x_height;
    int x_last_encoding;

    int  x_initialized;
    int  x_autocreate;
    int  x_interpol;

} t_pdp_glx;



static void pdp_glx_cursor(t_pdp_glx *x, t_floatarg f)
{
    if (x->x_initialized)
	pdp_xwindow_cursor(x->x_xwin, f);
}

static void pdp_glx_destroy(t_pdp_glx* x)
{
    t_pdp_procqueue *q = pdp_queue_get_queue();
    XEvent e;

    if (x->x_initialized){
	pdp_procqueue_finish(q, x->x_queue_id);
	x->x_queue_id = -1;
	glXDestroyContext(x->x_xdpy->dpy, x->x_glx_context);
	pdp_xwindow_free(x->x_xwin);
	pdp_xdisplay_free(x->x_xdpy);
	x->x_xwin = 0;
	x->x_xdpy = 0;
	x->x_initialized = false;
    }

}


static void pdp_glx_fullscreen(t_pdp_glx *x)
{
    if (x->x_initialized)
	pdp_xwindow_fullscreen(x->x_xwin);
}

static void pdp_glx_resize(t_pdp_glx* x, t_floatarg width, t_floatarg height)
{
    if (x->x_initialized)
	pdp_xwindow_resize(x->x_xwin, width, height);
}

static void pdp_glx_move(t_pdp_glx* x, t_floatarg width, t_floatarg height)
{
    if (x->x_initialized)
	pdp_xwindow_move(x->x_xwin, width, height);
}

static void pdp_glx_moveresize(t_pdp_glx* x, t_floatarg xoff, t_floatarg yoff, t_floatarg width, t_floatarg height)
{
    if (x->x_initialized)
	pdp_xwindow_moveresize(x->x_xwin, xoff, yoff, width, height);
}

static void pdp_glx_tile(t_pdp_glx* x, t_floatarg xtiles, t_floatarg ytiles, t_floatarg i, t_floatarg j)
{
    if (x->x_initialized)
	pdp_xwindow_tile(x->x_xwin, xtiles, ytiles, i, j);
}




void pdp_glx_generate_texture(t_pdp_glx *x)
{
    u32 width = x->x_tex_width;
    u32 height = x->x_tex_height;
    u32 depth = 4;
    u32 i;

    u8 *dummydata = 0;

    while (x->x_width > width) width <<= 1;
    while (x->x_height > height) height <<= 1;

    dummydata = (u8 *)pdp_alloc(width*height*depth);

    for (i=0; i<width*height*depth; i++){dummydata[i] = random(); }

    /* set window context current */
    glXMakeCurrent(x->x_xdpy->dpy, x->x_xwin->win, x->x_glx_context);

    /* generate texture if necessary */
    if (!glIsTexture(x->x_texture)) glGenTextures(1, &(x->x_texture));

    glBindTexture(GL_TEXTURE_2D, x->x_texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, dummydata);

    pdp_dealloc(dummydata);

    x->x_tex_width = width;
    x->x_tex_height = height;
}

void pdp_glx_regenerate_texture(t_pdp_glx *x)
{
    if ((x->x_width > x->x_tex_width) || (x->x_height > x->x_tex_height)) pdp_glx_generate_texture(x);

}


static void pdp_glx_create(t_pdp_glx* x)
{
    unsigned int *uintdata = (unsigned int *)(x->x_data);
    XEvent e;
    unsigned int i;
    static int vis_attr[] = {GLX_RGBA, GLX_RED_SIZE, 4, GLX_GREEN_SIZE, 4, GLX_BLUE_SIZE, 4, 
			     GLX_DEPTH_SIZE, 16, GLX_DOUBLEBUFFER, None};


    if (x->x_initialized) return;

    /* manually open a display */
    if (NULL == (x->x_xdpy = pdp_xdisplay_new(x->x_display->s_name))){
	post("pdp_glx: cant open display %s\n",x->x_display->s_name);
	x->x_initialized = false;
	return;
    }

    /* create a window on the display */
    x->x_xwin = pdp_xwindow_new();
    if (!(x->x_initialized = pdp_xwindow_create_on_display(x->x_xwin, x->x_xdpy))) 
	goto exit_error;


    /* create a glx visual */
    if (!(x->x_vis_info = glXChooseVisual(x->x_xdpy->dpy, x->x_xdpy->screen, vis_attr))){
	post("pdp_glx: can't create visual");
	goto exit_error;
    }
    //post("visual: %x", x->x_vis_info);

    /* create the rendering context */
    if (!(x->x_glx_context = glXCreateContext(x->x_xdpy->dpy, x->x_vis_info,  0 /*share list*/, GL_TRUE))){
	post("pdp_glx: can't create render context");
	goto exit_error;
    }
    //post("context: %x", x->x_glx_context);


    /* create texture */
    pdp_glx_generate_texture(x);


    /* we're done initializing */
    x->x_initialized = true;

    /* disable/enable cursor */
    //pdp_glx_cursor(x, x->x_cursor);
    return;


 exit_error:
    if (x->x_xwin){
	pdp_xwindow_free(x->x_xwin);
	x->x_xwin = 0;
    }
    
    if (x->x_xdpy){
	pdp_xdisplay_free(x->x_xdpy);
	x->x_xdpy = 0;
    }
    
    x->x_initialized = false;
    return;
}

static int pdp_glx_try_autocreate(t_pdp_glx *x)
{

    if (x->x_autocreate){
	post("pdp_glx: autocreate window");
	pdp_glx_create(x);
	if (!(x->x_initialized)){
#ifdef __APPLE__
	    system("/usr/bin/open /Applications/Utilities/X11.app &");
#endif
	    x->x_autocreate--;
	    if (!x->x_autocreate){
		post ("pdp_glx: autocreate failed %d times: disabled", PDP_OGL_AUTOCREATE_RETRY);
		post ("pdp_glx: send [autocreate 1] message to re-enable");
		return 0;
	    }
	}
	else return 1;

    }
    return 0;
}

static void pdp_glx_bang(t_pdp_glx *x);

static void pdp_glx_fill_texture(t_pdp_glx *x)
{
    t_pdp *header = pdp_packet_header(x->x_packet0);
    void  *data   = pdp_packet_data  (x->x_packet0);

    int i = header->info.image.width;


    /* ensure image buffer is correct dim */
    if ((header->info.image.width != x->x_width) 
	|| (header->info.image.height != x->x_height)) {
	if (x->x_data) pdp_dealloc (x->x_data);
	x->x_width = header->info.image.width;
	x->x_height = header->info.image.height;
	x->x_data = pdp_alloc(4*x->x_width*x->x_height);
    }

    /* ensure texture is correct dim */
    pdp_glx_regenerate_texture(x);
    

    /* set window context current */
    glXMakeCurrent(x->x_xdpy->dpy, x->x_xwin->win, x->x_glx_context);
    glBindTexture(GL_TEXTURE_2D, x->x_texture);

    switch (header->info.image.encoding){
    case PDP_IMAGE_GREY:
	/* convert image to greyscale 8 bit */
	pdp_llconv(data,RIF_GREY______S16, x->x_data, RIF_GREY______U8, x->x_width, x->x_height);

	/* upload grey subtexture */
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, x->x_width, x->x_height, GL_LUMINANCE, GL_UNSIGNED_BYTE, x->x_data);
	    
	break;
    case PDP_IMAGE_YV12:

	/* convert image to rgb 8 bit */
	pdp_llconv(data,RIF_YVU__P411_S16, x->x_data,  RIF_RGB__P____U8, x->x_width, x->x_height);

	/* upload subtexture */
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, x->x_width, x->x_height, GL_RGB, GL_UNSIGNED_BYTE, x->x_data);
	
	break;
    default:
	break;
    }

    
}

static void pdp_glx_process(t_pdp_glx *x)
{
    t_pdp *header = pdp_packet_header(x->x_packet0);
    void  *data   = pdp_packet_data  (x->x_packet0);


    if (-1 != x->x_queue_id) return;

    /* check if window is initialized */
    if (!(x->x_initialized)){
        if (!pdp_glx_try_autocreate(x)) return;
    }

    /* check data packet */
    if (!(header)) {
	post("pdp_glx: invalid packet header");
	return;
    }
    if (PDP_IMAGE != header->type) {
	post("pdp_glx: packet is not a PDP_IMAGE");
	return;
    }
    if ((PDP_IMAGE_YV12 != header->info.image.encoding)
	&& (PDP_IMAGE_GREY != header->info.image.encoding)) {
	post("pdp_glx: packet is not a PDP_IMAGE_YV12/GREY");
	return;
    }

    
    /* fill the texture with the data in the packet */
    pdp_glx_fill_texture(x);

    /* display the new image */
    pdp_glx_bang(x);


}



static void pdp_glx_display_texture(t_pdp_glx *x)
{
    float fx = (float)x->x_width / x->x_tex_width;
    float fy = (float)x->x_height / x->x_tex_height;

    if (!x->x_initialized) return;

    /* set window context current */
    glXMakeCurrent(x->x_xdpy->dpy, x->x_xwin->win, x->x_glx_context);

    /* setup viewport, projection and modelview */
    glViewport(0, 0, x->x_xwin->winwidth, x->x_xwin->winheight);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0.0, x->x_xwin->winwidth, 0.0, x->x_xwin->winheight);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();


    /* enable default texture */
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, x->x_texture);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    if (x->x_interpol){
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    }
    else {
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
    }


    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

    /* display texture */  
    glBegin(GL_QUADS);
        glTexCoord2f(fx, fy);
        glVertex2i(x->x_xwin->winwidth,0);
        glTexCoord2f(fx, 0);
	glVertex2i(x->x_xwin->winwidth, x->x_xwin->winheight);
        glTexCoord2f(0.0, 0.0);
	glVertex2i(0, x->x_xwin->winheight);
        glTexCoord2f(0, fy);
	glVertex2i(0,0);
    glEnd();


    glFlush();
    glXSwapBuffers(x->x_xdpy->dpy,x->x_xwin->win);

}



/* redisplays image */
static void pdp_glx_bang_thread(t_pdp_glx *x)
{


    pdp_glx_display_texture(x);
    XFlush(x->x_xdpy->dpy);

}

static void pdp_glx_bang_callback(t_pdp_glx *x)
{
    /* receive events + send to outputs */
    t_pdp_list *eventlist = pdp_xwindow_get_eventlist(x->x_xwin);
    t_pdp_atom *a;

    for (a=eventlist->first; a; a=a->next){
	//pdp_list_print(a->w.w_list);
	outlet_pdp_list(x->x_outlet, a->w.w_list);
    }

    /* free list */
    pdp_tree_free(eventlist);

    /* release the packet if there is one */
    pdp_packet_mark_unused(x->x_packet0);
    x->x_packet0 = -1;

}
static void pdp_glx_bang(t_pdp_glx *x)
{

    /* check if window is initialized */
    if (!(x->x_initialized)){
        if (!pdp_glx_try_autocreate(x)) return;
    }


    /* if previous queued method returned
       schedule a new one, else ignore */

/*
    if (-1 == x->x_queue_id) {
	pdp_queue_add(x, pdp_glx_bang_thread, pdp_glx_bang_callback, &x->x_queue_id);
    }
*/
    /* don't process in thread */
    pdp_glx_bang_thread(x);
    pdp_glx_bang_callback(x);
    
}



static void pdp_glx_input_0(t_pdp_glx *x, t_symbol *s, t_floatarg f)
{

    if (s == gensym("register_ro")) pdp_packet_copy_ro_or_drop(&x->x_packet0, (int)f);
    if (s == gensym("process")) pdp_glx_process(x);
}



static void pdp_glx_vga(t_pdp_glx *x)
{
    pdp_glx_resize(x, 640, 480);
}

static void pdp_glx_autocreate(t_pdp_glx *x, t_floatarg f)
{
  if (f != 0.0f) x->x_autocreate = PDP_OGL_AUTOCREATE_RETRY;
  else x->x_autocreate = 0;
}

static void pdp_glx_display(t_pdp_glx *x, t_symbol *s)
{
    t_pdp_procqueue *q = pdp_queue_get_queue();
    pdp_procqueue_finish(q, x->x_queue_id);
    x->x_queue_id = -1;
    x->x_display = s;
    if (x->x_initialized){
	pdp_glx_destroy(x);
	pdp_glx_create(x);
    }
}

static void pdp_glx_interpol(t_pdp_glx *x, t_float finterpol){
    x->x_interpol = (int)finterpol;
}


static void pdp_glx_free(t_pdp_glx *x)
{
    t_pdp_procqueue *q = pdp_queue_get_queue();
    pdp_procqueue_finish(q, x->x_queue_id);
    pdp_glx_destroy(x);
    if (x->x_data) pdp_dealloc (x->x_data);
    pdp_packet_mark_unused(x->x_packet0);
}

t_class *pdp_glx_class;



void *pdp_glx_new(void)
{
    t_pdp_glx *x = (t_pdp_glx *)pd_new(pdp_glx_class);

    x->x_xwin = 0;
    x->x_xdpy = 0;

    x->x_outlet = outlet_new(&x->x_obj, &s_anything);

    x->x_packet0 = -1;
    x->x_queue_id = -1;
    x->x_display = gensym(":0");

    x->x_width = PDP_OGL_W;
    x->x_height = PDP_OGL_H;

    x->x_data = pdp_alloc(4*PDP_OGL_W*PDP_OGL_H);

    x->x_initialized = 0;
    pdp_glx_autocreate(x,1);
    x->x_last_encoding = -1;

    x->x_tex_width = 64;
    x->x_tex_height = 64;

    x->x_interpol = 1;

    //pdp_glx_create(x);

    return (void *)x;
}





#ifdef __cplusplus
extern "C"
{
#endif


void pdp_glx_setup(void)
{


    pdp_glx_class = class_new(gensym("pdp_glx"), (t_newmethod)pdp_glx_new,
    	(t_method)pdp_glx_free, sizeof(t_pdp_glx), 0, A_NULL);

    /* add creator for pdp_tex_win */

    class_addmethod(pdp_glx_class, (t_method)pdp_glx_bang, gensym("bang"), A_NULL);
    class_addmethod(pdp_glx_class, (t_method)pdp_glx_create, gensym("open"), A_NULL);
    class_addmethod(pdp_glx_class, (t_method)pdp_glx_create, gensym("create"), A_NULL);
    class_addmethod(pdp_glx_class, (t_method)pdp_glx_autocreate, gensym("autocreate"), A_FLOAT, A_NULL);
    class_addmethod(pdp_glx_class, (t_method)pdp_glx_destroy, gensym("destroy"), A_NULL);
    class_addmethod(pdp_glx_class, (t_method)pdp_glx_destroy, gensym("close"), A_NULL);
    class_addmethod(pdp_glx_class, (t_method)pdp_glx_move, gensym("move"), A_FLOAT, A_FLOAT, A_NULL);
    class_addmethod(pdp_glx_class, (t_method)pdp_glx_move, gensym("pos"), A_FLOAT, A_FLOAT, A_NULL);
    class_addmethod(pdp_glx_class, (t_method)pdp_glx_resize, gensym("dim"), A_FLOAT, A_FLOAT, A_NULL);
    class_addmethod(pdp_glx_class, (t_method)pdp_glx_resize, gensym("size"), A_FLOAT, A_FLOAT, A_NULL);
    class_addmethod(pdp_glx_class, (t_method)pdp_glx_display, gensym("display"), A_SYMBOL, A_NULL);
    class_addmethod(pdp_glx_class, (t_method)pdp_glx_cursor, gensym("cursor"), A_FLOAT, A_NULL);
    class_addmethod(pdp_glx_class, (t_method)pdp_glx_fullscreen, gensym("fullscreen"), A_NULL);
    class_addmethod(pdp_glx_class, (t_method)pdp_glx_moveresize, gensym("posdim"), A_FLOAT, A_FLOAT, A_FLOAT, A_FLOAT, A_NULL);
    class_addmethod(pdp_glx_class, (t_method)pdp_glx_interpol, gensym("interpol"), A_FLOAT, A_NULL);
    class_addmethod(pdp_glx_class, (t_method)pdp_glx_tile, gensym("tile"), A_FLOAT, A_FLOAT, A_FLOAT, A_FLOAT, A_NULL);


    /* accept both pdp and pdp_tex packets */
    class_addmethod(pdp_glx_class, (t_method)pdp_glx_input_0, gensym("pdp"),  A_SYMBOL, A_DEFFLOAT, A_NULL);


    /* some shortcuts for the lazy */
    class_addmethod(pdp_glx_class, (t_method)pdp_glx_vga, gensym("vga"), A_NULL);

}

#ifdef __cplusplus
}
#endif


