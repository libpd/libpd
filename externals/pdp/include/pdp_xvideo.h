
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
#include <X11/extensions/Xv.h>
#include <X11/extensions/Xvlib.h>

// image formats for communication with the X Server
#define FOURCC_YV12 0x32315659  /* YV12   YUV420P */
#define FOURCC_YUV2 0x32595559  /* YUV2   YUV422 */
#define FOURCC_I420 0x30323449  /* I420   Intel Indeo 4 */



/* xvideo class */
typedef struct _pdp_xvideo
{

    t_pdp_xdisplay *xdpy;
    t_pdp_xwindow  *xwin;
    //Display *dpy;
    //int screen;
    //Window win;
    


    int xv_format;
    int xv_port;

    XvImage *xvi;
    unsigned char *data;
    unsigned int width;
    unsigned int height;
    int last_encoding;

    int  initialized;

} t_pdp_xvideo;


/* cons */
void pdp_xvideo_init(t_pdp_xvideo *x);
t_pdp_xvideo *pdp_xvideo_new(void);

/* des */
void pdp_xvideo_cleanup(t_pdp_xvideo* x);
void pdp_xvideo_free(t_pdp_xvideo* x);


/* open an xv port (and create XvImage) */
int pdp_xvideo_open_on_display(t_pdp_xvideo *x, t_pdp_xdisplay *d);

/* close xv port (and delete XvImage */
void pdp_xvideo_close(t_pdp_xvideo* x);

/* display a packet */
void pdp_xvideo_display_packet(t_pdp_xvideo *x, t_pdp_xwindow *w, int packet);
