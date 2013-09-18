/* Copyright (c) 2002 Yves Degoyon.                                             */
/* For information on usage and redistribution, and for a DISCLAIMER OF ALL     */
/* WARRANTIES, see the file, "LICENSE.txt," in this distribution.               */
/*                                                                              */
/* exciter : a graphical object which enables                                  */
/* to schedule bang events on a time scale                                      */
/*                                                                              */
/* This program is free software; you can redistribute it and/or                */
/* modify it under the terms of the GNU General Public License                  */
/* as published by the Free Software Foundation; either version 2               */
/* of the License, or (at your option) any later version.                       */
/*                                                                              */
/* See file LICENSE for further informations on licensing terms.                */
/*                                                                              */
/* This program is distributed in the hope that it will be useful,              */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of               */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the                */
/* GNU General Public License for more details.                                 */
/*                                                                              */
/* You should have received a copy of the GNU General Public License            */
/* along with this program; if not, write to the Free Software                  */
/* Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.  */
/*                                                                              */
/* Based on PureData by Miller Puckette and others.                             */
/*                                                                              */
/* Made while listening to :                                                    */
/*                                                                              */
/* "You're a maniac"                                                            */
/* "To fullfill the emptyness"                                                  */
/* Mecano ( no, not the spanish ones !!! ) - Escape The Human Myth              */
/* ---------------------------------------------------------------------------- */


#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <ctype.h>
#include <time.h>
#include <sys/time.h>
#include "m_pd.h"
#include "m_imp.h"
#include "g_canvas.h"

#include "exciter.h"

#ifdef _WIN32
#include <io.h>
#else
#include <unistd.h>
#endif

/* needed to create a exciter from PD's menu
void canvas_objtext(t_glist *gl, int xpos, int ypos, int selected, t_binbuf *b);
void canvas_startmotion(t_canvas *x);
*/

#define DEFAULT_EXCITER_WIDTH 200
#define DEFAULT_EXCITER_HEIGHT 200
#define DEFAULT_EXCITER_GRAIN 0.1
#define DEFAULT_EXCITER_NBEVENTS 8

#define EXCITER_PIXEL_GRAIN 5

static char   *exciter_version = "exciter: a bang-events sequencer, version 0.5 (ydegoyon@free.fr)";

t_widgetbehavior exciter_widgetbehavior;
static t_class *exciter_class;
static int excitercount=0;

static int guidebug=0;
static int pointsize = 5;

#define SYS_VGUI2(a,b) if (guidebug) \
                         post(a,b);\
                         sys_vgui(a,b)

#define SYS_VGUI3(a,b,c) if (guidebug) \
                         post(a,b,c);\
                         sys_vgui(a,b,c)

#define SYS_VGUI4(a,b,c,d) if (guidebug) \
                         post(a,b,c,d);\
                         sys_vgui(a,b,c,d)

#define SYS_VGUI5(a,b,c,d,e) if (guidebug) \
                         post(a,b,c,d,e);\
                         sys_vgui(a,b,c,d,e)

#define SYS_VGUI6(a,b,c,d,e,f) if (guidebug) \
                         post(a,b,c,d,e,f);\
                         sys_vgui(a,b,c,d,e,f)

#define SYS_VGUI7(a,b,c,d,e,f,g) if (guidebug) \
                         post(a,b,c,d,e,f,g);\
                         sys_vgui(a,b,c,d,e,f,g)

#define SYS_VGUI8(a,b,c,d,e,f,g,h) if (guidebug) \
                         post(a,b,c,d,e,f,g,h);\
                         sys_vgui(a,b,c,d,e,f,g,h)

#define SYS_VGUI9(a,b,c,d,e,f,g,h,i) if (guidebug) \
                         post(a,b,c,d,e,f,g,h,i);\
                         sys_vgui(a,b,c,d,e,f,g,h,i)

#define SYS_VGUI13(a,b,c,d,e,f,g,h,i,j,k,l,m) if (guidebug) \
                         post(a,b,c,d,e,f,g,h,i,j,k,l,m);\
                         sys_vgui(a,b,c,d,e,f,g,h,i,j,k,l,m)

/* drawing functions */

/* draw an event */
static void exciter_draw_gem(t_exciter *x, t_glist *glist, t_int ix, t_int iy)
{
    t_int polyray = ( x->x_height / x->x_nbevents) / 2;
    t_canvas *canvas=glist_getcanvas(glist);

    SYS_VGUI13(".x%lx.c create polygon %d %d %d %d %d %d %d %d -outline #000000 -fill #FFFFFF -tags %xEVENT%.4d%.4d\n",
               canvas,
               text_xpix(&x->x_obj, glist) + ix*EXCITER_PIXEL_GRAIN,
               text_ypix(&x->x_obj, glist) + x->x_height - (iy+1)*x->x_height/x->x_nbevents + 2*polyray,
               text_xpix(&x->x_obj, glist) + ix*EXCITER_PIXEL_GRAIN + polyray,
               text_ypix(&x->x_obj, glist) + x->x_height - (iy+1)*x->x_height/x->x_nbevents + polyray,
               text_xpix(&x->x_obj, glist) + ix*EXCITER_PIXEL_GRAIN + 2*polyray,
               text_ypix(&x->x_obj, glist) + x->x_height - (iy+1)*x->x_height/x->x_nbevents,
               text_xpix(&x->x_obj, glist) + ix*EXCITER_PIXEL_GRAIN + polyray,
               text_ypix(&x->x_obj, glist) + x->x_height - (iy+1)*x->x_height/x->x_nbevents - polyray,
               x, ix, iy);
}

/* delete an event */
static void exciter_delete_gem(t_exciter *x, t_glist *glist, t_int ix, t_int iy)
{
    t_canvas *canvas=glist_getcanvas(glist);

    SYS_VGUI5(".x%lx.c delete %xEVENT%.4d%.4d\n", canvas, x, ix, iy );
}

static void exciter_draw_update(t_exciter *x, t_glist *glist)
{
    int ei, gi;

    t_canvas *canvas=glist_getcanvas(glist);
    for ( ei=0; ei<x->x_nbevents; ei++ )
    {
        for ( gi=0; gi<(x->x_width/EXCITER_PIXEL_GRAIN); gi++ )
        {
            if( *(x->x_sbangs+ei*(x->x_width/EXCITER_PIXEL_GRAIN)+gi) == 1 )
            {
                exciter_draw_gem(x, glist, gi, ei );
            }
            else
            {
                exciter_delete_gem(x, glist, gi, ei );
            }
        }
    }
}

static void exciter_draw_new(t_exciter *x, t_glist *glist)
{
    t_canvas *canvas=glist_getcanvas(glist);
    t_int ei;

    SYS_VGUI7(".x%lx.c create rectangle %d %d %d %d -outline #000000 -fill #902181 -tags %xLINE\n",
              canvas, text_xpix(&x->x_obj, glist), text_ypix(&x->x_obj, glist),
              text_xpix(&x->x_obj, glist)+x->x_width, text_ypix(&x->x_obj, glist)+x->x_height,
              x);
    SYS_VGUI5(".x%lx.c create text %d %d -font -*-courier-bold--normal--10-* -text \"0 s\" -tags %xLOWERCAPTION\n",
              canvas, text_xpix(&x->x_obj, glist), text_ypix(&x->x_obj, glist) + x->x_height + 10, x );
    SYS_VGUI6(".x%lx.c create text %d %d -font -*-courier-bold--normal--10-* -text \"%.2f s\" -tags %xHIGHERCAPTION\n",
              canvas, text_xpix(&x->x_obj, glist) + x->x_width, text_ypix(&x->x_obj, glist) + x->x_height + 10,
              x->x_width/EXCITER_PIXEL_GRAIN*x->x_timegrain , x);
    SYS_VGUI7(".x%lx.c create rectangle %d %d %d %d -outline #000000 -fill #000000 -tags %xIN\n",
              canvas, text_xpix(&x->x_obj, glist),
              text_ypix(&x->x_obj, glist) - 2,
              text_xpix(&x->x_obj, glist) + 5,
              text_ypix(&x->x_obj, glist) ,
              x);
    if ( x->x_nbevents > 1 )
    {
        for ( ei=0; ei<x->x_nbevents; ei++ )
        {
            SYS_VGUI8(".x%lx.c create rectangle %d %d %d %d -outline #000000 -fill #000000 -tags %xOUT%d\n",
                      canvas, text_xpix(&x->x_obj, glist) + ( ei * (x->x_width - 5) )/ (x->x_nbevents-1),
                      text_ypix(&x->x_obj, glist) + x->x_height,
                      text_xpix(&x->x_obj, glist) + ( ei * (x->x_width - 5) )/ (x->x_nbevents-1) + 5,
                      text_ypix(&x->x_obj, glist) + x->x_height + 2,
                      x, ei);
        }
    }
    else
    {
        SYS_VGUI8(".x%lx.c create rectangle %d %d %d %d -outline #000000 -fill #000000 -tags %xOUT%d\n",
                  canvas, text_xpix(&x->x_obj, glist),
                  text_ypix(&x->x_obj, glist) + x->x_height,
                  text_xpix(&x->x_obj, glist) + 5,
                  text_ypix(&x->x_obj, glist) + x->x_height + 2,
                  x, 0);
    }
    canvas_fixlinesfor( canvas, (t_text*)x );
}

static void exciter_draw_move(t_exciter *x, t_glist *glist)
{
    t_canvas *canvas=glist_getcanvas(glist);
    t_int polyray = ( x->x_height / x->x_nbevents) / 2;
    t_int ei, gi;

    SYS_VGUI7(".x%lx.c coords %xLINE %d %d %d %d \n",
              canvas, x, text_xpix(&x->x_obj, glist), text_ypix(&x->x_obj, glist),
              text_xpix(&x->x_obj, glist)+x->x_width, text_ypix(&x->x_obj, glist)+x->x_height
             );
    SYS_VGUI5(".x%lx.c coords %xLOWERCAPTION %d %d\n",
              canvas, x, text_xpix(&x->x_obj, glist), text_ypix(&x->x_obj, glist) + x->x_height + 10 );
    SYS_VGUI5(".x%lx.c coords %xHIGHERCAPTION %d %d\n",
              canvas, x, text_xpix(&x->x_obj, glist) + x->x_width, text_ypix(&x->x_obj, glist) + x->x_height + 10);
    SYS_VGUI7(".x%lx.c coords %xIN %d %d %d %d\n",
              canvas, x, text_xpix(&x->x_obj, glist),
              text_ypix(&x->x_obj, glist) - 2,
              text_xpix(&x->x_obj, glist) + 5,
              text_ypix(&x->x_obj, glist)
             );
    if ( x->x_nbevents > 1 )
    {
        for ( ei=0; ei<x->x_nbevents; ei++ )
        {
            SYS_VGUI8(".x%lx.c coords %xOUT%d %d %d %d %d\n",
                      canvas, x, ei, text_xpix(&x->x_obj, glist) + ( ei * (x->x_width - 5) )/ (x->x_nbevents-1),
                      text_ypix(&x->x_obj, glist) + x->x_height,
                      text_xpix(&x->x_obj, glist) + ( ei * (x->x_width - 5) )/ (x->x_nbevents-1) + 5,
                      text_ypix(&x->x_obj, glist) + x->x_height + 2
                     );
        }
    }
    else
    {
        SYS_VGUI8(".x%lx.c coords %xOUT%d %d %d %d %d\n",
                  canvas, x, 0, text_xpix(&x->x_obj, glist),
                  text_ypix(&x->x_obj, glist) + x->x_height,
                  text_xpix(&x->x_obj, glist) + 5,
                  text_ypix(&x->x_obj, glist) + x->x_height + 2
                 );
    }
    for ( ei=0; ei<x->x_nbevents; ei++ )
    {
        for ( gi=0; gi<(x->x_width/EXCITER_PIXEL_GRAIN); gi++ )
        {
            SYS_VGUI13(".x%lx.c coords %xEVENT%.4d%.4d %d %d %d %d %d %d %d %d\n",
                       canvas, x, gi, ei,
                       text_xpix(&x->x_obj, glist) + gi*EXCITER_PIXEL_GRAIN,
                       text_ypix(&x->x_obj, glist) + x->x_height - (ei+1)*x->x_height/x->x_nbevents + 2*polyray,
                       text_xpix(&x->x_obj, glist) + gi*EXCITER_PIXEL_GRAIN + polyray,
                       text_ypix(&x->x_obj, glist) + x->x_height - (ei+1)*x->x_height/x->x_nbevents + polyray,
                       text_xpix(&x->x_obj, glist) + gi*EXCITER_PIXEL_GRAIN + 2*polyray,
                       text_ypix(&x->x_obj, glist) + x->x_height - (ei+1)*x->x_height/x->x_nbevents,
                       text_xpix(&x->x_obj, glist) + gi*EXCITER_PIXEL_GRAIN + polyray,
                       text_ypix(&x->x_obj, glist) + x->x_height - (ei+1)*x->x_height/x->x_nbevents - polyray
                      );
        }
    }
    canvas_fixlinesfor( canvas, (t_text*)x );
}

static void exciter_draw_erase(t_exciter* x,t_glist* glist)
{
    t_canvas *canvas=glist_getcanvas(glist);
    int i, ei, gi;

    SYS_VGUI3(".x%lx.c delete %xLINE\n", canvas, x );
    SYS_VGUI3(".x%lx.c delete %xLOWERCAPTION\n", canvas, x );
    SYS_VGUI3(".x%lx.c delete %xHIGHERCAPTION\n", canvas, x );
    SYS_VGUI3(".x%lx.c delete %xIN\n", canvas, x );
    for ( ei=0; ei<x->x_nbevents; ei++ )
    {
        for ( gi=0; gi<(x->x_width/EXCITER_PIXEL_GRAIN); gi++ )
        {
            if( *(x->x_sbangs+ei*(x->x_width/EXCITER_PIXEL_GRAIN)+gi) != 0 )
            {
                exciter_delete_gem( x, glist, gi, ei );
            }
        }
        SYS_VGUI4(".x%lx.c delete %xOUT%d\n", canvas, x, ei );
    }
}

static void exciter_draw_select(t_exciter* x,t_glist* glist)
{
    t_canvas *canvas=glist_getcanvas(glist);

    if(x->x_selected)
    {
        /* sets the main item in blue */
        SYS_VGUI3(".x%lx.c itemconfigure %xLINE -outline #0000FF\n", canvas, x);

    }
    else
    {
        /* sets the main item in black */
        SYS_VGUI3(".x%lx.c itemconfigure %xLINE -outline #000000\n", canvas, x);
    }
}

/* ------------------------ exciter widgetbehaviour----------------------------- */


static void exciter_getrect(t_gobj *z, t_glist *owner,
                            int *xp1, int *yp1, int *xp2, int *yp2)
{
    t_exciter* x = (t_exciter*)z;

    *xp1 = text_xpix(&x->x_obj, owner);
    *yp1 = text_ypix(&x->x_obj, owner);
    *xp2 = text_xpix(&x->x_obj, owner)+x->x_width;
    *yp2 = text_ypix(&x->x_obj, owner)+x->x_height;
}

static void exciter_save(t_gobj *z, t_binbuf *b)
{
    t_exciter *x = (t_exciter *)z;
    int ei,gi;

    binbuf_addv(b, "ssiisiiifii", gensym("#X"),gensym("obj"),
                (t_int)x->x_obj.te_xpix, (t_int)x->x_obj.te_ypix,
                atom_getsymbol(binbuf_getvec(x->x_obj.te_binbuf)),
                x->x_width, x->x_height,
                x->x_nbevents, x->x_timegrain,
                x->x_loop, x->x_save
               );
    if ( x->x_save )
    {
        for ( ei=0; ei<x->x_nbevents; ei++ )
        {
            for ( gi=0; gi<(x->x_width/EXCITER_PIXEL_GRAIN); gi++ )
            {
                if( *(x->x_sbangs+ei*(x->x_width/EXCITER_PIXEL_GRAIN)+gi) == 1 )
                {
                    // post( "exciter : saving ( %d, %d )", ei, gi );
                    binbuf_addv(b, "ii", ei, gi );
                }
            }
        }
    }
    binbuf_addv(b, ";");
}

static void exciter_properties(t_gobj *z, t_glist *owner)
{
    char buf[800];
    t_exciter *x=(t_exciter *)z;

    sprintf(buf, "pdtk_exciter_dialog %%s %d %d %d %.2f %d %d\n",
            x->x_width, x->x_height, x->x_nbevents, x->x_timegrain, x->x_loop, x->x_save );
    // post("exciter_properties : %s", buf );
    gfxstub_new(&x->x_obj.ob_pd, x, buf);
}

static void exciter_select(t_gobj *z, t_glist *glist, int selected)
{
    t_exciter *x = (t_exciter *)z;

    x->x_selected = selected;
    exciter_draw_select( x, glist );
}

static void exciter_vis(t_gobj *z, t_glist *glist, int vis)
{
    t_exciter *x = (t_exciter *)z;

    post("exciter_vis : %d", vis );
    if (vis)
    {
        exciter_draw_new( x, glist );
        exciter_draw_update( x, glist );
    }
    else
    {
        exciter_draw_erase( x, glist );
    }
}

/* resuming the triggering of events */
static void exciter_resume(t_exciter *x)
{
    x->x_started = 1;
}

/* pausing the triggering of events */
static void exciter_pause(t_exciter *x)
{
    x->x_started = 0;
}

/* trigger events -- no dsp processing */
static void exciter_dialog(t_exciter *x, t_symbol *s, int argc, t_atom *argv)
{
    t_int onbevents, owidth, ei, gi, bi;
    t_canvas *canvas=glist_getcanvas(x->x_glist);

    if ( !x )
    {
        post( "exciter : error :tried to set properties on an unexisting object" );
    }
    onbevents = x->x_nbevents;
    owidth = x->x_width;
    exciter_draw_erase(x, x->x_glist);
    if ( argc < 6 )
    {
        post( "exciter : error in the number of arguments ( %d )", argc );
        return;
    }
    if ( argv[0].a_type != A_FLOAT || argv[1].a_type != A_FLOAT ||
            argv[2].a_type != A_FLOAT || argv[3].a_type != A_FLOAT ||
            argv[4].a_type != A_FLOAT || argv[5].a_type != A_FLOAT )
    {
        post( "exciter : wrong arguments" );
        return;
    }

    exciter_pause( x );
    x->x_width = argv[0].a_w.w_float;
    if ( x->x_width <= 0 ) x->x_width = 100;
    x->x_height = argv[1].a_w.w_float;
    if ( x->x_height <= 0 ) x->x_height = 100;
    x->x_nbevents = argv[2].a_w.w_float;
    if ( x->x_nbevents < 1 ) x->x_nbevents = 1;
    x->x_timegrain = argv[3].a_w.w_float;
    if ( x->x_timegrain < 0.01 )
    {
        post ("exciter : incorrect time grain : forced to 1 tick ( 10 ms )" );
        x->x_timegrain = 0.01;
    }
    x->x_loop = argv[4].a_w.w_float;
    x->x_save = argv[5].a_w.w_float;
    x->x_started = 0;
    x->x_reltime = 0L;
    x->x_plooptime = 0L;
    x->x_gindex = -1;
    x->x_looplength = x->x_timegrain * x->x_width * 1000 / EXCITER_PIXEL_GRAIN;

    // re-allocate arrays and keep old events
    post( "exciter : re-allocate events" );
    if ( onbevents != x->x_nbevents || owidth != x->x_width )
    {
        t_int mevents = ( onbevents > x->x_nbevents ) ? x->x_nbevents : onbevents;
        t_int mwidth = ( owidth > x->x_width ) ? x->x_width : owidth;
        t_int *newbangs;

        newbangs = (t_int*) getbytes( x->x_nbevents*x->x_width/EXCITER_PIXEL_GRAIN*sizeof(t_int) );
        memset( newbangs, 0x0, x->x_nbevents*x->x_width/EXCITER_PIXEL_GRAIN*sizeof(t_int) );
        for ( ei=0; ei<mevents; ei++ )
        {
            for ( gi=0; gi<(mwidth/EXCITER_PIXEL_GRAIN); gi++ )
            {
                *(newbangs+ei*(x->x_width/EXCITER_PIXEL_GRAIN)+gi ) =
                    *(x->x_sbangs+ei*(owidth/EXCITER_PIXEL_GRAIN)+gi );
            }
        }
        if ( x->x_sbangs )
            freebytes( x->x_sbangs, onbevents*owidth/EXCITER_PIXEL_GRAIN*sizeof(t_int) );
        x->x_sbangs = newbangs;
    }

    // re-allocate outlets
    post( "exciter : re-allocate outlets" );
    if ( onbevents != x->x_nbevents )
    {
        post( "exciter : cleaning up old outlets" );
        if ( x->x_bangs )
        {
            for ( ei=0; ei<onbevents; ei++ )
            {
                outlet_free( x->x_bangs[ei] );
            }
            freebytes( x->x_bangs, onbevents*sizeof(t_outlet*) );
        }
        post( "exciter : creating new ones" );
        x->x_bangs = (t_outlet **) getbytes( x->x_nbevents*sizeof(t_outlet **) );
        for ( bi=0; bi<x->x_nbevents; bi++ )
        {
            x->x_bangs[bi] = outlet_new( &x->x_obj, &s_bang );
        }
    }
    canvas_fixlinesfor( canvas, (t_text*)x );
    exciter_draw_new(x, x->x_glist);
    exciter_draw_update(x, x->x_glist);
    exciter_resume( x );
}

static void exciter_delete(t_gobj *z, t_glist *glist)
{
    canvas_deletelinesfor(glist, (t_text *)z);
}

static void exciter_displace(t_gobj *z, t_glist *glist, int dx, int dy)
{
    t_exciter *x = (t_exciter *)z;
    t_int xold = text_xpix(&x->x_obj, glist);
    t_int yold = text_ypix(&x->x_obj, glist);

    // post( "exciter_displace dx=%d dy=%d", dx, dy );

    x->x_obj.te_xpix += dx;
    x->x_obj.te_ypix += dy;
    if(xold != text_xpix(&x->x_obj, glist) || yold != text_ypix(&x->x_obj, glist))
    {
        exciter_draw_move(x, glist);
    }
}

static int exciter_click(t_gobj *z, struct _glist *glist,
                         int xpix, int ypix, int shift, int alt, int dbl, int doit)
{
    t_exciter* x = (t_exciter *)z;
    t_int nevent, npix;

    if ( doit)
    {
        nevent = ( 1 - ( ( ypix - ( (float)x->x_height / (float) x->x_nbevents / 2 )
                           - text_ypix(&x->x_obj, glist)) / (float)x->x_height ) )*(x->x_nbevents-1);
        npix = ( xpix - 1  - text_xpix(&x->x_obj, glist)) / EXCITER_PIXEL_GRAIN;
        // post( "exciter : selected event (%d,%d)", nevent, npix );
        // set or unset event
        {
            if ( *(x->x_sbangs+nevent*(x->x_width/EXCITER_PIXEL_GRAIN)+npix ) == 1 )
            {
                *(x->x_sbangs+nevent*(x->x_width/EXCITER_PIXEL_GRAIN)+npix ) = 0;
                exciter_delete_gem( x, glist, npix, nevent );
            }
            else
            {
                *(x->x_sbangs+nevent*(x->x_width/EXCITER_PIXEL_GRAIN)+npix ) = 1;
                exciter_draw_gem( x, glist, npix, nevent );
            }
        }
    }
    return (1);
}

static t_exciter *exciter_new(t_symbol *s, int argc, t_atom *argv)
{
    int bi, i, ei, gi;
    t_exciter *x;
    t_pd *x2;

    // post( "exciter_new : create : %s argc =%d", s->s_name, argc );

    x = (t_exciter *)pd_new(exciter_class);
    x->x_glist = (t_glist *) canvas_getcurrent();
    // new exciter created from the gui
    if ( argc != 0 )
    {
        if ( argc < 6 )
        {
            post( "exciter : error in the number of arguments ( %d )", argc );
            return NULL;
        }
        if ( argv[0].a_type != A_FLOAT || argv[1].a_type != A_FLOAT ||
                argv[2].a_type != A_FLOAT || argv[3].a_type != A_FLOAT ||
                argv[4].a_type != A_FLOAT || argv[5].a_type != A_FLOAT )
        {
            post( "exciter : wrong arguments" );
            return NULL;
        }

        x->x_width = argv[0].a_w.w_float;
        x->x_height = argv[1].a_w.w_float;
        x->x_nbevents = argv[2].a_w.w_float;
        if ( x->x_nbevents < 1 ) x->x_nbevents = 1;
        x->x_timegrain = argv[3].a_w.w_float;
        x->x_loop = argv[4].a_w.w_float;
        x->x_save = argv[5].a_w.w_float;
    }
    else
    {
        x->x_width = DEFAULT_EXCITER_WIDTH;
        x->x_height = DEFAULT_EXCITER_HEIGHT;
        x->x_nbevents = DEFAULT_EXCITER_NBEVENTS;
        x->x_timegrain = DEFAULT_EXCITER_GRAIN;
        x->x_loop = 1;
        x->x_save = 1;

    }

    // common fields for new and restored exciters
    x->x_sbangs = (t_int*) getbytes( x->x_nbevents*x->x_width/EXCITER_PIXEL_GRAIN*sizeof(t_int) );
    for ( ei=0; ei<x->x_nbevents; ei++ )
    {
        for ( gi=0; gi<(x->x_width/EXCITER_PIXEL_GRAIN); gi++ )
        {
            *(x->x_sbangs+ei*(x->x_width/EXCITER_PIXEL_GRAIN)+gi) = 0;
        }
    }
    memset( x->x_sbangs, 0x0, x->x_nbevents*x->x_width/EXCITER_PIXEL_GRAIN*sizeof(t_int) );
    x->x_selected = 0;
    x->x_started = 0;
    x->x_reltime = 0L;
    x->x_plooptime = 0L;
    x->x_gindex = -1;
    x->x_looplength = x->x_timegrain * x->x_width * 1000 / EXCITER_PIXEL_GRAIN;

    x->x_bangs = (t_outlet **) getbytes( x->x_nbevents*sizeof(t_outlet **) );
    for ( bi=0; bi<x->x_nbevents; bi++ )
    {
        x->x_bangs[bi] = outlet_new( &x->x_obj, &s_bang );
    }

    // post( "exciter : argc : %d", argc );
    if ( ( argc != 0 ) && ( x->x_save ) )
    {
        int ai = 6;
        int si = 0;

        while ( ai < argc - 1 )
        {
            *(x->x_sbangs
              +((int)argv[ai].a_w.w_float)*(x->x_width/EXCITER_PIXEL_GRAIN)
              +(int)argv[ai+1].a_w.w_float) = 1;
            ai += 2;
        }
    }
    post( "exciter_new width: %d height : %d", x->x_width, x->x_height );

    return (x);
}

static void exciter_dump(t_exciter *x)
{
    t_int ei, gi;

    for ( ei=0; ei<x->x_nbevents; ei++ )
    {
        for ( gi=0; gi<(x->x_width/EXCITER_PIXEL_GRAIN); gi++ )
        {
            if ( *(x->x_sbangs+ei*(x->x_width/EXCITER_PIXEL_GRAIN)+gi ) != 0 )
            {
                post( "exciter : value ( %d, %d ) : %d", ei, gi,
                      *(x->x_sbangs+ei*(x->x_width/EXCITER_PIXEL_GRAIN)+gi ) );
            }
        }
    }
}

/* clearing all events */
static void exciter_clear(t_exciter *x)
{
    t_int ei, gi;

    for ( ei=0; ei<x->x_nbevents; ei++ )
    {
        for ( gi=0; gi<(x->x_width/EXCITER_PIXEL_GRAIN); gi++ )
        {
            *(x->x_sbangs+ei*(x->x_width/EXCITER_PIXEL_GRAIN)+gi ) = 0;
        }
    }
    exciter_draw_update( x, x->x_glist );
}


/* starting the triggering of events */
static void exciter_start(t_exciter *x)
{
    x->x_started = 1;
    x->x_reltime = 0L;
    x->x_plooptime = 0L;
    x->x_gindex = -1;
}

/* stop the triggering of events */
static void exciter_stop(t_exciter *x)
{
    x->x_started = 0;
}

/* reset the triggering of events */
static void exciter_reset(t_exciter *x)
{
    x->x_reltime = 0L;
    x->x_plooptime = 0L;
    x->x_gindex = -1;
}

static t_int *exciter_perform(t_int *w)
{
    t_int ei, gi;
    t_int gstart, gend;
    t_exciter* x = (t_exciter*)(w[1]);
    struct timeval tv;
    struct timezone tz;
    long long looptime = 0L;
    double preltime = x->x_reltime;
    t_canvas *canvas=glist_getcanvas(x->x_glist);

    if ( x->x_started )
    {
        // get current time in ms
        gettimeofday( &tv, &tz );
        looptime = tv.tv_sec*1000 + tv.tv_usec/1000;
        if ( x->x_plooptime == 0L )
        {
            x->x_plooptime = looptime;
        }
        x->x_reltime += ( looptime - x->x_plooptime );
        if ( x->x_reltime > x->x_looplength )
        {
            if ( x->x_loop )
            {
                // post( "exciter : restarting loop" );
                x->x_reltime = 0;
                preltime = 0;
                x->x_gindex = -1;
            }
            else
            {
                // post( "exciter : end of the loop" );
                x->x_reltime = 0;
                x->x_started = 0;
            }
        }
        gstart = preltime/(x->x_timegrain*1000);
        gend = x->x_reltime/(x->x_timegrain*1000);

        // prevent overflow due to long long precision
        if ( gstart > x->x_width/EXCITER_PIXEL_GRAIN-1 ) gstart = x->x_width/EXCITER_PIXEL_GRAIN-1;
        if ( gstart < 0 ) gstart = 0;
        if ( gend > x->x_width/EXCITER_PIXEL_GRAIN-1 ) gend = x->x_width/EXCITER_PIXEL_GRAIN-1 ;
        if ( gend < 0 ) gend = 0;
        if ( gstart > x->x_gindex )
        {
            // post( "exciter : focus slice : (%d,%d)", gstart, gend );
            for ( gi=x->x_gindex+1; gi<=gend; gi++ )
            {
                for ( ei=0; ei<x->x_nbevents; ei++ )
                {
                    if ( *(x->x_sbangs+ei*(x->x_width/EXCITER_PIXEL_GRAIN)+gi ) == 1 )
                    {
                        outlet_bang( x->x_bangs[ ei ] );
                        SYS_VGUI5(".x%lx.c itemconfigure %xEVENT%.4d%.4d -fill #00FF00\n",
                                  canvas, x, gi, ei);
                    }
                }
            }
            // unfocus previous events
            if ( gstart >= x->x_width/EXCITER_PIXEL_GRAIN - 1)
                gstart = x->x_width/EXCITER_PIXEL_GRAIN; // not too proud of this one
            // post( "exciter : unfocus slice : (%d,%d)", x->x_gindex, gstart-1 );
            for ( gi=x->x_gindex; gi<gstart; gi++ )
            {
                for ( ei=0; ei<x->x_nbevents; ei++ )
                {
                    if ( *(x->x_sbangs+ei*(x->x_width/EXCITER_PIXEL_GRAIN)+gi ) == 1 )
                    {
                        SYS_VGUI5(".x%lx.c itemconfigure %xEVENT%.4d%.4d -fill #FFFFFF\n",
                                  canvas, x, gi, ei);
                    }
                }
            }
            x->x_gindex = gend;
        }
    }

    x->x_plooptime = looptime;
    return (w+2);
}

static void exciter_dsp(t_exciter *x, t_signal **sp)
{
    dsp_add(exciter_perform, 1, x);
}

static void exciter_free(t_exciter *x)
{
    t_int ei;

    // post( "exciter~: exciter_free" );
    if ( x->x_bangs )
    {
        for ( ei=0; ei<x->x_nbevents; ei++ )
        {
            outlet_free( x->x_bangs[ei] );
        }
        freebytes( x->x_bangs, x->x_nbevents*sizeof(t_outlet*) );
    }
    if ( x->x_sbangs )
    {
        freebytes( x->x_sbangs, x->x_nbevents*x->x_width/EXCITER_PIXEL_GRAIN*sizeof(t_int) );
    }
}

void exciter_setup(void)
{
    logpost(NULL, 4,  exciter_version );
    exciter_class = class_new(gensym("exciter"), (t_newmethod)exciter_new,
                              (t_method)exciter_free, sizeof(t_exciter), 0, A_GIMME, 0);
    class_addmethod(exciter_class, (t_method)exciter_dialog, gensym("dialog"), A_GIMME, 0);
    class_addmethod(exciter_class, (t_method)exciter_dump, gensym("dump"), 0);
    class_addmethod(exciter_class, (t_method)exciter_clear, gensym("clear"), 0);
    class_addmethod(exciter_class, (t_method)exciter_start, gensym("start"), 0);
    class_addmethod(exciter_class, (t_method)exciter_stop, gensym("stop"), 0);
    class_addmethod(exciter_class, (t_method)exciter_reset, gensym("reset"), 0);
    class_addmethod(exciter_class, (t_method)exciter_pause, gensym("pause"), 0);
    class_addmethod(exciter_class, (t_method)exciter_resume, gensym("resume"), 0);
    class_addmethod(exciter_class, (t_method)exciter_dsp, gensym("dsp"), A_NULL);
    exciter_widgetbehavior.w_getrectfn =    exciter_getrect;
    exciter_widgetbehavior.w_displacefn =   exciter_displace;
    exciter_widgetbehavior.w_selectfn =     exciter_select;
    exciter_widgetbehavior.w_activatefn =   NULL;
    exciter_widgetbehavior.w_deletefn =     exciter_delete;
    exciter_widgetbehavior.w_visfn =        exciter_vis;
    exciter_widgetbehavior.w_clickfn =      exciter_click;

#if PD_MINOR_VERSION >= 37
    class_setpropertiesfn(exciter_class, exciter_properties);
    class_setsavefn(exciter_class, exciter_save);
#else
    exciter_widgetbehavior.w_propertiesfn = exciter_properties;
    exciter_widgetbehavior.w_savefn =       exciter_save;
#endif

    class_setwidget(exciter_class, &exciter_widgetbehavior);

    sys_vgui("eval [read [open {%s/%s.tcl}]]\n",
             exciter_class->c_externdir->s_name, exciter_class->c_name->s_name);
}
