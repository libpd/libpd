/* ---------------------------------------------------------------------------- */
/* Copyright (c) 2002 Yves Degoyon.                                             */
/* For information on usage and redistribution, and for a DISCLAIMER OF ALL     */
/* WARRANTIES, see the file, "LICENSE.txt," in this distribution.               */
/*                                                                              */
/* filterbank.c written by Yves Degoyon 2002                                    */
/* outputs frequency responses against a bank of filters                        */
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
/* The Three Johns -- Teenage Nightingales In Wax                               */
/* Kk Null & Jim O Rourke - Neuro Politics                                      */
/* ---------------------------------------------------------------------------- */


#include "filterbank~.h"

#ifdef _WIN32
# include <io.h>
# define random rand
#endif

#ifndef _MSC_VER /* only MSVC doesn't have unistd.h */
#include <unistd.h>
#endif

#define DEFAULT_FILTERBANK_LOWFREQ 0
#define DEFAULT_FILTERBANK_HIGHFREQ 1600
#define DEFAULT_FILTERBANK_NBFILTERS 10
#define FILTERBANK_OUTLET_WIDTH 5
#define FILTERBANK_HEIGHT 16

static char   *filterbank_version = "filterbank : responses from a set of band-pass filters, version 0.4 (ydegoyon@free.fr)";

t_widgetbehavior filterbank_widgetbehavior;
static t_class *filterbank_class_tilde;

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
                         post(a,b,c,d,e,f,g);\
                         sys_vgui(a,b,c,d,e,f,g)

#define SYS_VGUI8(a,b,c,d,e,f,g,h) if (guidebug) \
                         post(a,b,c,d,e,f,g,h);\
                         sys_vgui(a,b,c,d,e,f,g,h)

#define SYS_VGUI9(a,b,c,d,e,f,g,h,i) if (guidebug) \
                         post(a,b,c,d,e,f,g,h,i);\
                         sys_vgui(a,b,c,d,e,f,g,h,i)

#define SYS_VGUI10(a,b,c,d,e,f,g,h,i,j) if (guidebug) \
                         post(a,b,c,d,e,f,g,h,i,j);\
                         sys_vgui(a,b,c,d,e,f,g,h,i,j)

#define SYS_VGUI11(a,b,c,d,e,f,g,h,i,j,k) if (guidebug) \
                         post(a,b,c,d,e,f,g,h,i,j,k);\
                         sys_vgui(a,b,c,d,e,f,g,h,i,j,k)

/* this code is borrowed from pd's internal object : bp~ */

static float miller_sigbp_qcos(float f)
{
    if (f >= -(0.5f*M_PI) && f <= 0.5f*M_PI)
    {
        float g = f*f;
        return (((g*g*g * (-1.0f/720.0f) + g*g*(1.0f/24.0f)) - g*0.5) + 1);
    }
    else return (0);
}

static void miller_sigbp_docoef(t_filterbank_tilde *x, t_int index, t_floatarg f, t_floatarg q)
{
    float r, oneminusr, omega;
    if (f < 0.001) f = 10;
    if (q < 0) q = 0;
    x->x_freq[index] = f;
    x->x_q[index] = q;
    omega = f * (2.0f * 3.14159f) / x->x_sr;
    if (q < 0.001) oneminusr = 1.0f;
    else oneminusr = omega/q;
    if (oneminusr > 1.0f) oneminusr = 1.0f;
    r = 1.0f - oneminusr;
    x->x_ctl[index]->c_coef1 = 2.0f * miller_sigbp_qcos(omega) * r;
    x->x_ctl[index]->c_coef2 = - r * r;
    x->x_ctl[index]->c_gain = 2 * oneminusr * (oneminusr + r * omega);
}

static void filterbank_draw_new(t_filterbank_tilde *x, t_glist *glist)
{
    t_canvas *canvas=glist_getcanvas(glist);
    int fi;

    // draw the square
    {

        SYS_VGUI7(".x%lx.c create rectangle %d %d %d %d -fill #FFFFFF -tags %xFILTERBANK\n",
                  canvas,
                  text_xpix(&x->x_obj, glist),
                  text_ypix(&x->x_obj, glist),
                  text_xpix(&x->x_obj, glist) + x->x_width,
                  text_ypix(&x->x_obj, glist) + x->x_height,
                  x);

        SYS_VGUI7(".x%lx.c create rectangle %d %d %d %d -fill #000000 -tags %xSIN\n",
                  canvas,
                  text_xpix(&x->x_obj, glist),
                  text_ypix(&x->x_obj, glist)-1,
                  text_xpix(&x->x_obj, glist)+7,
                  text_ypix(&x->x_obj, glist),
                  x);

        for ( fi=0; fi<x->x_nbfilters; fi++ )
        {
            char color[8];

            sprintf( color, "#%.2x%.2x%.2x", (int)random() % 256, (int)random() % 256, (int)random() % 256 );

            SYS_VGUI11(".x%lx.c create polygon %d %d %d %d %d %d -outline #000000 -fill %s -tags %xFILTER%d\n",
                       canvas,
                       text_xpix(&x->x_obj, glist) + fi*x->x_width/x->x_nbfilters,
                       text_ypix(&x->x_obj, glist),
                       text_xpix(&x->x_obj, glist) + fi*x->x_width/x->x_nbfilters + x->x_width/(2*x->x_nbfilters),
                       text_ypix(&x->x_obj, glist) + x->x_height,
                       text_xpix(&x->x_obj, glist) + (fi+1)*x->x_width/x->x_nbfilters,
                       text_ypix(&x->x_obj, glist),
                       color, x, fi);
        }
    }

    canvas_fixlinesfor( canvas, (t_text*)x );
}

static void filterbank_draw_move(t_filterbank_tilde *x, t_glist *glist)
{
    t_canvas *canvas=glist_getcanvas(glist);
    t_int fi;

    SYS_VGUI7(".x%lx.c coords %xFILTERBANK %d %d %d %d\n",
              canvas, x,
              text_xpix(&x->x_obj, glist),
              text_ypix(&x->x_obj, glist),
              text_xpix(&x->x_obj, glist) + x->x_width,
              text_ypix(&x->x_obj, glist) + x->x_height
             );

    SYS_VGUI7(".x%lx.c coords %xSIN %d %d %d %d\n",
              canvas, x,
              text_xpix(&x->x_obj, glist),
              text_ypix(&x->x_obj, glist)-1,
              text_xpix(&x->x_obj, glist)+7,
              text_ypix(&x->x_obj, glist)
             );

    for ( fi=0; fi<x->x_nbfilters; fi++ )
    {
        SYS_VGUI10(".x%lx.c coords %xFILTER%d %d %d %d %d %d %d\n",
                   canvas, x, fi,
                   text_xpix(&x->x_obj, glist) + fi*x->x_width/x->x_nbfilters,
                   text_ypix(&x->x_obj, glist),
                   text_xpix(&x->x_obj, glist) + fi*x->x_width/x->x_nbfilters + x->x_width/(2*x->x_nbfilters),
                   text_ypix(&x->x_obj, glist) + x->x_height,
                   text_xpix(&x->x_obj, glist) + (fi+1)*x->x_width/x->x_nbfilters,
                   text_ypix(&x->x_obj, glist)
                  );
    }

    canvas_fixlinesfor( canvas, (t_text*)x );
}

static void filterbank_draw_erase(t_filterbank_tilde* x,t_glist* glist)
{
    t_canvas *canvas=glist_getcanvas(glist);
    int fi;

    SYS_VGUI3(".x%lx.c delete %xFILTERBANK\n", canvas, x );
    SYS_VGUI3(".x%lx.c delete %xSIN\n", canvas, x );
    for ( fi=0; fi<x->x_nbfilters; fi++ )
    {
        SYS_VGUI4(".x%lx.c delete %xFILTER%d\n", canvas, x, fi );
    }

}

static void filterbank_draw_select(t_filterbank_tilde* x,t_glist* glist)
{
    t_canvas *canvas=glist_getcanvas(glist);

    if(x->x_selected)
    {
        /* sets the item in blue */
        SYS_VGUI3(".x%lx.c itemconfigure %xFILTERBANK -outline #0000FF\n", canvas, x);
    }
    else
    {
        SYS_VGUI3(".x%lx.c itemconfigure %xFILTERBANK -outline #000000\n", canvas, x);
    }
}

/* ------------------------ filterbank widgetbehaviour----------------------------- */


static void filterbank_getrect(t_gobj *z, t_glist *owner,
                               int *xp1, int *yp1, int *xp2, int *yp2)
{
    t_filterbank_tilde* x = (t_filterbank_tilde*)z;

    *xp1 = text_xpix(&x->x_obj, owner);
    *yp1 = text_ypix(&x->x_obj, owner);
    *xp2 = text_xpix(&x->x_obj, owner)+x->x_width;
    *yp2 = text_ypix(&x->x_obj, owner)+x->x_height;
}

static void filterbank_save(t_gobj *z, t_binbuf *b)
{
    t_filterbank_tilde *x = (t_filterbank_tilde *)z;
    t_int ii;

    binbuf_addv(b, "ssiisiii", gensym("#X"),gensym("obj"),
                (t_int)x->x_obj.te_xpix, (t_int)x->x_obj.te_ypix,
                atom_getsymbol(binbuf_getvec(x->x_obj.te_binbuf)),
                x->x_lowfreq, x->x_highfreq,
                x->x_nbfilters );
    binbuf_addv(b, ";");
}

static void filterbank_properties(t_gobj *z, t_glist *owner)
{
    char buf[800];
    t_filterbank_tilde *x=(t_filterbank_tilde *)z;

    sprintf(buf, "pdtk_filterbank_dialog %%s %d %d\n",
            x->x_lowfreq, x->x_highfreq );
    gfxstub_new(&x->x_obj.ob_pd, x, buf);
}

static void filterbank_select(t_gobj *z, t_glist *glist, int selected)
{
    t_filterbank_tilde *x = (t_filterbank_tilde *)z;

    x->x_selected = selected;
    filterbank_draw_select( x, glist );
}

static void filterbank_vis(t_gobj *z, t_glist *glist, int vis)
{
    t_filterbank_tilde *x = (t_filterbank_tilde *)z;

    // post( "filterbank~ : vis : %d", vis );
    if (vis)
    {
        filterbank_draw_new( x, glist );
    }
    else
    {
        filterbank_draw_erase( x, glist );
    }
}

static void filterbank_dialog(t_filterbank_tilde *x, t_symbol *s, int argc, t_atom *argv)
{
    t_int olowfreq = x->x_lowfreq;
    t_int ohighfreq = x->x_highfreq;
    t_int fi, ei;
    t_int dspstate;
    t_float Q;
    t_float afreq, abandwidth;
    t_canvas *canvas=glist_getcanvas(x->x_glist);

    // !!paranoid
    if ( !x )
    {
        post( "filterbank~ : error :tried to set properties on an unexisting object" );
    }
    if ( argc != 2 )
    {
        post( "filterbank : error in the number of arguments ( %d instead of 2 )", argc );
        return;
    }
    if ( argv[0].a_type != A_FLOAT || argv[1].a_type != A_FLOAT )
    {
        post( "filterbank~ : wrong arguments" );
        return;
    }

    x->x_allocate = 1;
    x->x_lowfreq = (int)argv[0].a_w.w_float;
    if ( x->x_lowfreq < 0 ) x->x_lowfreq = 0;
    x->x_highfreq = (int)argv[1].a_w.w_float;
    if ( x->x_highfreq < x->x_lowfreq ) x->x_highfreq = x->x_lowfreq + 100;

    // recalculate filters if needed
    if ( ( olowfreq != x->x_lowfreq ) || ( ohighfreq != x->x_highfreq ) )
    {
        // free filters
        if ( x->x_freq )
        {
            freebytes( x->x_freq, x->x_nbfilters*sizeof(t_float) );
        }
        if ( x->x_q )
        {
            freebytes( x->x_q, x->x_nbfilters*sizeof(t_float) );
        }
        if ( x->x_cspace )
        {
            freebytes( x->x_cspace, x->x_nbfilters*sizeof(t_bpctl) );
        }
        if ( x->x_ctl )
        {
            freebytes( x->x_ctl, x->x_nbfilters*sizeof(t_bpctl*) );
        }
        // create filters
        x->x_freq = (t_float *) getbytes( x->x_nbfilters*sizeof(t_float) );
        x->x_q = (t_float *) getbytes( x->x_nbfilters*sizeof(t_float) );
        x->x_cspace = (t_bpctl *) getbytes( x->x_nbfilters*sizeof(t_bpctl) );
        x->x_ctl = (t_bpctl **) getbytes( x->x_nbfilters*sizeof(t_bpctl*) );
        if ( !x->x_freq || !x->x_q || !x->x_cspace || !x->x_ctl )
        {
            post( "filterbank~ : could not allocate filters" );
            return;
        }
        abandwidth = ( x->x_highfreq - x->x_lowfreq ) / x->x_nbfilters;
        afreq = x->x_lowfreq + ( abandwidth / 2 );
        for ( fi=0; fi<x->x_nbfilters; fi++ )
        {
            x->x_ctl[fi] = &x->x_cspace[fi];
            x->x_cspace[fi].c_x1 = 0;
            x->x_cspace[fi].c_x2 = 0;
            Q = ( (t_float) afreq )/ ( (t_float) abandwidth );
            miller_sigbp_docoef( x, fi, afreq, Q );
            afreq += abandwidth;
        }
    }

    x->x_allocate = 0;
}

static void filterbank_delete(t_gobj *z, t_glist *glist)
{
    t_filterbank_tilde *x = (t_filterbank_tilde *)z;

    // post( "filterbank~ : delete" );
    filterbank_draw_erase( x, glist );
    canvas_deletelinesfor(glist, (t_text *)z);
}

static void filterbank_displace(t_gobj *z, t_glist *glist, int dx, int dy)
{
    t_filterbank_tilde *x = (t_filterbank_tilde *)z;
    int xold = text_xpix(&x->x_obj, glist);
    int yold = text_ypix(&x->x_obj, glist);

    // post( "filterbank_displace dx=%d dy=%d", dx, dy );

    x->x_obj.te_xpix += dx;
    x->x_obj.te_ypix += dy;
    if(xold != text_xpix(&x->x_obj, glist) || yold != text_ypix(&x->x_obj, glist) )
    {
        filterbank_draw_move(x, x->x_glist);
    }
}


static void filterbank_randomize(t_filterbank_tilde *x, t_floatarg fflag )
{
    t_int shind, tmpi, fi;

    if ( fflag != 0.0 && fflag != 1.0 )
    {
        post( "filterbank~ : wrong argument in randomize message : should be 0 or 1" );
        return;
    }
    else if ( fflag == 1 )
    {
        for ( fi=0; fi<x->x_nbfilters; fi++ )
        {
            shind = rand() % x->x_nbfilters;
            tmpi = x->x_outmapping[ shind ];
            x->x_outmapping[ shind ] = x->x_outmapping[ fi ];
            x->x_outmapping[ fi ] = tmpi;
        }
    }
    else
    {
        for ( fi=0; fi<x->x_nbfilters; fi++ )
        {
            x->x_outmapping[ fi ] = fi;
        }
    }
}

static t_filterbank_tilde *filterbank_new(t_symbol *s, int argc, t_atom *argv)
{
    t_int fi, ei;
    t_filterbank_tilde *x;
    char *str;
    t_float Q;
    t_float afreq, abandwidth;

    x = (t_filterbank_tilde *)pd_new(filterbank_class_tilde);

    x->x_samplerate = (int)sys_getsr();
    x->x_sr = 44100;

    // new filterbank created from the gui
    if ( argc != 0 )
    {
        if ( argc != 3 )
        {
            post( "filterbank~ : error in the number of arguments ( %d )", argc );
            return NULL;
        }
        if ( argv[0].a_type != A_FLOAT || argv[1].a_type != A_FLOAT ||
                argv[2].a_type != A_FLOAT )
        {
            post( "filterbank~ : wrong arguments" );
            return NULL;
        }

        x->x_lowfreq = (int)argv[0].a_w.w_float;
        if ( x->x_lowfreq < 0 ) x->x_lowfreq = 0;
        x->x_highfreq = (int)argv[1].a_w.w_float;
        if ( x->x_highfreq < x->x_lowfreq ) x->x_highfreq = x->x_lowfreq + 100;
        x->x_nbfilters = (int)argv[2].a_w.w_float;
        if ( x->x_nbfilters < 1 ) x->x_nbfilters = 1;
    }
    else
    {
        x->x_lowfreq = DEFAULT_FILTERBANK_LOWFREQ;
        x->x_highfreq = DEFAULT_FILTERBANK_HIGHFREQ;
        x->x_nbfilters = DEFAULT_FILTERBANK_NBFILTERS;
    }

    // post( "filterbank~ : new [ %d,%d ] with %d filters", x->x_lowfreq, x->x_highfreq, x->x_nbfilters );

    // create outlets
    x->x_outputs = (t_outlet **) getbytes( x->x_nbfilters*sizeof(t_outlet *) );
    if ( !x->x_outputs )
    {
        post( "filterbank~ : could not allocate outputs" );
        return NULL;
    }
    for ( fi=0; fi<x->x_nbfilters; fi++ )
    {
        x->x_outputs[fi] = outlet_new( &x->x_obj, &s_signal );
    }

    // create filters
    x->x_freq = (t_float *) getbytes( x->x_nbfilters*sizeof(t_float) );
    x->x_q = (t_float *) getbytes( x->x_nbfilters*sizeof(t_float) );
    x->x_cspace = (t_bpctl *) getbytes( x->x_nbfilters*sizeof(t_bpctl) );
    x->x_ctl = (t_bpctl **) getbytes( x->x_nbfilters*sizeof(t_bpctl*) );
    if ( !x->x_freq || !x->x_q || !x->x_cspace || !x->x_ctl )
    {
        post( "filterbank~ : could not allocate filters" );
        return NULL;
    }
    abandwidth = ( x->x_highfreq - x->x_lowfreq ) / x->x_nbfilters;
    afreq = x->x_lowfreq + ( abandwidth / 2 );
    for ( fi=0; fi<x->x_nbfilters; fi++ )
    {
        x->x_ctl[fi] = &x->x_cspace[fi];
        x->x_cspace[fi].c_x1 = 0;
        x->x_cspace[fi].c_x2 = 0;
        Q = ( (t_float) afreq ) / ( (t_float) abandwidth );
        miller_sigbp_docoef( x, fi, afreq, Q );
        afreq += abandwidth;
    }

    x->x_width = x->x_nbfilters*FILTERBANK_OUTLET_WIDTH*2;
    x->x_height = FILTERBANK_HEIGHT;

    x->x_glist = (t_glist *) canvas_getcurrent();
    x->x_f = 0;

    x->x_outmapping = (t_int*) getbytes( x->x_nbfilters*sizeof( t_int ) );
    if ( !x->x_outmapping )
    {
        post( "filterbank~ : cannot allocate mapping array" );
        return NULL; // otherwise, pd schrieks
    }
    for ( fi=0; fi<x->x_nbfilters; fi++ )
    {
        x->x_outmapping[fi] = fi;
    }
    return (x);
}

static void filterbank_free(t_filterbank_tilde *x)
{
    t_int ei, fi;

    if ( x->x_outputs )
    {
        for ( ei=0; ei<x->x_nbfilters; ei++ )
        {
            outlet_free( x->x_outputs[ei] );
        }
        freebytes( x->x_outputs, x->x_nbfilters*sizeof(t_outlet*) );
    }
    if ( x->x_freq )
    {
        freebytes( x->x_freq, x->x_nbfilters*sizeof(t_float) );
    }
    if ( x->x_q )
    {
        freebytes( x->x_q, x->x_nbfilters*sizeof(t_float) );
    }
    if ( x->x_cspace )
    {
        freebytes( x->x_cspace, x->x_nbfilters*sizeof(t_bpctl) );
    }
    if ( x->x_ctl )
    {
        freebytes( x->x_ctl, x->x_nbfilters*sizeof(t_bpctl*) );
    }
    if ( x->x_outmapping )
    {
        freebytes( x->x_outmapping, x->x_nbfilters*sizeof(t_int) );
    }
}

static t_int *filterbank_perform(t_int *w)
{
    t_int fi, si;
    t_filterbank_tilde *x = (t_filterbank_tilde*)(w[1]);
    t_int n = w[2];
    t_float *in, *out;
    int i;
    t_float last, prev, coef1, coef2, gain;
    t_float *acopy;
    t_int noneedtofilter = 1;

    in = (t_float*)w[3];

    // copy input audio block
    acopy = (t_float*) getbytes( n*sizeof( t_float ) );
    if ( !acopy )
    {
        post( "filterbank~ : cannot allocate audio copy block" );
        return 0; // otherwise, pd schrieks
    }
    memcpy( acopy, in, n*sizeof(t_float) );

    for ( i=0; i<n; i++ )
    {
        if ( *(acopy+i) != 0.0 ) noneedtofilter = 0;
    }

    if ( !noneedtofilter )
    {
        for ( fi=0; fi<x->x_nbfilters; fi++ )
        {
            out = (t_float *)(w[x->x_outmapping[fi]+4]);

            last = x->x_ctl[fi]->c_x1;
            prev = x->x_ctl[fi]->c_x2;
            coef1 = x->x_ctl[fi]->c_coef1;
            coef2 = x->x_ctl[fi]->c_coef2;
            gain = x->x_ctl[fi]->c_gain;
            for (i=0; i < n; i++)
            {
                float output =  *(acopy+i) + coef1 * last + coef2 * prev;
                *out++ = gain * output;
                prev = last;
                last = output;
            }
            /* NAN protect */
            if (!((last <= 0) || (last >= 0)))
                last = 0;
            if (!((prev <= 0) || (prev >= 0)))
                prev = 0;
            x->x_ctl[fi]->c_x1 = last;
            x->x_ctl[fi]->c_x2 = prev;
        }
    }
    else
    {
        for ( fi=0; fi<x->x_nbfilters; fi++ )
        {
            out = (t_float *)(w[x->x_outmapping[fi]+4]);
            for (i=0; i < n; i++)
            {
                *out++ = 0.0;
            }
        }
    }

    if ( acopy ) freebytes( acopy, n*sizeof(t_float) );

    return (w+x->x_nbfilters+4);
}

static void filterbank_dsp(t_filterbank_tilde *x, t_signal **sp)
{
    t_int *dspargs, fi, nbargs;

    dspargs = (t_int*) getbytes( (x->x_nbfilters+3)*sizeof(t_int) );

    dspargs[0] = (t_int)x;
    dspargs[1] = (t_int)sp[0]->s_n;
    dspargs[2] = (t_int)sp[0]->s_vec;

    nbargs = 3;
    for ( fi=0; fi<x->x_nbfilters; fi++ )
    {
        dspargs[3+fi] = (t_int)sp[fi+1]->s_vec;
        nbargs++;
    }

    dsp_addv(filterbank_perform, nbargs, dspargs );

    if ( dspargs ) freebytes( dspargs, (x->x_nbfilters+3)*sizeof(t_int) );
}

void filterbank_tilde_setup(void)
{
    logpost(NULL, 4,  filterbank_version );
    filterbank_class_tilde = class_new(gensym("filterbank~"), (t_newmethod)filterbank_new,
                                       (t_method)filterbank_free, sizeof(t_filterbank_tilde), 0, A_GIMME, 0);
    CLASS_MAINSIGNALIN( filterbank_class_tilde, t_filterbank_tilde, x_f );
    class_addmethod(filterbank_class_tilde, (t_method)filterbank_dsp, gensym("dsp"), 0);
    class_addmethod(filterbank_class_tilde, (t_method)filterbank_dialog, gensym("dialog"), A_GIMME, 0);
    class_addmethod(filterbank_class_tilde, (t_method)filterbank_randomize, gensym("randomize"), A_DEFFLOAT, 0);
    filterbank_widgetbehavior.w_getrectfn =    filterbank_getrect;
    filterbank_widgetbehavior.w_displacefn =   filterbank_displace;
    filterbank_widgetbehavior.w_selectfn =     filterbank_select;
    filterbank_widgetbehavior.w_activatefn =   NULL;
    filterbank_widgetbehavior.w_deletefn =     filterbank_delete;
    filterbank_widgetbehavior.w_visfn =        filterbank_vis;
    filterbank_widgetbehavior.w_clickfn =      NULL;

#if PD_MINOR_VERSION >= 37
    class_setpropertiesfn(filterbank_class_tilde, filterbank_properties);
    class_setsavefn(filterbank_class_tilde, filterbank_save);
#else
    filterbank_widgetbehavior.w_propertiesfn = filterbank_properties;
    filterbank_widgetbehavior.w_savefn =       filterbank_save;
#endif
    class_setwidget(filterbank_class_tilde, &filterbank_widgetbehavior);

    sys_vgui("eval [read [open {%s/%s.tcl}]]\n",
             filterbank_class_tilde->c_externdir->s_name,
             filterbank_class_tilde->c_name->s_name);
}
