/*------------------------ cooled~ -------------------------------------------- */
/*                                                                              */
/* cooled~ : display sound, play parts and modify it with cut and paste         */
/* constructor : cooled~ | cooled~ <size> <width> <height>                      */
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
/* "I am the fly in the ointment" -- Wire + 154                                 */
/* Komet -- Instrumentals                                                       */
/* ---------------------------------------------------------------------------- */



#include <sys/types.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <ctype.h>
#include <pthread.h>
#include <math.h>

#ifdef __APPLE__
# include <sys/malloc.h>
#else
# include <malloc.h>
#endif

#ifdef _WIN32
# define M_PI 3.14159265358979323846
# include <windows.h>
#endif

#ifndef _MSC_VER
# include <unistd.h>
#endif

#include "m_pd.h"
#include "m_imp.h"
#include "g_canvas.h"


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

#define SYS_VGUI10(a,b,c,d,e,f,g,h,i,j) if (guidebug) \
                         post(a,b,c,d,e,f,g,h,i,j );\
                         sys_vgui(a,b,c,d,e,f,g,h,i,j)

#define SYS_VGUI11(a,b,c,d,e,f,g,h,i,j,k) if (guidebug) \
                         post(a,b,c,d,e,f,g,h,i,j,k );\
                         sys_vgui(a,b,c,d,e,f,g,h,i,j,k)

#define THREAD_SLEEP_TIME 10000   // 10000 us = 10 ms
#define COOLED_BGCOLOR "#000000"
#define COOLED_FGCOLOR "#34E112"
#define COOLED_FRCOLOR "#FFFFFF"

#define COOLED_DEFAULT_SIZE 1024
#define COOLED_DEFAULT_WIDTH 400
#define COOLED_DEFAULT_HEIGHT 200

static char   *cooled_version = "cooled~: version 0.13, written by Yves Degoyon (ydegoyon@free.fr)";

static t_class *cooled_class;
t_widgetbehavior cooled_widgetbehavior;

typedef struct _cooled
{
    t_object x_obj;

    int x_size;                  /* size of the stored sound                                  */
    t_float x_readpos;             /* data's playing position                                   */
    int x_writepos;              /* data's recording position                                 */
    t_float x_readstart;           /* data's starting position for reading                      */
    t_float x_readend;             /* data's ending position for reading                        */
    int x_play;                  /* playing on/off flag                                       */
    t_float x_readspeed;           /* speed increment                                           */
    t_float x_record;              /* flag to start recording process                           */
    t_float x_allocate;            /* flag to indicate pending allocation                       */
    t_float x_empty;               /* flag to indicate it's a brand new sound                   */
    t_float *x_rdata;              /* table containing right channel samples                    */
    t_float *x_ldata;              /* table containing leftt channel samples                    */
    t_outlet *x_end;               /* outlet for end of restitution                             */
    t_outlet *x_recend;            /* outlet for end of recording                               */
    t_outlet *x_sampstart;         /* outlet for sample number [ start ] when selecting         */
    t_outlet *x_sampend;           /* outlet for sample number [ end ] when selecting           */
    t_float *x_rsemp;              /* temporary sample buffer ( right )                         */
    t_float *x_lsemp;              /* temporary sample buffer ( left )                          */
    char*   x_gifdata;             /* buffer to store graphic data                              */
    char*   x_guicommand;          /* buffer to store gui command                               */
    int   x_draw;                /* drawing option                                            */

    /* graphical data block */
    int x_width;                 /* graphical width                             */
    int x_height;                /* graphical height                            */
    int x_selected;              /* flag to remember if we are seleted or not   */
    int x_erase;                 /* flag used when an erase is needed           */
    int x_redraw;                /* flag used when drawing  is needed           */
    t_glist *x_glist;              /* keep graphic context for various operations */
    int x_zoom;                    /* zoom factor                                 */
    pthread_t x_updatechild;       /* thread id for the update child              */
    int x_updatestart;             /* starting position for update                */
    int x_updateend;               /* ending position for update                  */
    int x_xpos;                    /* stuck x position                            */
    int x_ypos;                    /* stuck y position                            */
    int x_shifted;                 /* remember shift state from last click        */
    int x_alted;                   /* remember alt state from last click          */
    int x_xdraw;                   /* x drawing position                          */
    int x_edraw;                   /* end of drawing                              */

    t_float x_f;                   /* float needed for signal input */

} t_cooled;

/* ------------------------ drawing functions ---------------------------- */

static void cooled_update_block(t_cooled *x, t_glist *glist, int bnumber)
{
    int hi, i=0;
    t_float fspectrum=0.0;
    int phase=0;
    char color[8];

    memset( x->x_gifdata, 0x0, x->x_height*x->x_zoom*sizeof("#FFFFFF ") );

    // update cooled~
    for ( hi=x->x_height-1; hi>=0; hi-- )
    {
        if ( ( hi == x->x_height/4) || ( hi == 3*x->x_height/4) )
        {
            sprintf( color, "%s ", COOLED_FGCOLOR );
        }
        else if ( hi == x->x_height/2)
        {
            sprintf( color, "%s ", COOLED_FRCOLOR );
        }
        else
        {
            sprintf( color, "%s ", COOLED_BGCOLOR );
        }
        for ( i=0; i<x->x_zoom; i++ )
        {
            strncpy( x->x_gifdata+(hi*x->x_zoom+i)*8, color, 8 );
        }
    }

    // set all points
    {
        int fsamp = ( bnumber * x->x_size ) / x->x_width;
        int lsamp = ( ( bnumber+1) * x->x_size ) / x->x_width;
        int si;

        // post ( "cooled~ : updating samples [%d,%d]", fsamp, lsamp );

        for ( si=fsamp; si<lsamp; si++ )
        {
            // calculate right channel index
            {
                int rind =  3*x->x_height/4 + ( *(x->x_rdata+si) * (x->x_height/4) ) - 1;

                if ( rind > x->x_height - 1 ) rind = x->x_height - 1;
                if ( rind < x->x_height/2 ) rind = x->x_height/2;

                sprintf( color, "%s ", COOLED_FGCOLOR );
                for ( i=0; i<x->x_zoom; i++ )
                {
                    strncpy( x->x_gifdata+(rind*x->x_zoom+i)*8, color, 8 );
                }
            }

            // calculate left channel index
            {
                int lind =  x->x_height/4 + ( *(x->x_ldata+si) * (x->x_height/4) ) - 1;

                if ( lind > x->x_height/2 - 1 ) lind = x->x_height/2 - 1;
                if ( lind < 0 ) lind = 0;

                sprintf( color, "%s ", COOLED_FGCOLOR );
                for ( i=0; i<x->x_zoom; i++ )
                {
                    strncpy( x->x_gifdata+(lind*x->x_zoom+i)*8, color, 8 );
                }
            }
        }
    }

    for ( i=0; i<x->x_zoom; i++ )
    {
        sprintf( x->x_guicommand, "COOLEDIMAGE%x put {%s} -to %d 0\n", x, x->x_gifdata, (bnumber*x->x_zoom)+i );
        if ( glist_isvisible( x->x_glist ) )
            sys_gui( x->x_guicommand );
    }

}

static void cooled_erase_block(t_cooled *x, t_glist *glist, int sample )
{
    t_canvas *canvas=glist_getcanvas(glist);
    int hi;
    t_float fspectrum=0.0;
    char fillColor[ 16 ];

    for ( hi=0; hi<x->x_height; hi++)
    {
        {
            int i;

            for ( i=0; i<x->x_zoom; i++ )
            {
                strcpy( x->x_gifdata+i*sizeof("#FFFFFF "), strcat( COOLED_BGCOLOR, " ") );
            }
            if ( glist_isvisible( x->x_glist ) )
                SYS_VGUI5("COOLEDIMAGE%x put {%s} -to %d %d\n", x, x->x_gifdata,
                          sample*x->x_zoom, (x->x_height-hi)*x->x_zoom );
        }
    }
}

static void *cooled_do_update_part(void *tdata)
{
    t_cooled *x = (t_cooled*) tdata;
    t_canvas *canvas=glist_getcanvas(x->x_glist);
    int si;
    int nbpoints = 0;
    t_float percentage = 0, opercentage = 0;

    // loose synchro
    usleep( THREAD_SLEEP_TIME );

    // check bounds
    if ( x->x_updateend > x->x_size-1 ) x->x_updateend = x->x_size-1;
    if ( x->x_updatestart < 0 ) x->x_updatestart = 0;

    post("cooled~ : ok, let's go [updating %d, %d]", x->x_updatestart, x->x_updateend );

    if ( x->x_erase )
    {
        for ( si=x->x_updatestart; si<=x->x_updateend; si++ )
        {
            cooled_erase_block(x, x->x_glist, si);
            nbpoints++;
            percentage = (nbpoints*100/(x->x_updateend-x->x_updatestart+1));
            if ( (percentage == (int) percentage) && ((int)percentage%5 == 0) && ( percentage != opercentage ) )
            {
                // post( "cooled~ : erase part : %d %% completed", (int)percentage );
                opercentage = percentage;
            }
        }
    }

    percentage = opercentage = nbpoints = 0;

    if ( x->x_redraw )
    {
        for ( si=x->x_updatestart; si<=x->x_updateend; si++ )
        {
            cooled_update_block(x, x->x_glist, si);
            nbpoints++;
            percentage = (nbpoints*100/(x->x_updateend-x->x_updatestart+1));
            if ( (percentage == (int) percentage) && ((int)percentage%5 == 0) && ( percentage != opercentage ) )
            {
                // post( "cooled~ : update part : %d %% completed", (int)percentage );
                opercentage = percentage;
            }
        }
    }

    if ( glist_isvisible( x->x_glist ) )
    {
        SYS_VGUI3( ".x%lx.c delete rectangle %xCLIPZONE\n",
                   canvas, x);
        if ( ( x->x_readstart != 0 ) || ( x->x_readend != 100 ) )
        {
            SYS_VGUI7( ".x%lx.c create rectangle %d %d %d %d -outline #FF0000 -tags %xCLIPZONE -width 2\n",
                       canvas, x->x_xpos+(int)(x->x_readstart*(x->x_width)/100 ),
                       x->x_ypos, x->x_xpos+(int)(x->x_readend*(x->x_width)/100 ),
                       x->x_ypos+x->x_height*x->x_zoom, x );
        }
        // set borders in black
        SYS_VGUI3(".x%lx.c itemconfigure %xCOOLED -outline #000000\n", canvas, x);
    }

#ifndef _WIN32
    post("cooled~ : child thread %d ended", (int)x->x_updatechild );
    x->x_updatechild = NULL;
#endif
    return NULL;
}

static void cooled_update_part(t_cooled *x, t_glist *glist, int bstart, int bend,
                               int erase, int redraw, int keepframe)
{
    pthread_attr_t update_child_attr;
    t_canvas *canvas=glist_getcanvas(x->x_glist);

#ifndef _WIN32
    if ( x->x_updatechild != 0 )
    {
        // post( "cooled~ : error : no update is possible for now" );
        return;
    }
#endif
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
    // recreate the square if needed
    if ( glist_isvisible( x->x_glist ) )
    {
        if ( ( bstart == 0 ) && ( bend == x->x_width-1 ) && !keepframe )
        {
            SYS_VGUI3(".x%lx.c delete %xCOOLEDL\n", canvas, x );
            SYS_VGUI3(".x%lx.c delete %xCOOLEDR\n", canvas, x );
            SYS_VGUI7(".x%lx.c create rectangle %d %d %d %d -fill #FFFFFF -tags %xCOOLEDR\n",
                      canvas, x->x_xpos, x->x_ypos,
                      x->x_xpos + x->x_width*x->x_zoom,
                      x->x_ypos + x->x_height/2*x->x_zoom,
                      x);
            SYS_VGUI7(".x%lx.c create rectangle %d %d %d %d -fill #FFFFFF -tags %xCOOLEDL\n",
                      canvas, x->x_xpos,
                      x->x_ypos + x->x_height/2*x->x_zoom,
                      x->x_xpos + x->x_width*x->x_zoom,
                      x->x_ypos + x->x_height*x->x_zoom,
                      x);
            SYS_VGUI2("image delete COOLEDIMAGE%x\n", x );
            SYS_VGUI3(".x%lx.c delete ICOOLEDIMAGE%x\n", canvas, x );
            SYS_VGUI4("image create photo COOLEDIMAGE%x -format gif -width %d -height %d\n",
                      x, x->x_width*x->x_zoom, x->x_height*x->x_zoom );
            SYS_VGUI2("COOLEDIMAGE%x blank\n", x );
            SYS_VGUI6(".x%lx.c create image %d %d -image COOLEDIMAGE%x -tags ICOOLEDIMAGE%x\n",
                      canvas,
                      x->x_xpos+(x->x_width*x->x_zoom)/2,
                      x->x_ypos+(x->x_height*x->x_zoom)/2, x, x );
            canvas_fixlinesfor( canvas, (t_text*)x );
        }
        // set borders in red
        SYS_VGUI3(".x%lx.c itemconfigure %xCOOLED -outline #FF0000\n", canvas, x);
    }

    // launch update thread
    if ( pthread_attr_init( &update_child_attr ) < 0 )
    {
        post( "cooled~ : could not launch update thread" );
        perror( "pthread_attr_init" );
        return;
    }
    if ( pthread_attr_setdetachstate( &update_child_attr, PTHREAD_CREATE_DETACHED ) < 0 )
    {
        post( "cooled~ : could not launch update thread" );
        perror( "pthread_attr_setdetachstate" );
        return;
    }
    if ( pthread_create( &x->x_updatechild, &update_child_attr, cooled_do_update_part, x ) < 0 )
    {
        post( "cooled~ : could not launch update thread" );
        perror( "pthread_create" );
        return;
    }
    else
    {
        // post( "cooled~ : drawing thread %d launched", (int)x->x_updatechild );
    }
}

static void cooled_draw_new(t_cooled *x, t_glist *glist)
{
    t_canvas *canvas=glist_getcanvas(glist);

    x->x_xpos=text_xpix(&x->x_obj, glist);
    x->x_ypos=text_ypix(&x->x_obj, glist);
    x->x_xdraw=text_xpix(&x->x_obj, glist);
    SYS_VGUI7(".x%lx.c create rectangle %d %d %d %d -fill #FFFFFF -tags %xCOOLEDR\n",
              canvas, x->x_xpos, x->x_ypos,
              x->x_xpos + x->x_width*x->x_zoom,
              x->x_ypos + x->x_height/2*x->x_zoom,
              x);
    SYS_VGUI7(".x%lx.c create rectangle %d %d %d %d -fill #FFFFFF -tags %xCOOLEDL\n",
              canvas, x->x_xpos,
              x->x_ypos + x->x_height/2*x->x_zoom,
              x->x_xpos + x->x_width*x->x_zoom,
              x->x_ypos + x->x_height*x->x_zoom,
              x);
    SYS_VGUI4("image create photo COOLEDIMAGE%x -format gif -width %d -height %d\n",
              x, x->x_width*x->x_zoom, x->x_height*x->x_zoom );
    SYS_VGUI2("COOLEDIMAGE%x blank\n", x );
    SYS_VGUI6(".x%lx.c create image %d %d -image COOLEDIMAGE%x -tags ICOOLEDIMAGE%x\n",
              canvas,
              x->x_xpos+(x->x_width*x->x_zoom)/2,
              x->x_ypos+(x->x_height*x->x_zoom)/2, x, x );
    if ( x->x_draw) cooled_update_part(x, x->x_glist, 0, x->x_width-1, 0, 1, 0);
    canvas_fixlinesfor( canvas, (t_text*)x );
}

static void cooled_draw_delete(t_cooled *x, t_glist *glist)
{
    t_canvas *canvas=glist_getcanvas(glist);

    if ( glist_isvisible( glist ) )
    {
        SYS_VGUI3( ".x%lx.c delete %xCAPTURE\n", canvas, x );
        if ( ( x->x_readstart != 0 ) || ( x->x_readend != 100 ) )
        {
            SYS_VGUI3( ".x%lx.c delete rectangle %xCLIPZONE\n", canvas, x);
        }
        SYS_VGUI3( ".x%lx.c delete line %xINSERTHERE\n", canvas, x);
        SYS_VGUI3(".x%lx.c delete %xCOOLEDR\n", canvas, x );
        SYS_VGUI3(".x%lx.c delete %xCOOLEDL\n", canvas, x );
        SYS_VGUI3(".x%lx.c delete ICOOLEDIMAGE%x\n", canvas, x );
        SYS_VGUI2("image delete COOLEDIMAGE%x\n", x );
    }
}

static void cooled_draw_move(t_cooled *x, t_glist *glist)
{
    t_canvas *canvas=glist_getcanvas(glist);

    if ( glist_isvisible( x->x_glist ) )
    {
        SYS_VGUI7(".x%lx.c coords %xCOOLEDR %d %d %d %d\n",
                  canvas, x,
                  x->x_xpos, x->x_ypos,
                  x->x_xpos + x->x_width*x->x_zoom,
                  x->x_ypos + x->x_height/2*x->x_zoom);
        SYS_VGUI7(".x%lx.c coords %xCOOLEDL %d %d %d %d\n",
                  canvas, x,
                  x->x_xpos,
                  x->x_ypos + x->x_height/2*x->x_zoom,
                  x->x_xpos + x->x_width*x->x_zoom,
                  x->x_ypos + x->x_height*x->x_zoom );
        if ( ( x->x_readstart != 0 ) || ( x->x_readend != 100 ) )
        {
            SYS_VGUI7(".x%lx.c coords %xCLIPZONE %d %d %d %d\n",
                      canvas, x,
                      x->x_xpos+(int)(x->x_readstart*(x->x_width)/100*x->x_zoom ),
                      x->x_ypos,
                      x->x_xpos+(int)(x->x_readend*(x->x_width)/100*x->x_zoom ),
                      x->x_ypos+x->x_height*x->x_zoom );
        }
        SYS_VGUI7(".x%lx.c coords %xINSERTHERE %d %d %d %d\n",
                  canvas, x,
                  x->x_xdraw,
                  x->x_ypos,
                  x->x_xdraw,
                  x->x_ypos+x->x_height*x->x_zoom );
        SYS_VGUI5(".x%lx.c coords ICOOLEDIMAGE%x %d %d\n",
                  canvas, x,
                  x->x_xpos+((x->x_width*x->x_zoom)/2),
                  (x->x_ypos+((x->x_height*x->x_zoom)/2)) );
        canvas_fixlinesfor( canvas, (t_text*)x );
    }
}

static void cooled_draw_select(t_cooled* x,t_glist* glist)
{
    t_canvas *canvas=glist_getcanvas(glist);

    if ( glist_isvisible( x->x_glist ) )
    {
        if(x->x_selected)
        {
            /* sets the item in blue */
            SYS_VGUI3(".x%lx.c itemconfigure %xCOOLEDR -outline #0000FF\n", canvas, x);
            SYS_VGUI3(".x%lx.c itemconfigure %xCOOLEDL -outline #0000FF\n", canvas, x);
        }
        else
        {
            SYS_VGUI3(".x%lx.c itemconfigure %xCOOLEDR -outline #000000\n", canvas, x);
            SYS_VGUI3(".x%lx.c itemconfigure %xCOOLEDL -outline #000000\n", canvas, x);
        }
    }
}

/* ------------------------ widget callbacks ----------------------------- */


/* setting the starting point for reading ( in percent ) */
static void cooled_readstart(t_cooled *x, t_floatarg fstart)
{
    t_float startpoint = fstart;
    t_canvas *canvas=glist_getcanvas(x->x_glist);

    if (startpoint < 0) startpoint = 0;
    if (startpoint > 100) startpoint = 100;
    x->x_readstart = startpoint;
    // set readspeed sign
    if ( ( x->x_readstart > x->x_readend ) && ( x->x_readspeed > 0 ) ) x->x_readspeed = -x->x_readspeed;
    if ( ( x->x_readstart < x->x_readend ) && ( x->x_readspeed < 0 ) ) x->x_readspeed = -x->x_readspeed;
    if ( glist_isvisible( x->x_glist ) )
    {
        SYS_VGUI3( ".x%lx.c delete rectangle %xCLIPZONE\n",
                   canvas, x);
        if ( ( x->x_readstart != 0 ) || ( x->x_readend != 100 ) )
        {
            SYS_VGUI7( ".x%lx.c create rectangle %d %d %d %d -outline #FF0000 -tags %xCLIPZONE -width 2\n",
                       canvas, x->x_xpos+(int)(x->x_readstart*x->x_width*x->x_zoom/100 ),
                       x->x_ypos, x->x_xpos+(int)(x->x_readend*x->x_width*x->x_zoom/100 ),
                       x->x_ypos+x->x_height*x->x_zoom, x );
        }
    }
    outlet_float( x->x_sampstart, (x->x_readstart*x->x_size)/100 );
}

/* setting the ending point for reading ( in percent ) */
static void cooled_readend(t_cooled *x, t_floatarg fend)
{
    t_float endpoint = fend;
    t_canvas *canvas=glist_getcanvas(x->x_glist);

    if (endpoint < 0) endpoint = 0;
    if (endpoint > 100) endpoint = 100;
    x->x_readend = endpoint;
    // set readspeed sign
    if ( ( x->x_readstart > x->x_readend ) && ( x->x_readspeed > 0 ) ) x->x_readspeed = -x->x_readspeed;
    if ( ( x->x_readstart < x->x_readend ) && ( x->x_readspeed < 0 ) ) x->x_readspeed = -x->x_readspeed;
    if ( glist_isvisible( x->x_glist ) )
    {
        SYS_VGUI3( ".x%lx.c delete rectangle %xCLIPZONE\n",
                   canvas, x);
        if ( ( x->x_readstart != 0 ) || ( x->x_readend != 100 ) )
        {
            SYS_VGUI7( ".x%lx.c create rectangle %d %d %d %d -outline #FF0000 -tags %xCLIPZONE -width 2\n",
                       canvas,
                       x->x_xpos+(int)(x->x_readstart*x->x_width*x->x_zoom/100 ),
                       x->x_ypos, x->x_xpos+(int)(x->x_readend*x->x_width*x->x_zoom/100 ),
                       x->x_ypos+x->x_height*x->x_zoom, x );
        }
    }
    outlet_float( x->x_sampend, (x->x_readend*x->x_size)/100 );
}

static void cooled_getrect(t_gobj *z, t_glist *owner,
                           int *xp1, int *yp1, int *xp2, int *yp2)
{
    t_cooled* x = (t_cooled*)z;

    *xp1 = x->x_xpos;
    *yp1 = x->x_ypos;
    *xp2 = x->x_xpos+x->x_width*x->x_zoom;
    *yp2 = x->x_ypos+x->x_height*x->x_zoom+1;
}

static void cooled_save(t_gobj *z, t_binbuf *b)
{
    t_cooled *x = (t_cooled *)z;

    binbuf_addv(b, "ssiisiiii", gensym("#X"),gensym("obj"),
                (int)x->x_obj.te_xpix, (int)x->x_obj.te_ypix,
                atom_getsymbol(binbuf_getvec(x->x_obj.te_binbuf)),
                x->x_size, x->x_width, x->x_height, x->x_draw );
    binbuf_addv(b, ";");
}

static void cooled_select(t_gobj *z, t_glist *glist, int selected)
{
    t_cooled *x = (t_cooled *)z;

    x->x_selected = selected;
    cooled_draw_select( x, glist );
}

static void cooled_vis(t_gobj *z, t_glist *glist, int vis)
{
    t_cooled *x = (t_cooled *)z;
    t_rtext *y;

    if (vis)
    {
        cooled_draw_new( x, glist );
    }
    else
    {
        // erase all points
        cooled_draw_delete( x, glist );
    }
}

static void cooled_delete(t_gobj *z, t_glist *glist)
{
    canvas_deletelinesfor(glist, (t_text *)z);
}

static void cooled_displace(t_gobj *z, t_glist *glist, int dx, int dy)
{
    t_cooled *x = (t_cooled *)z;
    int xold = x->x_xpos;
    int yold = x->x_ypos;

    x->x_xpos += dx;
    x->x_ypos += dy;

    if ( ( x->x_xpos != xold ) || ( x->x_ypos != yold ) )
    {
        cooled_draw_move( x, glist );
    }

}

static void cooled_motion(t_cooled *x, t_floatarg dx, t_floatarg dy)
{
    // post( "cooled_motion @ [%d,%d] dx=%f dy=%f alt=%d", x->x_xdraw, x->x_ydraw, dx, dy, x->x_alted );
    if ( dx != 0 )
    {
        cooled_readstart( x, ((t_float)( x->x_xdraw - x->x_xpos ) * 100 )/ (t_float)( x->x_width * x->x_zoom ) );
        x->x_edraw += dx;
        cooled_readend( x, ((t_float)( x->x_edraw - x->x_xpos ) * 100 )/ (t_float)( x->x_width * x->x_zoom )  );
    }
}

/* erase data form readstart to readend */
static void cooled_erase( t_cooled *x )
{
    int startsamp, endsamp, si;
    int lreadstart = x->x_readstart, lreadend = x->x_readend;

    if (x->x_allocate)
    {
        post( "cooled~ : error : cannot erase while re-allocation" );
        return;
    }
    // post( "cooled~ : erase" );
    if ( lreadstart <= lreadend )
    {
        startsamp = (lreadstart*x->x_size)/100;
        endsamp = (lreadend*x->x_size)/100;
    }
    else
    {
        startsamp = (lreadend*x->x_size)/100;
        endsamp = (lreadstart*x->x_size)/100;
    }

    for ( si=startsamp; si<=endsamp; si++ )
    {
        *(x->x_rdata+si) = 0.;
        *(x->x_ldata+si) = 0.;
    }
    if ( x->x_draw ) cooled_update_part(x, x->x_glist, startsamp*x->x_width/x->x_size, endsamp*(x->x_width-1)/x->x_size, 0, 1, 1);
}
/* paste data form readstart to readend */
static void cooled_paste( t_cooled *x )
{
    int startsamp, endsamp, si, inssamp, hlimit, csize;
    int lreadstart = x->x_readstart, lreadend = x->x_readend;

    if (x->x_allocate)
    {
        post( "cooled~ : error : cannot paste while re-allocation" );
        return;
    }
    if ( lreadstart <= lreadend )
    {
        startsamp = (lreadstart*x->x_size)/100;
        endsamp = (lreadend*x->x_size)/100;
    }
    else
    {
        startsamp = (lreadend*x->x_size)/100;
        endsamp = (lreadstart*x->x_size)/100;
    }

    // insert data at insertion point
    inssamp = ( x->x_xdraw - x->x_xpos) * x->x_size / ( x->x_width * x->x_zoom );
    // post( "cooled~ : replace [%d,%d] to %d", startsamp, endsamp, inssamp );
    csize=endsamp-startsamp;
    for ( si=0; si<=csize; si++ )
    {
        if ( ( si + inssamp >= x->x_size ) || ( startsamp + si >= x->x_size ) ) break;
        *(x->x_rsemp+si) = *(x->x_rdata+startsamp+si);
        *(x->x_lsemp+si) = *(x->x_ldata+startsamp+si);
    }
    hlimit = si;
    for ( si=0 ; si<=hlimit; si++ )
    {
        *(x->x_rdata+inssamp+si) += *(x->x_rsemp+si);
        *(x->x_ldata+inssamp+si) += *(x->x_lsemp+si);
    }

    // post( "cooled~ : updating [%d,%d]", inssamp*x->x_width/x->x_size, (inssamp+hlimit)*x->x_width/x->x_size );
    if ( x->x_draw ) cooled_update_part(x, x->x_glist, inssamp*x->x_width/x->x_size, (inssamp+hlimit)*(x->x_width-1)/x->x_size, 0, 1, 1);
}
/* replace data form readstart to readend */
static void cooled_replace( t_cooled *x )
{
    int startsamp, endsamp, si, inssamp, hlimit, csize;
    int lreadstart = x->x_readstart, lreadend = x->x_readend;

    if (x->x_allocate)
    {
        post( "cooled~ : error : cannot replace while re-allocation" );
        return;
    }
    if ( lreadstart <= lreadend )
    {
        startsamp = (lreadstart*x->x_size)/100;
        endsamp = (lreadend*x->x_size)/100;
    }
    else
    {
        startsamp = (lreadend*x->x_size)/100;
        endsamp = (lreadstart*x->x_size)/100;
    }

    // insert data at insertion point
    inssamp = ( x->x_xdraw - x->x_xpos) * x->x_size / ( x->x_width * x->x_zoom );
    // post( "cooled~ : replace [%d,%d] to %d", startsamp, endsamp, inssamp );
    csize=endsamp-startsamp;
    for ( si=0; si<=csize; si++ )
    {
        if ( ( si + inssamp >= x->x_size ) || ( startsamp + si >= x->x_size ) ) break;
        *(x->x_rsemp+si) = *(x->x_rdata+startsamp+si);
        *(x->x_lsemp+si) = *(x->x_ldata+startsamp+si);
    }
    hlimit = si;
    for ( si=0 ; si<=hlimit; si++ )
    {
        *(x->x_rdata+inssamp+si) = *(x->x_rsemp+si);
        *(x->x_ldata+inssamp+si) = *(x->x_lsemp+si);
    }

    // post( "cooled~ : updating [%d,%d]", inssamp*x->x_width/x->x_size, (inssamp+hlimit)*x->x_width/x->x_size );
    if ( x->x_draw ) cooled_update_part(x, x->x_glist, inssamp*x->x_width/x->x_size, (inssamp+hlimit)*(x->x_width-1)/x->x_size, 0, 1, 1);

}

/* call editor's property dialog */
static void cooled_properties(t_gobj *z, t_glist *owner)
{
    char buf[800];
    t_cooled *x=(t_cooled *)z;

    sprintf(buf, "pdtk_cooled_dialog %%s %d %d %d\n",
            x->x_width, x->x_height, x->x_draw);
    // post("cooled_properties : %s", buf );
    gfxstub_new(&x->x_obj.ob_pd, x, buf);
}

/* handle properties change */
static void cooled_dialog(t_cooled *x, t_symbol *s, int argc, t_atom *argv)
{
    if ( !x )
    {
        post( "cooled~ : error :tried to set properties on an unexisting object" );
    }
    if ( argc != 3 )
    {
        post( "cooled~ : error in the number of arguments ( %d instead of 3 )", argc );
        return;
    }
    if ( argv[0].a_type != A_FLOAT || argv[1].a_type != A_FLOAT || argv[2].a_type != A_FLOAT )
    {
        post( "cooled~ : wrong arguments" );
        return;
    }
    cooled_draw_delete(x, x->x_glist);
    if ( x->x_gifdata != NULL )
    {
        freebytes(x->x_gifdata, x->x_height*x->x_zoom*sizeof("#FFFFFF ") );
        post( "Freed %d bytes", x->x_height*x->x_zoom*sizeof("#FFFFFF ") );
        x->x_gifdata = NULL;
    }
    if ( x->x_guicommand != NULL )
    {
        freebytes(x->x_guicommand, 128+x->x_height*x->x_zoom*sizeof("#FFFFFF ") );
        post( "Freed %d bytes", 128+x->x_height*x->x_zoom*sizeof("#FFFFFF ") );
        x->x_guicommand = NULL;
    }
    x->x_width = (int)argv[0].a_w.w_float;
    if ( x->x_width < 0 ) x->x_width = 100;
    x->x_height = (int)argv[1].a_w.w_float;
    if ( x->x_height < 0 ) x->x_height = 100;
    x->x_draw = (int)argv[2].a_w.w_float;
    if ( !(x->x_gifdata = ( char* ) getbytes( x->x_height*x->x_zoom*sizeof("#FFFFFF ") ) ) )
    {
        post( "cooled~ : error : could not allocate buffers" );
        return;
    }
    else
    {
        post( "cooled~ : allocated %d bytes", x->x_height*x->x_zoom*sizeof("#FFFFFF ") );
    }
    if ( !(x->x_guicommand = ( char* ) getbytes( 128+x->x_height*x->x_zoom*sizeof("#FFFFFF ") ) ) )
    {
        post( "cooled~ : error : could not allocate buffers" );
        return;
    }
    else
    {
        post( "cooled~ : allocated %d bytes", 128+x->x_height*x->x_zoom*sizeof("#FFFFFF ") );
    }
    cooled_draw_new(x, x->x_glist);
}

/* handle clicks */
static int cooled_click(t_gobj *z, struct _glist *glist,
                        int xpix, int ypix, int shift, int alt, int dbl, int doit)
{
    t_cooled* x = (t_cooled *)z;
    int pipos;
    t_canvas *canvas=glist_getcanvas(x->x_glist);

    // post( "cooled_click : x=%d y=%d doit=%d alt=%d, shift=%d", xpix, ypix, doit, alt, shift );
    if ( doit )
    {
        if ( shift && alt )
        {
            cooled_paste(x);
        }
        else if ( shift )
        {
            cooled_replace(x);
        }
        else if ( alt )
        {
            cooled_erase(x);
        }
        else
        {
            x->x_xdraw = xpix;
            x->x_edraw = xpix;
            x->x_shifted = shift;
            x->x_alted = alt;
            // activate motion callback
            glist_grab( glist, &x->x_obj.te_g, (t_glistmotionfn)cooled_motion,
                        0, xpix, ypix );

            // draw insertion line
            if ( glist_isvisible( x->x_glist ) )
            {
                SYS_VGUI3( ".x%lx.c delete line %xINSERTHERE\n",
                           canvas, x);
                SYS_VGUI7( ".x%lx.c create line %d %d %d %d -fill #00FFFF -tags %xINSERTHERE -width 2\n",
                           canvas, x->x_xdraw,
                           x->x_ypos, x->x_xdraw,
                           x->x_ypos+x->x_height*x->x_zoom, x );
            }

        }

    }
    else
    {
        // nothing
    }
    return (1);

}

/* clean up */
static void cooled_free(t_cooled *x)
{
    if ( x->x_rdata != NULL )
    {
        freebytes(x->x_rdata, x->x_size*sizeof(float) );
        post( "Freed %d bytes", x->x_size*sizeof(float) );
        x->x_rdata = NULL;
    }
    if ( x->x_ldata != NULL )
    {
        freebytes(x->x_ldata, x->x_size*sizeof(float) );
        post( "Freed %d bytes", x->x_size*sizeof(float) );
        x->x_ldata = NULL;
    }
    if ( x->x_rsemp != NULL )
    {
        freebytes(x->x_rsemp, x->x_size*sizeof(float) );
        post( "Freed %d bytes", x->x_size*sizeof(float) );
        x->x_rsemp = NULL;
    }
    if ( x->x_lsemp != NULL )
    {
        freebytes(x->x_lsemp, x->x_size*sizeof(float) );
        post( "Freed %d bytes", x->x_size*sizeof(float) );
        x->x_lsemp = NULL;
    }
    if ( x->x_gifdata != NULL )
    {
        freebytes(x->x_gifdata, x->x_height*x->x_zoom*sizeof("#FFFFFF ") );
        post( "Freed %d bytes", x->x_height*x->x_zoom*sizeof("#FFFFFF ") );
        x->x_gifdata = NULL;
    }
    if ( x->x_guicommand != NULL )
    {
        freebytes(x->x_guicommand, 128+x->x_height*x->x_zoom*sizeof("#FFFFFF ") );
        post( "Freed %d bytes", 128+x->x_height*x->x_zoom*sizeof("#FFFFFF ") );
        x->x_guicommand = NULL;
    }
}

/* allocate tables for storing sound and temporary copy */
static int cooled_allocate(t_cooled *x)
{
    int fi;

    if ( !(x->x_rdata = getbytes( x->x_size*sizeof(float) ) ) )
    {
        post( "cooled~ : error : could not allocate buffers" );
        return -1;
    }
    else
    {
        post( "cooled~ : allocated %d bytes", x->x_size*sizeof(float) );
    }
    if ( !(x->x_ldata = getbytes( x->x_size*sizeof(float) ) ) )
    {
        post( "cooled~ : error : could not allocate buffers" );
        return -1;
    }
    else
    {
        post( "cooled~ : allocated %d bytes", x->x_size*sizeof(float) );
    }
    if ( !(x->x_rsemp = getbytes( x->x_size*sizeof(float) ) ) )
    {
        post( "cooled~ : error : could not allocate buffers" );
        return -1;
    }
    else
    {
        post( "cooled~ : allocated %d bytes", x->x_size*sizeof(float) );
    }
    if ( !(x->x_lsemp = getbytes( x->x_size*sizeof(float) ) ) )
    {
        post( "cooled~ : error : could not allocate buffers" );
        return -1;
    }
    else
    {
        post( "cooled~ : allocated %d bytes", x->x_size*sizeof(float) );
    }
    if ( !(x->x_gifdata = ( char* ) getbytes( x->x_height*x->x_zoom*sizeof("#FFFFFF ") ) ) )
    {
        post( "cooled~ : error : could not allocate buffers" );
        return -1;
    }
    else
    {
        post( "cooled~ : allocated %d bytes", x->x_height*x->x_zoom*sizeof("#FFFFFF ") );
    }
    if ( !(x->x_guicommand = ( char* ) getbytes( 128+x->x_height*x->x_zoom*sizeof("#FFFFFF ") ) ) )
    {
        post( "cooled~ : error : could not allocate buffers" );
        return -1;
    }
    else
    {
        post( "cooled~ : allocated %d bytes", 128+x->x_height*x->x_zoom*sizeof("#FFFFFF ") );
    }

    return 0;
}

/* allocate tables for storing sound and temporary copy */
static int cooled_reallocate(t_cooled *x, int ioldsize, int inewsize)
{
    int fi, csize;
    t_float *prdata=x->x_rdata, *pldata=x->x_ldata, *prsemp=x->x_rsemp, *plsemp=x->x_lsemp;

    if ( !(x->x_rdata = getbytes( inewsize*sizeof(float) ) ) )
    {
        post( "cooled~ : error : could not allocate buffers" );
        return -1;
    }
    else
    {
        post( "cooled~ : allocated %d bytes", inewsize*sizeof(float) );
    }
    if ( !(x->x_ldata = getbytes( inewsize*sizeof(float) ) ) )
    {
        post( "cooled~ : error : could not allocate buffers" );
        return -1;
    }
    else
    {
        post( "cooled~ : allocated %d bytes", inewsize*sizeof(float) );
    }
    if ( !(x->x_rsemp = getbytes( inewsize*sizeof(float) ) ) )
    {
        post( "cooled~ : error : could not allocate buffers" );
        return -1;
    }
    else
    {
        post( "cooled~ : allocated %d bytes", inewsize*sizeof(float) );
    }
    if ( !(x->x_lsemp = getbytes( inewsize*sizeof(float) ) ) )
    {
        post( "cooled~ : error : could not allocate buffers" );
        return -1;
    }
    else
    {
        post( "cooled~ : allocated %d bytes", inewsize*sizeof(float) );
    }

    if ( ioldsize <= inewsize )
    {
        csize = ioldsize;
    }
    else
    {
        csize = inewsize;
    }
    memcpy( x->x_rdata, prdata, csize*sizeof(float) );
    memcpy( x->x_ldata, pldata, csize*sizeof(float) );
    memcpy( x->x_rsemp, prsemp, csize*sizeof(float) );
    memcpy( x->x_lsemp, plsemp, csize*sizeof(float) );

    if ( prdata != NULL )
    {
        freebytes(prdata, ioldsize*sizeof(float) );
        post( "Freed %d bytes", ioldsize*sizeof(float) );
    }
    if ( pldata != NULL )
    {
        freebytes(pldata, ioldsize*sizeof(float) );
        post( "Freed %d bytes", ioldsize*sizeof(float) );
    }
    if ( prsemp != NULL )
    {
        freebytes(prsemp, ioldsize*sizeof(float) );
        post( "Freed %d bytes", ioldsize*sizeof(float) );
    }
    if ( plsemp != NULL )
    {
        freebytes(plsemp, ioldsize*sizeof(float) );
        post( "Freed %d bytes", ioldsize*sizeof(float) );
    }

    return 0;
}

/* records and playback the sound  */
static t_int *cooled_perform(t_int *w)
{
    t_float *lin = (t_float *)(w[1]);
    t_float *rin = (t_float *)(w[2]);
    t_float *rout = (t_float *)(w[3]);
    t_float *lout = (t_float *)(w[4]);
    int   is;
    int n = (int)(w[5]);                      /* number of samples */
    int startsamp, endsamp;
    t_cooled *x = (t_cooled *)(w[6]);
    int lreadstart = x->x_readstart, lreadend = x->x_readend;

    if ( lreadstart <= lreadend )
    {
        startsamp = (lreadstart*x->x_size)/100;
        endsamp = (lreadend*x->x_size)/100;
    }
    else
    {
        startsamp = (lreadend*x->x_size)/100;
        endsamp = (lreadstart*x->x_size)/100;
    }

    while (n--)
    {
        // eventually records input
        if ( x->x_record )
        {
            *(x->x_ldata+x->x_writepos)=*(lin);
            *(x->x_rdata+x->x_writepos)=*(rin);
            x->x_writepos++;
            if ( x->x_writepos >= x->x_size )
            {
                x->x_record=0;
                x->x_writepos=0;
                if ( x->x_draw ) cooled_update_part(x, x->x_glist, 0, x->x_width-1, 0, 1, 0);
                outlet_bang(x->x_recend);
                if ( x->x_empty ) x->x_empty = 0;
                // post( "cooled~ : stopped recording" );
            }
        }
        // set outputs
        *rout = 0.0;
        *lout = 0.0;
        if ( x->x_play)
        {
            is=0;
            *lout = *(x->x_ldata+(int)x->x_readpos);
            *rout = *(x->x_rdata+(int)x->x_readpos);
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
        rout++;
        lout++;
        rin++;
        lin++;
    }
    // post( "cooled~ : read : %f:%d : write: %d:%d", x->x_readpos, x->x_play, x->x_writepos, x->x_record );
    return (w+7);
}

static void cooled_dsp(t_cooled *x, t_signal **sp)
{
    dsp_add(cooled_perform, 6, sp[0]->s_vec, sp[1]->s_vec, sp[2]->s_vec, sp[3]->s_vec, sp[0]->s_n, x);
}

/* record the cooled */
static void cooled_record(t_cooled *x)
{
    if (x->x_allocate)
    {
        post( "cooled~ : error : cannot record while re-allocation" );
        return;
    }
    x->x_record=1;
    x->x_writepos=0;
    post( "cooled~ : recording on" );
}

/* map to stereo */
static void cooled_stereo(t_cooled *x)
{
    int si;

    if (x->x_allocate)
    {
        post( "cooled~ : error : cannot map to stereo while re-allocation" );
        return;
    }
    for ( si=0; si<x->x_size; si++ )
    {
        *(x->x_rdata+si) = *(x->x_ldata+si);
    }
    if ( x->x_draw ) cooled_update_part(x, x->x_glist, 0, x->x_width-1, !x->x_empty, !x->x_empty, 0);
}

/* play the cooled */
static void cooled_play(t_cooled *x)
{
    x->x_play=1;
    // post( "cooled~ : playing on" );
}

/* sets the reading speed */
static void cooled_readspeed(t_cooled *x, t_floatarg freadspeed)
{
    x->x_readspeed=freadspeed;
}

/* resize cooled */
static void cooled_resize(t_cooled *x, t_floatarg fnewsize )
{
    if (fnewsize <= 0)
    {
        post( "cooled~ : error : wrong size" );
        return;
    }
    if (x->x_allocate)
    {
        post( "cooled~ : error : cannot resize while re-allocation" );
        return;
    }
#ifndef _WIN32
    if (x->x_updatechild > 0)
    {
        post( "cooled~ : can't resize now, an update is pending." );
        return;
    }
#endif
    post( "cooled~ : reallocating tables" );
    x->x_allocate = 1;
    x->x_play = 0;
    x->x_record = 0;
    cooled_readstart( x, 0);
    cooled_readend( x, 100);
    cooled_reallocate(x, x->x_size, fnewsize);
    x->x_size = fnewsize;
    x->x_empty = 1;
    if ( x->x_readstart <= x->x_readend )
    {
        x->x_readpos = (x->x_readstart*x->x_size)/100;
    }
    else
    {
        x->x_readpos = (x->x_readend*x->x_size)/100;
    }
    // erase all points, as data is zero no drawing is needed
    if ( x->x_draw) cooled_update_part(x, x->x_glist, 0, x->x_width-1, 0, !x->x_empty, 1);
    x->x_allocate = 0;
}

/* set zoom factor */
static void cooled_zoom(t_cooled *x, t_floatarg fzoom )
{
    if (fzoom < 1)
    {
        post( "cooled~ : error : wrong zoom factor" );
        return;
    }
    if (x->x_allocate)
    {
        post( "cooled~ : error : cannot zoom while re-allocation" );
        return;
    }
    if ( x->x_gifdata != NULL )
    {
        freebytes(x->x_gifdata, x->x_height*x->x_zoom*sizeof("#FFFFFF ") );
        post( "Freed %d bytes", x->x_height*x->x_zoom*sizeof("#FFFFFF ") );
        x->x_gifdata = NULL;
    }
    if ( x->x_guicommand != NULL )
    {
        freebytes(x->x_guicommand, 128+x->x_height*x->x_zoom*sizeof("#FFFFFF ") );
        post( "Freed %d bytes", 128+x->x_height*x->x_zoom*sizeof("#FFFFFF ") );
        x->x_guicommand = NULL;
    }
    x->x_zoom = (int)fzoom;
    if ( !(x->x_gifdata = ( char* ) getbytes( x->x_height*x->x_zoom*sizeof("#FFFFFF ") ) ) )
    {
        post( "cooled~ : error : could not allocate buffers" );
        return;
    }
    else
    {
        post( "cooled~ : allocated %d bytes", x->x_height*x->x_zoom*sizeof("#FFFFFF ") );
    }
    if ( !(x->x_guicommand = ( char* ) getbytes( 128+x->x_height*x->x_zoom*sizeof("#FFFFFF ") ) ) )
    {
        post( "cooled~ : error : could not allocate buffers" );
        return;
    }
    else
    {
        post( "cooled~ : allocated %d bytes", 128+x->x_height*x->x_zoom*sizeof("#FFFFFF ") );
    }
    if ( x->x_draw) cooled_update_part(x, x->x_glist, 0, x->x_width-1, !x->x_empty, !x->x_empty, 0);
    canvas_fixlinesfor(x->x_glist, (t_text*)x );
}

/* modify the loop positions */
static void cooled_loop(t_cooled *x, t_symbol *soperator )
{
    char *operator = soperator->s_name;
    int ci;
    t_float fvalue, freadstart = x->x_readstart, freadend = x->x_readend;

    if ( (soperator->s_name[0] != '*')  &&
            (soperator->s_name[0] != '/')  &&
            (soperator->s_name[0] != '>')  &&
            (soperator->s_name[0] != '<')  )
    {
        post( "cooled~ : error : wrong operator argument : should be *|/|>|<" );
        return;
    }
    for ( ci=1; ci<(int)strlen( soperator->s_name ); ci++ )
    {
        if ( !(isdigit(soperator->s_name[ci])||(soperator->s_name[ci]=='.')) )
        {
            post( "cooled~ : error : wrong operation value : %s : should be in a float format", soperator->s_name+1 );
            return;
        }
    }
    if ( sscanf( soperator->s_name+1, "%f", &fvalue ) != 1 )
    {
        post( "cooled~ : error : can't get operation value" );
        return;
    }
    switch( soperator->s_name[0] )
    {
    case '*' :
        freadend = x->x_readstart + fvalue*(x->x_readend-x->x_readstart);
        break;

    case '/' :
        if ( fvalue != 0 )
        {
            freadend = x->x_readstart + (x->x_readend-x->x_readstart)/fvalue;
        };
        break;

    case '>' :
        freadstart = x->x_readstart + fvalue*(x->x_readend-x->x_readstart);
        freadend = x->x_readend + fvalue*(x->x_readend-x->x_readstart);
        break;

    case '<' :
        freadstart = x->x_readstart - fvalue*(x->x_readend-x->x_readstart);
        freadend = x->x_readend - fvalue*(x->x_readend-x->x_readstart);
        break;
    }
    if ( freadstart < 0.0 ) freadstart = 0.0;
    if ( freadend < 0.0 ) freadend = 0.0;
    if ( freadstart > 100.0 ) freadstart = 100.0;
    if ( freadend > 100.0 ) freadend = 100.0;
    cooled_readstart( x, freadstart);
    cooled_readend( x, freadend);
}

/* refresh data    */
static void cooled_refresh(t_cooled *x)
{
    if (x->x_allocate)
    {
        post( "cooled~ : error : cannot refresh while re-allocation" );
        return;
    }
    if ( x->x_draw ) cooled_update_part(x, x->x_glist, 0, x->x_width-1, 0, 1, 1);
}

static void *cooled_new(t_symbol *s, int argc, t_atom *argv)
{
    t_cooled *x = (t_cooled *)pd_new(cooled_class);
    outlet_new(&x->x_obj, &s_signal);
    outlet_new(&x->x_obj, &s_signal);
    x->x_recend = outlet_new(&x->x_obj, &s_bang );
    x->x_end = outlet_new(&x->x_obj, &s_bang );
    x->x_sampstart = outlet_new(&x->x_obj, &s_float );
    x->x_sampend = outlet_new(&x->x_obj, &s_float );
    inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_signal, &s_signal);
    inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_float, gensym("readstart"));
    inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_float, gensym("readend"));

    if ( argc != 0 )
    {
        if ( argc < 3 )
        {
            post( "audience~ : error in the number of arguments ( %d )", argc );
            return NULL;
        }
        if ( argv[0].a_type != A_FLOAT || argv[1].a_type != A_FLOAT ||
                argv[2].a_type != A_FLOAT )
        {
            post( "audience~ : wrong arguments" );
            return NULL;
        }
        x->x_size = (int)argv[0].a_w.w_float;
        if ( x->x_size < 1 ) x->x_size = 1;
        x->x_width = (int)argv[1].a_w.w_float;
        if ( x->x_width < 10 ) x->x_width = 10;
        x->x_height = (int)argv[2].a_w.w_float;
        if ( x->x_height < 10 ) x->x_height = 10;
        if ( argc == 3 )
            x->x_draw = 1;
        else
            x->x_draw = (int)argv[3].a_w.w_float;
    }
    else
    {
        x->x_size = COOLED_DEFAULT_SIZE;
        x->x_width = COOLED_DEFAULT_WIDTH;
        x->x_height = COOLED_DEFAULT_HEIGHT;
        x->x_draw = 1;
    }

    // activate graphical callbacks
    class_setwidget(cooled_class, &cooled_widgetbehavior);

    x->x_play = 0;
    x->x_readspeed = 1.;
    x->x_record = 0;
    x->x_allocate = 0;
    x->x_readpos = 0.;
    x->x_writepos = 0;
    x->x_rdata = NULL;
    x->x_ldata = NULL;
    x->x_empty = 1;
    x->x_xpos = -1;
    x->x_ypos = -1;
    /* graphic data */
    x->x_selected = 0;
    x->x_zoom = 1;
#ifndef _WIN32
    x->x_updatechild = 0;
#endif
    x->x_glist = (t_glist*)canvas_getcurrent();
    // post( "cooled~ : new : readend=%d", x->x_readend );
    cooled_readstart( x, 0);
    cooled_readend( x, 100);

    if ( cooled_allocate(x) <0 )
    {
        return NULL;
    }
    else
    {
        return(x);
    }

}

void cooled_tilde_setup(void)
{
    logpost(NULL, 4, cooled_version);
    cooled_class = class_new(gensym("cooled~"), (t_newmethod)cooled_new, (t_method)cooled_free,
                             sizeof(t_cooled), 0, A_GIMME, 0);


    // set callbacks
    cooled_widgetbehavior.w_getrectfn =    cooled_getrect;
    cooled_widgetbehavior.w_displacefn =   cooled_displace;
    cooled_widgetbehavior.w_selectfn =     cooled_select;
    cooled_widgetbehavior.w_activatefn =   NULL;
    cooled_widgetbehavior.w_deletefn =     cooled_delete;
    cooled_widgetbehavior.w_visfn =        cooled_vis;
    cooled_widgetbehavior.w_clickfn =      cooled_click;

    class_setpropertiesfn(cooled_class, cooled_properties);
    class_setsavefn(cooled_class, cooled_save);

    CLASS_MAINSIGNALIN( cooled_class, t_cooled, x_f );
    class_addmethod(cooled_class, (t_method)cooled_dsp, gensym("dsp"), A_NULL);
    class_addmethod(cooled_class, (t_method)cooled_record, gensym("record"), A_NULL);
    class_addmethod(cooled_class, (t_method)cooled_resize, gensym("resize"), A_FLOAT, A_NULL);
    class_addmethod(cooled_class, (t_method)cooled_zoom, gensym("zoom"), A_FLOAT, A_NULL);
    class_addmethod(cooled_class, (t_method)cooled_play, gensym("play"), A_NULL);
    class_addmethod(cooled_class, (t_method)cooled_refresh, gensym("refresh"), A_NULL);
    class_addmethod(cooled_class, (t_method)cooled_readstart, gensym("readstart"), A_FLOAT, A_NULL);
    class_addmethod(cooled_class, (t_method)cooled_stereo, gensym("stereo"), A_NULL);
    class_addmethod(cooled_class, (t_method)cooled_readend, gensym("readend"), A_FLOAT, A_NULL);
    class_addmethod(cooled_class, (t_method)cooled_readspeed, gensym("readspeed"), A_FLOAT, A_NULL);
    class_addmethod(cooled_class, (t_method)cooled_loop, gensym("loop"), A_SYMBOL, A_NULL);
    class_addmethod(cooled_class, (t_method)cooled_dialog, gensym("dialog"), A_GIMME, 0);

    sys_vgui("eval [read [open {%s/%s.tcl}]]\n",
             cooled_class->c_externdir->s_name, cooled_class->c_name->s_name);
}
