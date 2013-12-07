/*------------------------ sonogram~ ------------------------------------------ */
/*                                                                              */
/* sonogram~ : lets you record, play back and modify a sonogram                 */
/* constructor : sonogram <size> <graphical=0|1> <phasogram=0|1>                */
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
/* "Living at night"                                                            */
/* "Doesn't help for my complexion"                                             */
/* David Thomas - Final Solution                                                */
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
#include <math.h>

#ifdef _WIN32
# include <io.h>
# define random rand
#endif

#ifndef _MSC_VER /* only MSVC doesn't have unistd.h */
# include <unistd.h>
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

#define SYS_VGUI9(a,b,c,d,e,f,g,h,i) if (guidebug) \
                         post(a,b,c,d,e,f,g,h,i );\
                         sys_vgui(a,b,c,d,e,f,g,h,i)

#define SYS_VGUI10(a,b,c,d,e,f,g,h,i,j) if (guidebug) \
                         post(a,b,c,d,e,f,g,h,i,j );\
                         sys_vgui(a,b,c,d,e,f,g,h,i,j)

#define SYS_VGUI11(a,b,c,d,e,f,g,h,i,j,k) if (guidebug) \
                         post(a,b,c,d,e,f,g,h,i,j,k );\
                         sys_vgui(a,b,c,d,e,f,g,h,i,j,k)

#define THREAD_SLEEP_TIME 100000   // 100000 us = 100 ms

static char   *sonogram_version = "sonogram~: version 0.14, written by Yves Degoyon (ydegoyon@free.fr)";

static t_class *sonogram_class;
t_widgetbehavior sonogram_widgetbehavior;


typedef struct _sonogram
{
    t_object x_obj;

    t_int x_size;                  /* size of the stored fft ( in blocks~ ) */
    t_float x_samplerate;          /* sample rate */
    t_int x_blocksize;             /* current block size ( might be modified by block~ object ) */
    t_float x_readpos;             /* data's playing position                                   */
    t_int x_writepos;              /* data's recording position                                 */
    t_int x_readstart;             /* data's starting position for reading                      */
    t_int x_readend;               /* data's ending position for reading                        */
    t_int x_modstart;              /* data's starting position for modifications                */
    t_int x_modend;                /* data's ending position for modifications                  */
    t_int x_play;                  /* playing on/off flag                                       */
    t_float x_readspeed;           /* number of grouped blocks for reading                      */
    t_float x_record;              /* flag to start recording process                           */
    t_float x_empty;               /* flag to indicate it's a brand new sonogram                */
    t_float *x_rdata;              /* table containing real part of the fft                     */
    t_float *x_rudata;             /* undo real data                                            */
    t_float *x_idata;              /* table containing imaginery part of the fft                */
    t_float *x_iudata;             /* undo imaginery data                                       */
    t_float x_phase;               /* phase to apply on output                                  */
    t_outlet *x_end;               /* outlet for end of restitution                             */
    t_outlet *x_recend;            /* outlet for end of recording                               */
    t_int *x_multfreq;             /* array of multiplicative factor                            */
    char  *x_gifdata;              /* buffer for graphical data                                 */
    char  *x_guicommand;           /* buffer for graphical command                              */
    t_int x_uxs;                   /* starting x position for undo                              */
    t_int x_uxe;                   /* ending x position for undo                                */
    t_int x_uys;                   /* starting y position for undo                              */
    t_int x_uye;                   /* ending y position for undo                                */

    /* graphical data block */
    t_int x_enhancemode;           /* flag to set enhance mode                    */
    t_int x_graphic;               /* flag to set graphic mode                    */
    t_int x_phaso;                 /* flag to indicate if phasogram is shown      */
    t_int x_selected;              /* flag to remember if we are seleted or not   */
    t_int x_erase;                 /* flag used when an erase is needed           */
    t_int x_redraw;                /* flag used when drawing  is needed           */
    t_int x_nbupdated;             /* number of points updated                    */
    t_glist *x_glist;              /* keep graphic context for various operations */
    t_int x_zoom;                  /* zoom factor                                 */
#ifndef _WIN32
    pthread_t x_updatechild;       /* thread id for the update child              */
#else
    int x_updatechild;
#endif
    t_int x_updatestart;           /* starting position for update                */
    t_int x_updateend;             /* ending position for update                  */
    t_int x_xpos;                  /* stuck x position                            */
    t_int x_ypos;                  /* stuck y position                            */
    t_int x_shifted;               /* remember shift state from last click        */
    t_int x_alted;                 /* remember alt state from last click          */
    t_int x_aftermousedown;        /* indicates the mousedown event               */
    t_int x_xstartcapture;         /* x of the start of the capture               */
    t_int x_ystartcapture;         /* y of the start of the capture               */
    t_int x_xendcapture;           /* x of the start of the capture               */
    t_int x_yendcapture;           /* y of the start of the capture               */
    t_int x_xdraw;                 /* x drawing position                          */
    t_int x_ydraw;                 /* y drawing position                          */
    t_float x_modstep;             /* step for graphical modifications            */

    t_float x_f;                   /* float needed for signal input */

} t_sonogram;

/* ------------------------ drawing functions ---------------------------- */
static char* sonogram_get_fill_color( t_float fspectrum )
{
    if ( fspectrum < 0.01 )
    {
        return "#EEEEEE ";
    }
    else if ( fspectrum < 0.1 )
    {
        return "#DDDDDD ";
    }
    else if ( fspectrum < 0.5 )
    {
        return "#CCCCCC ";
    }
    else if ( fspectrum < 1 )
    {
        return "#BBBBBB ";
    }
    else if ( fspectrum < 2 )
    {
        return "#AAAAAA ";
    }
    else if ( fspectrum < 5 )
    {
        return "#999999 ";
    }
    else if ( fspectrum < 10 )
    {
        return "#888888 ";
    }
    else if ( fspectrum < 20 )
    {
        return "#777777 ";
    }
    else if ( fspectrum < 30 )
    {
        return "#666666 ";
    }
    else if ( fspectrum < 40 )
    {
        return "#555555 ";
    }
    else if ( fspectrum < 50 )
    {
        return "#444444 ";
    }
    else if ( fspectrum < 60 )
    {
        return "#333333 ";
    }
    else if ( fspectrum < 80 )
    {
        return "#222222 ";
    }
    else if ( fspectrum < 100 )
    {
        return "#111111 ";
    }
    else
    {
        return "#000000 ";
    }
}

static char* phasogram_get_fill_color( t_int phase )
{
    if ( phase < 0 )
    {
        if ( phase > -10 )
        {
            return "#111111 ";
        }
        else if ( phase > -20 )
        {
            return "#222222 ";
        }
        else if ( phase > -30 )
        {
            return "#333333 ";
        }
        else if ( phase > -40 )
        {
            return "#444444 ";
        }
        else if ( phase > -50 )
        {
            return "#555555 ";
        }
        else if ( phase > -60 )
        {
            return "#666666 ";
        }
        else if ( phase > -70 )
        {
            return "#777777 ";
        }
        else if ( phase > -80 )
        {
            return "#888888 ";
        }
        else
        {
            return "#999999 ";
        }
    }
    else
    {
        if ( phase == 0 )
        {
            return "#FFFFFF ";
        }
        else if ( phase < 10 )
        {
            return "#111111 ";
        }
        else if ( phase < 20 )
        {
            return "#222222 ";
        }
        else if ( phase < 30 )
        {
            return "#333333 ";
        }
        else if ( phase < 40 )
        {
            return "#444444 ";
        }
        else if ( phase < 50 )
        {
            return "#555555 ";
        }
        else if ( phase < 60 )
        {
            return "#666666 ";
        }
        else if ( phase < 70 )
        {
            return "#777777 ";
        }
        else if ( phase < 80 )
        {
            return "#888888 ";
        }
        else
        {
            return "#999999 ";
        }
    }
    // normally never reached
    return "";
}

static void sonogram_update_point(t_sonogram *x, t_glist *glist, t_int sample, t_int frequency)
{
    t_canvas *canvas=glist_getcanvas(glist);
    t_float fspectrum=0.0;
    t_int phase=0.0;
    char newColor[ 8 ], olColor[8];
    int i;

    fspectrum =
        sqrt( pow( *(x->x_rdata+sample*x->x_blocksize+frequency), 2) +
              pow( *(x->x_idata+sample*x->x_blocksize+frequency), 2) );
    phase = (int) ( atan2( *(x->x_idata+(sample*x->x_blocksize)+frequency),
                           *(x->x_rdata+(sample*x->x_blocksize)+frequency) )*180/M_PI );
    if ( x->x_empty && ( fspectrum != 0 ))
    {
        x->x_empty = 0;
    }
    strncpy( newColor, sonogram_get_fill_color( fspectrum ), 8 );

    for ( i=0; i<x->x_zoom; i++ )
    {
        sprintf( x->x_gifdata, "%s", newColor );
    }
    for ( i=0; i<x->x_zoom; i++ )
    {
        SYS_VGUI5("SONIMAGE%x put {%s} -to %d %d\n", x, x->x_gifdata,
                  sample*x->x_zoom+i, (x->x_blocksize/2-frequency)*x->x_zoom );
    }

    if ( x->x_phaso )
    {
        strncpy( newColor, phasogram_get_fill_color( phase ), 8 );
        strcpy( x->x_gifdata, "" );
        for ( i=0; i<x->x_zoom; i++ )
        {
            sprintf( x->x_gifdata, "%s", newColor );
        }
        for ( i=0; i<x->x_zoom; i++ )
        {
            SYS_VGUI5("FAZIMAGE%x put {%s} -to %d %d\n", x, x->x_gifdata,
                      sample*x->x_zoom+i, (x->x_blocksize/2-frequency)*x->x_zoom );
        }
    }

    x->x_nbupdated++;
}

static void sonogram_update_block(t_sonogram *x, t_glist *glist, t_int bnumber)
{
    t_int fi, i=0;
    t_float fspectrum=0.0;
    t_int phase=0;
    char color[8];

    // update sonogram
    for ( fi=x->x_blocksize/2-1; fi>=0; fi-- )
    {
        fspectrum =
            sqrt( pow( *(x->x_rdata+bnumber*x->x_blocksize+fi), 2) +
                  pow( *(x->x_idata+bnumber*x->x_blocksize+fi), 2) );
        strncpy( color, sonogram_get_fill_color( fspectrum ), 8 );
        for ( i=0; i<x->x_zoom; i++ )
        {
            strncpy( x->x_gifdata+((x->x_blocksize/2-fi-1)*x->x_zoom+i)*8, color, 8 );
        }
    }
    for ( i=0; i<x->x_zoom; i++ )
    {
        sprintf( x->x_guicommand, "SONIMAGE%x put {%s} -to %d 0\n", (unsigned int)x, x->x_gifdata, (bnumber*x->x_zoom)+i );
        sys_gui( x->x_guicommand );
    }

    // update phasogram
    if ( x->x_phaso )
    {
        strcpy( x->x_gifdata, "" );
        for ( fi=x->x_blocksize/2-1; fi>=0; fi-- )
        {
            phase = (int) ( atan2( *(x->x_idata+bnumber*x->x_blocksize+fi),
                                   *(x->x_rdata+bnumber*x->x_blocksize+fi) )*180/M_PI );
            strncpy( color, phasogram_get_fill_color( phase ), 8 );
            for ( i=0; i<x->x_zoom; i++ )
            {
                strncpy( x->x_gifdata+((x->x_blocksize/2-fi-1)*x->x_zoom+i)*8, color, 8 );
            }
        }
        for ( i=0; i<x->x_zoom; i++ )
        {
            sprintf( x->x_guicommand, "FAZIMAGE%x put {%s} -to %d 0\n", (unsigned int)x, x->x_gifdata, (bnumber*x->x_zoom)+i );
            sys_gui( x->x_guicommand );
        }
    }

}

static void sonogram_erase_block(t_sonogram *x, t_glist *glist, t_int bnumber )
{
    t_canvas *canvas=glist_getcanvas(glist);
    t_int fi;
    t_float fspectrum=0.0;
    char fillColor[ 16 ];

    for ( fi=0; fi<x->x_blocksize/2; fi++)
    {
        {
            int i;

            for ( i=0; i<x->x_zoom; i++ )
            {
                strncpy( x->x_gifdata+i*sizeof("#FFFFFF "), "#FFFFFF ", 8 );
            }
            SYS_VGUI5("SONIMAGE%x put {%s} -to %d %d\n", x, x->x_gifdata,
                      bnumber*x->x_zoom, (x->x_blocksize/2-fi)*x->x_zoom );
        }
    }
}

static void *sonogram_do_update_part(void *tdata)
{
    t_sonogram *x = (t_sonogram*) tdata;
    t_int si;
    t_int nbpoints = 0;
    t_float percentage = 0, opercentage = 0;
    t_canvas *canvas=glist_getcanvas(x->x_glist);


    // loose synchro
    usleep( THREAD_SLEEP_TIME );

    // check boundaries
    if ( x->x_updateend > x->x_size-1 ) x->x_updateend = x->x_size-1;
    if ( x->x_updatestart < 0 ) x->x_updatestart = 0;

    // post("sonogram~ : ok, let's go [updating %d, %d]", x->x_updatestart, x->x_updateend );

    if ( x->x_erase )
    {
        for ( si=x->x_updatestart; si<=x->x_updateend; si++ )
        {
            sonogram_erase_block(x, x->x_glist, si);
            nbpoints++;
            percentage = (nbpoints*100/(x->x_updateend-x->x_updatestart+1));
            if ( (percentage == (int) percentage) && ((int)percentage%5 == 0) && ( percentage != opercentage ) )
            {
                // post( "sonogram~ : erase part : %d %% completed", (int)percentage );
                opercentage = percentage;
            }
        }
    }

    percentage = opercentage = nbpoints = 0;

    if ( x->x_redraw )
    {
        for ( si=x->x_updatestart; si<=x->x_updateend; si++ )
        {
            sonogram_update_block(x, x->x_glist, si);
            nbpoints++;
            percentage = (nbpoints*100/(x->x_updateend-x->x_updatestart+1));
            if ( (percentage == (int) percentage) && ((int)percentage%5 == 0) && ( percentage != opercentage ) )
            {
                // post( "sonogram~ : update part : %d %% completed", (int)percentage );
                opercentage = percentage;
            }
        }
    }

    // set borders in black
    SYS_VGUI3(".x%lx.c itemconfigure %xSONOGRAM -outline #000000\n", canvas, x);
    if ( x->x_phaso )
    {
        SYS_VGUI3(".x%lx.c itemconfigure %xPHASOGRAM -outline #000000\n", canvas, x);
    }

    // post("sonogram~ : child thread %d ended (nb_updated=%d)", (int)x->x_updatechild, x->x_nbupdated );
    x->x_updatechild = 0;
    return NULL;
}

static void sonogram_update_part(t_sonogram *x, t_glist *glist, t_int bstart, t_int bend,
                                 t_int erase, t_int redraw, t_int keepframe)
{
    pthread_attr_t update_child_attr;
    pthread_t  update_child;
    t_canvas *canvas=glist_getcanvas(glist);

    if ( x->x_graphic )
    {
        if ( x->x_updatechild != 0 )
        {
            // post( "sonogram~ : error : no update is possible for now" );
            return;
        }
        x->x_updatestart = bstart;
        x->x_updateend = bend;
        if ( !keepframe )
        {
            x->x_erase = 0;
        }
        else
        {
            x->x_erase = erase;
        }
        x->x_redraw = redraw;
        x->x_nbupdated = 0;
        // recreate the square if needed
        if ( ( bstart == 0 ) && ( bend == x->x_size-1 ) && !keepframe )
        {
            SYS_VGUI3(".x%lx.c delete %xSONOGRAM\n", canvas, x );
            SYS_VGUI7(".x%lx.c create rectangle %d %d %d %d -fill #FFFFFF -tags %xSONOGRAM\n",
                      glist_getcanvas(glist), x->x_xpos-1, x->x_ypos-1,
                      x->x_xpos + x->x_size*x->x_zoom+1,
                      x->x_ypos + x->x_blocksize/2*x->x_zoom+1,
                      x);
            SYS_VGUI2("image delete SONIMAGE%x\n", x );
            SYS_VGUI3(".x%lx.c delete ISONIMAGE%x\n", canvas, x );
            SYS_VGUI4("image create photo SONIMAGE%x -format gif -width %d -height %d\n",
                      x, x->x_size*x->x_zoom, x->x_blocksize/2*x->x_zoom );
            SYS_VGUI2("SONIMAGE%x blank\n", x);
            SYS_VGUI6(".x%lx.c create image %d %d -image SONIMAGE%x -tags ISONIMAGE%x\n",
                      canvas, x->x_xpos+((x->x_size*x->x_zoom)/2),
                      (x->x_ypos+((x->x_blocksize/2*x->x_zoom)/2)), x, x );
            if ( x->x_phaso )
            {
                SYS_VGUI3(".x%lx.c delete %xPHASOGRAM\n", canvas, x );
                SYS_VGUI7(".x%lx.c create rectangle %d %d %d %d -fill #FFFFFF -tags %xPHASOGRAM\n",
                          canvas, x->x_xpos-1, x->x_ypos+x->x_blocksize/2*x->x_zoom+2,
                          x->x_xpos + x->x_size*x->x_zoom +1,
                          x->x_ypos + x->x_blocksize*x->x_zoom + 3,
                          x);
                SYS_VGUI2("image delete FAZIMAGE%x\n", x );
                SYS_VGUI3(".x%lx.c delete IFAZIMAGE%x\n", canvas, x );
                SYS_VGUI4("image create photo FAZIMAGE%x -format gif -width %d -height %d\n",
                          x, x->x_size*x->x_zoom, x->x_blocksize/2*x->x_zoom );
                SYS_VGUI2("FAZIMAGE%x blank\n", x);
                SYS_VGUI6(".x%lx.c create image %d %d -image FAZIMAGE%x -tags IFAZIMAGE%x\n",
                          canvas, x->x_xpos+((x->x_size*x->x_zoom)/2),
                          x->x_ypos+3*((x->x_blocksize/2*x->x_zoom)/2)+2, x, x );
            }
            canvas_fixlinesfor( canvas, (t_text*)x );
        }
        // set borders in red
        SYS_VGUI3(".x%lx.c itemconfigure %xSONOGRAM -outline #FF0000\n", canvas, x);
        if ( x->x_phaso )
        {
            SYS_VGUI3(".x%lx.c itemconfigure %xPHASOGRAM -outline #FF0000\n", canvas, x);
        }

        // launch update thread
        if ( pthread_attr_init( &update_child_attr ) < 0 )
        {
            post( "sonogram~ : could not launch update thread" );
            perror( "pthread_attr_init" );
            return;
        }
        if ( pthread_attr_setdetachstate( &update_child_attr, PTHREAD_CREATE_DETACHED ) < 0 )
        {
            post( "sonogram~ : could not launch update thread" );
            perror( "pthread_attr_setdetachstate" );
            return;
        }
        if ( pthread_create( &x->x_updatechild, &update_child_attr, sonogram_do_update_part, x ) < 0 )
        {
            post( "sonogram~ : could not launch update thread" );
            perror( "pthread_create" );
            return;
        }
        else
        {
            // post( "sonogram~ : drawing thread %d launched", (int)x->x_updatechild );
        }
    }
}

/* paste selection at the drawing point */
static void sonogram_paste( t_sonogram* x)
{
    t_int pxstart = (x->x_xdraw-x->x_xpos)/x->x_zoom;
    t_int pystart = (x->x_ypos-x->x_ydraw)/x->x_zoom+x->x_blocksize/2;
    t_int cxs,cxe,cys,cye,si=0,fi=0;
    t_float *icopy;
    t_float *rcopy;
    t_int   copynd;

    if ( x->x_xstartcapture > x->x_xendcapture )
    {
        fi = x->x_xstartcapture;
        x->x_xstartcapture = x->x_xendcapture;
        x->x_xendcapture = fi;
    }
    if ( x->x_ystartcapture > x->x_yendcapture )
    {
        fi = x->x_ystartcapture;
        x->x_ystartcapture = x->x_yendcapture;
        x->x_yendcapture = fi;
    }
    cxs=(x->x_xstartcapture-x->x_xpos)/x->x_zoom;
    cxe=(x->x_xendcapture-x->x_xpos)/x->x_zoom;
    cys=(x->x_ypos-x->x_yendcapture)/x->x_zoom+x->x_blocksize/2;
    cye=(x->x_ypos-x->x_ystartcapture)/x->x_zoom+x->x_blocksize/2;
    if ( cye < 0 ) cye=0;
    if ( cys < 0 ) cys=0;
    if ( cye >= x->x_blocksize/2 ) cye=x->x_blocksize/2-1;
    if ( cys >= x->x_blocksize/2 ) cys=x->x_blocksize/2-1;
    if ( cxe >= x->x_size ) cxe=x->x_size-1;
    if ( cxs >= x->x_size ) cxs=x->x_size-1;

    // make a copy first
    icopy = ( t_float* ) getbytes( ( cxe-cxs+1 )*( cye-cys+1 )*sizeof( t_float ) );
    rcopy = ( t_float* ) getbytes( ( cxe-cxs+1 )*( cye-cys+1 )*sizeof( t_float ) );
    if ( !icopy || !rcopy )
    {
        post( "sonogram~ : cannot allocate buffers for pasting" );
        return;
    }
    // copy initial data
    copynd = 0;
    for ( si=cxs; si<=cxe; si++)
    {
        for ( fi=cys; fi<=cye; fi++)
        {
            *(rcopy+copynd) = *(x->x_rdata+(si)*x->x_blocksize+fi);
            *(icopy+copynd) = *(x->x_idata+(si)*x->x_blocksize+fi);
            copynd++;
        }
    }

    post( "sonogram~ : paste from [%d,%d,%d,%d] to [%d,%d]", cxs, cys, cxe, cye, pxstart, pystart );

    for ( si=cxs; si<=cxe; si++)
    {
        if ( pxstart+si-cxs >= x->x_size ) break;
        copynd = (si-cxs)*(cye-cys+1);
        for ( fi=cys; fi<=cye; fi++)
        {
            // post ( "sonogram~ : si : %d : fi : %d : copynd : %d", si, fi, copynd );
            if ( pystart+fi-cys >= x->x_blocksize/2 ) break;
            *(x->x_rudata+((si-cxs)*x->x_blocksize)+(fi-cys)) = *(x->x_rdata+((pxstart+si-cxs)*x->x_blocksize)+(pystart+fi-cys));
            *(x->x_iudata+((si-cxs)*x->x_blocksize)+(fi-cys)) = *(x->x_idata+((pxstart+si-cxs)*x->x_blocksize)+(pystart+fi-cys));
            if ( x->x_enhancemode )
            {
                // save data for undo
                *(x->x_rdata+((pxstart+si-cxs)*x->x_blocksize)+(pystart+fi-cys)) += *(rcopy+copynd);
                *(x->x_idata+((pxstart+si-cxs)*x->x_blocksize)+(pystart+fi-cys)) += *(icopy+copynd);
            }
            else
            {
                *(x->x_rdata+((pxstart+si-cxs)*x->x_blocksize)+(pystart+fi-cys)) = *(rcopy+copynd);
                *(x->x_idata+((pxstart+si-cxs)*x->x_blocksize)+(pystart+fi-cys)) = *(icopy+copynd);
            }
            copynd++;
        }
    }

    x->x_uxs = pxstart;
    x->x_uxe = pxstart+(si-1)-cxs;
    x->x_uys = pystart;
    x->x_uye = pystart+(fi-1)-cys;;

    freebytes( rcopy, ( cxe-cxs+1 )*( cye-cys+1 )*sizeof( t_float ) );
    freebytes( icopy, ( cxe-cxs+1 )*( cye-cys+1 )*sizeof( t_float ) );
}

/* paste phase at the drawing point */
static void sonogram_paste_phase( t_sonogram* x)
{
    t_int pxstart = (x->x_xdraw-x->x_xpos)/x->x_zoom;
    t_int pystart = (x->x_ypos+x->x_blocksize/2*x->x_zoom+1-x->x_ydraw)/x->x_zoom+x->x_blocksize/2;
    t_int cxs,cxe,cys,cye,si,fi;
    t_float fspectrum, fdestspectrum;
    t_float fphase, fdestphase;
    t_float *icopy;
    t_float *rcopy;
    t_int   copynd;
    t_canvas *canvas=glist_getcanvas(x->x_glist);


    if ( x->x_xstartcapture > x->x_xendcapture )
    {
        fi = x->x_xstartcapture;
        x->x_xstartcapture = x->x_xendcapture;
        x->x_xendcapture = fi;
    }
    if ( x->x_ystartcapture > x->x_yendcapture )
    {
        fi = x->x_ystartcapture;
        x->x_ystartcapture = x->x_yendcapture;
        x->x_yendcapture = fi;
    }
    cxs=(x->x_xstartcapture-x->x_xpos)/x->x_zoom;
    cxe=(x->x_xendcapture-x->x_xpos)/x->x_zoom;
    cys=(x->x_ypos+x->x_blocksize/2*x->x_zoom+1-x->x_yendcapture)/x->x_zoom+x->x_blocksize/2;
    cye=(x->x_ypos+x->x_blocksize/2*x->x_zoom+1-x->x_ystartcapture)/x->x_zoom+x->x_blocksize/2;
    if ( cye < 0 ) cye=0;
    if ( cys < 0 ) cys=0;
    if ( cye >= x->x_blocksize/2 ) cye=x->x_blocksize/2-1;
    if ( cys >= x->x_blocksize/2 ) cys=x->x_blocksize/2-1;
    if ( cxe >= x->x_size ) cxe=x->x_size-1;
    if ( cxs >= x->x_size ) cxs=x->x_size-1;

    // make a copy first
    icopy = ( t_float* ) getbytes( ( cxe-cxs+1 )*( cye-cys+1 )*sizeof( t_float ) );
    rcopy = ( t_float* ) getbytes( ( cxe-cxs+1 )*( cye-cys+1 )*sizeof( t_float ) );
    if ( !icopy || !rcopy )
    {
        post( "sonogram~ : cannot allocate buffers for pasting" );
        return;
    }
    // copy initial data
    copynd = 0;
    for ( si=cxs; si<=cxe; si++)
    {
        for ( fi=cys; fi<=cye; fi++)
        {
            *(rcopy+copynd) = *(x->x_rdata+(si)*x->x_blocksize+fi);
            *(icopy+copynd) = *(x->x_idata+(si)*x->x_blocksize+fi);
            copynd++;
        }
    }

    post( "sonogram~ : paste phase from [%d,%d,%d,%d] to [%d,%d]", cxs, cys, cxe, cye, pxstart, pystart );

    for ( si=cxs; si<=cxe; si++)
    {
        if ( pxstart+si-cxs >= x->x_size ) break;
        copynd = (si-cxs)*(cye-cys+1);
        for ( fi=cys; fi<=cye; fi++)
        {
            if ( pystart+fi-cys > x->x_blocksize+1 ) break;
            fphase = atan2( *(icopy+copynd), *(rcopy+copynd) );
            fdestspectrum =
                sqrt( pow( *(x->x_rdata+(pxstart+si-cxs)*x->x_blocksize+(pystart+fi-cys)), 2) +
                      pow( *(x->x_idata+(pxstart+si-cxs)*x->x_blocksize+(pystart+fi-cys)), 2) );
            fdestphase = atan2( *(x->x_idata+(pxstart+si-cxs)*x->x_blocksize+(pystart+fi-cys)),
                                *(x->x_rdata+(pxstart+si-cxs)*x->x_blocksize+(pystart+fi-cys)) );
            if ( x->x_enhancemode )
            {
                *(x->x_rdata+((pxstart+si-cxs)*x->x_blocksize)+(pystart+fi-cys)) +=
                    fdestspectrum*cos( fdestphase + fphase );
                *(x->x_idata+((pxstart+si-cxs)*x->x_blocksize)+(pystart+fi-cys)) +=
                    fdestspectrum*sin( fdestphase + fphase );
            }
            else
            {
                *(x->x_rdata+((pxstart+si-cxs)*x->x_blocksize)+(pystart+fi-cys)) =
                    fdestspectrum*cos( fphase );
                *(x->x_idata+((pxstart+si-cxs)*x->x_blocksize)+(pystart+fi-cys)) =
                    fdestspectrum*sin( fphase );
            }
            copynd++;
        }
    }

    freebytes( rcopy, ( cxe-cxs+1 )*( cye-cys+1 )*sizeof( t_float ) );
    freebytes( icopy, ( cxe-cxs+1 )*( cye-cys+1 )*sizeof( t_float ) );

    sonogram_update_part(x, x->x_glist, pxstart, pxstart+(si-1)-cxs, 0, 1, 1);
    // start a new capture
    SYS_VGUI3( ".x%lx.c delete %xCAPTURE\n", canvas, x );
    x->x_xstartcapture = x->x_xdraw;
    x->x_ystartcapture = x->x_ydraw;
    x->x_xendcapture = x->x_xdraw;
    x->x_yendcapture = x->x_ydraw;

}

static void sonogram_draw_new(t_sonogram *x, t_glist *glist)
{
    t_canvas *canvas=glist_getcanvas(glist);

    x->x_xpos=text_xpix(&x->x_obj, glist);
    x->x_ypos=text_ypix(&x->x_obj, glist);
    if ( x->x_graphic )
    {
        SYS_VGUI7(".x%lx.c create rectangle %d %d %d %d -fill #FFFFFF -tags %xSONOGRAM\n",
                  canvas, x->x_xpos-1, x->x_ypos-1,
                  x->x_xpos + x->x_size*x->x_zoom+1,
                  x->x_ypos + x->x_blocksize/2*x->x_zoom+1,
                  x);
        SYS_VGUI4("image create photo SONIMAGE%x -format gif -width %d -height %d\n",
                  x, x->x_size*x->x_zoom, x->x_blocksize/2*x->x_zoom );
        SYS_VGUI2("SONIMAGE%x blank\n", x);
        SYS_VGUI6(".x%lx.c create image %d %d -image SONIMAGE%x -tags ISONIMAGE%x\n",
                  canvas, x->x_xpos+((x->x_size*x->x_zoom)/2),
                  (x->x_ypos+((x->x_blocksize/2*x->x_zoom)/2)), x, x );
        if ( x->x_phaso )
        {
            SYS_VGUI7(".x%lx.c create rectangle %d %d %d %d -fill #FFFFFF -tags %xPHASOGRAM\n",
                      canvas, x->x_xpos-1, x->x_ypos+x->x_blocksize/2*x->x_zoom+2,
                      x->x_xpos + x->x_size*x->x_zoom +1,
                      x->x_ypos + x->x_blocksize*x->x_zoom + 3,
                      x);
            SYS_VGUI4("image create photo FAZIMAGE%x -format gif -width %d -height %d\n",
                      x, x->x_size*x->x_zoom, x->x_blocksize/2*x->x_zoom );
            SYS_VGUI2("FAZIMAGE%x blank\n", x);
            SYS_VGUI6(".x%lx.c create image %d %d -image FAZIMAGE%x -tags IFAZIMAGE%x\n",
                      canvas, x->x_xpos+((x->x_size*x->x_zoom)/2),
                      x->x_ypos+3*((x->x_blocksize/2*x->x_zoom)/2)+2, x, x );
        }
        canvas_fixlinesfor( canvas, (t_text*)x );
    }
}

static void sonogram_draw_delete(t_sonogram *x, t_glist *glist)
{
    t_canvas *canvas=glist_getcanvas(glist);

    if ( x->x_graphic && glist_isvisible( glist ) )
    {
        SYS_VGUI3( ".x%lx.c delete %xCAPTURE\n", canvas, x );
        SYS_VGUI3( ".x%lx.c delete line %xREADSTART\n", canvas, x);
        SYS_VGUI3( ".x%lx.c delete line %xREADEND\n", canvas, x);
        SYS_VGUI3( ".x%lx.c delete line %xMODSTART\n", canvas, x);
        SYS_VGUI3( ".x%lx.c delete line %xMODEND\n", canvas, x);
        SYS_VGUI3(".x%lx.c delete %xSONOGRAM\n", canvas, x );
        SYS_VGUI3(".x%lx.c delete %xPHASOGRAM\n", canvas, x );
        SYS_VGUI3(".x%lx.c delete %xISONIMAGE\n", canvas, x );
        SYS_VGUI2("image delete SONIMAGE%x\n", x );
        if ( x->x_phaso )
        {
            SYS_VGUI3(".x%lx.c delete %xIFAZIMAGE\n", canvas, x );
            SYS_VGUI2("image delete FAZIMAGE%x\n", x );
        }
    }
}

static void sonogram_draw_move(t_sonogram *x, t_glist *glist)
{
    t_canvas *canvas=glist_getcanvas(glist);

    if ( x->x_graphic && glist_isvisible( x->x_glist ) )
    {
        SYS_VGUI7(".x%lx.c coords %xSONOGRAM %d %d %d %d\n",
                  canvas, x,
                  x->x_xpos-1, x->x_ypos-1,
                  x->x_xpos+x->x_size*x->x_zoom+1,
                  x->x_ypos+x->x_blocksize/2*x->x_zoom+1);
        SYS_VGUI5(".x%lx.c coords ISONIMAGE%x %d %d\n",
                  canvas, x,
                  x->x_xpos+((x->x_size*x->x_zoom)/2),
                  (x->x_ypos+((x->x_blocksize/2*x->x_zoom)/2)) );
        if ( x->x_phaso )
        {
            SYS_VGUI7(".x%lx.c coords %xPHASOGRAM %d %d %d %d\n",
                      canvas, x,
                      x->x_xpos-1, x->x_ypos+(x->x_blocksize/2*x->x_zoom)+1,
                      x->x_xpos+x->x_size*x->x_zoom+1,
                      x->x_ypos+x->x_blocksize*x->x_zoom+3);
            SYS_VGUI5(".x%lx.c coords IFAZIMAGE%x %d %d\n",
                      canvas, x,
                      x->x_xpos+((x->x_size*x->x_zoom)/2),
                      x->x_ypos+3*((x->x_blocksize/2*x->x_zoom)/2)+2 );
        }
        canvas_fixlinesfor( canvas, (t_text*)x );
    }
}

static void sonogram_draw_select(t_sonogram* x,t_glist* glist)
{
    t_canvas *canvas=glist_getcanvas(glist);

    if ( x->x_graphic && glist_isvisible( x->x_glist ) )
    {
        if(x->x_selected)
        {
            /* sets the item in blue */
            SYS_VGUI3(".x%lx.c itemconfigure %xSONOGRAM -outline #0000FF\n", canvas, x);
            if ( x->x_phaso )
            {
                SYS_VGUI3(".x%lx.c itemconfigure %xPHASOGRAM -outline #0000FF\n", canvas, x);
            }
        }
        else
        {
            SYS_VGUI3(".x%lx.c itemconfigure %xSONOGRAM -outline #000000\n", canvas, x);
            if ( x->x_phaso )
            {
                SYS_VGUI3(".x%lx.c itemconfigure %xPHASOGRAM -outline #000000\n", canvas, x);
            }
        }
    }
}

/* ------------------------ widget callbacks ----------------------------- */


static void sonogram_getrect(t_gobj *z, t_glist *owner,
                             int *xp1, int *yp1, int *xp2, int *yp2)
{
    t_sonogram* x = (t_sonogram*)z;

    *xp1 = x->x_xpos;
    *yp1 = x->x_ypos;
    if ( !x->x_phaso )
    {
        *xp2 = x->x_xpos+x->x_size*x->x_zoom;
        *yp2 = x->x_ypos+x->x_blocksize/2*x->x_zoom+1;
    }
    else
    {
        *xp2 = x->x_xpos+x->x_size*x->x_zoom;
        *yp2 = x->x_ypos+x->x_blocksize*x->x_zoom+3;
    }
}

static void sonogram_save(t_gobj *z, t_binbuf *b)
{
    t_sonogram *x = (t_sonogram *)z;

    binbuf_addv(b, "ssiisiii", gensym("#X"),gensym("obj"),
                (t_int)x->x_obj.te_xpix, (t_int)x->x_obj.te_ypix,
                atom_getsymbol(binbuf_getvec(x->x_obj.te_binbuf)),
                x->x_size, x->x_graphic, x->x_phaso );
    binbuf_addv(b, ";");
}

static void sonogram_select(t_gobj *z, t_glist *glist, int selected)
{
    t_sonogram *x = (t_sonogram *)z;

    x->x_selected = selected;
    sonogram_draw_select( x, glist );
}

static void sonogram_vis(t_gobj *z, t_glist *glist, int vis)
{
    t_sonogram *x = (t_sonogram *)z;
    t_rtext *y;

    if (vis)
    {
        sonogram_draw_new( x, glist );
    }
    else
    {
        // erase all points
        sonogram_draw_delete( x, glist );
    }
}

static void sonogram_delete(t_gobj *z, t_glist *glist)
{
    canvas_deletelinesfor(glist, (t_text *)z);
}

static void sonogram_displace(t_gobj *z, t_glist *glist, int dx, int dy)
{
    t_sonogram *x = (t_sonogram *)z;
    int xold = x->x_xpos;
    int yold = x->x_ypos;

    x->x_xpos += dx;
    x->x_ypos += dy;

    if ( ( x->x_xpos != xold ) || ( x->x_ypos != yold ) )
    {
        sonogram_draw_move( x, glist );
    }

}

static void sonogram_modify_point( t_sonogram* x, t_int sample, t_int frequency, t_int alted )
{
    if ( alted )
    {
        *(x->x_rdata+(sample*x->x_blocksize)+frequency) = 0;
        *(x->x_idata+(sample*x->x_blocksize)+frequency) = 0;
    }
    else
    {
        if ( x->x_enhancemode )
        {
            *(x->x_rdata+(sample*x->x_blocksize)+frequency) *= x->x_modstep;
            *(x->x_idata+(sample*x->x_blocksize)+frequency) *= x->x_modstep;
        }
        else
        {
            *(x->x_rdata+(sample*x->x_blocksize)+frequency) += x->x_modstep;
            *(x->x_idata+(sample*x->x_blocksize)+frequency) += x->x_modstep;
        }
    }
}

static void sonogram_modify_point_phase( t_sonogram* x, t_int sample, t_int frequency, t_int alted )
{
    t_float fspectrum;
    t_float fphase;

    fspectrum =
        sqrt( pow( *(x->x_rdata+sample*x->x_blocksize+frequency), 2) +
              pow( *(x->x_idata+sample*x->x_blocksize+frequency), 2) );
    fphase = atan2( *(x->x_idata+sample*x->x_blocksize+frequency),
                    *(x->x_rdata+sample*x->x_blocksize+frequency) );
    if ( alted==4 )
    {
        // setting phase to 0
        *(x->x_rdata+(sample*x->x_blocksize)+frequency) = fspectrum;
        *(x->x_idata+(sample*x->x_blocksize)+frequency) = 0;
    }
    else
    {
        if ( x->x_enhancemode )
        {
            *(x->x_rdata+(sample*x->x_blocksize)+frequency) = fspectrum*cos( fphase*x->x_modstep );
            *(x->x_idata+(sample*x->x_blocksize)+frequency) = fspectrum*sin( fphase*x->x_modstep );
        }
        else
        {
            *(x->x_rdata+(sample*x->x_blocksize)+frequency) = fspectrum*cos( fphase+x->x_modstep );
            *(x->x_idata+(sample*x->x_blocksize)+frequency) = fspectrum*sin( fphase+x->x_modstep );
        }
    }
}

static void sonogram_motion(t_sonogram *x, t_floatarg dx, t_floatarg dy)
{
    t_int fdraw=0, sdraw=0;
    t_canvas *canvas=glist_getcanvas(x->x_glist);

    // post( "sonogram_motion @ [%d,%d] dx=%f dy=%f alt=%d", x->x_xdraw, x->x_ydraw, dx, dy, x->x_alted );
    if ( ( x->x_shifted || (x->x_alted==4) ) )
    {
        if ( (x->x_xdraw+dx) >= x->x_xpos &&
                (x->x_xdraw+dx) <= x->x_xpos+x->x_size*x->x_zoom )
        {
            x->x_xdraw += dx;
        }
        if ( (x->x_ydraw+dy) >= x->x_ypos &&
                (x->x_ydraw+dy) <= x->x_ypos+x->x_blocksize*x->x_zoom )
        {
            x->x_ydraw += dy;
        }
        sdraw=(x->x_xdraw-x->x_xpos)/x->x_zoom;
        if ( x->x_ydraw <= x->x_ypos+x->x_blocksize/2*x->x_zoom )
        {
            fdraw=(x->x_ypos-x->x_ydraw)/x->x_zoom+x->x_blocksize/2;
            // post( "modify point @ [%d, %d] alted=%d", sdraw, fdraw, x->x_alted );
            sonogram_modify_point( x, sdraw, fdraw, x->x_alted );
        }
        if ( x->x_ydraw >= x->x_ypos+x->x_blocksize/2*x->x_zoom+1 )
        {
            fdraw=(x->x_ypos+x->x_blocksize*x->x_zoom/2+1-x->x_ydraw)/x->x_zoom+x->x_blocksize/2;
            // post( "modify phase @ [%d, %d]", sdraw, fdraw );
            sonogram_modify_point_phase( x, sdraw, fdraw, x->x_alted );
        }
        sonogram_update_point( x, x->x_glist, sdraw, fdraw );
    }
    else
    {
        if ( (x->x_xendcapture+dx) >= x->x_xpos &&
                (x->x_xendcapture+dx) <= x->x_xpos+x->x_size*x->x_zoom )
        {
            x->x_xendcapture += dx;
        }
        if ( (x->x_yendcapture+dy) >= x->x_ypos &&
                (x->x_yendcapture+dy) <= x->x_ypos+x->x_blocksize*x->x_zoom )
        {
            x->x_yendcapture += dy;
        }
        SYS_VGUI3( ".x%lx.c delete %xCAPTURE\n", canvas, x );
        SYS_VGUI7( ".x%lx.c create rectangle %d %d %d %d -outline #0000FF -tags %xCAPTURE\n",
                   canvas, x->x_xstartcapture, x->x_ystartcapture, x->x_xendcapture, x->x_yendcapture, x );
    }
}

static int sonogram_click(t_gobj *z, struct _glist *glist,
                          int xpix, int ypix, int shift, int alt, int dbl, int doit)
{
    t_sonogram* x = (t_sonogram *)z;
    t_int si,fi;
    t_canvas *canvas=glist_getcanvas(x->x_glist);


    // post( "sonogram_click : x=%d y=%d doit=%d alt=%d, shift=%d", xpix, ypix, doit, alt, shift );
    if ( x->x_aftermousedown == 1 && doit == 0)
    {
        x->x_aftermousedown = 1;
    }
    else
    {
        x->x_aftermousedown = 0;
    }
    if ( doit )
    {
        x->x_xdraw = xpix;
        x->x_ydraw = ypix;
        x->x_shifted = shift;
        x->x_alted = alt;
        // activate motion callback
        glist_grab( glist, &x->x_obj.te_g, (t_glistmotionfn)sonogram_motion,
                    0, xpix, ypix );

        if ( shift && alt && (x->x_xstartcapture != x->x_xendcapture ) )
        {
            sonogram_paste(x);
            sonogram_paste_phase(x);
        }
        else if ( shift && (x->x_xstartcapture != x->x_xendcapture ) )
        {
            // add or multiply modstep
            if ( x->x_xstartcapture > x->x_xendcapture )
            {
                fi = x->x_xstartcapture;
                x->x_xstartcapture = x->x_xendcapture;
                x->x_xendcapture = fi;
            }
            if ( x->x_ystartcapture > x->x_yendcapture )
            {
                fi = x->x_ystartcapture;
                x->x_ystartcapture = x->x_yendcapture;
                x->x_yendcapture = fi;
            }
            for ( si=(x->x_xstartcapture-x->x_xpos)/x->x_zoom;
                    si<=(x->x_xendcapture-x->x_xpos)/x->x_zoom; si++)
            {
                for ( fi=(x->x_ypos-x->x_yendcapture)/x->x_zoom+x->x_blocksize/2;
                        fi<=(x->x_ypos-x->x_ystartcapture)/x->x_zoom+x->x_blocksize/2; fi++)
                {
                    sonogram_modify_point( x, si, fi, alt );
                }
                for ( fi=(x->x_ypos+x->x_blocksize/2*x->x_zoom+1-x->x_yendcapture)/x->x_zoom+x->x_blocksize/2;
                        fi<=(x->x_ypos+x->x_blocksize/2*x->x_zoom+1-x->x_ystartcapture)/x->x_zoom+x->x_blocksize/2; fi++)
                {
                    sonogram_modify_point_phase( x, si, fi, alt );
                }
            }
            // post( "modified y from %d to %d", (x->x_ypos-x->x_yendcapture)/x->x_zoom+x->x_blocksize/2,
            //       (x->x_ypos-x->x_ystartcapture)/x->x_zoom+x->x_blocksize/2 );
            sonogram_update_part(x, x->x_glist, (x->x_xstartcapture-x->x_xpos)/x->x_zoom,
                                 (x->x_xendcapture-x->x_xpos)/x->x_zoom, 0, 1, 1);
        }
        else if ( (alt==4) && (x->x_xstartcapture != x->x_xendcapture ) )
        {
            // clean up area
            if ( x->x_xstartcapture > x->x_xendcapture )
            {
                fi = x->x_xstartcapture;
                x->x_xstartcapture = x->x_xendcapture;
                x->x_xendcapture = fi;
            }
            if ( x->x_ystartcapture > x->x_yendcapture )
            {
                fi = x->x_ystartcapture;
                x->x_ystartcapture = x->x_yendcapture;
                x->x_yendcapture = fi;
            }
            for ( si=(x->x_xstartcapture-x->x_xpos)/x->x_zoom;
                    si<=(x->x_xendcapture-x->x_xpos)/x->x_zoom; si++)
            {
                for ( fi=(x->x_ypos-x->x_yendcapture)/x->x_zoom+x->x_blocksize/2;
                        fi<=(x->x_ypos-x->x_ystartcapture)/x->x_zoom+x->x_blocksize/2; fi++)
                {
                    sonogram_modify_point( x, si, fi, alt );
                }
                for ( fi=(x->x_ypos+x->x_blocksize/2*x->x_zoom+1-x->x_yendcapture)/x->x_zoom+x->x_blocksize/2;
                        fi<=(x->x_ypos+x->x_blocksize/2*x->x_zoom+1-x->x_ystartcapture)/x->x_zoom+x->x_blocksize/2; fi++)
                {
                    sonogram_modify_point_phase( x, si, fi, alt );
                }
            }
            sonogram_update_part(x, x->x_glist, (x->x_xstartcapture-x->x_xpos)/x->x_zoom,
                                 (x->x_xendcapture-x->x_xpos)/x->x_zoom, 0, 1, 1);
        }
        // start a new capture
        SYS_VGUI3( ".x%lx.c delete %xCAPTURE\n", canvas, x );
        x->x_xstartcapture = xpix;
        x->x_ystartcapture = ypix;
        x->x_xendcapture = xpix;
        x->x_yendcapture = ypix;
    }
    else
    {
        // nothing
    }
    x->x_aftermousedown = doit;
    return (1);
}

/* clean up */
static void sonogram_free(t_sonogram *x)
{
    if ( x->x_rdata != NULL )
    {
        freebytes(x->x_rdata, x->x_size*x->x_blocksize*sizeof(float) );
        post( "Freed %d bytes", x->x_size*x->x_blocksize*sizeof(float) );
        x->x_rdata = NULL;
    }
    if ( x->x_idata != NULL )
    {
        freebytes(x->x_idata, x->x_size*x->x_blocksize*sizeof(float) );
        post( "Freed %d bytes", x->x_size*x->x_blocksize*sizeof(float) );
        x->x_idata = NULL;
    }
    if ( x->x_rudata != NULL )
    {
        freebytes(x->x_rudata, x->x_size*x->x_blocksize*sizeof(float) );
        post( "Freed %d bytes", x->x_size*x->x_blocksize*sizeof(float) );
        x->x_rdata = NULL;
    }
    if ( x->x_iudata != NULL )
    {
        freebytes(x->x_iudata, x->x_size*x->x_blocksize*sizeof(float) );
        post( "Freed %d bytes", x->x_size*x->x_blocksize*sizeof(float) );
        x->x_idata = NULL;
    }
    if ( x->x_gifdata != NULL )
    {
        freebytes(x->x_gifdata, (x->x_blocksize/2)*x->x_zoom*sizeof("#FFFFFF ") );
        post( "Freed %d bytes", (x->x_blocksize/2)*x->x_zoom*sizeof("#FFFFFF ") );
        x->x_gifdata = NULL;
    }
    if ( x->x_guicommand != NULL )
    {
        freebytes(x->x_guicommand, 128+(x->x_blocksize/2)*x->x_zoom*sizeof("#FFFFFF ") );
        post( "Freed %d bytes", 128+(x->x_blocksize/2)*x->x_zoom*sizeof("#FFFFFF ") );
        x->x_guicommand = NULL;
    }
}

/* allocate tables for storing ffts */
static t_int sonogram_allocate(t_sonogram *x)
{
    t_int fi;

    if ( !(x->x_rdata = getbytes( x->x_size*x->x_blocksize*sizeof(float) ) ) )
    {
        post( "sonogram~ : error : could not allocate buffers" );
        return -1;
    }
    else
    {
        post( "sonogram~ : allocated %d bytes", x->x_size*x->x_blocksize*sizeof(float) );
    }
    if ( !(x->x_idata = getbytes( x->x_size*x->x_blocksize*sizeof(float) ) ) )
    {
        post( "sonogram~ : error : could not allocate buffers" );
        return -1;
    }
    else
    {
        post( "sonogram~ : allocated %d bytes", x->x_size*x->x_blocksize*sizeof(float) );
    }
    if ( !(x->x_multfreq = getbytes( x->x_blocksize*sizeof(t_int) ) ) )
    {
        post( "sonogram~ : error : could not allocate buffers" );
        return -1;
    }
    else
    {
        post( "sonogram~ : allocated %d bytes", x->x_blocksize*sizeof(t_int) );
    }
    for ( fi=0; fi<x->x_blocksize; fi++ )
    {
        *(x->x_multfreq+fi)=1;
    }
    // no undo is available
    x->x_uxs = x->x_uxe = x->x_uys = x->x_uye = -1;
    if ( !(x->x_rudata = getbytes( x->x_size*x->x_blocksize*sizeof(float) ) ) )
    {
        post( "sonogram~ : error : could not allocate buffers" );
        return -1;
    }
    else
    {
        post( "sonogram~ : allocated %d bytes", x->x_size*x->x_blocksize*sizeof(float) );
    }
    if ( !(x->x_iudata = getbytes( x->x_size*x->x_blocksize*sizeof(float) ) ) )
    {
        post( "sonogram~ : error : could not allocate buffers" );
        return -1;
    }
    else
    {
        post( "sonogram~ : allocated %d bytes", x->x_size*x->x_blocksize*sizeof(float) );
    }
    if ( !( x->x_gifdata = ( char* ) getbytes( (x->x_blocksize/2)*x->x_zoom*sizeof("#FFFFFF ") ) ) )
    {
        post( "sonogram~ : error : could not allocate buffers" );
        return -1;
    }
    else
    {
        post( "sonogram~ : allocated %d bytes",  (x->x_blocksize/2)*x->x_zoom*sizeof("#FFFFFF ") );
    }
    if ( !( x->x_guicommand = ( char* ) getbytes( 128+(x->x_blocksize/2)*x->x_zoom*sizeof("#FFFFFF ") ) ) )
    {
        post( "sonogram~ : error : could not allocate buffers" );
        return -1;
    }
    else
    {
        post( "sonogram~ : allocated %d bytes",  128+(x->x_blocksize/2)*x->x_zoom*sizeof("#FFFFFF ") );
    }

    return 0;
}

/* reallocate tables for storing ffts */
static t_int sonogram_reallocate(t_sonogram *x, t_int ioldsize, t_int inewsize)
{
    t_int fi;
    t_float *prdata=x->x_rdata, *pidata=x->x_idata;
    t_float *prudata=x->x_rudata, *piudata=x->x_iudata;

    if ( !(x->x_rdata = getbytes( inewsize*x->x_blocksize*sizeof(float) ) ) )
    {
        post( "sonogram~ : error : could not allocate buffers" );
        return -1;
    }
    else
    {
        post( "sonogram~ : allocated %d bytes", inewsize*x->x_blocksize*sizeof(float) );
    }
    if ( !(x->x_idata = getbytes( inewsize*x->x_blocksize*sizeof(float) ) ) )
    {
        post( "sonogram~ : error : could not allocate buffers" );
        return -1;
    }
    else
    {
        post( "sonogram~ : allocated %d bytes", inewsize*x->x_blocksize*sizeof(float) );
    }
    // no undo is available
    x->x_uxs = x->x_uxe = x->x_uys = x->x_uye = -1;
    if ( !(x->x_rudata = getbytes( inewsize*x->x_blocksize*sizeof(float) ) ) )
    {
        post( "sonogram~ : error : could not allocate buffers" );
        return -1;
    }
    else
    {
        post( "sonogram~ : allocated %d bytes", inewsize*x->x_blocksize*sizeof(float) );
    }
    if ( !(x->x_iudata = getbytes( inewsize*x->x_blocksize*sizeof(float) ) ) )
    {
        post( "sonogram~ : error : could not allocate buffers" );
        return -1;
    }
    else
    {
        post( "sonogram~ : allocated %d bytes", inewsize*x->x_blocksize*sizeof(float) );
    }
    if ( prdata != NULL )
    {
        freebytes(prdata, ioldsize*x->x_blocksize*sizeof(float) );
        post( "Freed %d bytes", ioldsize*x->x_blocksize*sizeof(float) );
    }
    if ( pidata != NULL )
    {
        freebytes(pidata, ioldsize*x->x_blocksize*sizeof(float) );
        post( "Freed %d bytes", ioldsize*x->x_blocksize*sizeof(float) );
    }
    if ( prudata != NULL )
    {
        freebytes(prudata, ioldsize*x->x_blocksize*sizeof(float) );
        post( "Freed %d bytes", ioldsize*x->x_blocksize*sizeof(float) );
    }
    if ( piudata != NULL )
    {
        freebytes(piudata, ioldsize*x->x_blocksize*sizeof(float) );
        post( "Freed %d bytes", ioldsize*x->x_blocksize*sizeof(float) );
    }

    return 0;
}
/* records or playback the sonogram */
static t_int *sonogram_perform(t_int *w)
{
    t_float *rin = (t_float *)(w[1]);
    t_float *iin = (t_float *)(w[2]);
    t_float *rout = (t_float *)(w[3]);
    t_float *iout = (t_float *)(w[4]);
    t_float fspectrum = 0.0;
    t_float fphase = 0.0;
    t_int   is;
    t_int n = (int)(w[5]);                      /* number of samples */
    t_sonogram *x = (t_sonogram *)(w[6]);
    t_int bi;
    t_int startsamp, endsamp;

    if ( x->x_readstart <= x->x_readend )
    {
        startsamp = (x->x_readstart*x->x_size)/100;
        endsamp = (x->x_readend*x->x_size)/100;
    }
    else
    {
        startsamp = (x->x_readend*x->x_size)/100;
        endsamp = (x->x_readstart*x->x_size)/100;
    }

    // reallocate tables if blocksize has been changed
    if ( n != x->x_blocksize && x->x_updatechild == 0 )
    {
        post( "sonogram~ : reallocating tables" );
        // erase all points
        sonogram_free(x);
        x->x_blocksize = n;
        sonogram_allocate(x);
        sonogram_update_part(x, x->x_glist, 0, x->x_size-1, !x->x_empty, 0, 0);
        canvas_fixlinesfor(x->x_glist, (t_text*)x );
    }

    bi = 0;
    while (bi<n)
    {
        // eventually records input
        if ( x->x_record)
        {
            *(x->x_rdata+(x->x_writepos*x->x_blocksize)+bi)=(*(rin))*(*(x->x_multfreq+bi));
            *(x->x_idata+(x->x_writepos*x->x_blocksize)+bi)=(*(iin))*(*(x->x_multfreq+bi));
        }
        // set outputs
        *rout = 0.0;
        *iout = 0.0;
        if ( x->x_play)
        {
            is=0;
            fspectrum =
                sqrt( pow( *(x->x_rdata+(((int)x->x_readpos+is)*x->x_blocksize)+bi), 2) +
                      pow( *(x->x_idata+(((int)x->x_readpos+is)*x->x_blocksize)+bi), 2) );
            fphase = atan2( *(x->x_idata+(((int)x->x_readpos+is)*x->x_blocksize)+bi),
                            *(x->x_rdata+(((int)x->x_readpos+is)*x->x_blocksize)+bi) );
            fphase += (x->x_phase/180.0)*(M_PI);
            *rout += fspectrum*cos( fphase );
            *iout += fspectrum*sin( fphase );
        }
        rout++;
        iout++;
        rin++;
        iin++;
        bi++;

    }
    // reset playing position until next play
    if ( x->x_play )
    {
        x->x_readpos+=x->x_readspeed;
        // post( "xreadpos : %f (added %f) %d", x->x_readpos, x->x_readspeed, x->x_readend );
        if ( ( x->x_readspeed >= 0 ) && ( x->x_readpos >= endsamp ) )
        {
            x->x_play=0;
            x->x_readpos=(float)(startsamp);
            // post( "cooled~ : stopped playing (readpos=%d)", x->x_readpos );
            outlet_bang(x->x_end);
        }
        if ( ( x->x_readspeed < 0 ) && ( x->x_readpos <= startsamp ) )
        {
            x->x_play=0;
            x->x_readpos = (float)(endsamp);
            // post( "cooled~ : stopped playing (readpos=%d)", x->x_readpos );
            outlet_bang(x->x_end);
        }
    }
    // reset recording position until next record
    if ( x->x_record )
    {
        x->x_writepos++;
        if ( x->x_writepos >= x->x_size )
        {
            x->x_record=0;
            x->x_writepos=0;
            sonogram_update_part(x, x->x_glist, 0, x->x_size-1, 0, 1, 0);
            outlet_bang(x->x_recend);
            if ( x->x_empty ) x->x_empty = 0;
            // post( "sonogram~ : stopped recording" );
        }
    }
    // post( "sonogram~ : read : %f:%d : write: %d:%d", x->x_readpos, x->x_play, x->x_writepos, x->x_record );
    return (w+7);
}

static void sonogram_dsp(t_sonogram *x, t_signal **sp)
{
    dsp_add(sonogram_perform, 6, sp[0]->s_vec, sp[1]->s_vec, sp[2]->s_vec, sp[3]->s_vec, sp[0]->s_n, x);
}

/* record the sonogram */
static void sonogram_record(t_sonogram *x)
{
    x->x_record=1;
    x->x_writepos=0;
    // post( "sonogram~ : recording on" );
}

/* play the sonogram */
static void sonogram_play(t_sonogram *x)
{
    x->x_play=1;
    x->x_readpos=(x->x_readstart*x->x_size)/100;
    // post( "sonogram~ : playing on" );
}

/* setting the starting point for reading ( in percent ) */
static void sonogram_readstart(t_sonogram *x, t_floatarg fstart)
{
    t_float startpoint = fstart;
    t_canvas *canvas=glist_getcanvas(x->x_glist);

    if (startpoint < 0) startpoint = 0;
    if (startpoint > 100) startpoint = 100;
    x->x_readstart=startpoint;
    // set readspeed sign
    if ( ( x->x_readstart > x->x_readend ) && ( x->x_readspeed > 0 ) ) x->x_readspeed = -x->x_readspeed;
    if ( ( x->x_readstart < x->x_readend ) && ( x->x_readspeed < 0 ) ) x->x_readspeed = -x->x_readspeed;
    if ( x->x_graphic && glist_isvisible( x->x_glist ) )
    {
        SYS_VGUI3( ".x%lx.c delete line %xREADSTART\n",
                   canvas, x);
        SYS_VGUI7( ".x%lx.c create line %d %d %d %d -fill #FF0000 -tags %xREADSTART -width 3\n",
                   canvas, x->x_xpos+(x->x_readstart*(x->x_size)/100 ),
                   x->x_ypos, x->x_xpos+(x->x_readstart*(x->x_size)/100 ),
                   x->x_ypos+x->x_blocksize*x->x_zoom, x );
    }
}

/* setting the starting point for modification ( in percent ) */
static void sonogram_modstart(t_sonogram *x, t_floatarg fstart)
{
    t_float startpoint = fstart;
    t_canvas *canvas=glist_getcanvas(x->x_glist);

    if (startpoint < 0) startpoint = 0;
    if (startpoint > 100) startpoint = 100;
    if ( startpoint > x->x_modend )
    {
        x->x_modstart = x->x_modend;
        post( "sonogram~ : warning : range for modifications is null" );
    }
    else
    {
        x->x_modstart=startpoint;
    }
    if ( x->x_graphic && glist_isvisible( x->x_glist ) )
    {
        SYS_VGUI3( ".x%lx.c delete line %xMODSTART\n",
                   canvas, x);
        SYS_VGUI7( ".x%lx.c create line %d %d %d %d -fill #11E834 -tags %xMODSTART -width 3\n",
                   canvas, x->x_xpos+(x->x_modstart*(x->x_size)/100 ),
                   x->x_ypos, x->x_xpos+(x->x_modstart*(x->x_size)/100 ),
                   x->x_ypos+x->x_blocksize*x->x_zoom, x );
    }
}

/* setting the modification step for graphical mode */
static void sonogram_modstep(t_sonogram *x, t_floatarg fmodstep)
{
    if ( x->x_graphic )
    {
        x->x_modstep = fmodstep;
    }
}

/* setting enhance mode */
static void sonogram_enhancemode(t_sonogram *x, t_floatarg fenhancemode)
{
    if ( x->x_graphic )
    {
        x->x_enhancemode = fenhancemode;
    }
}

/* setting the ending point for reading ( in percent ) */
static void sonogram_readend(t_sonogram *x, t_floatarg fend)
{
    t_float endpoint = fend;
    t_canvas *canvas=glist_getcanvas(x->x_glist);

    if (endpoint < 0) endpoint = 0;
    if (endpoint > 100) endpoint = 100;
    x->x_readend=endpoint;
    // set readspeed sign
    if ( ( x->x_readstart > x->x_readend ) && ( x->x_readspeed > 0 ) ) x->x_readspeed = -x->x_readspeed;
    if ( ( x->x_readstart < x->x_readend ) && ( x->x_readspeed < 0 ) ) x->x_readspeed = -x->x_readspeed;
    if ( x->x_graphic && glist_isvisible( x->x_glist ) )
    {
        SYS_VGUI3( ".x%lx.c delete line %xREADEND\n",
                   canvas, x);
        SYS_VGUI7( ".x%lx.c create line %d %d %d %d -fill #FF0000 -tags %xREADEND -width 3\n",
                   canvas, x->x_xpos+(x->x_readend*(x->x_size)/100 ),
                   x->x_ypos, x->x_xpos+(x->x_readend*(x->x_size)/100 ),
                   x->x_ypos+x->x_blocksize*x->x_zoom, x );
    }
}

/* setting the ending point for modification ( in percent ) */
static void sonogram_modend(t_sonogram *x, t_floatarg fend)
{
    t_float endpoint = fend;
    t_canvas *canvas=glist_getcanvas(x->x_glist);

    if (endpoint < 0) endpoint = 0;
    if (endpoint > 100) endpoint = 100;
    if ( endpoint < x->x_modstart )
    {
        x->x_modend = x->x_modstart;
        post( "sonogram~ : warning : range for modifications is null" );
    }
    else
    {
        x->x_modend=endpoint;
    }
    if ( x->x_graphic && glist_isvisible( x->x_glist ) )
    {
        SYS_VGUI3( ".x%lx.c delete line %xMODEND\n",
                   canvas, x);
        SYS_VGUI7( ".x%lx.c create line %d %d %d %d -fill #11E834 -tags %xMODEND -width 3\n",
                   canvas, x->x_xpos+(x->x_modend*(x->x_size)/100 ),
                   x->x_ypos, x->x_xpos+(x->x_modend*(x->x_size)/100 ),
                   x->x_ypos+x->x_blocksize*x->x_zoom, x );
    }
}

/* sets the reading speed */
static void sonogram_readspeed(t_sonogram *x, t_floatarg freadspeed)
{
    if (freadspeed <= 0 )
    {
        post( "sonogram~ : wrong readspeed argument" );
        return;
    }
    x->x_readspeed=freadspeed;
}

/* enhance frequencies */
static void sonogram_enhance(t_sonogram *x, t_floatarg fstartfreq, t_floatarg fendfreq, t_floatarg fenhance, t_floatarg fnoupdate )
{
    t_int samplestart, sampleend, si, fi=0, ffi=0;
    t_float oldenergy;

    if (fstartfreq < 0 || fendfreq < 0 ||
            fstartfreq > x->x_blocksize || fendfreq > x->x_blocksize ||
            fstartfreq > fendfreq )
    {
        post( "sonogram~ : error : wrong frequencies range" );
        return;
    }
    if ( fenhance < 0 )
    {
        post( "sonogram~ : error : wrong multiplicating factor" );
        return;
    }
    samplestart=(x->x_modstart*(x->x_size-1))/100;
    sampleend=(x->x_modend*(x->x_size-1))/100;
    // post("enhancing portion [%d,%d]", samplestart, sampleend );
    for ( si=samplestart; si<=sampleend; si++ )
    {
        for ( fi=(int)fstartfreq; fi<=(int)fendfreq; fi++ )
        {
            *(x->x_multfreq+fi) = fenhance;
            if ( (fi != 0) && (fi != x->x_blocksize/2-1) )
            {
                /* multiply both r*sin(a) and r*cos(a) to mutiply r */
                *(x->x_rdata+(si*x->x_blocksize)+fi) *= fenhance;
                *(x->x_idata+(si*x->x_blocksize)+fi) *= fenhance;
            }
        }
    }
    // post( "sonogram~ : enhanced %d,%d", fi, ffi );
    if ( !(int)fnoupdate )
    {
        sonogram_update_part(x, x->x_glist, samplestart, sampleend, 0, 1, 1);
    }
}

/* add a constant to frequencies */
static void sonogram_add(t_sonogram *x, t_floatarg fstartfreq, t_floatarg fendfreq, t_floatarg fadd)
{
    t_int samplestart, sampleend, si, fi;
    t_float oldenergy;

    if (fstartfreq < 0 || fendfreq < 0 ||
            fstartfreq > x->x_blocksize || fendfreq > x->x_blocksize ||
            fstartfreq > fendfreq )
    {
        post( "sonogram~ : error : wrong frequencies range" );
        return;
    }
    samplestart=(x->x_modstart*(x->x_size-1))/100;
    sampleend=(x->x_modend*(x->x_size-1))/100;
    // post("enhancing portion [%d,%d]", samplestart, sampleend );
    for ( si=samplestart; si<=sampleend; si++ )
    {
        for ( fi=(int)fstartfreq; fi<=(int)fendfreq; fi++ )
        {
            /* multiply both r*sin(a) and r*cos(a) to mutiply r */
            *(x->x_rdata+(si*x->x_blocksize)+fi) += fadd;
            *(x->x_idata+(si*x->x_blocksize)+fi) += fadd;
        }
    }
    sonogram_update_part(x, x->x_glist, samplestart, sampleend, 0, 1, 1);
}

/* resize sonogram */
static void sonogram_resize(t_sonogram *x, t_floatarg fnewsize )
{
    if (fnewsize <= 0)
    {
        post( "sonogram~ : error : wrong size" );
        return;
    }
    if (x->x_updatechild > 0)
    {
        post( "sonogram~ : can't resize now, an update is pending." );
        return;
    }
    post( "sonogram~ : reallocating tables" );
    x->x_record=0;
    x->x_play=0;
    sonogram_reallocate(x, x->x_size, fnewsize);
    x->x_size = fnewsize;
    // erase all points, as data is zero no drawing is needed
    sonogram_update_part(x, x->x_glist, 0, x->x_size-1, 0, 0, 0);
}

/* set zoom factor */
static void sonogram_zoom(t_sonogram *x, t_floatarg fzoom )
{
    post( "sonogram~: warning : zoom and big block factors might lead to a crash" );
    if (fzoom < 1)
    {
        post( "sonogram~ : error : wrong zoom factor" );
        return;
    }
    if ( x->x_gifdata != NULL )
    {
        freebytes(x->x_gifdata, (x->x_blocksize/2)*x->x_zoom*sizeof("#FFFFFF ") );
        post( "Freed %d bytes", (x->x_blocksize/2)*x->x_zoom*sizeof("#FFFFFF ") );
        x->x_gifdata = NULL;
    }
    if ( x->x_guicommand != NULL )
    {
        freebytes(x->x_guicommand, 128+(x->x_blocksize/2)*x->x_zoom*sizeof("#FFFFFF ") );
        post( "Freed %d bytes", 128+(x->x_blocksize/2)*x->x_zoom*sizeof("#FFFFFF ") );
        x->x_guicommand = NULL;
    }
    x->x_zoom = (int)fzoom;
    if ( !( x->x_gifdata = ( char* ) getbytes( (x->x_blocksize/2)*x->x_zoom*sizeof("#FFFFFF ") ) ) )
    {
        post( "sonogram~ : error : could not allocate buffers" );
        return;
    }
    else
    {
        post( "sonogram~ : allocated %d bytes",  (x->x_blocksize/2)*x->x_zoom*sizeof("#FFFFFF ") );
    }
    if ( !( x->x_guicommand = ( char* ) getbytes( 128+(x->x_blocksize/2)*x->x_zoom*sizeof("#FFFFFF ") ) ) )
    {
        post( "sonogram~ : error : could not allocate buffers" );
        return;
    }
    else
    {
        post( "sonogram~ : allocated %d bytes",  128+(x->x_blocksize/2)*x->x_zoom*sizeof("#FFFFFF ") );
    }
    sonogram_update_part(x, x->x_glist, 0, x->x_size-1, !x->x_empty, !x->x_empty, 0);
    canvas_fixlinesfor(x->x_glist, (t_text*)x );
}

/* refresh data    */
static void sonogram_refresh(t_sonogram *x)
{
    sonogram_update_part(x, x->x_glist, 0, x->x_size-1, 0, 1, 1);
}

/* flip frequencies */
static void sonogram_flipfreqs(t_sonogram *x)
{
    t_int samplestart, sampleend, si, fi;
    t_float fvalue;
    t_int ioperon;

    samplestart=(x->x_modstart*(x->x_size-1))/100;
    sampleend=(x->x_modend*(x->x_size-1))/100;
    ioperon=x->x_blocksize/2;
    for ( si=samplestart; si<=sampleend; si++ )
    {
        for ( fi=0; fi<=ioperon/2; fi++ )
        {
            fvalue = *(x->x_rdata+(si*x->x_blocksize)+fi);
            *(x->x_rdata+(si*x->x_blocksize)+fi) = *(x->x_rdata+(si*x->x_blocksize)+(ioperon-fi-1));
            *(x->x_rdata+(si*x->x_blocksize)+(ioperon-fi-1)) = fvalue;
            fvalue = *(x->x_idata+(si*x->x_blocksize)+fi);
            *(x->x_idata+(si*x->x_blocksize)+fi) = *(x->x_idata+(si*x->x_blocksize)+(ioperon-fi-1));
            *(x->x_idata+(si*x->x_blocksize)+(ioperon-fi-1)) = fvalue;
        }
    }
    sonogram_update_part(x, x->x_glist, samplestart, sampleend, 0, 1, 1);
}

/* flip blocks */
static void sonogram_flipblocks(t_sonogram *x)
{
    t_int samplestart, sampleend, middlesample, fi, si;
    t_float fvalue;

    samplestart=(x->x_modstart*(x->x_size-1))/100;
    sampleend=(x->x_modend*(x->x_size-1))/100;
    middlesample = ( sampleend+samplestart+1 ) / 2;
    for ( si=samplestart; si<=middlesample; si++ )
    {
        for ( fi=0; fi<x->x_blocksize; fi++ )
        {
            fvalue = *(x->x_rdata+((si)*x->x_blocksize)+fi);
            *(x->x_rdata+((si)*x->x_blocksize)+fi) = *(x->x_rdata+((sampleend+samplestart-si)*x->x_blocksize)+fi);
            *(x->x_rdata+((sampleend+samplestart-si)*x->x_blocksize)+fi) = fvalue;
            fvalue = *(x->x_idata+((si)*x->x_blocksize)+fi);
            *(x->x_idata+((si)*x->x_blocksize)+fi) = *(x->x_idata+((sampleend+samplestart-si)*x->x_blocksize)+fi);
            *(x->x_idata+((sampleend+samplestart-si)*x->x_blocksize)+fi) = fvalue;
        }
    }
    sonogram_update_part(x, x->x_glist, samplestart, sampleend, 0, 1, 1);
}

/* undo if available */
static void sonogram_undo(t_sonogram *x)
{
    t_int si,fi;

    if ( x->x_uxs == -1 )
    {
        post( "sonogram~ : nothing to undo, man" );
        return;
    }

    post( "sonogram~ : restoring region [%d,%d,%d,%d]", x->x_uxs, x->x_uys, x->x_uxe, x->x_uye );
    for ( si=x->x_uxs; si<=x->x_uxe; si++ )
    {
        for ( fi=x->x_uys; fi<=x->x_uye; fi++ )
        {
            *(x->x_rdata+((si)*x->x_blocksize)+fi) = *(x->x_rudata+(si-x->x_uxs)*x->x_blocksize+(fi-x->x_uys));
            *(x->x_idata+((si)*x->x_blocksize)+fi) = *(x->x_iudata+(si-x->x_uxs)*x->x_blocksize+(fi-x->x_uys));
        }
    }
    sonogram_update_part(x, x->x_glist, x->x_uxs, x->x_uxe, 0, 1, 1);
}

/* zswap exchanges real and imaginery part */
static void sonogram_zswap(t_sonogram *x)
{
    t_int samplestart, sampleend, fi, si;
    t_float fvalue;

    samplestart=(x->x_modstart*(x->x_size-1))/100;
    sampleend=(x->x_modend*(x->x_size-1))/100;
    for ( si=samplestart; si<=sampleend; si++ )
    {
        for ( fi=0; fi<x->x_blocksize; fi++ )
        {
            fvalue = *(x->x_rdata+(si*x->x_blocksize)+fi);
            *(x->x_rdata+(si*x->x_blocksize)+fi) = *(x->x_idata+(si*x->x_blocksize)+fi);
            *(x->x_idata+(si*x->x_blocksize)+fi) = fvalue;
        }
    }
}

/* swap points */
static void sonogram_swappoints(t_sonogram *x, t_floatarg fnbpoints)
{
    t_int samplestart, sampleend, sp;
    t_float s1, s2, f1, f2;
    t_float fvalue;

    if (fnbpoints <= 0)
    {
        post( "sonogram~ : error : bad number of points" );
        return;
    }
    samplestart=(x->x_modstart*(x->x_size-1))/100;
    sampleend=(x->x_modend*(x->x_size-1))/100;

    for ( sp=0; sp<fnbpoints; sp++ )
    {
        s1 = samplestart + (random()%(sampleend-samplestart));
        s2 = samplestart + (random()%(sampleend-samplestart));
        f1 = random()%( x->x_blocksize/2-1 );
        f2 = random()%( x->x_blocksize/2-1 );
        fvalue = *(x->x_rdata+((int)s1*x->x_blocksize)+(int)f1);
        *(x->x_rdata+((int)s1*x->x_blocksize)+(int)f1) = *(x->x_rdata+((int)s2*x->x_blocksize)+(int)f2);
        *(x->x_rdata+((int)s2*x->x_blocksize)+(int)f2) = fvalue;
        fvalue = *(x->x_idata+((int)s1*x->x_blocksize)+(int)f1);
        *(x->x_idata+((int)s1*x->x_blocksize)+(int)f1) = *(x->x_idata+((int)s2*x->x_blocksize)+(int)f2);
        *(x->x_idata+((int)s2*x->x_blocksize)+(int)f2) = fvalue;
    }
    sonogram_update_part(x, x->x_glist, samplestart, sampleend, 0, 1, 1);
}

/* average blocks according to a factor */
static void sonogram_average(t_sonogram *x, t_floatarg fnbblocks)
{
    t_int samplestart, sampleend, fi, si, ssi;
    t_float fraverage, fiaverage;

    if (fnbblocks < 1)
    {
        post( "sonogram~ : error : bad average factor" );
        return;
    }
    samplestart=(x->x_modstart*(x->x_size-1))/100;
    sampleend=(x->x_modend*(x->x_size-1))/100;

    fraverage=fiaverage=0.0;
    for ( fi=0; fi<x->x_blocksize; fi++ )
    {
        for ( si=samplestart; si<=sampleend-fnbblocks; si+=fnbblocks )
        {
            fraverage=fiaverage=0.0;
            for ( ssi=0; ssi<fnbblocks; ssi++ )
            {
                fraverage += *(x->x_rdata+((int)(si+ssi)*x->x_blocksize)+fi);
                fiaverage += *(x->x_idata+((int)(si+ssi)*x->x_blocksize)+fi);
            }
            fraverage /= fnbblocks;
            fiaverage /= fnbblocks;
            for ( ssi=0; ssi<fnbblocks; ssi++ )
            {
                *(x->x_rdata+((int)(si+ssi)*x->x_blocksize)+fi)=fraverage;
                *(x->x_idata+((int)(si+ssi)*x->x_blocksize)+fi)=fiaverage;
            }
        }
    }
    sonogram_update_part(x, x->x_glist, samplestart, sampleend, 0, 1, 1);
}

/* go up by the given number */
static void sonogram_goup(t_sonogram *x, t_floatarg fgoup)
{
    t_int samplestart, sampleend, sp, sf;

    if (fgoup <= 0 || fgoup > x->x_blocksize/2)
    {
        post( "sonogram~ : error : wrong offset in goup function" );
        return;
    }
    samplestart=(x->x_modstart*(x->x_size-1))/100;
    sampleend=(x->x_modend*(x->x_size-1))/100;

    for ( sp=samplestart; sp<=sampleend; sp++ )
    {
        for (sf=(x->x_blocksize/2)-fgoup-1; sf>=0; sf-- )
        {
            *(x->x_rdata+((int)sp*x->x_blocksize)+(sf+(int)fgoup)) =
                *(x->x_rdata+((int)sp*x->x_blocksize)+sf);
            *(x->x_idata+((int)sp*x->x_blocksize)+(sf+(int)fgoup)) =
                *(x->x_idata+((int)sp*x->x_blocksize)+sf);

        }
        for (sf=0; sf<fgoup; sf++ )
        {
            *(x->x_rdata+((int)sp*x->x_blocksize)+(int)sf) = 0.0;
            *(x->x_idata+((int)sp*x->x_blocksize)+(int)sf) = 0.0;
        }
    }
    sonogram_update_part(x, x->x_glist, samplestart, sampleend, 0, 1, 1);
}

/* roll up by the given number */
static void sonogram_roll(t_sonogram *x, t_floatarg froll)
{
    t_int samplestart, sampleend, sp, sf;
    t_float *fprvalues;
    t_float *fpivalues;

    if (froll <= 0 || froll > x->x_blocksize/2)
    {
        post( "sonogram~ : error : wrong offset in roll function" );
        return;
    }
    fprvalues = (t_float*)getbytes( ((int)froll)*sizeof( float ) );
    if ( !fprvalues )
    {
        post( "sonogram~ : error : could not allocate %d bytes", ((int)froll)*sizeof(float) );
        return;
    }
    fpivalues = (t_float*)getbytes( ((int)froll)*sizeof( float ) );
    if ( !fpivalues )
    {
        post( "sonogram~ : error : could not allocate %d bytes", ((int)froll)*sizeof(float) );
        return;
    }

    samplestart=(x->x_modstart*(x->x_size-1))/100;
    sampleend=(x->x_modend*(x->x_size-1))/100;

    for ( sp=samplestart; sp<=sampleend; sp++ )
    {

        // saving values
        for (sf=0; sf<froll; sf++ )
        {
            *(fprvalues+sf) =
                *(x->x_rdata+((int)sp*x->x_blocksize)+(x->x_blocksize/2-(int)froll+sf));
            *(fpivalues+sf) =
                *(x->x_idata+((int)sp*x->x_blocksize)+(x->x_blocksize/2-(int)froll+sf));
        }
        for (sf=(x->x_blocksize/2)-froll-1; sf>=0; sf-- )
        {
            *(x->x_rdata+((int)sp*x->x_blocksize)+(sf+(int)froll)) =
                *(x->x_rdata+((int)sp*x->x_blocksize)+sf);
            *(x->x_idata+((int)sp*x->x_blocksize)+(sf+(int)froll)) =
                *(x->x_idata+((int)sp*x->x_blocksize)+sf);
        }
        for (sf=0; sf<froll; sf++ )
        {
            *(x->x_rdata+((int)sp*x->x_blocksize)+(int)sf) = *(fprvalues+sf);
            *(x->x_idata+((int)sp*x->x_blocksize)+(int)sf) = *(fpivalues+sf);
        }
    }
    freebytes( fprvalues, (int)froll*sizeof(float) );
    freebytes( fpivalues, (int)froll*sizeof(float) );
    sonogram_update_part(x, x->x_glist, samplestart, sampleend, 0, 1, 1);
}

/* suppress point below the threshold */
static void sonogram_threshold(t_sonogram *x, t_floatarg fthreshold)
{
    t_int samplestart, sampleend, sp, sf;
    t_float fspectrum;

    if (fthreshold <= 0)
    {
        post( "sonogram~ : error : wrong threshold" );
        return;
    }
    samplestart=(x->x_modstart*(x->x_size-1))/100;
    sampleend=(x->x_modend*(x->x_size-1))/100;

    for ( sp=samplestart; sp<=sampleend; sp++ )
    {
        for (sf=0; sf<=(x->x_blocksize/2)-1; sf++ )
        {
            fspectrum = sqrt( pow( *(x->x_rdata+sp*x->x_blocksize+sf), 2) +
                              pow( *(x->x_idata+sp*x->x_blocksize+sf), 2) );
            if ( fspectrum < fthreshold )
            {
                *(x->x_rdata+sp*x->x_blocksize+sf) = 0.0;
                *(x->x_idata+sp*x->x_blocksize+sf) = 0.0;
            }
        }
    }
    sonogram_update_part(x, x->x_glist, samplestart, sampleend, 0, 1, 1);
}

/* change the phase */
static void sonogram_phase(t_sonogram *x, t_floatarg fincphase)
{
    if (fincphase < 0 || fincphase > 90)
    {
        post( "sonogram~ : error : wrong phase in phase function : out of [0,90]" );
        return;
    }
    x->x_phase = fincphase;
}

/* go down by the given number */
static void sonogram_godown(t_sonogram *x, t_floatarg fgodown)
{
    t_int samplestart, sampleend, sp, sf;

    if (fgodown <= 0 || fgodown > x->x_blocksize/2)
    {
        post( "sonogram~ : error : wrong offset in godown function" );
        return;
    }
    samplestart=(x->x_modstart*(x->x_size-1))/100;
    sampleend=(x->x_modend*(x->x_size-1))/100;

    for ( sp=samplestart; sp<=sampleend; sp++ )
    {
        for (sf=0; sf<=(x->x_blocksize/2)-fgodown-1; sf++ )
        {
            *(x->x_rdata+((int)sp*x->x_blocksize)+sf) =
                *(x->x_rdata+((int)sp*x->x_blocksize)+(sf+(int)fgodown));
            *(x->x_idata+((int)sp*x->x_blocksize)+sf) =
                *(x->x_idata+((int)sp*x->x_blocksize)+(sf+(int)fgodown));
        }
        for (sf=(x->x_blocksize/2)-fgodown; sf<(x->x_blocksize/2); sf++ )
        {
            *(x->x_rdata+((int)sp*x->x_blocksize)+(int)sf) = 0.0;
            *(x->x_idata+((int)sp*x->x_blocksize)+(int)sf) = 0.0;
        }
    }
    sonogram_update_part(x, x->x_glist, samplestart, sampleend, 0, 1, 1);
}

/* swap blocks */
static void sonogram_swapblocks(t_sonogram *x, t_floatarg fperstart, t_floatarg fperend, t_floatarg fpersize)
{
    t_int samplestart, samplestartb, samplesize, sp, sf;
    t_int iperstart, iperend, ipersize;
    t_float s1, s2;
    t_float fvalue;

    iperstart = fperstart;
    iperend = fperend;
    ipersize = fpersize;

    if (iperstart < 0 || iperstart > iperend ||
            iperend <= 0 || iperend+ipersize > 100 ||
            ipersize < 0 || fpersize > 100 )
    {
        post( "sonogram~ : error : wrong interval [%d%%, %d%%] <-> [%d%%, %d%%]",
              iperstart, iperstart+ipersize, iperend, iperend+ipersize );
        return;
    }

    samplestart=(x->x_modstart*(x->x_size-1))/100;
    samplestartb=(x->x_modend*(x->x_size-1))/100;
    samplesize=((samplestartb-samplestart)*ipersize)/100;
    samplestart=samplestart+((samplestartb-samplestart)*iperstart)/100;
    samplestartb=samplestart+((samplestartb-samplestart)*iperend)/100;

    post( "swap blocks [%d,%d] and [%d,%d]", samplestart, samplestart+samplesize, samplestartb, samplestartb+samplesize );

    for ( sp=samplesize; sp>=0; sp-- )
    {
        for ( sf=0; sf<x->x_blocksize; sf++)
        {
            fvalue = *(x->x_rdata+((int)(samplestart+sp)*x->x_blocksize)+sf);
            *(x->x_rdata+((int)(samplestart+sp)*x->x_blocksize)+sf) = *(x->x_rdata+((int)(samplestartb+sp)*x->x_blocksize)+sf);
            *(x->x_rdata+((int)(samplestartb+sp)*x->x_blocksize)+sf) = fvalue;
            fvalue = *(x->x_idata+((int)(samplestart+sp)*x->x_blocksize)+sf);
            *(x->x_idata+((int)(samplestart+sp)*x->x_blocksize)+sf) = *(x->x_idata+((int)(samplestartb+sp)*x->x_blocksize)+sf);
            *(x->x_idata+((int)(samplestartb+sp)*x->x_blocksize)+sf) = fvalue;
        }
    }
    sonogram_update_part(x, x->x_glist, 0, x->x_size-1, 0, 1, 1);
}

/* swap frequencies */
static void sonogram_swapfreqs(t_sonogram *x, t_floatarg ffirstfreq, t_floatarg fsecondfreq)
{
    t_int samplestart, sampleend, sp;
    t_float fvalue;

    if (ffirstfreq < 0 || fsecondfreq <0)
    {
        post( "sonogram~ : error : wrong frequencies" );
        return;
    }
    samplestart=(x->x_modstart*(x->x_size-1))/100;
    sampleend=(x->x_modend*(x->x_size-1))/100;

    for ( sp=samplestart; sp<=sampleend; sp++ )
    {
        fvalue = *(x->x_rdata+((int)sp*x->x_blocksize)+(int)ffirstfreq);
        *(x->x_rdata+((int)sp*x->x_blocksize)+(int)ffirstfreq) =
            *(x->x_rdata+((int)sp*x->x_blocksize)+(int)fsecondfreq);
        *(x->x_rdata+((int)sp*x->x_blocksize)+(int)fsecondfreq) = fvalue;
        fvalue = *(x->x_idata+((int)sp*x->x_blocksize)+(int)ffirstfreq);
        *(x->x_idata+((int)sp*x->x_blocksize)+(int)ffirstfreq) =
            *(x->x_idata+((int)sp*x->x_blocksize)+(int)fsecondfreq);
        *(x->x_idata+((int)sp*x->x_blocksize)+(int)fsecondfreq) = fvalue;
    }
    sonogram_update_part(x, x->x_glist, samplestart, sampleend, 0, 1, 1);
}

static void *sonogram_new(t_floatarg fsize, t_floatarg fgraphic, t_floatarg fphaso)
{
    t_sonogram *x = (t_sonogram *)pd_new(sonogram_class);
    outlet_new(&x->x_obj, &s_signal);
    outlet_new(&x->x_obj, &s_signal);
    x->x_recend = outlet_new(&x->x_obj, &s_bang );
    x->x_end = outlet_new(&x->x_obj, &s_bang );
    inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_signal, &s_signal);
    inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_float, gensym("readstart"));
    inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_float, gensym("readend"));
    inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_float, gensym("modstart"));
    inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_float, gensym("modend"));

    if ( fsize <= 0 || ( fgraphic != 0 && fgraphic != 1 ) || ( fphaso != 0 && fphaso != 1 ) )
    {
        error( "sonogram~ : missing or negative creation arguments" );
        return NULL;
    }

    // activate graphical callbacks
    if ( fgraphic != 0 )
    {
        class_setwidget(sonogram_class, &sonogram_widgetbehavior);
    }
    x->x_graphic = (int) fgraphic;
    x->x_phaso = (int) fphaso;

    x->x_size = fsize;
    x->x_blocksize = sys_getblksize();
    x->x_play = 0;
    x->x_readspeed = 1.;
    x->x_record = 0;
    x->x_readpos = 0.;
    x->x_writepos = 0;
    x->x_modstart = 0;
    x->x_readstart = 0;
    x->x_modend = 100;
    x->x_readend = 100;
    x->x_rdata = NULL;
    x->x_idata = NULL;
    x->x_phase = 0.0;
    x->x_empty = 1;
    x->x_xpos = -1;
    x->x_ypos = -1;
    x->x_samplerate = sys_getsr();
    /* graphic data */
    x->x_selected = 0;
    x->x_zoom = 1;
    x->x_updatechild = 0;
    x->x_modstep = 1.1;
    x->x_enhancemode = 0;
    x->x_glist = (t_glist*)canvas_getcurrent();

    if ( sonogram_allocate(x) <0 )
    {
        return NULL;
    }
    else
    {
        return(x);
    }

}

void sonogram_tilde_setup(void)
{
    logpost(NULL, 4, sonogram_version);
    sonogram_class = class_new(gensym("sonogram~"), (t_newmethod)sonogram_new, (t_method)sonogram_free,
                               sizeof(t_sonogram), 0, A_DEFFLOAT, A_DEFFLOAT, A_DEFFLOAT, 0);


    // set callbacks
    sonogram_widgetbehavior.w_getrectfn =    sonogram_getrect;
    sonogram_widgetbehavior.w_displacefn =   sonogram_displace;
    sonogram_widgetbehavior.w_selectfn =     sonogram_select;
    sonogram_widgetbehavior.w_activatefn =   NULL;
    sonogram_widgetbehavior.w_deletefn =     sonogram_delete;
    sonogram_widgetbehavior.w_visfn =        sonogram_vis;
    sonogram_widgetbehavior.w_clickfn =      sonogram_click;


#if PD_MINOR_VERSION >= 37
    class_setpropertiesfn(sonogram_class, NULL);
    class_setsavefn(sonogram_class, sonogram_save);
#else
    sonogram_widgetbehavior.w_propertiesfn = NULL;
    sonogram_widgetbehavior.w_savefn =       sonogram_save;
#endif

    CLASS_MAINSIGNALIN( sonogram_class, t_sonogram, x_f );
    class_addmethod(sonogram_class, (t_method)sonogram_dsp, gensym("dsp"), A_NULL);
    class_addmethod(sonogram_class, (t_method)sonogram_record, gensym("record"), A_NULL);
    class_addmethod(sonogram_class, (t_method)sonogram_enhance, gensym("enhance"), A_FLOAT, A_FLOAT, A_FLOAT, A_FLOAT, A_NULL);
    class_addmethod(sonogram_class, (t_method)sonogram_add, gensym("add"), A_FLOAT, A_FLOAT, A_FLOAT, A_NULL);
    class_addmethod(sonogram_class, (t_method)sonogram_resize, gensym("resize"), A_FLOAT, A_NULL);
    class_addmethod(sonogram_class, (t_method)sonogram_zoom, gensym("zoom"), A_FLOAT, A_NULL);
    class_addmethod(sonogram_class, (t_method)sonogram_swappoints, gensym("swappoints"), A_FLOAT, A_NULL);
    class_addmethod(sonogram_class, (t_method)sonogram_average, gensym("average"), A_FLOAT, A_NULL);
    class_addmethod(sonogram_class, (t_method)sonogram_swapblocks, gensym("swapblocks"), A_FLOAT, A_FLOAT, A_FLOAT, A_NULL);
    class_addmethod(sonogram_class, (t_method)sonogram_swapfreqs, gensym("swapfreqs"), A_FLOAT, A_FLOAT, A_NULL);
    class_addmethod(sonogram_class, (t_method)sonogram_flipfreqs, gensym("flipfreqs"), A_NULL);
    class_addmethod(sonogram_class, (t_method)sonogram_flipblocks, gensym("flipblocks"), A_NULL);
    class_addmethod(sonogram_class, (t_method)sonogram_play, gensym("play"), A_NULL);
    class_addmethod(sonogram_class, (t_method)sonogram_refresh, gensym("refresh"), A_NULL);
    class_addmethod(sonogram_class, (t_method)sonogram_enhancemode, gensym("enhancemode"), A_FLOAT, A_NULL);
    class_addmethod(sonogram_class, (t_method)sonogram_goup, gensym("goup"), A_FLOAT, A_NULL);
    class_addmethod(sonogram_class, (t_method)sonogram_godown, gensym("godown"), A_FLOAT, A_NULL);
    class_addmethod(sonogram_class, (t_method)sonogram_roll, gensym("roll"), A_FLOAT, A_NULL);
    class_addmethod(sonogram_class, (t_method)sonogram_threshold, gensym("threshold"), A_FLOAT, A_NULL);
    class_addmethod(sonogram_class, (t_method)sonogram_phase, gensym("phase"), A_FLOAT, A_NULL);
    class_addmethod(sonogram_class, (t_method)sonogram_zswap, gensym("zswap"), A_NULL);
    class_addmethod(sonogram_class, (t_method)sonogram_modstep, gensym("modstep"), A_FLOAT, A_NULL);
    class_addmethod(sonogram_class, (t_method)sonogram_modstart, gensym("modstart"), A_FLOAT, A_NULL);
    class_addmethod(sonogram_class, (t_method)sonogram_modend, gensym("modend"), A_FLOAT, A_NULL);
    class_addmethod(sonogram_class, (t_method)sonogram_readstart, gensym("readstart"), A_FLOAT, A_NULL);
    class_addmethod(sonogram_class, (t_method)sonogram_readend, gensym("readend"), A_FLOAT, A_NULL);
    class_addmethod(sonogram_class, (t_method)sonogram_readspeed, gensym("readspeed"), A_FLOAT, A_NULL);
    class_addmethod(sonogram_class, (t_method)sonogram_undo, gensym("undo"), A_NULL);
}
