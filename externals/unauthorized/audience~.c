/* Copyright (c) 2002 Yves Degoyon.                                             */
/* For information on usage and redistribution, and for a DISCLAIMER OF ALL     */
/* WARRANTIES, see the file, "LICENSE.txt," in this distribution.               */
/*                                                                              */
/* audience.c written by Yves Degoyon 2002                                      */
/* 2-dimensional audience simulation                                            */
/* ( simulates spatialization and interferences )                               */
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
/* Shalaby Effect -- Instrumentals                                              */
/* Honey Bane -- I Wish I Could Be Me                                           */
/* ---------------------------------------------------------------------------- */


#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <ctype.h>
#include "m_pd.h"
#include "m_imp.h"
#include "g_canvas.h"

#include "audience~.h"

#ifdef _WIN32
#include <io.h>
#else
#include <unistd.h>
#endif

#define DEFAULT_AUDIENCE_WIDTH 200
#define DEFAULT_AUDIENCE_HEIGHT 200
#define DEFAULT_AUDIENCE_NBINPUTS 4
#define DEFAULT_AUDIENCE_NBOUTPUTS 2
#define DEFAULT_AUDIENCE_ATTENUATION 0.01

#define LISTENER_WIDTH 15
#define LISTENER_HEIGHT 20
#define SPEAKER_WIDTH 15
#define SPEAKER_HEIGHT 20

// sound propagates at the speed of 340 m/s, isn't it ??
#define SOUNDSPEED 340.0

// a pixel is 0.1 meter
#define PIXELSIZE 0.1

static char   *audience_version = "audience : 2d audience simulation, version 0.7 (ydegoyon@free.fr)";

t_widgetbehavior audience_widgetbehavior;
static t_class *audience_class_tilde;

static int guidebug=0;

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
                         post(a,b,c,d,e,f,g );\
                         sys_vgui(a,b,c,d,e,f,g)

#define SYS_VGUI8(a,b,c,d,e,f,g,h) if (guidebug) \
                         post(a,b,c,d,e,f,g,h );\
                         sys_vgui(a,b,c,d,e,f,g,h)

#define SYS_VGUI9(a,b,c,d,e,f,g,h,i) if (guidebug) \
                         post(a,b,c,d,e,f,g,h,i );\
                         sys_vgui(a,b,c,d,e,f,g,h,i)

/* drawing functions */
static void audience_draw_update(t_audience_tilde *x, t_glist *glist)
{
    t_canvas *canvas=glist_getcanvas(glist);
    t_int ei;

    for ( ei=0; ei<x->x_nbinputs; ei++ )
    {
        SYS_VGUI6(".x%lx.c coords %xISPEAKER%d %d %d\n",
                  canvas, x, ei,
                  text_xpix(&x->x_obj, glist) + x->x_inputs_x[ei],
                  text_ypix(&x->x_obj, glist) + x->x_inputs_y[ei]
                 );
        SYS_VGUI6(".x%lx.c coords %xSPEAKERNUM%d %d %d\n",
                  canvas, x, ei,
                  text_xpix(&x->x_obj, glist) + x->x_inputs_x[ei] - SPEAKER_WIDTH/2,
                  text_ypix(&x->x_obj, glist) + x->x_inputs_y[ei] - SPEAKER_HEIGHT/2
                 );
    }
    for ( ei=0; ei<x->x_nboutputs; ei++ )
    {
        SYS_VGUI6(".x%lx.c coords %xILISTENER%d %d %d\n",
                  canvas, x, ei,
                  text_xpix(&x->x_obj, glist) + x->x_outputs_x[ei],
                  text_ypix(&x->x_obj, glist) + x->x_outputs_y[ei]
                 );
        SYS_VGUI6(".x%lx.c coords %xLISTENERNUM%d %d %d\n",
                  canvas, x, ei,
                  text_xpix(&x->x_obj, glist) + x->x_outputs_x[ei] + LISTENER_WIDTH/2,
                  text_ypix(&x->x_obj, glist) + x->x_outputs_y[ei] + LISTENER_HEIGHT/2
                 );
    }
}

static void audience_draw_new(t_audience_tilde *x, t_glist *glist)
{
    t_canvas *canvas=glist_getcanvas(glist);
    int ei;

    SYS_VGUI7(".x%lx.c create rectangle %d %d %d %d -fill #EAF1E2 -tags %xAAUDIENCE\n",
              canvas, text_xpix(&x->x_obj, glist), text_ypix(&x->x_obj, glist),
              text_xpix(&x->x_obj, glist) + x->x_width, text_ypix(&x->x_obj, glist) + x->x_height,
              x);
    // create captions
    SYS_VGUI5(".x%lx.c create text %d %d -font -*-courier-bold--normal--10-* -text \"0m\" -tags %xBLCAPTION\n",
              canvas, text_xpix(&x->x_obj, glist) - 10 , text_ypix(&x->x_obj, glist) + x->x_height + 10, x );
    SYS_VGUI6(".x%lx.c create text %d %d -font -*-courier-bold--normal--10-* -text \"%dm\" -tags %xBRCAPTION\n",
              canvas, text_xpix(&x->x_obj, glist) + x->x_width + 10 , text_ypix(&x->x_obj, glist) + x->x_height + 10, x->x_width, x );
    SYS_VGUI6(".x%lx.c create text %d %d -font -*-courier-bold--normal--10-* -text \"%dm\" -tags %xULCAPTION\n",
              canvas, text_xpix(&x->x_obj, glist) - 10 , text_ypix(&x->x_obj, glist), x->x_height, x );

    // draw all outlets
    if ( x->x_nboutputs > 1 )
    {
        for ( ei=0; ei<x->x_nboutputs; ei++ )
        {
            SYS_VGUI8(".x%lx.c create rectangle %d %d %d %d -outline #000000 -fill #000000 -tags %xOUT%d\n",
                      canvas, text_xpix(&x->x_obj, glist) + ( ei * (x->x_width - 5) )/ (x->x_nboutputs-1),
                      text_ypix(&x->x_obj, glist) + x->x_height,
                      text_xpix(&x->x_obj, glist) + ( ei * (x->x_width - 5) )/ (x->x_nboutputs-1) + 5,
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
    // draw all inlets
    for ( ei=0; ei<x->x_nbinputs+1; ei++ )
    {
        SYS_VGUI8(".x%lx.c create rectangle %d %d %d %d -outline #000000 -fill #000000 -tags %xIN%d\n",
                  canvas, text_xpix(&x->x_obj, glist) + ( ei * (x->x_width - 5) )/ (x->x_nbinputs),
                  text_ypix(&x->x_obj, glist) - 2,
                  text_xpix(&x->x_obj, glist) + ( ei * (x->x_width - 5) )/ (x->x_nbinputs) + 5,
                  text_ypix(&x->x_obj, glist),
                  x, ei);
    }
    // create speaker images
    for ( ei=0; ei<x->x_nbinputs; ei++ )
    {
        SYS_VGUI6("image create photo %xSPEAKER%d -file {%s/examples/speaker.gif} -format gif -width %d -height %d\n",
                  x, ei, audience_class_tilde->c_externdir->s_name, SPEAKER_WIDTH, SPEAKER_HEIGHT );
        SYS_VGUI8(".x%lx.c create image %d %d -image %xSPEAKER%d -tags %xISPEAKER%d\n",
                  canvas,
                  text_xpix(&x->x_obj, glist) + x->x_inputs_x[ei],
                  text_ypix(&x->x_obj, glist) + x->x_inputs_y[ei],
                  x, ei, x, ei );
        SYS_VGUI7(".x%lx.c create text %d %d -font -*-courier-bold--normal--10-* -text \"s%d\" -tags %xSPEAKERNUM%d\n",
                  canvas, text_xpix(&x->x_obj, glist) + x->x_inputs_x[ei] - SPEAKER_WIDTH/2,
                  text_ypix(&x->x_obj, glist) + x->x_inputs_y[ei] - SPEAKER_HEIGHT/2, ei+1, x, ei );
    }
    // create listener images
    for ( ei=0; ei<x->x_nboutputs; ei++ )
    {
        SYS_VGUI6("image create photo %xLISTENER%d -file {%s/examples/wanderer.gif} -format gif -width %d -height %d\n",
                  x, ei, audience_class_tilde->c_externdir->s_name, LISTENER_WIDTH, LISTENER_HEIGHT );
        SYS_VGUI8(".x%lx.c create image %d %d -image %xLISTENER%d -tags %xILISTENER%d\n",
                  canvas,
                  text_xpix(&x->x_obj, glist) + x->x_outputs_x[ei],
                  text_ypix(&x->x_obj, glist) + x->x_outputs_y[ei],
                  x, ei, x, ei );
        SYS_VGUI7(".x%lx.c create text %d %d -font -*-courier-bold--normal--10-* -text \"l%d\" -tags %xLISTENERNUM%d\n",
                  canvas, text_xpix(&x->x_obj, glist) + x->x_outputs_x[ei] + LISTENER_WIDTH/2,
                  text_ypix(&x->x_obj, glist) + x->x_outputs_y[ei] + LISTENER_HEIGHT/2, ei+1, x, ei );
    }
    canvas_fixlinesfor( canvas, (t_text*)x );
}

static void audience_draw_move(t_audience_tilde *x, t_glist *glist)
{
    t_canvas *canvas=glist_getcanvas(glist);
    t_int ei;

    SYS_VGUI7(".x%lx.c coords %xAAUDIENCE %d %d %d %d\n",
              canvas, x,
              text_xpix(&x->x_obj, glist), text_ypix(&x->x_obj, glist),
              text_xpix(&x->x_obj, glist)+x->x_width, text_ypix(&x->x_obj, glist)+x->x_height);
    SYS_VGUI5(".x%lx.c coords %xBLCAPTION %d %d\n",
              canvas, x,
              text_xpix(&x->x_obj, glist) - 10 , text_ypix(&x->x_obj, glist) + x->x_height + 10);
    SYS_VGUI5(".x%lx.c coords %xBRCAPTION %d %d\n",
              canvas, x,
              text_xpix(&x->x_obj, glist) + x->x_width + 10 , text_ypix(&x->x_obj, glist) + x->x_height + 10 );
    SYS_VGUI5(".x%lx.c coords %xULCAPTION %d %d\n",
              canvas, x,
              text_xpix(&x->x_obj, glist) - 10 , text_ypix(&x->x_obj, glist) );

    for ( ei=0; ei<x->x_nbinputs+1; ei++ )
    {
        SYS_VGUI8(".x%lx.c coords %xIN%d %d %d %d %d\n",
                  canvas, x, ei, text_xpix(&x->x_obj, glist) + ( ei * (x->x_width - 5) )/ (x->x_nbinputs),
                  text_ypix(&x->x_obj, glist) - 2,
                  text_xpix(&x->x_obj, glist) + ( ei * (x->x_width - 5) )/ (x->x_nbinputs) + 5,
                  text_ypix(&x->x_obj, glist)
                 );
    }
    for ( ei=0; ei<x->x_nbinputs+1; ei++ )
    {
        SYS_VGUI6(".x%lx.c coords %xISPEAKER%d %d %d\n",
                  canvas, x, ei,
                  text_xpix(&x->x_obj, glist) + x->x_inputs_x[ei],
                  text_ypix(&x->x_obj, glist) + x->x_inputs_y[ei]
                 );
        SYS_VGUI6(".x%lx.c coords %xSPEAKERNUM%d %d %d\n",
                  canvas, x, ei,
                  text_xpix(&x->x_obj, glist) + x->x_inputs_x[ei] - SPEAKER_WIDTH/2,
                  text_ypix(&x->x_obj, glist) + x->x_inputs_y[ei] - SPEAKER_HEIGHT/2
                 );
    }
    if ( x->x_nboutputs > 1 )
    {
        for ( ei=0; ei<x->x_nboutputs; ei++ )
        {
            SYS_VGUI8(".x%lx.c coords %xOUT%d %d %d %d %d\n",
                      canvas, x, ei, text_xpix(&x->x_obj, glist) + ( ei * (x->x_width - 5) )/ (x->x_nboutputs-1),
                      text_ypix(&x->x_obj, glist) + x->x_height,
                      text_xpix(&x->x_obj, glist) + ( ei * (x->x_width - 5) )/ (x->x_nboutputs-1) + 5,
                      text_ypix(&x->x_obj, glist) + x->x_height + 2
                     );
            SYS_VGUI6(".x%lx.c coords %xILISTENER%d %d %d\n",
                      canvas, x, ei,
                      text_xpix(&x->x_obj, glist) + x->x_outputs_x[ei],
                      text_ypix(&x->x_obj, glist) + x->x_outputs_y[ei]
                     );
            SYS_VGUI6(".x%lx.c coords %xLISTENERNUM%d %d %d\n",
                      canvas, x, ei,
                      text_xpix(&x->x_obj, glist) + x->x_outputs_x[ei] + LISTENER_WIDTH/2,
                      text_ypix(&x->x_obj, glist) + x->x_outputs_y[ei] + LISTENER_HEIGHT/2
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
        SYS_VGUI6(".x%lx.c coords %xILISTENER%d %d %d\n",
                  canvas, x, 0,
                  text_xpix(&x->x_obj, glist) + x->x_outputs_x[0],
                  text_ypix(&x->x_obj, glist) + x->x_outputs_y[0]
                 );
        SYS_VGUI6(".x%lx.c coords %xLISTENERNUM%d %d %d\n",
                  canvas, x, 0,
                  text_xpix(&x->x_obj, glist) + x->x_outputs_x[0] + LISTENER_WIDTH/2,
                  text_ypix(&x->x_obj, glist) + x->x_outputs_y[0] + LISTENER_HEIGHT/2
                 );
    }
    canvas_fixlinesfor( canvas, (t_text*)x );
}

static void audience_draw_erase(t_audience_tilde* x,t_glist* glist)
{
    t_canvas *canvas=glist_getcanvas(glist);
    int ei;

    SYS_VGUI3(".x%lx.c delete %xAAUDIENCE\n", canvas, x);
    SYS_VGUI3(".x%lx.c delete %xBLCAPTION\n", canvas, x);
    SYS_VGUI3(".x%lx.c delete %xBRCAPTION\n", canvas, x);
    SYS_VGUI3(".x%lx.c delete %xULCAPTION\n", canvas, x);
    for ( ei=0; ei<x->x_nbinputs+1; ei++ )
    {
        SYS_VGUI4(".x%lx.c delete %xIN%d\n", canvas, x, ei );
    }
    for ( ei=0; ei<x->x_nbinputs; ei++ )
    {
        SYS_VGUI4(".x%lx.c delete %xISPEAKER%d\n", canvas, x, ei );
        SYS_VGUI4(".x%lx.c delete %xSPEAKERNUM%d\n", canvas, x, ei );
        // SYS_VGUI3("image delete %xSPEAKER%d\n", x, ei );
    }
    for ( ei=0; ei<x->x_nboutputs; ei++ )
    {
        SYS_VGUI4(".x%lx.c delete %xOUT%d\n", canvas, x, ei );
        SYS_VGUI4(".x%lx.c delete %xILISTENER%d\n", canvas, x, ei );
        SYS_VGUI4(".x%lx.c delete %xLISTENERNUM%d\n", canvas, x, ei );
        // SYS_VGUI3("image delete %xLISTENER%d\n", x, ei );
    }
}

static void audience_draw_select(t_audience_tilde* x,t_glist* glist)
{
    t_canvas *canvas=glist_getcanvas(glist);

    if(x->x_selected)
    {
        /* sets the item in blue */
        SYS_VGUI3(".x%lx.c itemconfigure %xAAUDIENCE -outline #0000FF\n", canvas, x);
    }
    else
    {
        SYS_VGUI3(".x%lx.c itemconfigure %xAAUDIENCE -outline #000000\n", canvas, x);
    }
}

/* ------------------------ audience widgetbehaviour----------------------------- */


static void audience_getrect(t_gobj *z, t_glist *owner,
                             int *xp1, int *yp1, int *xp2, int *yp2)
{
    t_audience_tilde* x = (t_audience_tilde*)z;

    *xp1 = text_xpix(&x->x_obj, owner);
    *yp1 = text_ypix(&x->x_obj, owner);
    *xp2 = text_xpix(&x->x_obj, owner)+x->x_width;
    *yp2 = text_ypix(&x->x_obj, owner)+x->x_height;
}

static void audience_save(t_gobj *z, t_binbuf *b)
{
    t_audience_tilde *x = (t_audience_tilde *)z;
    t_int ii;

    binbuf_addv(b, "ssiisiiiifi", gensym("#X"),gensym("obj"),
                (t_int)x->x_obj.te_xpix, (t_int)x->x_obj.te_ypix,
                atom_getsymbol(binbuf_getvec(x->x_obj.te_binbuf)),
                x->x_width, x->x_height,
                x->x_nbinputs, x->x_nboutputs, x->x_attenuation, x->x_applydelay );
    for ( ii=0; ii<x->x_nbinputs; ii++ )
    {
        binbuf_addv(b, "ii", x->x_inputs_x[ii], x->x_inputs_y[ii] );
    }
    for ( ii=0; ii<x->x_nboutputs; ii++ )
    {
        binbuf_addv(b, "ii", x->x_outputs_x[ii], x->x_outputs_y[ii] );
    }
    binbuf_addv(b, ";");
}

static void audience_properties(t_gobj *z, t_glist *owner)
{
    char buf[800];
    t_audience_tilde *x=(t_audience_tilde *)z;

    sprintf(buf, "pdtk_audience_dialog %%s %d %d %d\n",
            x->x_width, x->x_height, x->x_nboutputs
           );
    // post("audience_properties : %s", buf );
    gfxstub_new(&x->x_obj.ob_pd, x, buf);
}

static void audience_select(t_gobj *z, t_glist *glist, int selected)
{
    t_audience_tilde *x = (t_audience_tilde *)z;

    x->x_selected = selected;
    audience_draw_select( x, glist );
}

static void audience_vis(t_gobj *z, t_glist *glist, int vis)
{
    t_audience_tilde *x = (t_audience_tilde *)z;

    // post( "audience~ : vis : %d", vis );
    if (vis)
    {
        audience_draw_new( x, glist );
    }
    else
    {
        audience_draw_erase( x, glist );
    }
}

static void audience_dialog(t_audience_tilde *x, t_symbol *s, int argc, t_atom *argv)
{
    t_int onbinputs = x->x_nbinputs;
    t_int onboutputs = x->x_nboutputs;
    t_int owidth = x->x_width;
    t_int oheight = x->x_height;
    t_int bi, ei;
    t_int dspstate;
    t_canvas *canvas=glist_getcanvas(x->x_glist);

    if ( !x )
    {
        post( "audience~ : error :tried to set properties on an unexisting object" );
    }
    if ( argc != 3 )
    {
        post( "audience : error in the number of arguments ( %d instead of 3 )", argc );
        return;
    }
    if ( argv[0].a_type != A_FLOAT || argv[1].a_type != A_FLOAT ||
            argv[2].a_type != A_FLOAT )
    {
        post( "audience~ : wrong arguments" );
        return;
    }

    dspstate = canvas_suspend_dsp();
    audience_draw_erase(x, x->x_glist);

    x->x_width = (int)argv[0].a_w.w_float;
    if ( x->x_width < 10 ) x->x_width = 10;
    x->x_height = (int)argv[1].a_w.w_float;
    if ( x->x_height < 10 ) x->x_height = 10;
    x->x_nboutputs = (int)argv[2].a_w.w_float;
    if ( x->x_nboutputs < 1 ) x->x_nboutputs = 1;

    // re-allocate audio buffers if needed
    if ( ( owidth != x->x_width ) || ( oheight != x->x_height ) )
    {
        if ( x->x_audiobuffer )
        {
            for ( ei=0; ei<onbinputs; ei++ )
            {
                freebytes( x->x_audiobuffer[ei], x->x_audiobuffersize*sizeof(t_float) );
            }
            freebytes( x->x_audiobuffer, onbinputs*sizeof(t_float*) );
        }
        // allocate audio buffer
        x->x_audiobuffer = (t_float **) getbytes( x->x_nbinputs*sizeof(t_float *) );
        if ( !x->x_audiobuffer )
        {
            post( "audience~ : could not allocate audio buffer" );
            return;
        }
        x->x_audiobuffersize = ( t_int ) ( ( ( ( t_float ) sqrt( pow( x->x_width, 2 )
                                               + pow( x->x_height, 2 ) ) ) / SOUNDSPEED )
                                           * ( (float ) sys_getsr() ) );
        post( "audience~ : audio buffer size : %d samples", x->x_audiobuffersize );
        for ( bi=0; bi<x->x_nbinputs; bi++ )
        {
            x->x_audiobuffer[bi] = (t_float *) getbytes( x->x_audiobuffersize*sizeof(t_float) );
            if ( !x->x_audiobuffer[bi] )
            {
                post( "audience~ : could not allocate audio buffer" );
                return;
            }
        }
        x->x_audiowritepos = 0;
    }

    // re-allocate inlets : CRASHES PD,I GUESS IT'S NOT SUPPORTED
    if ( onbinputs != x->x_nbinputs )
    {
        // post( "audience~ : cleaning up old inlets" );
        if ( x->x_inputs )
        {
            for ( ei=0; ei<onbinputs; ei++ )
            {
                // post( "audience~ : freeing input ! %d", ei );
                inlet_free( x->x_inputs[ei] );
            }
            freebytes( x->x_inputs, onbinputs*sizeof(t_inlet*) );
        }
        if ( x->x_inputs_x )
        {
            freebytes( x->x_inputs_x, onbinputs*sizeof(t_int) );
        }
        if ( x->x_inputs_y )
        {
            freebytes( x->x_inputs_y, onbinputs*sizeof(t_int) );
        }
        // post( "audience~ : creating new ones" );
        x->x_inputs = (t_inlet **) getbytes( x->x_nbinputs*sizeof(t_inlet *) );
        x->x_inputs_x = (t_int *) getbytes( x->x_nbinputs*sizeof(t_int) );
        x->x_inputs_y = (t_int *) getbytes( x->x_nbinputs*sizeof(t_int) );
        if ( !x->x_inputs || !x->x_inputs_x || !x->x_inputs_y )
        {
            error( "audience~ : fatal : could not create new object" );
            return;
        }
        for ( bi=0; bi<x->x_nbinputs; bi++ )
        {
            // post( "audience~ : allocating input ! %d", bi );
            x->x_inputs[bi] = inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_signal, &s_signal);
        }
    }

    // re-allocate outlets
    if ( onboutputs != x->x_nboutputs )
    {
        // post( "audience~ : cleaning up old outlets" );
        if ( x->x_outputs )
        {
            for ( ei=0; ei<onboutputs; ei++ )
            {
                canvas_rmoutlet(canvas, x->x_outputs[ei] );
                // outlet_free( x->x_outputs[ei] );
            }
            freebytes( x->x_outputs, onboutputs*sizeof(t_outlet*) );
        }
        if ( x->x_outputs_x )
        {
            freebytes( x->x_outputs_x, onboutputs*sizeof(t_int) );
        }
        if ( x->x_outputs_y )
        {
            freebytes( x->x_outputs_y, onboutputs*sizeof(t_int) );
        }
        // post( "audience~ : creating new ones" );
        x->x_outputs = (t_outlet **) getbytes( x->x_nboutputs*sizeof(t_outlet *) );
        x->x_outputs_x = (t_int *) getbytes( x->x_nboutputs*sizeof(t_int) );
        x->x_outputs_y = (t_int *) getbytes( x->x_nboutputs*sizeof(t_int) );
        if ( !x->x_outputs || !x->x_outputs_x || !x->x_outputs_y )
        {
            // error( "audience~ : fatal : could not create new object" );
            return;
        }
        for ( bi=0; bi<x->x_nboutputs; bi++ )
        {
            x->x_outputs[bi] = outlet_new( &x->x_obj, &s_signal );
        }
        // set default coordinates
        if ( x->x_nboutputs > 1 )
        {
            for ( ei=0; ei<x->x_nboutputs; ei++ )
            {
                x->x_outputs_x[ei] = ei * (x->x_width - 5) / ( x->x_nboutputs - 1 );
                x->x_outputs_y[ei] = x->x_height - LISTENER_HEIGHT/2;
            }
        }
        else
        {
            x->x_outputs_x[0] = x->x_width;
            x->x_outputs_y[0] = x->x_height - LISTENER_HEIGHT/2;
        }
    }

    canvas_fixlinesfor( canvas, (t_text*)x );
    audience_draw_new(x, x->x_glist);
    canvas_resume_dsp(dspstate);
}

static void audience_delete(t_gobj *z, t_glist *glist)
{
    t_audience_tilde *x = (t_audience_tilde *)z;

    // post( "audience~ : delete" );
    audience_draw_erase( x, glist );
    canvas_deletelinesfor(glist, (t_text *)z);
}

static void audience_displace(t_gobj *z, t_glist *glist, int dx, int dy)
{
    t_audience_tilde *x = (t_audience_tilde *)z;
    int xold = text_xpix(&x->x_obj, glist);
    int yold = text_ypix(&x->x_obj, glist);

    // post( "audience_displace dx=%d dy=%d", dx, dy );

    x->x_obj.te_xpix += dx;
    x->x_obj.te_ypix += dy;
    if(xold != x->x_obj.te_xpix || yold != x->x_obj.te_ypix)
    {
        audience_draw_move(x, x->x_glist);
    }
}

static void audience_motion(t_audience_tilde *x, t_floatarg dx, t_floatarg dy)
{
    // post( "audience_motion dx=%f dy=%f", dx, dy );

    switch( x->x_type_selected )
    {
    case AUDIENCE_INPUT:
        x->x_inputs_x[ x->x_nselected ] += dx;
        if ( x->x_inputs_x[ x->x_nselected ] < 0 ) x->x_inputs_x[ x->x_nselected ] = 0;
        if ( x->x_inputs_x[ x->x_nselected ] > x->x_width ) x->x_inputs_x[ x->x_nselected ] = x->x_width;
        x->x_inputs_y[ x->x_nselected ] += dy;
        if ( x->x_inputs_y[ x->x_nselected ] < 0 ) x->x_inputs_y[ x->x_nselected ] = 0;
        if ( x->x_inputs_y[ x->x_nselected ] > x->x_height ) x->x_inputs_y[ x->x_nselected ] = x->x_height;
        break;
    case AUDIENCE_OUTPUT:
        x->x_outputs_x[ x->x_nselected ] += dx;
        if ( x->x_outputs_x[ x->x_nselected ] < 0 ) x->x_outputs_x[ x->x_nselected ] = 0;
        if ( x->x_outputs_x[ x->x_nselected ] > x->x_width ) x->x_outputs_x[ x->x_nselected ] = x->x_width;
        x->x_outputs_y[ x->x_nselected ] += dy;
        if ( x->x_outputs_y[ x->x_nselected ] < 0 ) x->x_outputs_y[ x->x_nselected ] = 0;
        if ( x->x_outputs_y[ x->x_nselected ] > x->x_height ) x->x_outputs_y[ x->x_nselected ] = x->x_height;
        break;
    }

    audience_draw_update(x, x->x_glist);
}

static int audience_click(t_gobj *z, struct _glist *glist,
                          int xpix, int ypix, int shift, int alt, int dbl, int doit)
{
    t_audience_tilde* x = (t_audience_tilde *)z;
    t_int bi;

    // post( "audience_click doit=%d x=%d y=%d", doit, xpix, ypix );
    if ( doit)
    {
        t_int relx = xpix-text_xpix(&x->x_obj, glist);
        t_int rely = ypix-text_ypix(&x->x_obj, glist);

        // post( "audience~ : relx : %d : rely : %d", relx, rely );
        x->x_type_selected = AUDIENCE_NONE;
        x->x_nselected = -1;
        for ( bi=0; bi<x->x_nbinputs; bi++ )
        {
            if ( ( abs( relx - x->x_inputs_x[bi] ) < SPEAKER_WIDTH ) &&
                    ( abs( rely - x->x_inputs_y[bi] ) < SPEAKER_HEIGHT ) )
            {
                x->x_type_selected = AUDIENCE_INPUT;
                x->x_nselected = bi;
                break;
            }
        }
        if ( x->x_type_selected == AUDIENCE_NONE )
        {
            for ( bi=0; bi<x->x_nboutputs; bi++ )
            {
                if ( ( abs( relx - x->x_outputs_x[bi] ) < LISTENER_WIDTH ) &&
                        ( abs( rely - x->x_outputs_y[bi] ) < LISTENER_HEIGHT ) )
                {
                    x->x_type_selected = AUDIENCE_OUTPUT;
                    x->x_nselected = bi;
                    break;
                }
            }
        }
        audience_draw_update(x, glist);
        glist_grab(glist, &x->x_obj.te_g, (t_glistmotionfn)audience_motion,
                   0, xpix, ypix);
    }
    return (1);
}

static t_audience_tilde *audience_new(t_symbol *s, int argc, t_atom *argv)
{
    t_int bi, ei;
    t_audience_tilde *x;
    t_pd *x2;
    char *str;

    // post( "audience_new : create : %s argc =%d", s->s_name, argc );

    x = (t_audience_tilde *)pd_new(audience_class_tilde);
    // new audience created from the gui
    if ( argc != 0 )
    {
        if ( argc < 5 )
        {
            post( "audience~ : error in the number of arguments ( %d )", argc );
            return NULL;
        }
        if ( argv[0].a_type != A_FLOAT || argv[1].a_type != A_FLOAT ||
                argv[2].a_type != A_FLOAT || argv[3].a_type != A_FLOAT ||
                argv[4].a_type != A_FLOAT || argv[5].a_type != A_FLOAT )
        {
            post( "audience~ : wrong arguments" );
            return NULL;
        }

        x->x_width = (int)argv[0].a_w.w_float;
        if ( x->x_width < 10 ) x->x_width = 10;
        x->x_height = (int)argv[1].a_w.w_float;
        if ( x->x_height < 10 ) x->x_height = 10;
        x->x_nbinputs = (int)argv[2].a_w.w_float;
        if ( x->x_nbinputs < 1 ) x->x_nbinputs = 1;
        x->x_nboutputs = (int)argv[3].a_w.w_float;
        if ( x->x_nboutputs < 1 ) x->x_nboutputs = 1;
        x->x_attenuation = argv[4].a_w.w_float;
        if ( x->x_attenuation < 0 ) x->x_attenuation = 0;
        x->x_applydelay = argv[5].a_w.w_float;
    }
    else
    {
        x->x_width = DEFAULT_AUDIENCE_WIDTH;
        x->x_height = DEFAULT_AUDIENCE_HEIGHT;
        x->x_nbinputs = DEFAULT_AUDIENCE_NBINPUTS;
        x->x_nboutputs = DEFAULT_AUDIENCE_NBOUTPUTS;
        x->x_attenuation = DEFAULT_AUDIENCE_ATTENUATION;
        x->x_applydelay = 0;
    }

    // create inlets and outlets
    x->x_outputs = (t_outlet **) getbytes( x->x_nboutputs*sizeof(t_outlet *) );
    x->x_outputs_x = (t_int *) getbytes( x->x_nboutputs*sizeof(t_int) );
    x->x_outputs_y = (t_int *) getbytes( x->x_nboutputs*sizeof(t_int) );
    if ( !x->x_outputs || !x->x_outputs_x || !x->x_outputs_y )
    {
        post( "audience~ : could not allocate outputs" );
        return NULL;
    }
    for ( bi=0; bi<x->x_nboutputs; bi++ )
    {
        x->x_outputs[bi] = outlet_new( &x->x_obj, &s_signal );
    }
    x->x_inputs = (t_inlet **) getbytes( x->x_nbinputs*sizeof(t_inlet *) );
    x->x_inputs_x = (t_int *) getbytes( x->x_nbinputs*sizeof(t_int) );
    x->x_inputs_y = (t_int *) getbytes( x->x_nbinputs*sizeof(t_int) );
    if ( !x->x_inputs || !x->x_inputs_x || !x->x_inputs_y )
    {
        post( "audience~ : could not allocate inputs" );
        return NULL;
    }
    for ( bi=0; bi<x->x_nbinputs; bi++ )
    {
        x->x_inputs[bi] =  inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_signal, &s_signal);
    }

    // allocate audio buffer
    x->x_audiowritepos = 0;
    x->x_audiobuffer = (t_float **) getbytes( x->x_nbinputs*sizeof(t_float *) );
    if ( !x->x_audiobuffer )
    {
        post( "audience~ : could not allocate audio buffer" );
        return NULL;
    }
    x->x_audiobuffersize = ( t_int ) ( ( ( ( t_float ) sqrt( pow( x->x_width, 2 )
                                           + pow( x->x_height, 2 ) ) ) / SOUNDSPEED )
                                       * ( (float ) sys_getsr() ) );
    post( "audience~ : audio buffer size : %d samples", x->x_audiobuffersize );
    for ( bi=0; bi<x->x_nbinputs; bi++ )
    {
        x->x_audiobuffer[bi] = (t_float *) getbytes( x->x_audiobuffersize*sizeof(t_float) );
        if ( !x->x_audiobuffer[bi] )
        {
            post( "audience~ : could not allocate audio buffer" );
            return NULL;
        }
    }

    if ( argc == 0 )
    {
        // set default coordinates
        if ( x->x_nbinputs > 1 )
        {
            for ( ei=0; ei<x->x_nbinputs; ei++ )
            {
                x->x_inputs_x[ei] = (ei+1) * (x->x_width - 5) / x->x_nbinputs;
                x->x_inputs_y[ei] = SPEAKER_HEIGHT/2;
            }
        }
        else
        {
            x->x_inputs_x[0] = x->x_width;
            x->x_inputs_y[0] = SPEAKER_HEIGHT/2;
        }

        if ( x->x_nboutputs > 1 )
        {
            for ( ei=0; ei<x->x_nboutputs; ei++ )
            {
                x->x_outputs_x[ei] = ei * (x->x_width - 5) / ( x->x_nboutputs - 1 );
                x->x_outputs_y[ei] = x->x_height - LISTENER_HEIGHT/2;
            }
        }
        else
        {
            x->x_outputs_x[0] = x->x_width;
            x->x_outputs_y[0] = x->x_height - LISTENER_HEIGHT/2;
        }
    }
    else
    {
        t_int ai = 6;

        // restore coordinates from arguments
        for ( ei=0; ei<x->x_nbinputs; ei++ )
        {
            x->x_inputs_x[ei] = argv[ai++].a_w.w_float;
            x->x_inputs_y[ei] = argv[ai++].a_w.w_float;
        }
        for ( ei=0; ei<x->x_nboutputs; ei++ )
        {
            x->x_outputs_x[ei] = argv[ai++].a_w.w_float;
            x->x_outputs_y[ei] = argv[ai++].a_w.w_float;
        }
    }

    x->x_glist = (t_glist *) canvas_getcurrent();
    x->x_type_selected = AUDIENCE_NONE;
    x->x_nselected = -1;

    // post( "audience~ : new object : inlets : %d : outlets : %d : attenuation : %f", x->x_nbinputs, x->x_nboutputs, x->x_attenuation );
    return (x);
}

static void audience_free(t_audience_tilde *x)
{
    t_int ei;

    if ( x->x_outputs )
    {
        for ( ei=0; ei<x->x_nboutputs; ei++ )
        {
            outlet_free( x->x_outputs[ei] );
        }
        freebytes( x->x_outputs, x->x_nboutputs*sizeof(t_outlet*) );
    }
    if ( x->x_inputs )
    {
        for ( ei=0; ei<x->x_nbinputs; ei++ )
        {
            inlet_free( x->x_inputs[ei] );
        }
        freebytes( x->x_inputs, x->x_nbinputs*sizeof(t_outlet*) );
    }
    if ( x->x_audiobuffer )
    {
        for ( ei=0; ei<x->x_nbinputs; ei++ )
        {
            freebytes( x->x_audiobuffer[ei], x->x_audiobuffersize*sizeof(t_float) );
        }
        freebytes( x->x_audiobuffer, x->x_nbinputs*sizeof(t_float*) );
    }
}

static t_int *audience_perform(t_int *w)
{
    t_int ii, oi, op;
    t_audience_tilde *x = (t_audience_tilde*)(w[1]);
    t_int bsize = w[2];

    {
        // save input sounds in the audio buffer
        for ( ii=0; ii<x->x_nbinputs; ii++ )
        {
            t_float* isound = (t_float*)w[ii+3];

            if ( x->x_audiobuffer[ii] )
            {
                op = 0;
                while ( op < bsize )
                {
                    *(x->x_audiobuffer[ii] + x->x_audiowritepos + op ) = *(isound + op);
                    op++;
                }
            }
        }

        // set outputs
        for ( oi=0; oi<x->x_nboutputs; oi++ )
        {
            t_float* osound = (t_float*)w[oi+3+x->x_nbinputs];

            // zeroing output
            memset( osound, 0x00, bsize*sizeof( t_float ) );

            for ( ii=0; ii<x->x_nbinputs; ii++ )
            {
                t_int delay;
                t_int dist;
                t_int readpos;
                t_int maxwritepos;

                maxwritepos = ( x->x_audiobuffersize / bsize ) * bsize;
                dist = sqrt( pow( (x->x_outputs_x[oi] - x->x_inputs_x[ii]), 2 )
                             + pow( (x->x_outputs_y[oi] - x->x_inputs_y[ii]), 2 ) );
                delay = ( t_int ) ( ( ( ( t_float ) dist ) * ( (float ) sys_getsr() ) ) / SOUNDSPEED );
                delay = ( delay / bsize ) * bsize; // set a block frontier
                if ( x->x_applydelay )
                {
                    if ( x->x_audiowritepos >= delay )
                    {
                        readpos = x->x_audiowritepos - delay;
                    }
                    else
                    {
                        readpos = maxwritepos - delay + x->x_audiowritepos;
                    }
                }
                else
                {
                    readpos = x->x_audiowritepos;
                }
                // if ( ii == 0 )
                // {
                //    post( "audience~ : dist : %d : delay : %d : readpos : %d : writepos : %d",
                //           dist, delay, readpos, x->x_audiowritepos );
                // }

                op = 0;
                while ( op < bsize )
                {
                    if ( ( readpos < x->x_audiobuffersize ) && ( readpos >= 0 ) )
                    {
                        if ( 1.0-x->x_attenuation*dist < 0 )
                        {
                            *(osound+op) += 0.0;
                        }
                        else
                        {
                            *(osound+op) += *(x->x_audiobuffer[ii] + readpos + op)*(1.0-x->x_attenuation*dist);
                        }
                    }
                    else
                    {
                        error( "audience~ : delay : %d : wrong readpos !!! : %d >= %d or < 0", delay, readpos, x->x_audiobuffersize );
                    }
                    op++;
                }
            }
        }

        // update write position
        if ( x->x_audiowritepos + bsize > x->x_audiobuffersize - bsize )
        {
            // post( "audience~ : write back to zero @ %d", x->x_audiowritepos  );
            x->x_audiowritepos = 0;
        }
        else
        {
            x->x_audiowritepos += bsize;
        }

    }

    return (w+x->x_nbinputs+x->x_nboutputs+3);
}

static void audience_dsp(t_audience_tilde *x, t_signal **sp)
{
    switch ( x->x_nbinputs+x->x_nboutputs )
    {
    case 2 :
        dsp_add(audience_perform, 4, x, sp[1]->s_n, sp[1]->s_vec, sp[2]->s_vec );
        break;

    case 3 :
        dsp_add(audience_perform, 5, x, sp[1]->s_n, sp[1]->s_vec, sp[2]->s_vec, sp[3]->s_vec );
        break;

    case 4 :
        dsp_add(audience_perform, 6, x, sp[1]->s_n, sp[1]->s_vec, sp[2]->s_vec, sp[3]->s_vec, sp[4]->s_vec );
        break;

    case 5 :
        dsp_add(audience_perform, 7, x, sp[1]->s_n, sp[1]->s_vec, sp[2]->s_vec, sp[3]->s_vec, sp[4]->s_vec, sp[5]->s_vec );
        break;

    case 6 :
        dsp_add(audience_perform, 8, x, sp[1]->s_n, sp[1]->s_vec, sp[2]->s_vec, sp[3]->s_vec, sp[4]->s_vec,
                sp[5]->s_vec, sp[6]->s_vec );
        break;

    case 7 :
        dsp_add(audience_perform, 9, x, sp[1]->s_n, sp[1]->s_vec, sp[2]->s_vec, sp[3]->s_vec, sp[4]->s_vec,
                sp[5]->s_vec, sp[6]->s_vec, sp[7]->s_vec );
        break;

    case 8 :
        dsp_add(audience_perform, 10, x, sp[1]->s_n, sp[1]->s_vec, sp[2]->s_vec, sp[3]->s_vec, sp[4]->s_vec,
                sp[5]->s_vec, sp[6]->s_vec, sp[7]->s_vec, sp[8]->s_vec );
        break;

    case 9 :
        dsp_add(audience_perform, 11, x, sp[1]->s_n, sp[1]->s_vec, sp[2]->s_vec, sp[3]->s_vec, sp[4]->s_vec,
                sp[5]->s_vec, sp[6]->s_vec, sp[7]->s_vec, sp[8]->s_vec, sp[9]->s_vec );
        break;

    case 10 :
        dsp_add(audience_perform, 12, x, sp[1]->s_n, sp[1]->s_vec, sp[2]->s_vec, sp[3]->s_vec, sp[4]->s_vec,
                sp[5]->s_vec, sp[6]->s_vec, sp[7]->s_vec, sp[8]->s_vec,
                sp[9]->s_vec, sp[10]->s_vec );
        break;

    default :
        post( "audience~ : number of inlets/outlets not supported" );
        break;
    }
}

// set attenuation
static void audience_attenuation(t_audience_tilde *x, t_floatarg fattenuation )
{
    if ( fattenuation < 0 )
    {
        post( "audience~ : error : wrong attenuation : %f", fattenuation );
        return;
    }
    x->x_attenuation = fattenuation;
}

// set delay
static void audience_delay(t_audience_tilde *x, t_floatarg fdelay )
{
    if ( fdelay == 0. )
    {
        x->x_applydelay = 0;
    }
    else
    {
        x->x_applydelay = 1;
    }
}

void audience_tilde_setup(void)
{
    logpost(NULL, 4,  audience_version );
    audience_class_tilde = class_new(gensym("audience~"), (t_newmethod)audience_new,
                                     (t_method)audience_free, sizeof(t_audience_tilde), 0, A_GIMME, 0);
    CLASS_MAINSIGNALIN( audience_class_tilde, t_audience_tilde, x_f );
    class_addmethod(audience_class_tilde, (t_method)audience_dsp, gensym("dsp"), 0);
    class_addmethod(audience_class_tilde, (t_method)audience_dialog, gensym("dialog"), A_GIMME, 0);
    class_addmethod(audience_class_tilde, (t_method)audience_attenuation, gensym("attenuation"), A_DEFFLOAT, 0);
    class_addmethod(audience_class_tilde, (t_method)audience_delay, gensym("delay"), A_DEFFLOAT, 0);
    audience_widgetbehavior.w_getrectfn =    audience_getrect;
    audience_widgetbehavior.w_displacefn =   audience_displace;
    audience_widgetbehavior.w_selectfn =     audience_select;
    audience_widgetbehavior.w_activatefn =   NULL;
    audience_widgetbehavior.w_deletefn =     audience_delete;
    audience_widgetbehavior.w_visfn =        audience_vis;
    audience_widgetbehavior.w_clickfn =      audience_click;

#if PD_MINOR_VERSION >= 37
    class_setpropertiesfn(audience_class_tilde, audience_properties);
    class_setsavefn(audience_class_tilde, audience_save);
#else
    audience_widgetbehavior.w_propertiesfn = audience_properties;
    audience_widgetbehavior.w_savefn =       audience_save;
#endif

    class_setwidget(audience_class_tilde, &audience_widgetbehavior);

    sys_vgui("eval [read [open {%s/%s.tcl}]]\n",
             audience_class_tilde->c_externdir->s_name, audience_class_tilde->c_name->s_name);
}
