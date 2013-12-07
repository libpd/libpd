/*------------------------ scratcher~ ------------------------------------------ */
/*                                                                              */
/* scratcher~ : lets you record, and then, scratch a sound.                     */
/* constructor : scratcher~                                                     */
/*                                                                              */
/* Copyleft Yves Degoyon ( ydegoyon@free.fr )                                   */
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
/* "I've lost my kids and wife"                                                 */
/* "Because I got high da da dap da da dap"                                     */
/* Afro Man -- Because I Got High                                               */
/* ---------------------------------------------------------------------------- */



#include <sys/types.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>
#ifdef __APPLE__
#include <sys/malloc.h>
#else
#include <malloc.h>
#endif
#include <ctype.h>
#include <pthread.h>
#include <time.h>
#include <sys/time.h>
#include <math.h>

#ifdef _WIN32
#define M_PI 3.14159265358979323846
#else
#include <unistd.h>
#endif

#include "m_pd.h"
#include "m_imp.h"
#include "g_canvas.h"


static int guidebug=0;
static int ignorevisible=1; // ignore visible test
// because this seems to lead to bad refresh
// wait for a fix

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
                         post(a,b,c,d,e,f,g,h);\
                         sys_vgui(a,b,c,d,e,f,g,h)

#define SYS_VGUI9(a,b,c,d,e,f,g,h,i) if (guidebug) \
                         post(a,b,c,d,e,f,g,h,i );\
                         sys_vgui(a,b,c,d,e,f,g,h,i)

#define SYS_VGUI10(a,b,c,d,e,f,g,h,i,j) if (guidebug) \
                         post(a,b,c,d,e,f,g,h,i,j );\
                         sys_vgui(a,b,c,d,e,f,g,h,i,j)

#define SYS_VGUI11(a,b,c,d,e,f,g,h,i,j,k) if (guidebug) \
                         post(a,b,c,d,e,f,g,h,i,j,k );\
                         sys_vgui(a,b,c,d,e,f,g,h,i,j,k)

#define DEFAULT_SCRATCHER_SIZE 88200   // 2 seconds at 44100 hertz
#define DEFAULT_SCRATCHER_WIDTH 200
#define DEFAULT_SCRATCHER_HEIGHT 200
#define DEFAULT_SCRATCHER_SENSIBILITY 25
#define DEFAULT_SCRATCHER_MAX_SPEED 2.0
#define DEFAULT_TURNTABLE_INERTIA 0.01

#define SCRATCHER_NB_GROOVES 20
#define SCRATCHER_MOVE_TIMEOUT 20

static char   *scratcher_version = "scratcher~: version 0.10, written by Yves Degoyon (ydegoyon@free.fr)";

static t_class *scratcher_class;
t_widgetbehavior scratcher_widgetbehavior;

typedef struct _scratcher
{
    t_object x_obj;

    t_int x_size;                  /* size of the recorded sound ( in samples ) */
    t_float x_samplerate;          /* sample rate */
    t_int x_blocksize;             /* current block size ( might be modified by block~ object ) */
    t_float x_readpos;             /* data's playing position */
    t_int x_writepos;              /* data's recording position */
    t_int x_play;                  /* playing on/off flag */
    t_float x_readspeed;           /* number of grouped blocks for reading */
    t_float x_maxspeed;            /* maximum speed limit                  */
    t_float x_record;              /* flag to start recording process */
    t_int x_empty;                 /* flag to indicate it's a brand new scratcher   */
    t_float x_speedinc;            /* speed increment                               */
    long long x_lastmovetime;      /* instant ( in ms ) of the last movement        */
    t_float *x_sdata;              /* sound data                                */
    t_int   x_sensibility;         /* sensibility ( amount of change for a dy ) */
    t_int   x_motioned;            /* flag to indicate the mouse motion         */
    t_int   x_mousemoved;          /* flag to indicate the mouse movement       */
    t_float x_inertia;             /* turntable inertia                         */

    /* graphical data block */
    t_int x_selected;              /* flag to remember if we are seleted or not   */
    t_int x_width;                 /* width of the graphical object               */
    t_int x_height;                /* height of the graphical object              */
    t_int x_showspeed;             /* toggle on/off speed line                    */
    t_glist *x_glist;              /* keep graphic context for various operations */

    t_float x_f;                   /* float needed for signal input */

} t_scratcher;

/* ------------------------ drawing functions ---------------------------- */

static void scratcher_draw_new(t_scratcher *x, t_glist *glist)
{
    t_canvas *canvas=glist_getcanvas(glist);
    t_int ci;

    SYS_VGUI7(".x%lx.c create oval %d %d %d %d -fill #000000 -tags %xSCRATCHER\n",
              canvas, text_xpix(&x->x_obj, glist), text_ypix(&x->x_obj, glist),
              text_xpix(&x->x_obj, glist) + x->x_width,
              text_ypix(&x->x_obj, glist) + x->x_height,
              x);
    for ( ci=0; ci<SCRATCHER_NB_GROOVES; ci ++)
    {
        SYS_VGUI8(".x%lx.c create oval %d %d %d %d -outline #FFFFFF -tags %xGROOVE%d\n",
                  canvas, text_xpix(&x->x_obj, glist) + ci*x->x_width/(2*SCRATCHER_NB_GROOVES),
                  text_ypix(&x->x_obj, glist) + ci*x->x_height/(2*SCRATCHER_NB_GROOVES),
                  text_xpix(&x->x_obj, glist) + x->x_width - ci*x->x_width/(2*SCRATCHER_NB_GROOVES),
                  text_ypix(&x->x_obj, glist) + x->x_height - ci*x->x_height/(2*SCRATCHER_NB_GROOVES),
                  x, ci);
    }
    if ( x->x_showspeed )
    {
        SYS_VGUI7( ".x%lx.c create line %d %d %d %d -fill #FF0000 -tags %xSPEEDBAR -width 3\n",
                   canvas, text_xpix(&x->x_obj, glist)+x->x_width/2,
                   text_ypix(&x->x_obj, glist)+x->x_height/2,
                   text_xpix(&x->x_obj, glist)+x->x_width/2 + (int)(x->x_width/2*cos( x->x_readspeed - 1. )),
                   text_ypix(&x->x_obj, glist)+x->x_height/2 - (int)(x->x_width/2*sin( x->x_readspeed - 1. )),
                   x );
    }
    SYS_VGUI7(".x%lx.c create rectangle %d %d %d %d -tags %xFSCRATCHER\n",
              canvas, text_xpix(&x->x_obj, glist), text_ypix(&x->x_obj, glist),
              text_xpix(&x->x_obj, glist) + x->x_width,
              text_ypix(&x->x_obj, glist) + x->x_height,
              x);
    canvas_fixlinesfor( canvas, (t_text*)x );
}

static void scratcher_draw_delete(t_scratcher *x, t_glist *glist)
{
    t_int ci;
    t_canvas *canvas=glist_getcanvas(glist);

    if ( glist_isvisible( glist ) )
    {
        SYS_VGUI3( ".x%lx.c delete %xSCRATCHER\n", canvas, x );
        SYS_VGUI3( ".x%lx.c delete %xFSCRATCHER\n", canvas, x );
        SYS_VGUI3( ".x%lx.c delete %xSPEEDBAR\n", canvas, x );
        for ( ci=0; ci<SCRATCHER_NB_GROOVES; ci ++)
        {
            SYS_VGUI4( ".x%lx.c delete %xGROOVE%d\n", canvas, x, ci );
        }
    }
}

static void scratcher_draw_move(t_scratcher *x, t_glist *glist)
{
    t_canvas *canvas=glist_getcanvas(glist);
    t_int ci;

    if ( glist_isvisible( x->x_glist ) )
    {
        SYS_VGUI7(".x%lx.c coords %xSCRATCHER %d %d %d %d\n",
                  canvas, x,
                  text_xpix(&x->x_obj, glist)-1, text_ypix(&x->x_obj, glist)-1,
                  text_xpix(&x->x_obj, glist)+x->x_width+1,
                  text_ypix(&x->x_obj, glist)+x->x_height+1);
        SYS_VGUI7(".x%lx.c coords %xFSCRATCHER %d %d %d %d\n",
                  canvas, x,
                  text_xpix(&x->x_obj, glist)-1, text_ypix(&x->x_obj, glist)-1,
                  text_xpix(&x->x_obj, glist)+x->x_width+1,
                  text_ypix(&x->x_obj, glist)+x->x_height+1);
        if ( x->x_showspeed )
        {
            SYS_VGUI7( ".x%lx.c coords %xSPEEDBAR %d %d %d %d\n",
                       canvas, x,
                       text_xpix(&x->x_obj, glist)+x->x_width/2,
                       text_ypix(&x->x_obj, glist)+x->x_height/2,
                       text_xpix(&x->x_obj, glist)+x->x_width/2 + (int)(x->x_width/2*cos( x->x_readspeed - 1 )),
                       text_ypix(&x->x_obj, glist)+x->x_height/2 - (int)(x->x_width/2*sin( x->x_readspeed - 1 ))
                     );
        }
        for ( ci=0; ci<SCRATCHER_NB_GROOVES; ci ++)
        {
            SYS_VGUI8(".x%lx.c coords %xGROOVE%d %d %d %d %d\n",
                      canvas, x, ci,
                      text_xpix(&x->x_obj, glist) + ci*x->x_width/(2*SCRATCHER_NB_GROOVES),
                      text_ypix(&x->x_obj, glist) + ci*x->x_height/(2*SCRATCHER_NB_GROOVES),
                      text_xpix(&x->x_obj, glist) + x->x_width - ci*x->x_width/(2*SCRATCHER_NB_GROOVES),
                      text_ypix(&x->x_obj, glist) + x->x_height - ci*x->x_height/(2*SCRATCHER_NB_GROOVES)
                     );
        }
        canvas_fixlinesfor( canvas, (t_text*)x );
    }
}

static void scratcher_draw_select(t_scratcher* x,t_glist* glist)
{
    t_canvas *canvas=glist_getcanvas(glist);

    if ( glist_isvisible( x->x_glist ) )
    {
        if(x->x_selected)
        {
        }
        else
        {
        }
    }
}

/* ------------------------ widget callbacks ----------------------------- */


static void scratcher_getrect(t_gobj *z, t_glist *owner,
                              int *xp1, int *yp1, int *xp2, int *yp2)
{
    t_scratcher* x = (t_scratcher*)z;

    *xp1 = text_xpix(&x->x_obj, owner);
    *yp1 = text_ypix(&x->x_obj, owner);
    *xp2 = text_xpix(&x->x_obj, owner)+x->x_width;
    *yp2 = text_ypix(&x->x_obj, owner)+x->x_height;
}

static void scratcher_save(t_gobj *z, t_binbuf *b)
{
    t_scratcher *x = (t_scratcher *)z;

    binbuf_addv(b, "ssiisiiiiff", gensym("#X"),gensym("obj"),
                (t_int)x->x_obj.te_xpix, (t_int)x->x_obj.te_ypix,
                atom_getsymbol(binbuf_getvec(x->x_obj.te_binbuf)),
                x->x_size, x->x_width, x->x_height,
                x->x_sensibility, x->x_maxspeed, x->x_inertia );
    binbuf_addv(b, ";");
}

static void scratcher_select(t_gobj *z, t_glist *glist, int selected)
{
    t_scratcher *x = (t_scratcher *)z;

    x->x_selected = selected;
    scratcher_draw_select( x, glist );
}

static void scratcher_vis(t_gobj *z, t_glist *glist, int vis)
{
    t_scratcher *x = (t_scratcher *)z;
    t_rtext *y;

    if (vis)
    {
        scratcher_draw_new( x, glist );
    }
    else
    {
        scratcher_draw_delete( x, glist );
    }
}

static void scratcher_delete(t_gobj *z, t_glist *glist)
{
    canvas_deletelinesfor(glist, (t_text *)z);
}

static void scratcher_displace(t_gobj *z, t_glist *glist, int dx, int dy)
{
    t_scratcher *x = (t_scratcher *)z;

    x->x_obj.te_xpix += dx;
    x->x_obj.te_ypix += dy;

    scratcher_draw_move( x, glist );
}

static void scratcher_motion(t_scratcher *x, t_floatarg dx, t_floatarg dy)
{
    struct timeval tv;
    struct timezone tz;
    t_canvas *canvas=glist_getcanvas(x->x_glist);

    // post( "scratcher_motion dx=%f dy=%f", dx, dy );

    x->x_speedinc += dy / x->x_sensibility;

    x->x_mousemoved = 1;
    // get current time in ms
    gettimeofday( &tv, &tz );
    x->x_lastmovetime = tv.tv_sec*1000 + tv.tv_usec/1000;
    // post( "scratcher~ : move time : %ld", x->x_lastmovetime );

    if ( x->x_showspeed )
    {
        SYS_VGUI7( ".x%lx.c coords %xSPEEDBAR %d %d %d %d\n",
                   canvas, x,
                   text_xpix(&x->x_obj, x->x_glist)+x->x_width/2,
                   text_ypix(&x->x_obj, x->x_glist)+x->x_height/2,
                   text_xpix(&x->x_obj, x->x_glist)+x->x_width/2 + (int)(x->x_width/2*cos( x->x_readspeed - 1 )),
                   text_ypix(&x->x_obj, x->x_glist)+x->x_height/2 - (int)(x->x_width/2*sin( x->x_readspeed - 1 ))
                 );
    }
}

static void scratcher_properties(t_gobj *z, t_glist *owner)
{
    char buf[800];
    t_scratcher *x=(t_scratcher *)z;

    sprintf(buf, "pdtk_scratcher_dialog %%s %d %d\n",
            x->x_width, x->x_height );
    // post("scratcher_properties : %s", buf );
    gfxstub_new(&x->x_obj.ob_pd, x, buf);
}

static void scratcher_dialog(t_scratcher *x, t_symbol *s, int argc, t_atom *argv)
{
    if ( !x )
    {
        post( "scratcher : error :tried to set properties on an unexisting object" );
    }
    if ( argc != 2 )
    {
        post( "scratcher : error in the number of arguments ( %d instead of 2 )", argc );
        return;
    }
    if ( argv[0].a_type != A_FLOAT || argv[1].a_type != A_FLOAT )
    {
        post( "scratcher : wrong arguments" );
        return;
    }
    x->x_width = (int)argv[0].a_w.w_float;
    x->x_height = (int)argv[1].a_w.w_float;
    scratcher_draw_delete(x, x->x_glist);
    scratcher_draw_new(x, x->x_glist);
}

/* reset reading speed */
static void scratcher_reset(t_scratcher *x)
{
    t_canvas *canvas=glist_getcanvas(x->x_glist);

    x->x_readspeed=1.;
    if ( x->x_showspeed )
    {
        SYS_VGUI7( ".x%lx.c coords %xSPEEDBAR %d %d %d %d\n",
                   canvas, x,
                   text_xpix(&x->x_obj, x->x_glist)+x->x_width/2,
                   text_ypix(&x->x_obj, x->x_glist)+x->x_height/2,
                   text_xpix(&x->x_obj, x->x_glist)+x->x_width/2 + (int)(x->x_width/2*cos( x->x_readspeed - 1 )),
                   text_ypix(&x->x_obj, x->x_glist)+x->x_height/2 - (int)(x->x_width/2*sin( x->x_readspeed - 1 ))
                 );
    }
}

static int scratcher_click(t_gobj *z, struct _glist *glist,
                           int xpix, int ypix, int shift, int alt, int dbl, int doit)
{
    t_scratcher* x = (t_scratcher *)z;
    t_canvas *canvas=glist_getcanvas(x->x_glist);

    // post( "scratcher_click : x=%d y=%d doit=%d alt=%d, shift=%d", xpix, ypix, doit, alt, shift );
    if ( doit )
    {
        // activate motion callback
        glist_grab( glist, &x->x_obj.te_g, (t_glistmotionfn)scratcher_motion,
                    0, xpix, ypix );
        x->x_readspeed=0.;
        x->x_motioned = 1;
        if ( x->x_showspeed )
        {
            SYS_VGUI7( ".x%lx.c coords %xSPEEDBAR %d %d %d %d\n",
                       canvas, x,
                       text_xpix(&x->x_obj, glist)+x->x_width/2,
                       text_ypix(&x->x_obj, glist)+x->x_height/2,
                       text_xpix(&x->x_obj, glist)+x->x_width/2 + (int)(x->x_width/2*cos( x->x_readspeed - 1 )),
                       text_ypix(&x->x_obj, glist)+x->x_height/2 - (int)(x->x_width/2*sin( x->x_readspeed - 1 ))
                     );
        }
    }
    else
    {
        if ( x->x_play ) scratcher_reset(x);
        x->x_motioned = 0;
    }
    return (1);
}

/* clean up */
static void scratcher_free(t_scratcher *x)
{
    if ( x->x_sdata != NULL )
    {
        freebytes(x->x_sdata, x->x_size*sizeof(t_float) );
        post( "scratcher~ : freed %d bytes", x->x_size*sizeof(t_float) );
        x->x_sdata = NULL;
    }
}

/* allocate tables for storing sample data */
static t_int scratcher_allocate(t_scratcher *x)
{
    if ( !(x->x_sdata = getbytes( x->x_size*sizeof(float) ) ) )
    {
        post( "scratcher~ : could not allocate %d bytes", x->x_size*sizeof(t_float) );
        return -1;
    }
    else
    {
        post( "scratcher~ : allocated %d bytes", x->x_size*sizeof(t_float) );
    }
    return 0;
}

/* reallocate tables for storing sample data */
static t_int scratcher_reallocate(t_scratcher *x, t_int ioldsize, t_int inewsize)
{
    t_float *pdata;

    pdata = x->x_sdata;
    if ( !(x->x_sdata = getbytes( inewsize*sizeof(float) ) ) )
    {
        post( "scratcher~ : could not allocate %d bytes", inewsize*sizeof(t_float) );
        return -1;
    }
    else
    {
        post( "scratcher~ : allocated %d bytes", inewsize*sizeof(t_float) );
    }
    if ( pdata != NULL )
    {
        freebytes(pdata, ioldsize*sizeof(t_float) );
        post( "scratcher~ : freed %d bytes", ioldsize*sizeof(t_float) );
    }
    return 0;
}

/* records or playback the scratcher */
static t_int *scratcher_perform(t_int *w)
{
    t_float *in = (t_float *)(w[1]);
    t_float *out = (t_float *)(w[2]);
    t_int n = (int)(w[3]);                      /* number of samples */
    t_scratcher *x = (t_scratcher *)(w[4]);
    struct timeval tv;
    struct timezone tz;
    long long perftime = 0L;
    t_canvas *canvas=glist_getcanvas(x->x_glist);

    x->x_readspeed += x->x_speedinc;
    if ( x->x_readspeed > x->x_maxspeed )
    {
        x->x_readspeed = x->x_maxspeed;
    }
    if ( x->x_readspeed < -x->x_maxspeed )
    {
        x->x_readspeed = -x->x_maxspeed;
    }
    x->x_speedinc = 0;

    // if the mouse hasn't moved for a certain time, reset speed

    if ( x->x_mousemoved )
    {
        // get current time in ms
        gettimeofday( &tv, &tz );
        perftime = tv.tv_sec*1000 + tv.tv_usec/1000;
        if ( perftime - x->x_lastmovetime > SCRATCHER_MOVE_TIMEOUT )
        {
            // post( "scratcher~ : mouse timeout (m=%ld)", perftime );
            // post( "scratcher~ : (m=%ld)", x->x_lastmovetime );
            if ( x->x_readspeed > 0. )
            {
                x->x_readspeed -= x->x_inertia;
                // post( "scratcher~ : dec speed" );
            }
            if ( x->x_readspeed < 0. )
            {
                x->x_readspeed += x->x_inertia;
                // post( "scratcher~ : inc speed" );
            }
            if ( ( x->x_readspeed <= x->x_inertia && x->x_readspeed > 0 ) ||
                    ( x->x_readspeed >= -x->x_inertia && x->x_readspeed < 0 ) )
            {
                x->x_mousemoved = 0;
                x->x_readspeed = 0.;
            }
            if ( x->x_showspeed )
            {
                SYS_VGUI7( ".x%lx.c coords %xSPEEDBAR %d %d %d %d\n",
                           canvas, x,
                           text_xpix(&x->x_obj, x->x_glist)+x->x_width/2,
                           text_ypix(&x->x_obj, x->x_glist)+x->x_height/2,
                           text_xpix(&x->x_obj, x->x_glist)+x->x_width/2 + (int)(x->x_width/2*cos( x->x_readspeed - 1 )),
                           text_ypix(&x->x_obj, x->x_glist)+x->x_height/2 - (int)(x->x_width/2*sin( x->x_readspeed - 1 ))
                         );
            }
        }
    }

    while (n--)
    {
        // eventually records input
        if ( x->x_record)
        {
            *(x->x_sdata+x->x_writepos)=*in;
            x->x_writepos++;
            if ( x->x_writepos >= x->x_size )
            {
                x->x_record=0;
                x->x_writepos=0;
                if ( x->x_empty ) x->x_empty = 0;
                // post( "scratcher~ : stopped recording" );
            }
        }
        // set outputs
        if ( x->x_play)
        {
            *out = *(x->x_sdata+(int)x->x_readpos);
            x->x_readpos+=x->x_readspeed;
            if ( x->x_readpos < 0 ) x->x_readpos = x->x_size-1;
            if ( x->x_readpos >= x->x_size ) x->x_readpos = 0;
        }
        else
        {
            *out=0.0;
        }

        in++;
        out++;
    }

    return (w+5);
}

static void scratcher_dsp(t_scratcher *x, t_signal **sp)
{
    dsp_add(scratcher_perform, 4, sp[0]->s_vec, sp[1]->s_vec, sp[0]->s_n, x);
}

/* play the sound */
static void scratcher_play(t_scratcher *x)
{
    t_canvas *canvas=glist_getcanvas(x->x_glist);

    x->x_play=1;
    // reset read position
    x->x_readpos=0;
    x->x_readspeed=1.;
    if ( x->x_showspeed )
    {
        SYS_VGUI3( ".x%lx.c delete %xSPEEDBAR\n", canvas, x );
        SYS_VGUI7( ".x%lx.c create line %d %d %d %d -fill #FF0000 -tags %xSPEEDBAR -width 3\n",
                   canvas, text_xpix(&x->x_obj, x->x_glist)+x->x_width/2,
                   text_ypix(&x->x_obj, x->x_glist)+x->x_height/2,
                   text_xpix(&x->x_obj, x->x_glist)+x->x_width/2 + (int)(x->x_width/2*cos( x->x_readspeed - 1. )),
                   text_ypix(&x->x_obj, x->x_glist)+x->x_height/2 - (int)(x->x_width/2*sin( x->x_readspeed - 1. )),
                   x );
    }
}

/* stop playing   */
static void scratcher_stop(t_scratcher *x)
{
    t_canvas *canvas=glist_getcanvas(x->x_glist);

    x->x_play=0;
    // reset read position
    x->x_readpos=0;
    x->x_readspeed=0.;
    if ( x->x_showspeed )
    {
        SYS_VGUI3( ".x%lx.c delete %xSPEEDBAR\n", canvas, x );
        SYS_VGUI7( ".x%lx.c create line %d %d %d %d -fill #FF0000 -tags %xSPEEDBAR -width 3\n",
                   canvas, text_xpix(&x->x_obj, x->x_glist)+x->x_width/2,
                   text_ypix(&x->x_obj, x->x_glist)+x->x_height/2,
                   text_xpix(&x->x_obj, x->x_glist)+x->x_width/2 + (int)(x->x_width/2*cos( x->x_readspeed - 1. )),
                   text_ypix(&x->x_obj, x->x_glist)+x->x_height/2 - (int)(x->x_width/2*sin( x->x_readspeed - 1. )),
                   x );
    }
}

/* record the sound */
static void scratcher_record(t_scratcher *x)
{
    scratcher_stop(x);
    x->x_record=1;
    x->x_writepos=0;
}

/* resize sound */
static void scratcher_resize(t_scratcher *x, t_floatarg fnewsize )
{
    t_int dspstate;
    struct timespec ts;

    if (fnewsize <= 0)
    {
        post( "scratcher~ : error : wrong size" );
        return;
    }
    post( "scratcher~ : reallocating tables" );
    x->x_record = 0;
    x->x_play = 0;

    scratcher_reallocate(x, x->x_size, fnewsize);
    x->x_size = fnewsize;
}

/* set sensibility */
static void scratcher_sensibility(t_scratcher *x, t_floatarg fsensibility )
{
    if (fsensibility <= 0)
    {
        post( "scratcher~ : error : wrong sensibility" );
        return;
    }
    x->x_sensibility = fsensibility;
}

/* set maximum speed */
static void scratcher_maxspeed(t_scratcher *x, t_floatarg fmaxspeed )
{
    if (fmaxspeed < 0)
    {
        post( "scratcher~ : error : wrong maximum speed" );
        return;
    }
    x->x_maxspeed = fmaxspeed;
}

/* set turntable inertia */
static void scratcher_inertia(t_scratcher *x, t_floatarg finertia )
{
    if (finertia < 0)
    {
        post( "scratcher~ : error : wrong inertia" );
        return;
    }
    x->x_inertia = finertia;
}

/* toggle speed line */
static void scratcher_showspeed(t_scratcher *x, t_floatarg fshowspeed )
{
    t_canvas *canvas=glist_getcanvas(x->x_glist);

    if (fshowspeed == 0)
    {
        x->x_showspeed = 0;
        SYS_VGUI3( ".x%lx.c delete %xSPEEDBAR\n", canvas, x );
    }
    else
    {
        x->x_showspeed = 1;
        SYS_VGUI3( ".x%lx.c delete %xSPEEDBAR\n", canvas, x );
        SYS_VGUI7( ".x%lx.c create line %d %d %d %d -fill #FF0000 -tags %xSPEEDBAR -width 3\n",
                   canvas, text_xpix(&x->x_obj, x->x_glist)+x->x_width/2,
                   text_ypix(&x->x_obj, x->x_glist)+x->x_height/2,
                   text_xpix(&x->x_obj, x->x_glist)+x->x_width/2 + (int)(x->x_width/2*cos( x->x_readspeed - 1. )),
                   text_ypix(&x->x_obj, x->x_glist)+x->x_height/2 - (int)(x->x_width/2*sin( x->x_readspeed - 1. )),
                   x );
    }
}

static void *scratcher_new(t_symbol *s, int argc, t_atom *argv)
{
    t_scratcher *x = (t_scratcher *)pd_new(scratcher_class);
    outlet_new(&x->x_obj, &s_signal);

    // new scratcher created from the gui
    if ( argc != 0 )
    {
        if ( argc < 3 )
        {
            post( "scratcher~ : error in the number of arguments ( < 3 )", argc );
            return NULL;
        }
        if ( argv[0].a_type != A_FLOAT ||
                argv[1].a_type != A_FLOAT ||
                argv[2].a_type != A_FLOAT )
        {
            post( "scratcher~ : wrong arguments" );
            return NULL;
        }
        x->x_size = argv[0].a_w.w_float;
        x->x_width = argv[1].a_w.w_float;
        x->x_height = argv[2].a_w.w_float;
        if ( argc >= 4 )
        {
            x->x_sensibility = argv[3].a_w.w_float;
        }
        else
        {
            x->x_sensibility = DEFAULT_SCRATCHER_SENSIBILITY;
        }
        if ( argc >= 5 )
        {
            x->x_maxspeed = argv[4].a_w.w_float;
        }
        else
        {
            x->x_maxspeed = DEFAULT_SCRATCHER_MAX_SPEED;
        }
        if ( argc >= 6 )
        {
            x->x_inertia = argv[5].a_w.w_float;
        }
        else
        {
            x->x_inertia = DEFAULT_TURNTABLE_INERTIA;
        }
    }
    else
    {
        x->x_size = DEFAULT_SCRATCHER_SIZE;
        x->x_width = DEFAULT_SCRATCHER_WIDTH;
        x->x_height = DEFAULT_SCRATCHER_HEIGHT;
        x->x_sensibility = DEFAULT_SCRATCHER_SENSIBILITY;
        x->x_maxspeed = DEFAULT_SCRATCHER_MAX_SPEED;
        x->x_inertia = DEFAULT_TURNTABLE_INERTIA;
    }

    x->x_play = 0;
    x->x_record = 0;
    x->x_readspeed = 1.;
    x->x_readpos = 0.;
    x->x_speedinc = 0.;
    x->x_writepos = 0;
    x->x_sdata = NULL;
    x->x_empty = 1;
    x->x_showspeed = 1;
    x->x_mousemoved = 0;
    x->x_lastmovetime = 0L;
    x->x_selected = 0;
    x->x_glist = (t_glist*)canvas_getcurrent();

    // activate graphical callbacks
    class_setwidget(scratcher_class, &scratcher_widgetbehavior);

    // post( "scratcher~ : new scratcher : size = %d", x->x_size );
    if ( scratcher_allocate(x) <0 )
    {
        return NULL;
    }
    else
    {
        return(x);
    }

}

void scratcher_tilde_setup(void)
{
    logpost(NULL, 4, scratcher_version);
    scratcher_class = class_new(gensym("scratcher~"), (t_newmethod)scratcher_new, (t_method)scratcher_free,
                                sizeof(t_scratcher), 0, A_GIMME, 0);


    // set callbacks
    scratcher_widgetbehavior.w_getrectfn =    scratcher_getrect;
    scratcher_widgetbehavior.w_displacefn =   scratcher_displace;
    scratcher_widgetbehavior.w_selectfn =     scratcher_select;
    scratcher_widgetbehavior.w_activatefn =   NULL;
    scratcher_widgetbehavior.w_deletefn =     scratcher_delete;
    scratcher_widgetbehavior.w_visfn =        scratcher_vis;
    scratcher_widgetbehavior.w_clickfn =      scratcher_click;

#if PD_MINOR_VERSION >= 37
    class_setpropertiesfn(scratcher_class, scratcher_properties);
    class_setsavefn(scratcher_class, scratcher_save);
#else
    scratcher_widgetbehavior.w_propertiesfn = scratcher_properties;
    scratcher_widgetbehavior.w_savefn =       scratcher_save;
#endif

    CLASS_MAINSIGNALIN( scratcher_class, t_scratcher, x_f );
    class_addmethod(scratcher_class, (t_method)scratcher_dsp, gensym("dsp"), A_NULL);
    class_addmethod(scratcher_class, (t_method)scratcher_record, gensym("record"), A_NULL);
    class_addmethod(scratcher_class, (t_method)scratcher_resize, gensym("resize"), A_FLOAT, A_NULL);
    class_addmethod(scratcher_class, (t_method)scratcher_sensibility, gensym("sensibility"), A_FLOAT, A_NULL);
    class_addmethod(scratcher_class, (t_method)scratcher_maxspeed, gensym("maxspeed"), A_FLOAT, A_NULL);
    class_addmethod(scratcher_class, (t_method)scratcher_inertia, gensym("inertia"), A_FLOAT, A_NULL);
    class_addmethod(scratcher_class, (t_method)scratcher_showspeed, gensym("showspeed"), A_FLOAT, A_NULL);
    class_addmethod(scratcher_class, (t_method)scratcher_play, gensym("play"), A_NULL);
    class_addmethod(scratcher_class, (t_method)scratcher_stop, gensym("stop"), A_NULL);
    class_addmethod(scratcher_class, (t_method)scratcher_reset, gensym("reset"), A_NULL);
    class_addmethod(scratcher_class, (t_method)scratcher_dialog, gensym("dialog"), A_GIMME, A_NULL);

    sys_vgui("eval [read [open {%s/%s.tcl}]]\n",
             scratcher_class->c_externdir->s_name,
             scratcher_class->c_name->s_name);
}
