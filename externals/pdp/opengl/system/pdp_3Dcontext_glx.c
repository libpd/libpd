
/*
 *   OpenGL Extension Module for pdp - opengl system stuff
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

/* this file contains the platform dependent opengl setup routines (glx)
   and pdp_packet_3Dcontext methods */

#include "pdp_opengl.h"
#include "pdp_xwindow.h"
#include "pdp_internals.h"
#include <GL/gl.h>
#include <GL/glx.h>
#include <GL/glu.h>

/* all symbols are C-style */
#ifdef __cplusplus
//extern "C"
//{
#endif

// this is buggy: disabled
#define PRIVATE_CONTEXT 0


/* structure to hold the (platform dependent) gl environment setup */
typedef struct _gl_env
{
    bool initialized;        /* data structure is consistent */

    XVisualInfo *visual;     /* the visual info structure for the context */
    GLXContext context;      /* the rendering context used to render to windows or pbufs */
    GLXFBConfig *config;     /* the framebuffer config object */
    
    t_pdp_xdisplay *xdpy;    /* pdp's x display object */

    //Display *dpy;            /* x display connection */
    //int screen;              /* x screen */
    int last_context_packet; /* the packet that is currently rendered too (for caching) */
} t_gl_env;

static t_gl_env pdp_glx_env;
static t_pdp_class *context_class;

/* PDP_3DCONTEXT packet methods */

/* set/unset ogl rendering context to pbuf */
void pdp_packet_3Dcontext_set_rendering_context(int packet)
{
    t_3Dcontext *c = pdp_packet_3Dcontext_info(packet);


    if (!c) return;


    /* don't do a glx call if the context is still the same */
    if (pdp_glx_env.last_context_packet == packet) return;

    //post("new current context is %d", packet);


    /* pbuffer */
    switch(c->encoding){
    case PDP_3DCONTEXT_WINDOW:
	//glFinish();
	//glXMakeCurrent(pdp_glx_env.dpy, ((t_pdp_xwindow *)c->drawable)->win, pdp_glx_env.context);
	glXMakeCurrent(pdp_glx_env.xdpy->dpy, ((t_pdp_xwindow *)c->drawable)->win, (GLXContext)c->context);
	pdp_glx_env.last_context_packet = packet;
	break;
    case PDP_3DCONTEXT_PBUFFER:
	//glXMakeCurrent(pdp_glx_env.dpy, (GLXPbuffer)c->drawable, pdp_glx_env.context);
	//glXMakeContextCurrent(c->dpy, c->drawable.pbuf, c->drawable.pbuf, c->context);
	pdp_glx_env.last_context_packet = -1;
	break;
    default:
	pdp_glx_env.last_context_packet = -1;
	break;
    }
    
}

void pdp_packet_3Dcontext_unset_rendering_context(int packet)
{
    t_3Dcontext *c = pdp_packet_3Dcontext_info(packet);
    if (!c) return;

    /* pbuffer */
    switch(c->encoding){
    case PDP_3DCONTEXT_WINDOW:
	glXMakeCurrent(pdp_glx_env.xdpy->dpy, None, NULL);
	pdp_glx_env.last_context_packet = -1;
	break;
    case PDP_3DCONTEXT_PBUFFER:
	//glXMakeCurrent(pdp_glx_env.dpy, None, NULL);
	//glXMakeContextCurrent(c->dpy, c->drawable.pbuf, c->drawable.pbuf, c->context);
	break;
    default:
	break;
    }
}


/* cons/des */
static void _3Dcontext_clone(t_pdp *dst, t_pdp *src)
{
    post("ERROR: clone not supported for 3Dcontext packets");
}

static void _3Dcontext_copy(t_pdp *dst, t_pdp *src)
{
    post("ERROR: copy not supported for 3Dcontext packets");
}

static void _3Dcontext_reinit(t_pdp *dst)
{
    /* leave the packet as is */
}
static void _3Dcontext_cleanup(t_pdp *dst)
{
    t_3Dcontext *c = (t_3Dcontext *)(&dst->info.raw);

    /* reset context packet cache, in case this packet was the current context. */
    pdp_glx_env.last_context_packet = -1;

    switch(c->encoding){
    case PDP_3DCONTEXT_WINDOW:
#if PRIVATE_CONTEXT
	glXDestroyContext (pdp_glx_env.dpy, (GLXContext)c->context);
#endif
	pdp_xwindow_cleanup((t_pdp_xwindow *)c->drawable);
	free(c->drawable);
	break;
	
    case PDP_3DCONTEXT_PBUFFER:
	break;
	//glXDestroyContext(c->dpy, c->context);
	//glXDestroyPbuffer(c->dpy, c->drawable.pbuf);
    default:
	break;
    }
}


/* setup packet methods */
static void _3Dcontext_init_methods(t_pdp *header)
{
    header->theclass = context_class;
    header->flags = PDP_FLAG_DONOTCOPY;
}



/* window specific methods */


void _pdp_3Dcontext_set_window_size(t_3Dcontext *c, t_pdp_xwindow *xwin)
{
    c->width     = xwin->winwidth;
    c->sub_width = xwin->winwidth;
    c->height    = xwin->winheight;
    c->sub_height= xwin->winheight;
}

/* resize the window */
void pdp_packet_3Dcontext_win_resize(int packet, int width, int height)
{
    t_pdp_xwindow *xwin;
    t_3Dcontext *c = pdp_packet_3Dcontext_info(packet);
    if (!c) return;
    if (PDP_3DCONTEXT_WINDOW != c->encoding) return;
    xwin = (t_pdp_xwindow *)c->drawable;
    pdp_xwindow_resize(xwin, width, height);
    _pdp_3Dcontext_set_window_size(c, xwin);
}


t_pdp_list *pdp_packet_3Dcontext_win_get_eventlist(int packet)
{
    t_pdp_list *eventlist;
    t_pdp_xwindow *xwin;
    t_3Dcontext *c = pdp_packet_3Dcontext_info(packet);
    if (!c) return 0;
    if (PDP_3DCONTEXT_WINDOW != c->encoding) return 0;
    xwin = (t_pdp_xwindow *)c->drawable;
    eventlist = pdp_xwindow_get_eventlist(xwin);
    _pdp_3Dcontext_set_window_size(c, xwin);
    return eventlist;
}

void pdp_packet_3Dcontext_win_cursor(int packet, bool toggle)
{
    t_pdp_xwindow *xwin;
    t_3Dcontext *c = pdp_packet_3Dcontext_info(packet);
    if (!c) return;
    if (PDP_3DCONTEXT_WINDOW != c->encoding) return;
    xwin = (t_pdp_xwindow *)c->drawable;
    pdp_xwindow_cursor(xwin, toggle);

}

void pdp_packet_3Dcontext_win_swapbuffers(int packet)
{
    t_pdp_xwindow *xwin;
    t_3Dcontext *c = pdp_packet_3Dcontext_info(packet);
    if (!c) return;
    if (PDP_3DCONTEXT_WINDOW != c->encoding) return;
    xwin = (t_pdp_xwindow *)c->drawable;
    glXSwapBuffers(xwin->xdisplay->dpy,xwin->win);
    //glFinish();

}


/* constructors */

/* construct (or reuse) a window packet */
int pdp_packet_new_3Dcontext_win(void)
{
    /* $$$FIXME: this assumes packet can't be reused */
    int p = pdp_packet_new(PDP_3DCONTEXT, 0);
    t_pdp_xwindow *xwin;
    t_3Dcontext *c;
    t_pdp *header = pdp_packet_header(p);
    if (!header) return -1; /* pool full ? */
    c = (t_3Dcontext *)&header->info.raw;

    if (c->drawable){
	xwin = (t_pdp_xwindow *)c->drawable;
    }
    else{
	xwin = (t_pdp_xwindow *)malloc(sizeof(*xwin));
    }

    pdp_xwindow_init(xwin);
    pdp_xwindow_create_on_display(xwin, pdp_glx_env.xdpy);

    /* init subheader */
#if PRIVATE_CONTEXT
    if (NULL == (c->context = (void *)glXCreateContext(pdp_glx_env.dpy, pdp_glx_env.visual, pdp_glx_env.context, True))){
	post("pdp_packet_new_3Dcontext_wind: ERROR: can't create rendering context");
    }
#else
    c->context = (void *)pdp_glx_env.context;
#endif
    c->drawable = xwin;
    c->encoding  = PDP_3DCONTEXT_WINDOW;
    _pdp_3Dcontext_set_window_size(c, xwin);

    /* init packet methods */
    _3Dcontext_init_methods(header);

    /* init header */
    header->desc = pdp_gensym("3Dcontext/window");
    header->flags = PDP_FLAG_DONOTCOPY;
    
    return p;


}

/* pbuf constructor */
int pdp_packet_new_3Dcontext_pbuf(u32 width, u32 height, u32 depth)
{
    post("ERROR: 3Dcontext/pbuffer packets not implemented");
    return -1;
}


/* this is a notifier sent when the processing thread which
   executes gl commands is changed. we need to release the current context
   before another thread can take it. */
void pdp_3Dcontext_prepare_for_thread_switch(void)
{
    pdp_packet_3Dcontext_unset_rendering_context(pdp_glx_env.last_context_packet);
}





/* setup routine */
static void pdp_3Dcontext_glx_setup_inthread(void)
{
    /* this opens the connection to the x server and creates a render context
       for windows (glx < 1.3) or windows/pbufs (glx >= 1.3) */

    static int dblBuf24[] =  {GLX_RGBA,
			      GLX_RED_SIZE, 1, 
	                      GLX_GREEN_SIZE, 1, 
			      GLX_BLUE_SIZE, 1, 
			      GLX_ALPHA_SIZE, 0, 
			      GLX_DEPTH_SIZE, 1,
                              GLX_DOUBLEBUFFER,
			      None};

    pdp_glx_env.initialized = 0;


    /* init xlib for thread usage */
    if (!XInitThreads()){
    	post("pdp_opengl_system_setup: can't init Xlib for thread usage.");
    	goto init_failed;
    }
 

    /* open display:
       the first display on the local machine is opened, not DISPLAY.
       since pdp_opengl is all about direct rendering, and there
       is no way to specify another display, or even close it and
       open it again, this seems to be the "least surprise" solution.
       it enables the pd interface to be displayed on another display,
       using the DISPLAY environment variable. */
    

    


    if (NULL == (pdp_glx_env.xdpy = pdp_xdisplay_new(":0"))){
	post("pdp_opengl_system_setup: can't open display");
	goto init_failed;
    }


    /* get visual */
    if (NULL == (pdp_glx_env.visual = glXChooseVisual(pdp_glx_env.xdpy->dpy, pdp_glx_env.xdpy->screen, dblBuf24))){
	post("pdp_opengl_system_setup: can't find appropriate visual");
	goto init_failed_close_dpy;
    }


    /* create a (direct) rendering context */
    if (NULL == (pdp_glx_env.context = glXCreateContext(pdp_glx_env.xdpy->dpy, pdp_glx_env.visual, 0, True))){
	post("pdp_opengl_system_setup: can't create rendering context");
	goto init_failed_close_dpy;
    }


    //post("pdp_opengl_system_setup: pdp_opengl init OK.");
    pdp_glx_env.last_context_packet = -1;
    pdp_glx_env.initialized = 1;

    /* setup class object */
    context_class = pdp_class_new(pdp_gensym("3Dcontext/*"), 0);
    context_class->cleanup = _3Dcontext_cleanup;
    context_class->wakeup = _3Dcontext_reinit;
    //context_class->clone = _3Dcontext_clone;
    context_class->copy = _3Dcontext_copy;


    /* setup conversion programs: NOT IMPLEMENTED */
    return;
    

 init_failed_close_dpy:
    pdp_xdisplay_free(pdp_glx_env.xdpy);
    pdp_glx_env.xdpy = 0;
 init_failed:
    post("pdp_opengl_system_setup: FATAL ERROR: pdp_opengl init failed.");
    exit(1);

}

/* run the setup routine in the procqueue thread, and wait for it to finish */
/* NOTE: this seems to make an Xlib deadlock problem go away when running
   pd with realtime scheduling. frankly, i'm very puzzled by this problem
   and even more by the way this workaround solves it. anyhow... */
void pdp_3Dcontext_glx_setup(void)
{
    t_pdp_procqueue *q = pdp_opengl_get_queue();
    pdp_procqueue_add(q, 0, pdp_3Dcontext_glx_setup_inthread, 0, 0);
    pdp_procqueue_flush(q);
}

#ifdef __cplusplus
//}
#endif
