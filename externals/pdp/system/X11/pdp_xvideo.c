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
#include "pdp_xvideo.h"
#include "pdp_post.h"
#include "pdp_packet.h"

#define D if(0)




/************************************* PDP_XVIDEO ************************************/

static void pdp_xvideo_create_xvimage(t_pdp_xvideo *xvid, int width, int height)
{
    int i;
    long size;
    
    //post("pdp_xvideo_create_xvimage");

    xvid->width = width;
    xvid->height = height;
    size = (xvid->width * xvid->height + (((xvid->width>>1)*(xvid->height>>1))<<1));
    //post("create xvimage %d %d", xvid->width, xvid->height);
    xvid->data = (unsigned char *)pdp_alloc(size);
    for (i=0; i<size; i++) xvid->data[i] = i;
    xvid->xvi = XvCreateImage(xvid->xdpy->dpy, xvid->xv_port, xvid->xv_format, (char *)xvid->data, xvid->width, xvid->height);
    xvid->last_encoding = -1;
    if ((!xvid->xvi) || (!xvid->data)) pdp_post ("ERROR CREATING XVIMAGE");
    //pdp_post("created xvimag data:%x xvi:%x",xvid->data,xvid->xvi);

}

static void pdp_xvideo_destroy_xvimage(t_pdp_xvideo *xvid)
{
    if(xvid->data) pdp_dealloc(xvid->data);
    if (xvid->xvi) XFree(xvid->xvi);
    xvid->xvi = 0;
    xvid->data = 0;
}

void pdp_xvideo_display_packet(t_pdp_xvideo *xvid, t_pdp_xwindow *xwin, int packet)
{
    t_pdp *header = pdp_packet_header(packet);
    void *data = pdp_packet_data(packet);
    t_bitmap * bm = pdp_packet_bitmap_info(packet);
    unsigned int width, height, encoding, size, nbpixels;

    /* some checks: only display when initialized and when pacet is bitmap YV12 */
    if (!xvid->initialized) return;
    if (!header) return;
    if (!bm) return;

    width = bm->width;
    height = bm->height;
    encoding = bm->encoding;
    size = (width * height + (((width>>1)*(height>>1))<<1));
    nbpixels = width * height;

    if (PDP_BITMAP != header->type) return;
    if (PDP_BITMAP_YV12 != encoding) return;

    /* check if xvimage needs to be recreated */
    if ((width != xvid->width) || (height != xvid->height)){
	//pdp_post("pdp_xv: replace image");
	pdp_xvideo_destroy_xvimage(xvid);
	pdp_xvideo_create_xvimage(xvid, width, height);
    }

    /* copy the data to the XvImage buffer */
    memcpy(xvid->data, data, size);

    /* display */
    XvPutImage(xvid->xdpy->dpy,xvid->xv_port, xwin->win,xwin->gc,xvid->xvi, 
               0,0,xvid->width,xvid->height, 0,0,xwin->winwidth,xwin->winheight);
    XFlush(xvid->xdpy->dpy);


   
}



void pdp_xvideo_close(t_pdp_xvideo* xvid)
{
    if (xvid->initialized){
	if (xvid->xvi) pdp_xvideo_destroy_xvimage(xvid);
	XvUngrabPort(xvid->xdpy->dpy, xvid->xv_port, CurrentTime);
	xvid->xv_port = 0;
	xvid->xdpy = 0;
	xvid->last_encoding = -1;
	xvid->initialized = false;
    }
}

void pdp_xvideo_cleanup(t_pdp_xvideo* xvid)
{
    // close xvideo port (and delete XvImage)
    pdp_xvideo_close(xvid);

    // no more dynamic data to free

}

void pdp_xvideo_free(t_pdp_xvideo* xvid){
    pdp_xvideo_cleanup(xvid);
    pdp_dealloc(xvid);
}

void pdp_xvideo_init(t_pdp_xvideo *xvid)
{

    xvid->xdpy = 0;

    xvid->xv_format = FOURCC_YV12;
    xvid->xv_port      = 0;

    xvid->width = 320;
    xvid->height = 240;

    xvid->data = 0;
    xvid->xvi = 0;

    xvid->initialized = 0;
    xvid->last_encoding = -1;

}
t_pdp_xvideo *pdp_xvideo_new(void)
{
    t_pdp_xvideo *xvid = pdp_alloc(sizeof(*xvid));
    pdp_xvideo_init(xvid);
    return xvid;
}

int pdp_xvideo_open_on_display(t_pdp_xvideo *xvid, t_pdp_xdisplay *d)
{
    unsigned int ver, rel, req, ev, err, i, j;
    unsigned int adaptors;
    int formats;
    XvAdaptorInfo        *ai;

    if (xvid->initialized) return 1;
    if (!d) return 0;
    xvid->xdpy = d;
    
    if (Success != XvQueryExtension(xvid->xdpy->dpy,&ver,&rel,&req,&ev,&err))	return 0;

    /* find + lock port */
    if (Success != XvQueryAdaptors(xvid->xdpy->dpy,DefaultRootWindow(xvid->xdpy->dpy),&adaptors,&ai))
	return 0;
    for (i = 0; i < adaptors; i++) {
	if ((ai[i].type & XvInputMask) && (ai[i].type & XvImageMask)) {
	    for (j=0; j < ai[i].num_ports; j++){
		if (Success != XvGrabPort(xvid->xdpy->dpy,ai[i].base_id+j,CurrentTime)) {
		    //fprintf(stderr,"INFO: Xvideo port %ld on adapter %d: is busy, skipping\n",ai[i].base_id+j, i);
		}
		else {
		    xvid->xv_port = ai[i].base_id + j;
		    goto breakout;
		}
	    }
	}
    }


 breakout:

    XFree(ai);
    if (0 == xvid->xv_port) return 0;
    pdp_post("pdp_xvideo: grabbed port %d on adaptor %d", xvid->xv_port, i);
    xvid->initialized = 1;
    pdp_xvideo_create_xvimage(xvid, xvid->width, xvid->height);
    return 1;
}


