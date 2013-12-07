/* ---------------------- blinkenlights~ -------------------------------------- */
/*                                                                              */
/* Blinkenlights is a BL movies player but also a pixel grid                    */
/* Written by Yves Degoyon (ydegoyon@free.fr).                                  */
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
/* Currently listening :                                                        */
/* Pop Group : We Are All Prostitutes                                           */
/* Culturcide : Bruce                                                           */
/* ---------------------------------------------------------------------------- */

//added functions:
//"static void blinkenlights_findframes(t_blinkenlights *x)"
//this is to remember the frame positions in the .blm file

//"static void blinkenlights_goto(t_blinkenlights* x, t_float frame)"
//with a "goto $1" message, you can stratch the blm film. note that the
//range of $1 is 0 to 1


#include <sys/types.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>
#ifndef __APPLE__
#include <malloc.h>
#endif
#include <ctype.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>

#include "m_pd.h"            /* standard pd stuff */
#include "g_canvas.h"        /* some pd's graphical functions */

static char   *blinkenlights_version = "blinkenlights: a blinkenlights movies player version 0.2 ( bugs @ ydegoyon@free.fr and chun@goto10.org )";

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

#define BL_BACKGROUND_COLOR "#000000"
#define BL_FOREGROUND_COLOR "#00FF00"
#define BL_MAX_LENGTH 1024

static t_class *blinkenlights_class;

typedef struct _blinkenlights
{
    t_object x_obj;
    t_int x_width;       /* number of pixels ( width ) */
    t_int x_height;      /* number of pixels ( height) */
    t_int x_xsize;       /* x size of each pixel         */
    t_int x_ysize;       /* y size of each pixel         */
    char  *x_background; /* color of the background #RRGGBB */
    char  *x_foreground; /* color of the foreground #RRGGBB */
    t_int x_ecanvas;     /* flag that indicates if the canvas has been created */
    t_glist *x_glist;    /* graphic context */
    FILE  *x_filed;      /* file descriptor */
    t_int x_timer;       /* timer read from bl movie */
    t_int *x_frame;      /* frame contents */
    t_clock *x_clock;    /* clock used for reading frames */
    t_outlet *outlet_bang;
    t_int frame_no;
    t_int frame_pos[BL_MAX_LENGTH];
    t_clock *x_clock2;
    t_int x_timer2;
    t_float frame_inc;

} t_blinkenlights;

static void blinkenlights_close(t_blinkenlights *x);

/* clean up */
static void blinkenlights_free(t_blinkenlights *x)
{
    post( "blinkenlights : freeing colors" );
    if ( x->x_background ) freebytes( x->x_background, 8 );
    if ( x->x_foreground ) freebytes( x->x_foreground, 8 );
    post( "blinkenlights : closing file" );
    blinkenlights_close(x);
    post( "blinkenlights : cancelling clock" );
    if ( x->x_clock != NULL )
    {
        clock_unset( x->x_clock );
        clock_free( x->x_clock );
    }
    if ( x->x_clock2 != NULL )
    {
        clock_unset( x->x_clock2 );
        clock_free( x->x_clock2 );
    }
    post( "blinkenlights : done" );
}

static void *blinkenlights_new(t_float fwidth, t_float fheight, t_float fxpixsize, t_float fypixsize )
{
    t_blinkenlights *x = (t_blinkenlights *)pd_new(blinkenlights_class);
    if ( fwidth <= 0 )
    {
        post( "blinkenlights: wrong creation argument : width : %f", fwidth );
        return NULL;
    }
    if ( fheight <= 0 )
    {
        post( "blinkenlights: wrong creation argument : height : %f", fheight );
        return NULL;
    }
    if ( fxpixsize <= 0 )
    {
        post( "blinkenlights: wrong creation argument : x pixel size : %f", fxpixsize );
        return NULL;
    }
    if ( fypixsize <= 0 )
    {
        post( "blinkenlights: wrong creation argument : y pixel size : %f", fypixsize );
        return NULL;
    }
    x->x_width = (int) fwidth;
    x->x_height = (int) fheight;
    x->x_xsize = (int) fxpixsize;
    x->x_ysize = (int) fypixsize;
    x->x_ecanvas = 0;
    x->x_filed = NULL;
    x->x_frame = NULL;
    x->x_clock = NULL;
    x->x_glist = (t_glist *) canvas_getcurrent();
    x->x_background = (char*) getbytes( 8 );
    strncpy( x->x_background, BL_BACKGROUND_COLOR, 7 );
    x->x_background[7] = '\0';
    x->x_foreground = (char*) getbytes( 8 );
    strncpy( x->x_foreground, BL_FOREGROUND_COLOR, 7 );
    x->x_foreground[7] = '\0';
    x->x_timer2 = 40;
    x->frame_inc = 0;

    x->outlet_bang = outlet_new(&x->x_obj, &s_bang);

    return(x);
}

static void blinkenlights_draw_new(t_blinkenlights* x)
{
    t_canvas *canvas=glist_getcanvas(x->x_glist);
    t_int xi, yi;

    SYS_VGUI4("toplevel .x%lx -width %d -height %d -borderwidth 0 -background #000000\n", x,
              x->x_width*x->x_xsize, x->x_height*x->x_ysize );
    SYS_VGUI2("frame .x%lx.m -relief raised -bd 2\n", x);
    SYS_VGUI2("wm title .x%lx blinkenlights\n", x);

    SYS_VGUI4("canvas .x%lx.c -width %d -height %d\n",
              x,
              x->x_width*x->x_xsize, x->x_height*x->x_ysize );
    x->x_ecanvas = 1;

    for ( xi=1; xi<=x->x_width; xi++ )
    {
        for ( yi=1; yi<=x->x_height; yi++ )
        {
            SYS_VGUI10(".x%lx.c create rectangle %d %d %d %d -fill %s -outline #555555 -tags %xPIX%.5d%.5d\n",
                       x,
                       (xi-1)*x->x_xsize, (yi-1)*x->x_ysize,
                       xi*x->x_xsize, yi*x->x_ysize,
                       x->x_background,
                       x, xi, yi);
        }
    }

    SYS_VGUI2("pack .x%lx.c -side left -expand 1 -fill both\n", x);
    SYS_VGUI2("pack .x%lx.m -side top -fill x\n", x);
    SYS_VGUI2("wm geometry .x%lx +0+0\n", x);
    SYS_VGUI2("wm geometry .x%lx +0+0\n", x);

}

static void blinkenlights_create(t_blinkenlights* x)
{

    if ( x->x_ecanvas )
    {
        post("blinkenlights : create : canvas already exists" );
        return;
    }

    blinkenlights_draw_new(x);

}

static void blinkenlights_erase(t_blinkenlights* x)
{
    t_canvas *canvas=glist_getcanvas(x->x_glist);
    t_int xi, yi;

    for ( xi=1; xi<=x->x_width; xi++ )
    {
        for ( yi=1; yi<=x->x_height; yi++ )
        {
            SYS_VGUI5(".x%lx.c delete %xPIX%.5d%.5d\n", x, x, xi, yi);
        }
    }
    SYS_VGUI2("destroy .x%lx\n", x);
    x->x_ecanvas=0;
}

static void blinkenlights_destroy(t_blinkenlights* x)
{
    if ( !x->x_ecanvas )
    {
        post("blinkenlights : destroy : canvas does not exist" );
        return;
    }

    blinkenlights_erase(x);
}

static void blinkenlights_width(t_blinkenlights* x, t_float fwidth)
{
    if ( fwidth <= 0 )
    {
        post( "blinkenlights~: wrong width : ignored" );
        return;
    }
    else
    {
        if ( x->x_width == (int)fwidth ) return;
        x->x_width=(int)fwidth;
    }
    if (x->x_ecanvas) blinkenlights_erase( x );
    blinkenlights_draw_new( x );
}

static void blinkenlights_height(t_blinkenlights* x, t_float fheight)
{
    if ( fheight <= 0 )
    {
        post( "blinkenlights~: wrong height : ignored" );
        return;
    }
    else
    {
        if ( x->x_height == (int)fheight ) return;
        x->x_height=(int)fheight;
    }
    if (x->x_ecanvas) blinkenlights_erase( x );
    blinkenlights_draw_new( x );
}

static void blinkenlights_xsize(t_blinkenlights* x, t_float fxsize)
{
    if ( fxsize <= 0 )
    {
        post( "blinkenlights~: wrong x pixel size : ignored" );
        return;
    }
    else
    {
        if ( x->x_xsize == (int)fxsize ) return;
        x->x_xsize=(int)fxsize;
    }
    if (x->x_ecanvas) blinkenlights_erase( x );
    blinkenlights_draw_new( x );
}

static void blinkenlights_ysize(t_blinkenlights* x, t_float fysize)
{
    if ( fysize <= 0 )
    {
        post( "blinkenlights~: wrong y pixel size : ignored" );
        return;
    }
    else
    {
        if ( x->x_ysize == (int)fysize ) return;
        x->x_ysize=(int)fysize;
    }
    if (x->x_ecanvas) blinkenlights_erase( x );
    blinkenlights_draw_new( x );
}

static void blinkenlights_background(t_blinkenlights* x, t_float fR, t_float fG, t_float fB)
{
    if ( fR <0 || fR>255 )
    {
        post("blinkenlights : wrong color component : fR : %d", fR);
        return;
    }
    if ( fG <0 || fG>255 )
    {
        post("blinkenlights : wrong color component : fG : %d", fG);
        return;
    }
    if ( fB <0 || fB>255 )
    {
        post("blinkenlights : wrong color component : fB : %d", fB);
        return;
    }
    sprintf( x->x_background, "#%.2x%.2x%.2x", (int)fR, (int)fG, (int)fB );
    post("blinkenlights : background color set to : %s", x->x_background );
}

static void blinkenlights_foreground(t_blinkenlights* x, t_float fR, t_float fG, t_float fB)
{
    if ( fR <0 || fR>255 )
    {
        post("blinkenlights : wrong color component : fR : %d", fR);
        return;
    }
    if ( fG <0 || fG>255 )
    {
        post("blinkenlights : wrong color component : fG : %d", fG);
        return;
    }
    if ( fB <0 || fB>255 )
    {
        post("blinkenlights : wrong color component : fB : %d", fB);
        return;
    }
    sprintf( x->x_foreground, "#%.2x%.2x%.2x", (int)fR, (int)fG, (int)fB );
    post("blinkenlights : foreground color set to : %s", x->x_foreground );
}

static void blinkenlights_pixon(t_blinkenlights* x, t_float fX, t_float fY)
{
    if ( !x->x_ecanvas )
    {
        post("blinkenlights : pixon : canvas does not exist" );
        return;
    }
    if ( fX<1 || fX>x->x_width )
    {
        post("blinkenlights : pixon : wrong x coordinate : %d : should be in [1,%d]", (int)fX, x->x_width );
        return;
    }
    if ( fY<1 || fY>x->x_height )
    {
        post("blinkenlights : pixon : wrong y coordinate : %d : should be in [1,%d]", (int)fY, x->x_height );
        return;
    }
    SYS_VGUI6(".x%lx.c itemconfigure %xPIX%.5d%.5d -fill %s\n", x, x, (int)fX, (int)fY, x->x_foreground );
}

static void blinkenlights_pixoff(t_blinkenlights* x, t_float fX, t_float fY)
{
    if ( !x->x_ecanvas )
    {
        post("blinkenlights : pixoff : canvas does not exist" );
        return;
    }
    if ( fX<1 || fX>x->x_width )
    {
        post("blinkenlights : pixoff : wrong x coordinate : %d : should be in [1,%d]", (int)fX, x->x_width );
        return;
    }
    if ( fY<1 || fY>x->x_height )
    {
        post("blinkenlights : pixoff : wrong y coordinate : %d : should be in [1,%d]", (int)fY, x->x_height );
        return;
    }
    SYS_VGUI6(".x%lx.c itemconfigure %xPIX%.5d%.5d -fill %s\n", x, x, (int)fX, (int)fY, x->x_background );
}

static void blinkenlights_pixel(t_blinkenlights* x, t_float fX, t_float fY, t_float fR, t_float fG, t_float fB)
{
    if ( !x->x_ecanvas )
    {
        post("blinkenlights : pixel : canvas does not exist" );
        return;
    }
    if ( fX<1 || fX>x->x_width )
    {
        post("blinkenlights : pixel : wrong x coordinate : %d : should be in [1,%d]", (int)fX, x->x_width );
        return;
    }
    if ( fY<1 || fY>x->x_height )
    {
        post("blinkenlights : pixel : wrong y coordinate : %d : should be in [1,%d]", (int)fY, x->x_height );
        return;
    }
    if ( fR <0 || fR>255 )
    {
        post("blinkenlights : pixel : wrong color component : fR : %d", fR);
        return;
    }
    if ( fG <0 || fG>255 )
    {
        post("blinkenlights : pixel : wrong color component : fG : %d", fG);
        return;
    }
    if ( fB <0 || fB>255 )
    {
        post("blinkenlights : pixel : wrong color component : fB : %d", fB);
        return;
    }
    SYS_VGUI8(".x%lx.c itemconfigure %xPIX%.5d%.5d -fill #%.2X%.2X%.2X\n", x, x, (int)fX, (int)fY, (int)fR, (int)fG, (int)fB );
}

static void blinkenlights_clear(t_blinkenlights* x)
{
    t_int xi, yi;

    for ( xi=1; xi<=x->x_width; xi++ )
    {
        for ( yi=1; yi<=x->x_height; yi++ )
        {
            blinkenlights_pixoff( x, xi, yi );
        }
    }
}

/* close the current movie */
static void blinkenlights_close(t_blinkenlights *x)
{
    /* closing previous file descriptor */
    if ( x->x_filed != NULL )
    {
        if(fclose(x->x_filed) < 0)
        {
            perror( "blinkenlights : closing file" );
        }
        x->x_filed = NULL;
    }
    if ( x->x_frame )
    {
        freebytes( x->x_frame, x->x_width*x->x_height*sizeof(t_int) );
        x->x_frame = NULL;
    }
}

/* read the next frame     */
static void blinkenlights_readframe(t_blinkenlights *x)
{
    char *lineread = (char*) getbytes( BL_MAX_LENGTH );
    t_int flineno = 0;
    t_int width, height, nwidth;

    //post( "blinkenlights: being readframe:>%s<", lineread );

    if ( !x->x_ecanvas )
    {
        post("blinkenlights : next : canvas does not exist" );
        return;
    }

    if ( x->x_filed == NULL )
    {
        post( "blinkenlights : no file is opened for reading a frame" );
        blinkenlights_close(x);
        return;
    }

    // skip header and empty lines
    while ( lineread[0] == '#' || lineread[0] == '\n' || lineread[0] == '\0' )
    {

        //post( "blinkenlights : skipped line : >%s<", lineread );
        if ( fgets( lineread, BL_MAX_LENGTH, x->x_filed ) == 0 )
        {
            post( "blinkenlights : end of file detected : looping..." );
            fseek( x->x_filed, 0L, SEEK_SET );
            outlet_bang(x->outlet_bang);
        }
    }

    if ( lineread[0] != '@' )
    {
        post( "blinkenlights : format error : should find a time lime here : @XXX : got : >%s<", lineread );
        blinkenlights_close(x);
        return;
    }
    else
    {
        x->x_timer = atoi( lineread+1 );
        // post( "blinkenlights : setting timer to %d", x->x_timer );
    }

    // read the contents of one frame

    // when reading first frame the height and width are read from the file
    height = 0;
    width = 0;
    while ( 1 )
    {

        if ( fgets( lineread, BL_MAX_LENGTH, x->x_filed ) == NULL )
        {
            post( "blinkenlights : EOF not expected here !!! ");
            blinkenlights_close(x);
            return;
        }
        else
        {
            if ( (lineread[0] == '\0') || (lineread[0] == '#') || (lineread[0] == '\n') ) break;
            // post( "blinkenlights : lineread : %s", lineread );

            nwidth = strlen( lineread )-1; // because of the carriage return
            flineno++;
            height = flineno;
            if ( ( nwidth != width ) && ( width != 0 ) )
            {
                post( "blinkenlights : weird file : width has changed (nwidth=%d) (width=%d)", nwidth, width );
                blinkenlights_close( x );
                return;
            }
            width = nwidth;
            if ( x->x_frame != NULL )
            {
                t_int pint = 0;
                t_int newvalue;

                while ( pint < width )
                {
                    newvalue = (int) *(lineread+pint) - 48 /* ascii value for '0' */;
                    if ( newvalue != *(x->x_frame+(flineno-1)*x->x_width+pint ) )
                    {
                        *(x->x_frame+(flineno-1)*x->x_width+pint ) = newvalue;
                        switch ( newvalue )
                        {
                        case 0:
                            // post( "pixoff %d %d", pint+1, flineno );
                            blinkenlights_pixoff( x, pint+1, flineno );
                            break;
                        case 1:
                            // post( "pixon %d %d", pint+1, flineno );
                            blinkenlights_pixon( x, pint+1, flineno );
                            break;
                        default:
                            // post("blinkenlights : wrong value found for pixel : %d (c=%c)", newvalue, *(lineread+pint) );
                            break;
                        }
                    }
                    pint++;
                }
            }
            if ( x->x_frame == NULL ) x->x_height++;
        }
    }
    if ( x->x_frame == NULL )
    {
        if ( x->x_filed != NULL ) if ( fseek(x->x_filed, 0L, SEEK_SET) < 0 )
            {
                post( "blinkenlights : could not rewind file" );
                blinkenlights_close( x );
                return;
            }
        blinkenlights_width(x, width);
        blinkenlights_height(x, height);
        x->x_frame = ( t_int* ) getbytes( x->x_width*x->x_height*sizeof(t_int) );
        blinkenlights_readframe(x);
    }

    if ( lineread ) freebytes( lineread, BL_MAX_LENGTH );
}
//-------------------------------------------------------------------------
//--chun's functions begin here...
//-------------------------------------------------------------------------
/* remember all the frame positions */
static void blinkenlights_findframes(t_blinkenlights *x)
{
    int i =0;

    x->frame_no = 0;

    for(i=0;; i++)
    {
        char *lineread = (char*) getbytes( BL_MAX_LENGTH );

        fgets( lineread, BL_MAX_LENGTH, x->x_filed );
        if(strlen(lineread) == 0) break;

        if(lineread[0] == '@')
        {
            x->frame_pos[x->frame_no] = ftell(x->x_filed);
            x->frame_no++;
        }
        if (lineread) freebytes( lineread, BL_MAX_LENGTH );
    }
    fseek( x->x_filed, 0L, SEEK_SET );
    post("the end:: %d frames!", x->frame_no);
}

//-------------------------------------------------------------------------
static void blinkenlights_goto(t_blinkenlights* x)
{
//char *lineread = (char*) getbytes( BL_MAX_LENGTH );
    char lineread[BL_MAX_LENGTH];
    int current_frame = x->frame_pos[(int)(x->frame_inc * (x->frame_no-1))];
    int i, n, width, newvalue, height, nwidth;

    t_int flineno = 0;

    fseek(x->x_filed, current_frame, SEEK_SET);

    height = 0;
    width = 0;
    while ( 1 )
    {

        if ( fgets( lineread, BL_MAX_LENGTH, x->x_filed ) == NULL )
        {
            post( "blinkenlights : EOF not expected here !!! ");
            blinkenlights_close(x);
            return;
        }
        else
        {
            if ( (lineread[0] == '\0') || (lineread[0] == '#') || (lineread[0] == '\n') ) break;
            // post( "blinkenlights : lineread : %s", lineread );

            nwidth = strlen( lineread )-1; // because of the carriage return
            flineno++;
            height = flineno;
            if ( ( nwidth != width ) && ( width != 0 ) )
            {
                post( "blinkenlights : weird file : width has changed (nwidth=%d) (width=%d)", nwidth, width );
                blinkenlights_close( x );
                return;
            }
            width = nwidth;
            if ( x->x_frame != NULL )
            {
                t_int pint = 0;
                t_int newvalue;

                while ( pint < width )
                {
                    newvalue = (int) *(lineread+pint) - 48 /* ascii value for '0' */;
                    if ( newvalue != *(x->x_frame+(flineno-1)*x->x_width+pint ) )
                    {
                        *(x->x_frame+(flineno-1)*x->x_width+pint ) = newvalue;
                        switch ( newvalue )
                        {
                        case 0:
                            // post( "pixoff %d %d", pint+1, flineno );
                            blinkenlights_pixoff( x, pint+1, flineno );
                            break;
                        case 1:
                            // post( "pixon %d %d", pint+1, flineno );
                            blinkenlights_pixon( x, pint+1, flineno );
                            break;
                        default:
                            // post("blinkenlights : wrong value found for pixel : %d (c=%c)", newvalue, *(lineread+pint) );
                            break;
                        }
                    }
                    pint++;
                }
            }
            if ( x->x_frame == NULL ) x->x_height++;
        }
    }
    if ( x->x_frame == NULL )
    {
        if ( x->x_filed != NULL ) if ( fseek(x->x_filed, 0L, SEEK_SET) < 0 )
            {
                post( "blinkenlights : could not rewind file" );
                blinkenlights_close( x );
                return;
            }
        blinkenlights_width(x, width);
        blinkenlights_height(x, height);
        x->x_frame = ( t_int* ) getbytes( x->x_width*x->x_height*sizeof(t_int) );
        blinkenlights_readframe(x);
    }

}

static void blinkenlights_frame_pos(t_blinkenlights* x, t_float pos)
{
    if(pos > 1 | pos < 0) post ("dude, don't be crazy!");
    else x->frame_inc = pos;;
//post("frame %d", x->frame_inc);
}

static void blinkenlights_timer2(t_blinkenlights* x, t_float timer)
{
    x->x_timer2 = timer;
//post("frame %d", x->frame_inc);
}

/* open movie */
static void blinkenlights_open(t_blinkenlights *x, t_symbol *sfile)
{
    if ( !x->x_ecanvas )
    {
        post("blinkenlights : open : canvas does not exist" );
        return;
    }

    //----------------------------------
    /* closing previous file descriptor */
    if ( x->x_filed != NULL )
    {
        if(fclose(x->x_filed) < 0)
        {
            perror( "blinkenlights : closing file" );
        }
        x->x_filed = NULL;
    }
    if ( x->x_frame )
    {
        blinkenlights_clear(x);
    }

    //--------------------------------

    if ( ( x->x_filed = sys_fopen( sfile->s_name, "r" ) ) == NULL )
    {
        error( "blinkenlights : cannot open >%s<", sfile->s_name);
        return;
    }
    post( "blinkenlights : opened >%s<", sfile->s_name);
    // don't read the first frame when open..
    //blinkenlights_readframe(x);
    blinkenlights_findframes(x);
}

/* play frames */
static void blinkenlights_playframes(t_blinkenlights *x)
{
    blinkenlights_readframe( x );
    clock_delay( x->x_clock, (double)x->x_timer );
}

/* play frames2 */
static void blinkenlights_playframes2(t_blinkenlights *x)
{
    blinkenlights_goto(x);
    clock_delay( x->x_clock2, (double)x->x_timer2 );
}

/* play movie */
static void blinkenlights_play(t_blinkenlights *x)
{
    if ( !x->x_ecanvas )
    {
        post("blinkenlights : play : canvas does not exist" );
        return;
    }

    if ( x->x_filed == NULL )
    {
        post( "blinkenlights : no file is opened for playing" );
        blinkenlights_close(x);
        return;
    }

    if ( x->x_clock == NULL ) x->x_clock = clock_new( x, (t_method)blinkenlights_playframes);
    clock_delay( x->x_clock, (double)x->x_timer );
}

/* vj movie */
static void blinkenlights_vj(t_blinkenlights *x, t_float start_vj)
{
    if ( !x->x_ecanvas )
    {
        post("blinkenlights : play : canvas does not exist" );
        return;
    }

    if ( x->x_filed == NULL )
    {
        post( "blinkenlights : no file is opened for playing" );
        blinkenlights_close(x);
        return;
    }

    if(start_vj)
    {
        if ( x->x_clock2 == NULL ) x->x_clock2 = clock_new( x, (t_method)blinkenlights_playframes2);
        clock_delay( x->x_clock2, (double)x->x_timer2 );
    }
    else
    {
        clock_unset( x->x_clock2 );
    }
}

/* stop movie */
static void blinkenlights_stop(t_blinkenlights *x)
{
    if ( !x->x_ecanvas )
    {
        post("blinkenlights : play : canvas does not exist" );
        return;
    }

    if ( x->x_filed == NULL )
    {
        post( "blinkenlights : no file is opened for playing" );
        blinkenlights_close(x);
        return;
    }

    if ( x->x_clock != NULL )
    {
        clock_unset( x->x_clock );
    }
}

/* jump to next frame */
static void blinkenlights_next(t_blinkenlights *x)
{
    blinkenlights_readframe(x);
}

void blinkenlights_setup(void)
{
    logpost(NULL, 4, blinkenlights_version);
    blinkenlights_class = class_new(gensym("blinkenlights"), (t_newmethod)blinkenlights_new,
                                    (t_method)blinkenlights_free,
                                    sizeof(t_blinkenlights), 0, A_DEFFLOAT, A_DEFFLOAT, A_DEFFLOAT, A_DEFFLOAT, 0);
    class_addmethod( blinkenlights_class, (t_method)blinkenlights_width, gensym("width"), A_FLOAT, 0);
    class_addmethod( blinkenlights_class, (t_method)blinkenlights_height, gensym("height"), A_FLOAT, 0);
    class_addmethod( blinkenlights_class, (t_method)blinkenlights_xsize, gensym("xsize"), A_FLOAT, 0);
    class_addmethod( blinkenlights_class, (t_method)blinkenlights_ysize, gensym("ysize"), A_FLOAT, 0);
    class_addmethod( blinkenlights_class, (t_method)blinkenlights_open, gensym("open"), A_SYMBOL, 0);
    class_addmethod( blinkenlights_class, (t_method)blinkenlights_create, gensym("create"), 0);
    class_addmethod( blinkenlights_class, (t_method)blinkenlights_destroy, gensym("destroy"), 0);
    class_addmethod( blinkenlights_class, (t_method)blinkenlights_next, gensym("next"), 0);
    class_addmethod( blinkenlights_class, (t_method)blinkenlights_play, gensym("play"), 0);
    class_addmethod( blinkenlights_class, (t_method)blinkenlights_stop, gensym("stop"), 0);
    class_addmethod( blinkenlights_class, (t_method)blinkenlights_frame_pos, gensym("goto"), A_FLOAT, 0);
    class_addmethod( blinkenlights_class, (t_method)blinkenlights_vj, gensym("vj"), A_FLOAT, 0);
    class_addmethod( blinkenlights_class, (t_method)blinkenlights_timer2, gensym("timer2"), A_FLOAT, 0);
    class_addmethod( blinkenlights_class, (t_method)blinkenlights_background, gensym("background"), A_FLOAT, A_FLOAT, A_FLOAT, 0);
    class_addmethod( blinkenlights_class, (t_method)blinkenlights_foreground, gensym("foreground"), A_FLOAT, A_FLOAT, A_FLOAT, 0);
    class_addmethod( blinkenlights_class, (t_method)blinkenlights_destroy, gensym("destroy"), 0);
    class_addmethod( blinkenlights_class, (t_method)blinkenlights_pixon, gensym("pixon"), A_FLOAT, A_FLOAT, 0);
    class_addmethod( blinkenlights_class, (t_method)blinkenlights_pixoff, gensym("pixoff"), A_FLOAT, A_FLOAT, 0);
    class_addmethod( blinkenlights_class, (t_method)blinkenlights_pixel, gensym("pixel"), A_FLOAT, A_FLOAT, A_FLOAT, A_FLOAT, A_FLOAT, 0);
    class_addmethod( blinkenlights_class, (t_method)blinkenlights_clear, gensym("clear"), 0);
}
