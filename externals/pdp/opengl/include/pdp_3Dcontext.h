/*
 *   pdp system module - 3d render context packet type
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


/* the 3d render context packet: platform independent data structure
   and method prototypes */


#ifndef PDP_3DCONTEXT_H
#define PDP_3DCONTEXT_H

#include "pdp.h"




typedef struct _3dcontext
{
    u32 encoding;            /* the kind of render context */
    u32 width;               /* context width */
    u32 height;              /* context height */
    u32 sub_width;           /* portion that is currently used */
    u32 sub_height;
    void *drawable;          /* context's drawable (i.e. Window, GLXPbuffer, ...) */
    void *context;           /* context's context object */

} t_3Dcontext;

#define PDP_3DCONTEXT         5  /* 3d context packet id */
#define PDP_3DCONTEXT_WINDOW  1  /* window context packet id */
#define PDP_3DCONTEXT_PBUFFER 2  /* pbuf context packet id */

/* all symbols are C-style */
#ifdef __cplusplus
extern "C"
{
#endif

    /* info methods */
    u32 pdp_packet_3Dcontext_width(int packet);
    u32 pdp_packet_3Dcontext_subwidth(int packet);
    u32 pdp_packet_3Dcontext_height(int packet);
    u32 pdp_packet_3Dcontext_subheight(int packet);
    float pdp_packet_3Dcontext_subaspect(int packet);
    int pdp_packet_3Dcontext_isvalid(int packet);
    t_3Dcontext *pdp_packet_3Dcontext_info(int packet);


    /* setters */
    void  pdp_packet_3Dcontext_set_subwidth(int packet, u32 w);
    void  pdp_packet_3Dcontext_set_subheight(int packet, u32 h);


    /* render context activation and initialization */
    void pdp_packet_3Dcontext_set_rendering_context(int packet);
    void pdp_packet_3Dcontext_unset_rendering_context(int packet);
    void pdp_packet_3Dcontext_setup_3d_context(int p);
    void pdp_packet_3Dcontext_setup_2d_context(int p);

    /* constructors */
    int pdp_packet_new_3Dcontext_pbuf(u32 width, u32 height, u32 depth);
    int pdp_packet_new_3Dcontext_win(void);

    /* window specific methods */
    void pdp_packet_3Dcontext_win_resize(int packet, int width, int height);
    t_pdp_list *pdp_packet_3Dcontext_win_get_eventlist(int packet);
    void pdp_packet_3Dcontext_win_cursor(int packet, bool toggle);
    void pdp_packet_3Dcontext_win_swapbuffers(int packet);

    /* converters */
    int pdp_packet_3Dcontext_snap_to_bitmap(int packet, int w, int h);

#ifdef __cplusplus
}
#endif

#endif
