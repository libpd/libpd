/*
 *   Pure Data Packet module.
 *   Copyright (c) 2003 by martin pi <pi@attacksyour.net>
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

/*

pdp sdl output

DONE:

TODO:
 * close window (event)
 * fullscreen chose resolution
 * event handling in different object (and look at mplayer for that!)

*/


#include <stdio.h>
#include <SDL/SDL.h>
#include "pdp.h"
#include "pdp_llconv.h"




/* initial image dimensions */

//#define WINWIDTH 640
//#define WINHEIGHT 480
#define WINWIDTH 320
#define WINHEIGHT 240

#define OVERLAYWIDTH 320
#define OVERLAYHEIGHT 240



typedef struct pdp_sdl_struct {
    t_object x_obj;

    SDL_Surface *x_surface;
    SDL_Overlay *x_overlay;
    int x_surface_flags;
	
    // current values
    int x_surface_width;
    int x_surface_height;
    unsigned int x_overlay_width;
    unsigned int x_overlay_height;

    // object creation args (wanted)
    int x_w;
    int x_h;

    t_outlet *x_outlet;



} t_pdp_sdl;


static int pdp_sdl_open(t_pdp_sdl *x);
#define IF_NO_OVERLAY(x) if (!pdp_sdl_open(x))

static t_pdp_sdl *sdl_singleton; // only one instance allowed

static void destroy_overlay(t_pdp_sdl *x) {
    if (x->x_overlay){
	SDL_FreeYUVOverlay(x->x_overlay);
	x->x_overlay = 0;
    }
}

static void create_overlay(t_pdp_sdl *x, int width, int height) {
    if (x->x_surface){
	if (x->x_overlay = SDL_CreateYUVOverlay(width, height, SDL_YV12_OVERLAY, x->x_surface)){
	    x->x_overlay_width = width;
	    x->x_overlay_height = height;
	    return;
	}
    }
    pdp_post("SDL: can't create overlay.");
}

static void check_overlay(t_pdp_sdl *x, unsigned int width, unsigned int height){
    if (!x->x_overlay
	|| (x->x_overlay_width != width)
	|| (x->x_overlay_height != height)){
	destroy_overlay(x);
	create_overlay(x, width, height);
    }
}


static void create_surface(t_pdp_sdl *x, int width, int height, int flags)
{
    flags |= SDL_HWSURFACE|SDL_ANYFORMAT|SDL_RESIZABLE; // add default flags
//    flags |= SDL_HWSURFACE|SDL_ANYFORMAT; // add default flags
//    flags |= SDL_SWSURFACE|SDL_ANYFORMAT; // add default flags

    /* flags:
       SDL_HWSURFACE use hardware surface
       SDL_ANYFORMAT return current surface, even if it doesn't match
       SDL_OPENGL|SDL_DOUBLEBUF double buffer and opengl
       SDL_RLEACCEL rle accelleration for blitting
       SDL_FULLSCREEN fullscreen window
     */

    //pdp_post("create_surface %d %d %d", width, height, flags);

    /* check args */
    if (width < 1) width = 1;
    if (height < 1) height = 1;

    /* free old stuff */
    if (x->x_overlay) destroy_overlay(x);
    /* form manpage:
       The framebuffer surface, or NULL if it fails. The surface returned 
       is freed by SDL_Quit() and should nt be freed by the caller. */
    if (x->x_surface) { /*SDL_FreeSurface(surface);*/ }


    /* create new surface */
    if (!(x->x_surface = SDL_SetVideoMode(width, height, 16, flags))){
	pdp_post("SDL: Couldn't create a surface: %s", SDL_GetError());
	return;
    }

    /* setup surface */
    SDL_WM_SetCaption("pdp", "pdp");
    SDL_ShowCursor(0);
    /* set event mask to something conservative
       and add a word to ask for some types of events */
    x->x_surface_width = width;
    x->x_surface_height = height;
    x->x_surface_flags = flags;

}

static void poll_events(t_pdp_sdl *x){
    IF_NO_OVERLAY(x) { return; }

    SDL_Event event;
    static t_symbol *keydown=0, *keyup, *quit, *motion;
    t_atom atom;

    /* cache symbols */
    if (!keydown){
	keydown = gensym("keypress");
	keyup = gensym("keyrelease");
	quit = gensym("quit");
    }

    if (!x->x_surface) return;

    /* poll events */
    while(SDL_PollEvent(&event)){  
        switch(event.type){

	case SDL_KEYDOWN:
	    SETFLOAT(&atom, (float)event.key.keysym.scancode);
	    outlet_anything(x->x_outlet, keydown, 1, &atom);
	    break;
	    
	case SDL_KEYUP:
	    SETFLOAT(&atom, (float)event.key.keysym.scancode);
	    outlet_anything(x->x_outlet, keyup, 1, &atom);
	    break;
		
	case SDL_QUIT:
	    outlet_symbol(x->x_outlet, quit);
	    break;

	case SDL_VIDEORESIZE:
	    create_surface(x, event.resize.w, event.resize.h, x->x_surface_flags);
	    break;
        }
    }
}


static void fullscreen(t_pdp_sdl *x, t_floatarg f)
{
    post("fullscreen not implemented");
}


static void resize(t_pdp_sdl *x, t_floatarg fw, t_floatarg fh)
{
    create_surface(x, (int)fw, (int)fh, 0);
}



static void input_0(t_pdp_sdl *x, t_symbol *s, t_floatarg f) {

    IF_NO_OVERLAY(x) { return; }

    int input_packet = (int)f;
    if (s == gensym("register_ro")){
	int p = pdp_packet_convert_ro(input_packet, pdp_gensym("bitmap/yv12/*"));

	/* poll anyway. */
	//poll_events(x);

	/* check packet */
	if (-1 == p){
	    post("SDL: can't convert image to bitmap/yv12/*");
	    return;
	}
	else {
	    t_bitmap *bitmap = pdp_packet_subheader(p);
	    unsigned char *data = pdp_packet_data(p);
	    int planesize = bitmap->width * bitmap->height;
	    check_overlay(x, bitmap->width, bitmap->height);
	    if (x->x_overlay){
		SDL_Rect rect = {0, 0, x->x_surface_width, x->x_surface_height};

		/* copy */
		SDL_LockYUVOverlay(x->x_overlay);
		memcpy(x->x_overlay->pixels[0], data, planesize); data += planesize;
		memcpy(x->x_overlay->pixels[1], data, planesize >> 2); data += (planesize >> 2);
		memcpy(x->x_overlay->pixels[2], data, planesize >> 2);
		SDL_UnlockYUVOverlay(x->x_overlay);

		/* display */
		if (SDL_DisplayYUVOverlay(x->x_overlay, &rect)){
		    pdp_post("SDL: can't display overlay");
		    return;
		}
	    }
		
	    else {
		PDP_ASSERT(0);
	    }
		
	    pdp_packet_mark_unused(p);
	    return;
	}
    }
}





static void pdp_sdl_free(t_pdp_sdl *x)
{
    destroy_overlay(x);
    sdl_singleton = 0;
    SDL_Quit();
}


t_class *pdp_sdl_class;


static int pdp_sdl_open(t_pdp_sdl *x){

    if (x->x_overlay) return -1;

    /* try to create a surface */
    create_surface(x, x->x_w ? x->x_w : WINWIDTH, x->x_h ? x->x_h : WINHEIGHT, 0);
    if (!x->x_surface){
	pdp_post("Can't create surface");
	goto error_cleanup;
    }

    /* try to create overlay */
    check_overlay(x, OVERLAYHEIGHT, OVERLAYWIDTH);
    if (!x->x_overlay){
	pdp_post("Can't create overlay");
	goto error_cleanup;
    }
    return -1;


  error_cleanup:
    pdp_sdl_free(x);
    return 0;

}


static void *pdp_sdl_new(t_floatarg width, t_floatarg height) {

    t_pdp_sdl *x;

    if (sdl_singleton) {
	post("Only one sdl object allowed.");
	return 0;
    }

    if( SDL_Init(SDL_INIT_VIDEO) < 0 ) {
	pdp_post("Could not initialize SDL: %s", SDL_GetError());
	return 0;
    }
    atexit(SDL_Quit);


    x = (t_pdp_sdl *)pd_new(pdp_sdl_class);
    sdl_singleton = x;

    
    x->x_surface = NULL;
    x->x_overlay = NULL;
    x->x_w = (int)width;
    x->x_h = (int)height;

    x->x_outlet = outlet_new(&x->x_obj, &s_anything);

    return (void *)x;

}



#ifdef __cplusplus
extern "C"
{
#endif


void pdp_sdl_setup(void)
{

    sdl_singleton = 0;

    pdp_sdl_class = class_new(gensym("pdp_sdl"), (t_newmethod)pdp_sdl_new,
    	(t_method)pdp_sdl_free, sizeof(t_pdp_sdl), 0, A_NULL);


    class_addmethod(pdp_sdl_class, (t_method)resize, gensym("size"), A_FLOAT, A_FLOAT, A_NULL);
    class_addmethod(pdp_sdl_class, (t_method)poll_events, gensym("poll"), A_NULL);
    class_addmethod(pdp_sdl_class, (t_method)fullscreen, gensym("fullscreen"), A_FLOAT, A_NULL);
    class_addmethod(pdp_sdl_class, (t_method)input_0, gensym("pdp"),  A_SYMBOL, A_DEFFLOAT, A_NULL);

}

#ifdef __cplusplus
}
#endif

