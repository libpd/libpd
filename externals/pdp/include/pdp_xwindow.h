
/*
 *   Pure Data Packet header file: xwindow glue code
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


// x stuff
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include "pdp_list.h"
#include "pdp_mem.h"

/* x display class */
typedef struct _pdp_xdisplay
{
    Display *dpy;              // the display connection
    int screen;                // the screen
    t_pdp_list *windowlist;    // all windows belonging to this connection
                               // this contains (id, eventlist)

    int dragbutton;

} t_pdp_xdisplay;


/* cons */
t_pdp_xdisplay *pdp_xdisplay_new(char *dpy_string);

/* des */
void pdp_xdisplay_free(t_pdp_xdisplay *d);

struct _pdp_xwindow;

void pdp_xdisplay_register_window(t_pdp_xdisplay *d, struct _pdp_xwindow *w);
void pdp_xdisplay_unregister_window(t_pdp_xdisplay *d, struct _pdp_xwindow *w);

/* x window class */
typedef struct _pdp_xwindow
{
    //Display *dpy;
    //int screen;
    t_pdp_xdisplay *xdisplay; // the display object
    Window win;               // window reference
    GC gc;                    // graphics context
    Atom WM_DELETE_WINDOW;


    int winwidth;             // dim states
    int winheight;
    int winxoffset;
    int winyoffset;

    int  initialized;
    int  autocreate;

    char lastbut; // last button pressed (for drag)

    //t_symbol *dragbutton;
    
    float cursor;

} t_pdp_xwindow;

/* cons */
void pdp_xwindow_init(t_pdp_xwindow *b);
t_pdp_xwindow *pdp_xwindow_new(void);

/* des */    
void pdp_xwindow_cleanup(t_pdp_xwindow *b);
void pdp_xwindow_free(t_pdp_xwindow *b);

/* move the pointer */
void pdp_xwindow_warppointer(t_pdp_xwindow *xwin, int x, int y);


/* fullscreen message */
void pdp_xwindow_fullscreen(t_pdp_xwindow *xwin);

/* resize window */
void pdp_xwindow_resize(t_pdp_xwindow *b, int width, int height);

/* resize window */
void pdp_xwindow_moveresize(t_pdp_xwindow *b, int xoffset, int yoffset, int width, int height);

/* fill a tile of the screen */
void pdp_xwindow_tile(t_pdp_xwindow *xwin, int x_tiles, int y_tiles, int i, int j);

/* move window */
void pdp_xwindow_move(t_pdp_xwindow *xwin, int xoffset, int yoffset);

/* receive events */
t_pdp_list *pdp_xwindow_get_eventlist(t_pdp_xwindow *xwin);

/* enable/disable cursor */
void pdp_xwindow_cursor(t_pdp_xwindow *b, int flag);

/* create xwindow. return code != NULL on succes */
int pdp_xwindow_create_on_display(t_pdp_xwindow *b, t_pdp_xdisplay *d);

/* close window */    
void pdp_xwindow_close(t_pdp_xwindow *b);

/* set title */
void pdp_xwindow_title(t_pdp_xwindow *xwin, char *title);


