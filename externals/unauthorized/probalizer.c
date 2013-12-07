/* ---------------------------------------------------------------------------- */
/* Copyright (c) 2002 Yves Degoyon.                                             */
/* For information on usage and redistribution, and for a DISCLAIMER OF ALL     */
/* WARRANTIES, see the file, "LICENSE.txt," in this distribution.               */
/*                                                                              */
/* probalizer : outputs integer values according to a drawn probability curve   */
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
/* "I am a product, I am a sample"                                              */
/* "I'm hopeless, aimless"                                                      */
/* Crass -- Feeding of the 5000                                                 */
/* ---------------------------------------------------------------------------- */


#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <ctype.h>
#include "m_pd.h"
#include "m_imp.h"
#include "g_canvas.h"

#include "probalizer.h"

#ifdef _WIN32
# include <io.h>
# define random rand
#endif

#ifndef _MSC_VER /* only MSVC doesn't have unistd.h */
#include <unistd.h>
#endif

#define DEFAULT_PROBALIZER_WIDTH 200
#define DEFAULT_PROBALIZER_HEIGHT 200
#define DEFAULT_PROBALIZER_NBVALUES 100
#define DEFAULT_PROBALIZER_NBOCCURRENCES 100
#define DEFAULT_PROB_VALUE 10

static char   *probalizer_version = "probalizer : outputs integer values according to a drawn probability curve , version 0.4 (ydegoyon@free.fr)";

t_widgetbehavior probalizer_widgetbehavior;
static t_class *probalizer_class;

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
static void probalizer_draw_new(t_probalizer *x, t_glist *glist)
{
    t_canvas *canvas=glist_getcanvas(glist);
    t_int ei;

    SYS_VGUI7(".x%lx.c create rectangle %d %d %d %d -outline #000000 -fill #6790E2 -tags %xPROBALIZER\n",
              canvas, text_xpix(&x->x_obj, glist), text_ypix(&x->x_obj, glist),
              text_xpix(&x->x_obj, glist)+x->x_width, text_ypix(&x->x_obj, glist)+x->x_height,
              x);
    SYS_VGUI5(".x%lx.c create text %d %d -font -*-courier-bold--normal--10-* -text \"0\" -tags %xLTCAPTION\n",
              canvas, text_xpix(&x->x_obj, glist)-15, text_ypix(&x->x_obj, glist) + x->x_height, x );
    SYS_VGUI6(".x%lx.c create text %d %d -font -*-courier-bold--normal--10-* -text \"%d\" -tags %xLBCAPTION\n",
              canvas, text_xpix(&x->x_obj, glist)-15, text_ypix(&x->x_obj, glist), x->x_noccurrences, x );
    SYS_VGUI5(".x%lx.c create text %d %d -font -*-courier-bold--normal--10-* -text \"1\" -tags %xBLCAPTION\n",
              canvas, text_xpix(&x->x_obj, glist)+2, text_ypix(&x->x_obj, glist) + x->x_height + 10, x );
    SYS_VGUI6(".x%lx.c create text %d %d -font -*-courier-bold--normal--10-* -text \"%d\" -tags %xBRCAPTION\n",
              canvas, text_xpix(&x->x_obj, glist) + x->x_width-5, text_ypix(&x->x_obj, glist) + x->x_height + 10, x->x_nvalues, x );
    SYS_VGUI7(".x%lx.c create rectangle %d %d %d %d -outline #000000 -fill #000000 -tags %xIN\n",
              canvas, text_xpix(&x->x_obj, glist),
              text_ypix(&x->x_obj, glist) - 2,
              text_xpix(&x->x_obj, glist) + 5,
              text_ypix(&x->x_obj, glist) ,
              x);
    SYS_VGUI7(".x%lx.c create rectangle %d %d %d %d -outline #000000 -fill #000000 -tags %xOUT\n",
              canvas, text_xpix(&x->x_obj, glist),
              text_ypix(&x->x_obj, glist) + x->x_height,
              text_xpix(&x->x_obj, glist) + 5,
              text_ypix(&x->x_obj, glist) + x->x_height + 2,
              x);
    SYS_VGUI7(".x%lx.c create rectangle %d %d %d %d -outline #000000 -fill #000000 -tags %xOUT2\n",
              canvas, text_xpix(&x->x_obj, glist) + x->x_width -5,
              text_ypix(&x->x_obj, glist) + x->x_height,
              text_xpix(&x->x_obj, glist) + x->x_width,
              text_ypix(&x->x_obj, glist) + x->x_height + 2,
              x);
    for ( ei=0; ei<x->x_nvalues; ei++ )
    {
        SYS_VGUI8(".x%lx.c create rectangle %d %d %d %d -outline #000000 -fill #118373 -tags %xPROB%d\n",
                  canvas,
                  text_xpix(&x->x_obj, glist) + ei * x->x_width/x->x_nvalues,
                  text_ypix(&x->x_obj, glist) + x->x_height - ( *(x->x_probs+ei) * x->x_height / x->x_noccurrences ),
                  text_xpix(&x->x_obj, glist) + (ei+1) * x->x_width/x->x_nvalues,
                  text_ypix(&x->x_obj, glist) + x->x_height,
                  x, ei);
    }
    canvas_fixlinesfor( canvas, (t_text*)x );
}

static void probalizer_draw_update(t_probalizer *x)
{
    t_canvas *canvas=glist_getcanvas(x->x_glist);
    t_int ei;

    for ( ei=0; ei<x->x_nvalues; ei++ )
    {
        SYS_VGUI8(".x%lx.c coords %xPROB%d %d %d %d %d\n",
                  canvas, x, ei,
                  text_xpix(&x->x_obj, x->x_glist) + ei * x->x_width / x->x_nvalues,
                  text_ypix(&x->x_obj, x->x_glist) + x->x_height - ( *(x->x_probs+ei) * x->x_height / x->x_noccurrences ),
                  text_xpix(&x->x_obj, x->x_glist) + (ei+1) * x->x_width / x->x_nvalues,
                  text_ypix(&x->x_obj, x->x_glist) + x->x_height );
    }
    canvas_fixlinesfor( canvas, (t_text*)x );
}

static void probalizer_draw_move(t_probalizer *x, t_glist *glist)
{
    t_canvas *canvas=glist_getcanvas(glist);
    t_int ei;

    SYS_VGUI7(".x%lx.c coords %xPROBALIZER %d %d %d %d \n",
              canvas, x, text_xpix(&x->x_obj, glist), text_ypix(&x->x_obj, glist),
              text_xpix(&x->x_obj, glist)+x->x_width, text_ypix(&x->x_obj, glist)+x->x_height
             );
    SYS_VGUI7(".x%lx.c coords %xIN %d %d %d %d\n",
              canvas, x, text_xpix(&x->x_obj, glist),
              text_ypix(&x->x_obj, glist) - 2,
              text_xpix(&x->x_obj, glist) + 5,
              text_ypix(&x->x_obj, glist)
             );
    SYS_VGUI7(".x%lx.c coords %xOUT %d %d %d %d\n",
              canvas, x, text_xpix(&x->x_obj, glist),
              text_ypix(&x->x_obj, glist) + x->x_height,
              text_xpix(&x->x_obj, glist) + 5,
              text_ypix(&x->x_obj, glist) + x->x_height + 2
             );
    SYS_VGUI7(".x%lx.c coords %xOUT2 %d %d %d %d\n",
              canvas, x, text_xpix(&x->x_obj, glist) + x->x_width - 5,
              text_ypix(&x->x_obj, glist) + x->x_height,
              text_xpix(&x->x_obj, glist) + x->x_width,
              text_ypix(&x->x_obj, glist) + x->x_height + 2
             );
    SYS_VGUI5(".x%lx.c coords %xLTCAPTION %d %d\n",
              canvas, x, text_xpix(&x->x_obj, glist)-15,
              text_ypix(&x->x_obj, glist) + x->x_height
             );
    SYS_VGUI5(".x%lx.c coords %xLBCAPTION %d %d\n",
              canvas, x, text_xpix(&x->x_obj, glist)-15,
              text_ypix(&x->x_obj, glist)
             );
    SYS_VGUI5(".x%lx.c coords %xBLCAPTION %d %d\n",
              canvas, x,  text_xpix(&x->x_obj, glist)+2,
              text_ypix(&x->x_obj, glist) + x->x_height + 10
             );
    SYS_VGUI5(".x%lx.c coords %xBRCAPTION %d %d\n",
              canvas, x,  text_xpix(&x->x_obj, glist) + x->x_width - 5,
              text_ypix(&x->x_obj, glist) + x->x_height + 10
             );
    for ( ei=0; ei<x->x_nvalues; ei++ )
    {
        SYS_VGUI8(".x%lx.c coords %xPROB%d %d %d %d %d\n",
                  canvas, x, ei,
                  text_xpix(&x->x_obj, glist) + ei * x->x_width / x->x_nvalues,
                  text_ypix(&x->x_obj, glist) + x->x_height - ( *(x->x_probs+ei) * x->x_height / x->x_noccurrences ),
                  text_xpix(&x->x_obj, glist) + (ei+1) * x->x_width / x->x_nvalues,
                  text_ypix(&x->x_obj, glist) + x->x_height );
    }
    canvas_fixlinesfor( canvas, (t_text*)x );
}

static void probalizer_draw_erase(t_probalizer* x,t_glist* glist)
{
    t_canvas *canvas=glist_getcanvas(glist);
    int i, ei, gi;

    SYS_VGUI3(".x%lx.c delete %xPROBALIZER\n", canvas, x );
    SYS_VGUI3(".x%lx.c delete %xIN\n", canvas, x );
    SYS_VGUI3(".x%lx.c delete %xOUT\n", canvas, x );
    SYS_VGUI3(".x%lx.c delete %xOUT2\n", canvas, x );
    SYS_VGUI3(".x%lx.c delete %xLTCAPTION\n", canvas, x );
    SYS_VGUI3(".x%lx.c delete %xLBCAPTION\n", canvas, x );
    SYS_VGUI3(".x%lx.c delete %xBLCAPTION\n", canvas, x );
    SYS_VGUI3(".x%lx.c delete %xBRCAPTION\n", canvas, x );
    for ( ei=0; ei<x->x_nvalues; ei++ )
    {
        SYS_VGUI4(".x%lx.c delete %xPROB%d\n", canvas, x, ei );
    }
}

static void probalizer_draw_select(t_probalizer* x,t_glist* glist)
{
    t_canvas *canvas=glist_getcanvas(glist);

    if(x->x_selected)
    {
        /* sets the main item in blue */
        SYS_VGUI3(".x%lx.c itemconfigure %xPROBALIZER -outline #0000FF\n", glist_getcanvas(glist), x);

    }
    else
    {
        /* sets the main item in black */
        SYS_VGUI3(".x%lx.c itemconfigure %xPROBALIZER -outline #000000\n", glist_getcanvas(glist), x);
    }
}

/* ------------------------ probalizer widgetbehaviour----------------------------- */


static void probalizer_getrect(t_gobj *z, t_glist *owner,
                               int *xp1, int *yp1, int *xp2, int *yp2)
{
    t_probalizer* x = (t_probalizer*)z;

    *xp1 = text_xpix(&x->x_obj, owner);
    *yp1 = text_ypix(&x->x_obj, owner);
    *xp2 = text_xpix(&x->x_obj, owner)+x->x_width;
    *yp2 = text_ypix(&x->x_obj, owner)+x->x_height;
}

static void probalizer_save(t_gobj *z, t_binbuf *b)
{
    t_probalizer *x = (t_probalizer *)z;
    int ei,gi;

    binbuf_addv(b, "ssiisiiiii", gensym("#X"),gensym("obj"),
                (t_int)x->x_obj.te_xpix, (t_int)x->x_obj.te_ypix,
                atom_getsymbol(binbuf_getvec(x->x_obj.te_binbuf)),
                x->x_width, x->x_height,
                x->x_nvalues, x->x_noccurrences, x->x_save );
    if ( x->x_save )
    {
        for ( ei=0; ei<x->x_nvalues; ei++ )
        {
            // post( "probalizer : saving ( %d, %d )", ei, gi );
            binbuf_addv(b, "ii", ei, *(x->x_probs+ei) );
        }
    }
    binbuf_addv(b, ";");
}

static void probalizer_properties(t_gobj *z, t_glist *owner)
{
    char buf[800];
    t_probalizer *x=(t_probalizer *)z;

    sprintf(buf, "pdtk_probalizer_dialog %%s %d %d %d %d %d\n",
            x->x_width, x->x_height, x->x_nvalues, x->x_noccurrences, x->x_save );
    // post("probalizer_properties : %s", buf );
    gfxstub_new(&x->x_obj.ob_pd, x, buf);
}

static void probalizer_select(t_gobj *z, t_glist *glist, int selected)
{
    t_probalizer *x = (t_probalizer *)z;

    x->x_selected = selected;
    probalizer_draw_select( x, glist );
}

static void probalizer_vis(t_gobj *z, t_glist *glist, int vis)
{
    t_probalizer *x = (t_probalizer *)z;
    t_rtext *y;

    // post("probalizer_vis : %d", vis );
    if (vis)
    {
        probalizer_draw_new( x, glist );
    }
    else
    {
        probalizer_draw_erase( x, glist );
    }
}

/* handle parameters sent by the property dialog */
static void probalizer_dialog(t_probalizer *x, t_symbol *s, int argc, t_atom *argv)
{
    t_int onvalues, owidth, ei, gi, bi;
    t_canvas *canvas=glist_getcanvas(x->x_glist);

    if ( !x )
    {
        post( "probalizer : error :tried to set properties on an unexisting object" );
    }
    onvalues = x->x_nvalues;
    owidth = x->x_width;
    probalizer_draw_erase(x, x->x_glist);
    if ( argc < 5 )
    {
        post( "probalizer : error in the number of arguments ( %d )", argc );
        return;
    }
    if ( argv[0].a_type != A_FLOAT || argv[1].a_type != A_FLOAT ||
            argv[2].a_type != A_FLOAT || argv[3].a_type != A_FLOAT ||
            argv[4].a_type != A_FLOAT )
    {
        post( "probalizer : wrong arguments" );
        return;
    }

    x->x_width = argv[0].a_w.w_float;
    if ( x->x_width <= 0 ) x->x_width = 100;
    x->x_height = argv[1].a_w.w_float;
    if ( x->x_height <= 0 ) x->x_height = 100;
    x->x_nvalues = argv[2].a_w.w_float;
    if ( x->x_nvalues < 1 ) x->x_nvalues = 1;
    x->x_noccurrences = argv[3].a_w.w_float;
    if ( x->x_noccurrences < 1 ) x->x_noccurrences = 1;
    x->x_save = argv[4].a_w.w_float;

    // re-allocate arrays of values
    post( "probalizer : re-allocate values" );
    if ( onvalues != x->x_nvalues )
    {
        t_int mvalues = ( onvalues > x->x_nvalues ) ? x->x_nvalues : onvalues;
        t_int *newprobs, *newovalues;

        newprobs = (t_int*) getbytes( x->x_nvalues*sizeof(t_int) );
        newovalues = (t_int*) getbytes( x->x_nvalues*sizeof(t_int) );
        for ( ei=0; ei<x->x_nvalues; ei++ )
        {
            *(newprobs+ei)=DEFAULT_PROB_VALUE;
        }
        memset( newovalues, 0x0, x->x_nvalues*sizeof(t_int) );
        for ( ei=0; ei<mvalues; ei++ )
        {
            *(newprobs+ei ) = *(x->x_probs+ei);
        }
        if ( x->x_probs )
            freebytes( x->x_probs, onvalues*sizeof(t_int) );
        x->x_probs = newprobs;
        if ( x->x_ovalues )
            freebytes( x->x_ovalues, onvalues*sizeof(t_int) );
        x->x_ovalues = newovalues;
    }

    probalizer_draw_new(x, x->x_glist);
    canvas_fixlinesfor( canvas, (t_text*)x );
}

static void probalizer_delete(t_gobj *z, t_glist *glist)
{
    canvas_deletelinesfor(glist, (t_text *)z);
}

static void probalizer_displace(t_gobj *z, t_glist *glist, int dx, int dy)
{
    t_probalizer *x = (t_probalizer *)z;
    int xold = text_xpix(&x->x_obj, glist);
    int yold = text_ypix(&x->x_obj, glist);

    // post( "probalizer_displace dx=%d dy=%d", dx, dy );

    x->x_obj.te_xpix += dx;
    x->x_obj.te_ypix += dy;
    if(xold != x->x_obj.te_xpix || yold != x->x_obj.te_ypix)
    {
        probalizer_draw_move(x, x->x_glist);
    }
}

static int probalizer_click(t_gobj *z, struct _glist *glist,
                            int xpix, int ypix, int shift, int alt, int dbl, int doit)
{
    t_probalizer* x = (t_probalizer *)z;
    t_canvas *canvas=glist_getcanvas(glist);
    t_int nevent, npix;

    if ( doit)
    {
        int nevent;
        int newvalue;

        nevent = ((float)( xpix - text_xpix(&x->x_obj, glist) ))/((float)x->x_width/(float)x->x_nvalues);
        newvalue = ((float)(text_ypix(&x->x_obj, glist) + x->x_height - ypix))/( (float)x->x_height/(float)x->x_noccurrences);

        // post( "changed %d to %d", nevent, newvalue );

        // consistency check
        if ( nevent < 0 )
        {
            post( "probalizer : warning : negative event : %d", nevent );
            nevent = 0;
        }
        if ( nevent > x->x_nvalues - 1 )
        {
            post( "probalizer : warning : event bigger than number of values: %d", nevent );
            nevent =  x->x_nvalues - 1 ;
        }
        if ( newvalue < 0 )
        {
            post( "probalizer : warning : negative value : %d", newvalue );
            newvalue = 0;
        }
        if ( newvalue > x->x_noccurrences )
        {
            post( "probalizer : warning : value bigger than max occurrences : %d", newvalue );
            newvalue =  x->x_noccurrences ;
        }

        *( x->x_probs + nevent ) = newvalue;
        // reset all counters
        memset( x->x_ovalues, 0x0, x->x_nvalues*sizeof(t_int) );
        probalizer_draw_update( x );
    }
    return (1);
}

static t_probalizer *probalizer_new(t_symbol *s, int argc, t_atom *argv)
{
    int bi, i, ei;
    t_probalizer *x;
    t_pd *x2;

    // post( "probalizer_new : create : %s argc =%d", s->s_name, argc );

    x = (t_probalizer *)pd_new(probalizer_class);
    // create float output
    outlet_new(&x->x_obj, &s_float);
    // create bang output
    x->x_endoutlet = outlet_new(&x->x_obj, &s_bang);

    // new probalizer created from the gui
    if ( argc != 0 )
    {
        if ( argc < 5 )
        {
            post( "probalizer : error in the number of arguments ( %d )", argc );
            return NULL;
        }
        if ( argv[0].a_type != A_FLOAT || argv[1].a_type != A_FLOAT ||
                argv[2].a_type != A_FLOAT || argv[3].a_type != A_FLOAT ||
                argv[4].a_type != A_FLOAT )
        {
            post( "probalizer : wrong arguments" );
            return NULL;
        }

        x->x_width = argv[0].a_w.w_float;
        x->x_height = argv[1].a_w.w_float;
        x->x_nvalues = argv[2].a_w.w_float;
        if ( x->x_nvalues < 1 ) x->x_nvalues = 1;
        x->x_noccurrences = argv[3].a_w.w_float;
        if ( x->x_noccurrences < 1 ) x->x_noccurrences = 1;
        x->x_save = argv[4].a_w.w_float;
    }
    else
    {
        x->x_width = DEFAULT_PROBALIZER_WIDTH;
        x->x_height = DEFAULT_PROBALIZER_HEIGHT;
        x->x_nvalues = DEFAULT_PROBALIZER_NBVALUES;
        x->x_noccurrences = DEFAULT_PROBALIZER_NBOCCURRENCES;
        x->x_save = 1;
    }
    // common fields for new and restored probalizers
    x->x_probs = (t_int*) getbytes( x->x_nvalues*sizeof(t_int) );
    if ( !x->x_probs )
    {
        error( "probalizer : could not allocate buffer" );
        return NULL;
    }
    x->x_ovalues = (t_int*) getbytes( x->x_nvalues*sizeof(t_int) );
    if ( !x->x_ovalues )
    {
        error( "probalizer : could not allocate buffer" );
        return NULL;
    }
    for ( ei=0; ei<x->x_nvalues; ei++ )
    {
        *(x->x_probs+ei)=DEFAULT_PROB_VALUE;
    }
    memset( x->x_ovalues, 0x0, x->x_nvalues*sizeof(t_int) );
    x->x_selected = 0;
    x->x_glist = (t_glist *) canvas_getcurrent();

    // post( "probalizer : argc : %d", argc );
    if ( ( argc != 0 ) && ( x->x_save ) )
    {
        int ai = 5;
        int si = 0;

        while ( ai < argc - 1 )
        {
            *(x->x_probs + (int)argv[ai].a_w.w_float) = (int)argv[ai+1].a_w.w_float;
            ai += 2;
        }
    }
    // post( "probalizer_new width: %d height : %d", x->x_width, x->x_height );

    return (x);
}

/* zero counters */
static void probalizer_zero(t_probalizer *x)
{
    t_int ei;

    for ( ei=0; ei<x->x_nvalues; ei++ )
    {
        *(x->x_probs+ei)=0;
    }
    memset( x->x_ovalues, 0x0, x->x_nvalues*sizeof(t_int) );
    probalizer_draw_update( x );
}

/* the core of it, output an integer value out of the serial */
static void probalizer_bang(t_probalizer *x)
{
    t_int ei, ci;
    t_int *candidates;
    t_int nbcandidates = 0;
    t_int nevalues=0;

    // calculate number of eligible values
    for ( ei=0; ei<x->x_nvalues; ei++ )
    {
        nevalues += ( *(x->x_probs+ei) - *(x->x_ovalues+ei) );
    }

    if ( nevalues == 0 )
    {
        error( "probalizer : probabilities are null, sorry" );
        return;
    }

    candidates = ( t_int* ) getbytes( nevalues*sizeof( t_int ) );
    if ( !candidates )
    {
        error( "probalizer : could not allocate buffer for internal computation" );
        return;
    }

    // select eligible values
    for ( ei=0; ei<x->x_nvalues; ei++ )
    {
        if( *(x->x_ovalues+ei) < *(x->x_probs+ei) )
        {
            for ( ci=0; ci<*(x->x_probs+ei) - *(x->x_ovalues+ei); ci++)
            {
                *(candidates+nbcandidates) = ei;
                nbcandidates++;
            }
        }
    }

    // output one of the values
    {
        int chosen = random() % nbcandidates;
        int volue = *(candidates+chosen);

        outlet_float( x->x_obj.ob_outlet, volue+1 );
        *(x->x_ovalues+volue) = *(x->x_ovalues+volue) + 1;
        // for ( ei=0; ei<nbcandidates; ei++ )
        // {
        //     post( "probalizer : output : %d", *(x->x_ovalues+*(candidates+ei)) );
        // }

        // test end of the serial
        if (nbcandidates == 1 )
        {
            // post( "probalizer : end of the serial" );
            outlet_bang( x->x_endoutlet );
            memset( x->x_ovalues, 0x0, x->x_nvalues*sizeof(t_int) );
        }
    }

    if ( candidates )
    {
        freebytes( candidates,  nevalues*sizeof( t_int ) );
    }
}

/* increase a probability if possible */
static void probalizer_float(t_probalizer *x, t_float fvalue)
{
    if ( ( (fvalue-1 ) < 0 ) || ( (fvalue-1 ) >= x->x_nvalues ) )
    {
        post( "probalizer : wrong float value : %d", (int)fvalue );
        return;
    }
    if ( ( *(x->x_probs+(int)(fvalue-1))+1 ) <= x->x_noccurrences )
    {
        *(x->x_probs+(int)(fvalue-1))+=1;
        probalizer_draw_update( x );
    }
}

/* reset output */
static void probalizer_reset(t_probalizer *x)
{
    memset( x->x_ovalues, 0x0, x->x_nvalues*sizeof(t_int) );
}

/* equi probability */
static void probalizer_equi(t_probalizer *x)
{
    t_int ei;

    for ( ei=0; ei<x->x_nvalues; ei++ )
    {
        *(x->x_probs+ei)=1;
    }
    memset( x->x_ovalues, 0x0, x->x_nvalues*sizeof(t_int) );
    probalizer_draw_update( x );
}

static void probalizer_free(t_probalizer *x)
{
    t_int ei;

    if ( x->x_probs )
    {
        freebytes( x->x_probs, x->x_nvalues*sizeof(int) );
    }
    if ( x->x_ovalues )
    {
        freebytes( x->x_ovalues, x->x_nvalues*sizeof(int) );
    }
}

void probalizer_setup(void)
{
    logpost(NULL, 4,  probalizer_version );
    probalizer_class = class_new(gensym("probalizer"), (t_newmethod)probalizer_new,
                                 (t_method)probalizer_free, sizeof(t_probalizer), 0, A_GIMME, 0);
    class_addmethod(probalizer_class, (t_method)probalizer_dialog, gensym("dialog"), A_GIMME, 0);
    class_addmethod(probalizer_class, (t_method)probalizer_float, &s_float, A_FLOAT, 0);
    class_addmethod(probalizer_class, (t_method)probalizer_bang, &s_bang, 0);
    class_addmethod(probalizer_class, (t_method)probalizer_zero, gensym("zero"), 0);
    class_addmethod(probalizer_class, (t_method)probalizer_equi, gensym("equi"), 0);
    class_addmethod(probalizer_class, (t_method)probalizer_reset, gensym("reset"), 0);
    probalizer_widgetbehavior.w_getrectfn =    probalizer_getrect;
    probalizer_widgetbehavior.w_displacefn =   probalizer_displace;
    probalizer_widgetbehavior.w_selectfn =     probalizer_select;
    probalizer_widgetbehavior.w_activatefn =   NULL;
    probalizer_widgetbehavior.w_deletefn =     probalizer_delete;
    probalizer_widgetbehavior.w_visfn =        probalizer_vis;
    probalizer_widgetbehavior.w_clickfn =      probalizer_click;

#if PD_MINOR_VERSION >= 37
    class_setpropertiesfn(probalizer_class, probalizer_properties);
    class_setsavefn(probalizer_class, probalizer_save);
#else
    probalizer_widgetbehavior.w_propertiesfn = probalizer_properties;
    probalizer_widgetbehavior.w_savefn =       probalizer_save;
#endif

    class_setwidget(probalizer_class, &probalizer_widgetbehavior);


    sys_vgui("eval [read [open {%s/%s.tcl}]]\n",
             probalizer_class->c_externdir->s_name,
             probalizer_class->c_name->s_name);
}
