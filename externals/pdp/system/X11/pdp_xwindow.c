/*
 *   Pure Data Packet system module. - x window glue code (fairly tied to pd and pdp)
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


// this code is fairly tied to pd and pdp. serves mainly as reusable glue code
// for pdp_xv, pdp_glx, pdp_3d_windowcontext, ...


#include <string.h>
#include "pdp_xwindow.h"
#include "pdp_post.h"
#include "pdp_debug.h"
#include "pdp_symbol.h"
#include "pdp_list.h"

#define D if(0)


//    xwin->xdisplay->screen = DefaultScreen(xwin->xdisplay->dpy);

/* x display class 
typedef struct _pdp_xdisplay
{
    Display *dpy;              // the display connection
    int screen;                // the screen
    t_pdp_list *windowlist;    // all windows belonging to this connection
                               // this contains (xwindow object, eventlist)
} t_pdp_xdisplay; */


/************************************* PDP_XDISPLAY ************************************/


t_pdp_xdisplay *pdp_xdisplay_new(char *dpy_string)
{
    t_pdp_xdisplay *d = pdp_alloc(sizeof(*d));
    if (!(d->dpy = XOpenDisplay(dpy_string))){
	pdp_post ("pdp_xdisplay_new: can't open display %s", dpy_string);
	pdp_dealloc(d);
	return (0);
    }
	

    d->windowlist = pdp_list_new(0);
    d->screen = DefaultScreen(d->dpy);
    d->dragbutton = -1;
    return d;
}

void pdp_xdisplay_free(t_pdp_xdisplay *d)
{
    XCloseDisplay(d->dpy);
    PDP_ASSERT(0 == d->windowlist->elements); // make sure there are no dangling xwindow objects
    pdp_list_free(d->windowlist);
    pdp_dealloc(d);
}

/* some private members */
static int _windowset_contains(t_pdp_xdisplay *d, t_pdp_xwindow *w)
{
    t_pdp_atom *a;
    for (a=d->windowlist->first; a; a=a->next){
	if (w == a->w.w_list->first->w.w_pointer) return 1;
    }
    return 0;
}

static void _windowset_add(t_pdp_xdisplay *d, t_pdp_xwindow *w)
{
    t_pdp_list *l = pdp_list_new(0);    // a new list for this window
    t_pdp_list *l_ev = pdp_list_new(0); // the event list
    pdp_list_add_back_pointer(l, w);
    pdp_list_add_back(l, a_list, (t_pdp_word)l_ev);

    pdp_list_add_back(d->windowlist, a_list, (t_pdp_word)l);
}

/* get the list describing this window */
static t_pdp_list *_windowset_get_info_for_Window(t_pdp_xdisplay *d, Window win)
{
    t_pdp_atom *a;
    for (a=d->windowlist->first; a; a=a->next){
	if (win == ((t_pdp_xwindow *)a->w.w_list->first->w.w_pointer)->win) return a->w.w_list;
    }
    return 0;
}

static t_pdp_list *_windowset_get_info(t_pdp_xdisplay *d, t_pdp_xwindow *w)
{
    t_pdp_atom *a;
    for (a=d->windowlist->first; a; a=a->next){
	if (w == a->w.w_list->first->w.w_pointer) return a->w.w_list;
    }
    return 0;
}

static void _windowset_remove(t_pdp_xdisplay *d, t_pdp_xwindow *w)
{
    t_pdp_list *l = _windowset_get_info(d, w);
    if (l){
	pdp_list_remove(d->windowlist, a_list, (t_pdp_word)l);
	pdp_tree_free(l);
    }
}

void pdp_xdisplay_register_window(t_pdp_xdisplay *d, t_pdp_xwindow *w)
{
    
    if (!_windowset_contains(d, w)) _windowset_add(d, w);
}
void pdp_xdisplay_unregister_window(t_pdp_xdisplay *d, t_pdp_xwindow *w)
{
    if (_windowset_contains(d, w)) _windowset_remove(d, w);
}


/* LOCKING !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!1111*/
/* get events from display and store in queues */
void pdp_xdisplay_get_events(t_pdp_xdisplay *d)
{

    unsigned int i;
    XEvent e;

    /* event tags */
    char tag_drag[]    = "drag0";
    char tag_press[]   = "press0";
    char tag_release[] = "release0";
    char tag_motion[]  = "motion0";

    char *BUT(char *c) {return c + strlen(c) - 1;}

    /* button chars */
    char *but_drag  =   BUT(tag_drag);
    char *but_press =   BUT(tag_press);
    char *but_release = BUT(tag_release);
    char *but_motion =  BUT(tag_motion);

    int nbEvents =  XEventsQueued(d->dpy, QueuedAlready);
    int bmask = Button1Mask
	| Button2Mask
	| Button3Mask
	| Button4Mask
	| Button5Mask;


    while (XPending(d->dpy)){
	XNextEvent(d->dpy, &e);


	/* get the window info list for this X11 Window */
	t_pdp_list *winfo = _windowset_get_info_for_Window(d, e.xany.window);

	/* get the window object */
	t_pdp_xwindow *xwin = (t_pdp_xwindow *)winfo->first->w.w_pointer;

	/* get the event list corresponding to this window */
	t_pdp_list *eventlist = winfo->first->next->w.w_list;

	/* set dim scalers */
	float inv_x = 1.0f / (float)(xwin->winwidth);
	float inv_y = 1.0f / (float)(xwin->winheight);

	/* list to store new event */
	t_pdp_list *newevent = 0;

	/* event tag */
	char *tag;
	char *but;


	/* handle event */
	switch(e.type){
	case ConfigureNotify:
	    /* store new dimensions */
	    xwin->winwidth = e.xconfigure.width;
	    xwin->winheight = e.xconfigure.height;
	    break;
	    
	case ClientMessage:
	    if ((Atom)e.xclient.data.l[0] == xwin->WM_DELETE_WINDOW) {
		newevent = pdp_list_new(1);
		pdp_list_set_0(newevent, a_symbol, (t_pdp_word)pdp_gensym("close"));
		pdp_list_add_back(eventlist, a_list, (t_pdp_word)newevent);
	    }
	    break;

	case KeyPress:
	case KeyRelease:
	    newevent = pdp_list_new(2);
	    pdp_list_set_0(newevent, a_symbol, (t_pdp_word)pdp_gensym(e.type == KeyPress ? "keypress" : "keyrelease"));
	    pdp_list_set_1(newevent, a_int, (t_pdp_word)(int)e.xkey.keycode);
	    pdp_list_add_back(eventlist, a_list, (t_pdp_word)newevent);
	    break;

	case ButtonPress:
	case ButtonRelease:
	    /* event specific stuff */
	    if (e.type == ButtonPress){
		tag = tag_press;
		but = but_press;
	    }
	    else {
		tag = tag_release;
		but = but_release;
	    }
	    
	    /* send generic event */
	    *but = 0;
	    newevent = pdp_list_new(3);
	    pdp_list_set_0(newevent, a_symbol, (t_pdp_word)pdp_gensym(tag));
	    pdp_list_set_1(newevent, a_float, (t_pdp_word)(inv_x * (float)e.xbutton.x));
	    pdp_list_set_2(newevent, a_float, (t_pdp_word)(inv_y * (float)e.xbutton.y));
	    pdp_list_add_back(eventlist, a_list, (t_pdp_word)newevent);

	    /* send button specific event */
	    *but = '1' + e.xbutton.button - Button1;
	    newevent = pdp_list_new(3);
	    pdp_list_set_0(newevent, a_symbol, (t_pdp_word)pdp_gensym(tag));
	    pdp_list_set_1(newevent, a_float, (t_pdp_word)(inv_x * (float)e.xbutton.x));
	    pdp_list_set_2(newevent, a_float, (t_pdp_word)(inv_y * (float)e.xbutton.y));
	    pdp_list_add_back(eventlist, a_list, (t_pdp_word)newevent);

	    /* save drag button */
	    xwin->lastbut = *but;

	    break;

	case MotionNotify:
	    if (e.xbutton.state & bmask){
		/* button is down: it is a drag event */
		tag = tag_drag;
		but = but_drag;

		/* send generic event */
		*but = 0;
		newevent = pdp_list_new(3);
		pdp_list_set_0(newevent, a_symbol, (t_pdp_word)pdp_gensym(tag));
		pdp_list_set_1(newevent, a_float, (t_pdp_word)(inv_x * (float)e.xbutton.x));
		pdp_list_set_2(newevent, a_float, (t_pdp_word)(inv_y * (float)e.xbutton.y));
		pdp_list_add_back(eventlist, a_list, (t_pdp_word)newevent);

		/* send button specific event */
		*but = xwin->lastbut;
		newevent = pdp_list_new(3);
		pdp_list_set_0(newevent, a_symbol, (t_pdp_word)pdp_gensym(tag));
		pdp_list_set_1(newevent, a_float, (t_pdp_word)(inv_x * (float)e.xbutton.x));
		pdp_list_set_2(newevent, a_float, (t_pdp_word)(inv_y * (float)e.xbutton.y));
		pdp_list_add_back(eventlist, a_list, (t_pdp_word)newevent);

	    }
	    else {
		tag = tag_motion;
		but = but_motion;
		*but = 0;

		/* send generic event */
		newevent = pdp_list_new(3);
		pdp_list_set_0(newevent, a_symbol, (t_pdp_word)pdp_gensym(tag));
		pdp_list_set_1(newevent, a_float, (t_pdp_word)(inv_x * (float)e.xbutton.x));
		pdp_list_set_2(newevent, a_float, (t_pdp_word)(inv_y * (float)e.xbutton.y));
		pdp_list_add_back(eventlist, a_list, (t_pdp_word)newevent);

	    }
	    


	default:
	    //pdp_post("pdp_xv: unknown event");
	    break;
	}

	
    }


}


/* return a list containing event lists */

t_pdp_list *pdp_xdisplay_get_events_for_window(t_pdp_xdisplay *d, t_pdp_xwindow *w)
{
    t_pdp_list *info = _windowset_get_info(d, w);
    t_pdp_list *eventlist;
    PDP_ASSERT(info);
    
    /* get all pending events from display */
    pdp_xdisplay_get_events(d);

    /* get the event list for this window and create a new one */
    eventlist = info->first->next->w.w_list;
    info->first->next->w.w_list = pdp_list_new(0);

    return eventlist;
    
}


/************************************* PDP_XWINDOW ************************************/

void pdp_xwindow_warppointer(t_pdp_xwindow *xwin, int x, int y)
{
    if (xwin->initialized){
	XWarpPointer(xwin->xdisplay->dpy, None, xwin->win, 0, 0, 0, 0, x, y);
    }
}




static void pdp_xwindow_overrideredirect(t_pdp_xwindow *xwin, int b)
{
    XSetWindowAttributes new_attr;
    new_attr.override_redirect = b ? True : False;
    XChangeWindowAttributes(xwin->xdisplay->dpy, xwin->win, CWOverrideRedirect, &new_attr);
    //XFlush(xwin->xdisplay->dpy);

}


void pdp_xwindow_moveresize(t_pdp_xwindow *xwin, int xoffset, int yoffset, int width, int height)
{

    D pdp_post("_pdp_xwindow_moveresize");
    if ((width > 0) && (height > 0)){
	xwin->winwidth = width;
	xwin->winheight = height;
	xwin->winxoffset = xoffset;
	xwin->winyoffset = yoffset;

	if (xwin->initialized){
	    XMoveResizeWindow(xwin->xdisplay->dpy, xwin->win, xoffset, yoffset, width,  height);
	    XFlush(xwin->xdisplay->dpy);
	}
    }
}


void pdp_xwindow_fullscreen(t_pdp_xwindow *xwin)
{
    XWindowAttributes rootwin_attr;

    D pdp_post("pdp_xwindow_fullscreen");

    /* hmm.. fullscreen and xlib the big puzzle..
       if it looks like a hack it is a hack. */

    if (xwin->initialized){

        XGetWindowAttributes(xwin->xdisplay->dpy, RootWindow(xwin->xdisplay->dpy, xwin->xdisplay->screen), &rootwin_attr );

	//pdp_xwindow_overrideredirect(xwin, 0);
	pdp_xwindow_moveresize(xwin, 0, 0, rootwin_attr.width,  rootwin_attr.height);
	//pdp_xwindow_overrideredirect(xwin, 1);
	//XRaiseWindow(xwin->xdisplay->dpy, xwin->win);
	//pdp_xwindow_moveresize(xwin, 0, 0, rootwin_attr.width,   rootwin_attr.height);
	//pdp_xwindow_overrideredirect(xwin, 0);

 


    }
}


void pdp_xwindow_tile(t_pdp_xwindow *xwin, int x_tiles, int y_tiles, int i, int j)
{
    XWindowAttributes rootwin_attr;
    XSetWindowAttributes new_attr;

    D pdp_post("pdp_xwindow_fullscreen");

    if (xwin->initialized){
	int tile_w;
	int tile_h;
        XGetWindowAttributes(xwin->xdisplay->dpy, RootWindow(xwin->xdisplay->dpy, xwin->xdisplay->screen), &rootwin_attr );

	tile_w = rootwin_attr.width / x_tiles;
	tile_h = rootwin_attr.height / y_tiles;

	xwin->winwidth = (x_tiles-1) ? rootwin_attr.width - (x_tiles-1)*tile_w : tile_w;
	xwin->winheight = (y_tiles-1) ? rootwin_attr.height - (y_tiles-1)*tile_h : tile_h;
	xwin->winxoffset = i * tile_w;
	xwin->winyoffset = j * tile_h;

        //new_attr.override_redirect = True;
        //XChangeWindowAttributes(xwin->xdisplay->dpy, xwin->win, CWOverrideRedirect, &new_attr );
	XMoveResizeWindow(xwin->xdisplay->dpy, xwin->win, xwin->winxoffset, xwin->winyoffset, xwin->winwidth,  xwin->winheight);

    }
}

/* resize window */
void pdp_xwindow_resize(t_pdp_xwindow *xwin, int width, int height)
{
    D pdp_post("pdp_xwindow_resize");
    if ((width > 0) && (height > 0)){
	xwin->winwidth = width;
	xwin->winheight = height;
	if (xwin->initialized){
	    XResizeWindow(xwin->xdisplay->dpy, xwin->win,  width,  height);
	    XFlush(xwin->xdisplay->dpy);
	}
    }
    //_pdp_xwindow_moveresize(xwin, xwin->winxoffset, xwin->winyoffset, width, height);
}

/* move window */
void pdp_xwindow_move(t_pdp_xwindow *xwin, int xoffset, int yoffset)
{
    D pdp_post("pdp_xwindow_move");
    pdp_xwindow_moveresize(xwin, xoffset, yoffset, xwin->winwidth, xwin->winheight);
}

/* send events to a pd outlet (don't call this outside the pd thread) */
t_pdp_list *pdp_xwindow_get_eventlist(t_pdp_xwindow *xwin)
{
    t_pdp_list *eventlist;
   
    eventlist = pdp_xdisplay_get_events_for_window(xwin->xdisplay, xwin);
    D pdp_list_print(eventlist);

    return eventlist;

}



/* set an arbitrary cursor image */
void pdp_xwindow_cursor_image(t_pdp_xwindow *xwin, char *data, int width, int height)
{
    if (!xwin->initialized) return;

    Cursor cursor;
    Pixmap pm;
    XColor fg;
    XColor bg;

    fg.red = fg.green = fg.blue = 0xffff;
    bg.red = bg.green = bg.blue = 0x0000;

    pm = XCreateBitmapFromData(xwin->xdisplay->dpy, xwin->win, data, width, height);
    cursor = XCreatePixmapCursor(xwin->xdisplay->dpy, pm, pm, &fg,
				 &bg, width/2, height/2);
    XFreePixmap(xwin->xdisplay->dpy, pm);
    XDefineCursor(xwin->xdisplay->dpy, xwin->win,cursor);
}

/* enable / disable cursor */
void pdp_xwindow_cursor(t_pdp_xwindow *xwin, int i){
    if (!xwin->initialized) return;
    if (i == 0) {
        char data[] = {0};
	pdp_xwindow_cursor_image(xwin, data, 1, 1);
    }
    else
        XUndefineCursor(xwin->xdisplay->dpy, xwin->win);

    xwin->cursor = i;
}


void pdp_xwindow_title(t_pdp_xwindow *xwin, char *title)
{
    if (xwin->initialized)
	XStoreName(xwin->xdisplay->dpy, xwin->win, title);
}


/* create xwindow */
int pdp_xwindow_create_on_display(t_pdp_xwindow *xwin, t_pdp_xdisplay *d) 
{
    XEvent e;
    unsigned int i;

    /* check if already opened */
    if(  xwin->initialized ){
	pdp_post("pdp_xwindow_create_on_display: window already created");
	goto exit;
    }

    xwin->xdisplay = d;
    PDP_ASSERT(xwin->xdisplay);

    /* create a window */
    xwin->win = XCreateSimpleWindow(
	xwin->xdisplay->dpy, 
	RootWindow(xwin->xdisplay->dpy, xwin->xdisplay->screen), xwin->winxoffset, xwin->winyoffset, xwin->winwidth, xwin->winheight, 0, 
	BlackPixel(xwin->xdisplay->dpy, xwin->xdisplay->screen),
	BlackPixel(xwin->xdisplay->dpy, xwin->xdisplay->screen));


    /* enable handling of close window event */
    xwin->WM_DELETE_WINDOW = XInternAtom(xwin->xdisplay->dpy, "WM_DELETE_WINDOW", True);
    (void)XSetWMProtocols(xwin->xdisplay->dpy, xwin->win, &xwin->WM_DELETE_WINDOW, 1);

    if(!(xwin->win)){
	/* clean up mess */
	pdp_post("pdp_xwindow_create_on_display: could not create window. closing.\n");
	//XCloseDisplay(xwin->xdisplay->dpy); NOT OWNER
	xwin->xdisplay = 0;
	xwin->initialized = 0;
	goto exit;
    }

    /* select input events */
    XSelectInput(xwin->xdisplay->dpy, xwin->win, 
		 StructureNotifyMask  
		 | KeyPressMask 
		 | KeyReleaseMask 
		 | ButtonPressMask 
		 | ButtonReleaseMask 
		 | MotionNotify 
		 | PointerMotionMask);
    //	 | ButtonMotionMask);
    //XSelectInput(xwin->xdisplay->dpy, xwin->win, StructureNotifyMask);



    /* map */
    XMapWindow(xwin->xdisplay->dpy, xwin->win);

    /* create graphics context */
    xwin->gc = XCreateGC(xwin->xdisplay->dpy, xwin->win, 0, 0);

    /* catch mapnotify */
    for(;;){
	XNextEvent(xwin->xdisplay->dpy, &e);
	if (e.type == MapNotify) break;
    }


    /* we're done initializing */
    xwin->initialized = 1;

    /* disable/enable cursor */
    pdp_xwindow_cursor(xwin, xwin->cursor);

    /* set window title */
    pdp_xwindow_title(xwin, "pdp");

    /* register window for events */
    /* TODO: move event selection ETC to xdisplay object */

    pdp_xdisplay_register_window(xwin->xdisplay, xwin);

 exit:
    return xwin->initialized;

}

void pdp_xwindow_init(t_pdp_xwindow *xwin)
{
    xwin->xdisplay = 0;

    xwin->winwidth = 320;
    xwin->winheight = 240;
    xwin->winxoffset = 0;
    xwin->winyoffset = 0;

    xwin->initialized = 0;

    xwin->cursor = 0;
    //xwin->dragbutton = gensym("drag1");

}
t_pdp_xwindow *pdp_xwindow_new(void)
{
    t_pdp_xwindow *xwin = pdp_alloc(sizeof(*xwin));
    pdp_xwindow_init(xwin);
    return xwin;
}
    

void pdp_xwindow_close(t_pdp_xwindow *xwin)
{
    
    XEvent e;

    if (xwin->initialized){
	XFreeGC(xwin->xdisplay->dpy, xwin->gc);
	XDestroyWindow(xwin->xdisplay->dpy, xwin->win);
	while(XPending(xwin->xdisplay->dpy)) XNextEvent(xwin->xdisplay->dpy, &e);
	pdp_xdisplay_unregister_window(xwin->xdisplay, xwin);
	xwin->xdisplay = 0;
	xwin->initialized = 0;
    }

}

void pdp_xwindow_cleanup(t_pdp_xwindow *x)
{
    // close win
    pdp_xwindow_close(x);

    // no more dynamic data to free
}

void pdp_xwindow_free(t_pdp_xwindow *xwin)
{
    pdp_xwindow_cleanup(xwin);
    pdp_dealloc(xwin);
}


