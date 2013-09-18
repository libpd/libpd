/* Copyright (c) 1997-1999 Miller Puckette.                                     */
/* For information on usage and redistribution, and for a DISCLAIMER OF ALL     */
/* WARRANTIES, see the file, "LICENSE.txt," in this distribution.               */
/*                                                                              */
/* pianoroll : a graphical object which enables                                 */
/* to control a sequencer ( pitch and volume )                                  */
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
/* "If you obey society's rules"                                                */
/* "You'll be society's fool"                                                   */
/* Devo - Society's rules                                                       */
/* ---------------------------------------------------------------------------- */


#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <ctype.h>
#include "m_pd.h"
#include "m_imp.h"
#include "g_canvas.h"

#include "pianoroll.h"

#ifdef _WIN32
#include <io.h>
#else
#include <unistd.h>
#endif

/* needed to create a pianoroll from PD's menu
void canvas_objtext(t_glist *gl, int xpos, int ypos, int selected, t_binbuf *b);
void canvas_startmotion(t_canvas *x);
*/

#define DEFAULT_SEQUENCER_WIDTH 200
#define DEFAULT_SEQUENCER_HEIGHT 200
#define DEFAULT_SEQUENCER_STEPS 16
#define DEFAULT_SEQUENCER_NBGRADES 31
#define DEFAULT_SEQUENCER_PITCH_MIN -15
#define DEFAULT_SEQUENCER_PITCH_MAX 15

static char   *pianoroll_version = "pianoroll: a graphical sequencer controller, version 0.10 (ydegoyon@free.fr)";

t_widgetbehavior pianoroll_widgetbehavior;
static t_class *pianoroll_class;
static int pianorollcount=0;

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

#define SYS_VGUI7(a,b,c,d,e,f,g) if (guidebug) \
                         post(a,b,c,d,e,f,g );\
                         sys_vgui(a,b,c,d,e,f,g)

#define SYS_VGUI9(a,b,c,d,e,f,g,h,i) if (guidebug) \
                         post(a,b,c,d,e,f,g,h,i );\
                         sys_vgui(a,b,c,d,e,f,g,h,i)

/* drawing functions */
static void pianoroll_draw_update(t_pianoroll *x, t_glist *glist)
{
    t_int si;
    t_canvas *canvas=glist_getcanvas(glist);

    for ( si=0; si<x->x_nbsteps; si++ )
    {
        int vi = (int)((1.0-x->x_volumes[si])*(x->x_nbgrades-1));
        int pi = (int)((x->x_pmax-x->x_peaches[si])/(x->x_pmax-x->x_pmin)*(x->x_nbgrades-1));

        x->x_ivolumes[ si ] = vi;
        x->x_ipeaches[ si ] = pi;
        SYS_VGUI5(".x%lx.c itemconfigure %xPITCH%.4d%.4d -fill #FFFF00\n", canvas, x, si, pi);
        SYS_VGUI5(".x%lx.c itemconfigure %xVOLUME%.4d%.4d -fill #FF0000\n", canvas, x, si, vi);
    }
}

static void pianoroll_draw_new(t_pianoroll *x, t_glist *glist)
{
    t_canvas *canvas=glist_getcanvas(glist);

    // draw the grid
    {
        int gi, gj;
        t_float xgstep = x->x_width/x->x_nbsteps;
        t_float ygstep = x->x_height/x->x_nbgrades;
        for ( gi=0; gi<x->x_nbsteps; gi++ )
        {
            for ( gj=0; gj<x->x_nbgrades; gj++ )
            {
                SYS_VGUI9(".x%lx.c create rectangle %d %d %d %d -fill #771623 -outline #998121 -tags %xPITCH%.4d%.4d\n",
                          canvas,
                          text_xpix(&x->x_obj, glist)+(int)(gi*xgstep),
                          text_ypix(&x->x_obj, glist)+(int)(gj*ygstep),
                          text_xpix(&x->x_obj, glist)+(int)(gi*xgstep)+(int)(2*xgstep/3),
                          text_ypix(&x->x_obj, glist)+(int)((gj+1)*ygstep),
                          x, gi, gj );
                SYS_VGUI9(".x%lx.c create rectangle %d %d %d %d -fill #562663 -outline #998121 -tags %xVOLUME%.4d%.4d\n",
                          canvas,
                          text_xpix(&x->x_obj, glist)+(int)(gi*xgstep)+(int)(2*xgstep/3),
                          text_ypix(&x->x_obj, glist)+(int)(gj*ygstep),
                          text_xpix(&x->x_obj, glist)+(int)((gi+1)*xgstep),
                          text_ypix(&x->x_obj, glist)+(int)((gj+1)*ygstep),
                          x, gi, gj );
            }
        }
        // adjust height and width
        x->x_width = (int)((x->x_nbsteps)*xgstep);
        x->x_height = (int)((x->x_nbgrades)*ygstep);
    }
    SYS_VGUI7(".x%lx.c create rectangle %d %d %d %d -tags %xIN\n",
              canvas, text_xpix(&x->x_obj, glist), text_ypix(&x->x_obj, glist) - 1,
              text_xpix(&x->x_obj, glist)+7, text_ypix(&x->x_obj, glist),
              x);
    SYS_VGUI7(".x%lx.c create rectangle %d %d %d %d -tags %xOUTL\n",
              canvas, text_xpix(&x->x_obj, glist), text_ypix(&x->x_obj, glist) + x->x_height+1,
              text_xpix(&x->x_obj, glist)+7, text_ypix(&x->x_obj, glist) + x->x_height+2,
              x);
    SYS_VGUI7(".x%lx.c create rectangle %d %d %d %d -tags %xOUTR\n",
              canvas, text_xpix(&x->x_obj, glist)+x->x_width-7, text_ypix(&x->x_obj, glist) + x->x_height+1,
              text_xpix(&x->x_obj, glist)+x->x_width, text_ypix(&x->x_obj, glist) + x->x_height+2,
              x);

    canvas_fixlinesfor( canvas, (t_text*)x );
}

static void pianoroll_draw_move(t_pianoroll *x, t_glist *glist)
{
    t_canvas *canvas=glist_getcanvas(glist);

    // move the grid
    {
        int gi, gj;
        t_float xgstep = x->x_width/x->x_nbsteps;
        t_float ygstep = x->x_height/x->x_nbgrades;
        for ( gi=0; gi<x->x_nbsteps; gi++ )
        {
            for ( gj=0; gj<x->x_nbgrades; gj++ )
            {
                SYS_VGUI9(".x%lx.c coords  %xPITCH%.4d%.4d %d %d %d %d\n",
                          canvas, x, gi, gj,
                          text_xpix(&x->x_obj, glist)+(int)(gi*xgstep),
                          text_ypix(&x->x_obj, glist)+(int)(gj*ygstep),
                          text_xpix(&x->x_obj, glist)+(int)(gi*xgstep)+(int)(2*xgstep/3),
                          text_ypix(&x->x_obj, glist)+(int)((gj+1)*ygstep)
                         );
                SYS_VGUI9(".x%lx.c coords %xVOLUME%.4d%.4d %d %d %d %d\n",
                          canvas, x, gi, gj,
                          text_xpix(&x->x_obj, glist)+(int)(gi*xgstep)+(int)(2*xgstep/3),
                          text_ypix(&x->x_obj, glist)+(int)(gj*ygstep),
                          text_xpix(&x->x_obj, glist)+(int)((gi+1)*xgstep),
                          text_ypix(&x->x_obj, glist)+(int)((gj+1)*ygstep)
                         );
            }
        }
    }
    SYS_VGUI7(".x%lx.c coords %xIN %d %d %d %d \n",
              canvas, x, text_xpix(&x->x_obj, glist), text_ypix(&x->x_obj, glist) - 1,
              text_xpix(&x->x_obj, glist)+7, text_ypix(&x->x_obj, glist)
             );
    SYS_VGUI7(".x%lx.c coords %xOUTL %d %d %d %d\n",
              canvas, x, text_xpix(&x->x_obj, glist), text_ypix(&x->x_obj, glist) + x->x_height+1,
              text_xpix(&x->x_obj, glist)+7, text_ypix(&x->x_obj, glist) + x->x_height+2
             );
    SYS_VGUI7(".x%lx.c coords %xOUTR %d %d %d %d\n",
              canvas, x, text_xpix(&x->x_obj, glist)+x->x_width-7, text_ypix(&x->x_obj, glist) + x->x_height+1,
              text_xpix(&x->x_obj, glist)+x->x_width, text_ypix(&x->x_obj, glist) + x->x_height+2
             );
    canvas_fixlinesfor( canvas, (t_text*)x );
}

static void pianoroll_draw_erase(t_pianoroll* x,t_glist* glist)
{
    t_canvas *canvas=glist_getcanvas(glist);
    t_int i;

    SYS_VGUI3(".x%lx.c delete %xIN\n", canvas, x);
    SYS_VGUI3(".x%lx.c delete %xOUTL\n", canvas, x);
    SYS_VGUI3(".x%lx.c delete %xOUTR\n", canvas, x);
    // delete the grid
    {
        int gi, gj;
        for ( gi=0; gi<x->x_nbsteps; gi++ )
        {
            for ( gj=0; gj<x->x_nbgrades; gj++ )
            {
                SYS_VGUI5(".x%lx.c delete %xPITCH%.4d%.4d\n", canvas, x, gi, gj);
                SYS_VGUI5(".x%lx.c delete %xVOLUME%.4d%.4d\n", canvas, x, gi, gj);
            }
        }
    }
}

static void pianoroll_draw_select(t_pianoroll* x,t_glist* glist)
{
    t_canvas *canvas=glist_getcanvas(glist);

    if(x->x_selected)
    {
        pd_bind(&x->x_obj.ob_pd, x->x_name);
        /* sets the item in blue */
    }
    else
    {
        pd_unbind(&x->x_obj.ob_pd, x->x_name);
    }
}

/* ------------------------ pianoroll widgetbehaviour----------------------------- */


static void pianoroll_getrect(t_gobj *z, t_glist *owner,
                              int *xp1, int *yp1, int *xp2, int *yp2)
{
    t_pianoroll* x = (t_pianoroll*)z;

    *xp1 = text_xpix(&x->x_obj, owner);
    *yp1 = text_ypix(&x->x_obj, owner);
    *xp2 = text_xpix(&x->x_obj, owner)+x->x_width;
    *yp2 = text_ypix(&x->x_obj, owner)+x->x_height;
}

static void pianoroll_save(t_gobj *z, t_binbuf *b)
{
    t_pianoroll *x = (t_pianoroll *)z;
    t_int i;

    // post( "saving pianoroll : %s", x->x_name->s_name );
    binbuf_addv(b, "ssiissiiffiiifi", gensym("#X"),gensym("obj"),
                (t_int)x->x_obj.te_xpix, (t_int)x->x_obj.te_ypix,
                atom_getsymbol(binbuf_getvec(x->x_obj.te_binbuf)),
                x->x_name, x->x_width, x->x_height,
                x->x_pmin, x->x_pmax,
                x->x_nbgrades, x->x_nbsteps,
                x->x_defvalue, x->x_transpose, x->x_save
               );
    if ( x->x_save )
    {
        for ( i=0; i<x->x_nbsteps; i++ )
        {
            binbuf_addv(b, "ff", x->x_peaches[i], x->x_volumes[i] );
        }
    }
    binbuf_addv(b, ";");
}

static void pianoroll_properties(t_gobj *z, t_glist *owner)
{
    char buf[800];
    t_pianoroll *x=(t_pianoroll *)z;

    sprintf(buf, "pdtk_pianoroll_dialog %%s %s %d %d %.2f %.2f %d %d %d %d\n",
            x->x_name->s_name, x->x_width, x->x_height, x->x_pmin, x->x_pmax,
            x->x_nbgrades, x->x_nbsteps, x->x_defvalue, x->x_save );
    // post("pianoroll_properties : %s", buf );
    gfxstub_new(&x->x_obj.ob_pd, x, buf);
}

static void pianoroll_select(t_gobj *z, t_glist *glist, int selected)
{
    t_pianoroll *x = (t_pianoroll *)z;

    x->x_selected = selected;
    pianoroll_draw_select( x, glist );
}

static void pianoroll_vis(t_gobj *z, t_glist *glist, int vis)
{
    t_pianoroll *x = (t_pianoroll *)z;
    t_rtext *y;

    // post("pianoroll_vis : %d", vis );
    if (vis)
    {
        pianoroll_draw_new( x, glist );
        pianoroll_draw_update( x, glist );
    }
    else
    {
        pianoroll_draw_erase( x, glist );
    }
}

static void pianoroll_dialog(t_pianoroll *x, t_symbol *s, int argc, t_atom *argv)
{
    int si, onbsteps;
    t_float *newpeaches, *newvolumes;

    if ( !x )
    {
        post( "pianoroll : error :tried to set properties on an unexisting object" );
    }
    if ( ( argv[5].a_w.w_float <= 0 ) || ( argv[5].a_w.w_float <= 0 ) )
    {
        post( "pianoroll : error : wrong number of steps or grades" );
        return;
    }
    if ( ( argv[3].a_w.w_float >= argv[4].a_w.w_float ) )
    {
        post( "pianoroll : error : min pitch is >= to max pitch" );
        return;
    }
    if ( argc != 9 )
    {
        post( "pianoroll : error in the number of arguments ( %d )", argc );
        return;
    }
    if ( argv[0].a_type != A_SYMBOL || argv[1].a_type != A_FLOAT ||
            argv[2].a_type != A_FLOAT || argv[3].a_type != A_FLOAT ||
            argv[4].a_type != A_FLOAT || argv[5].a_type != A_FLOAT ||
            argv[6].a_type != A_FLOAT || argv[7].a_type != A_FLOAT ||
            argv[8].a_type != A_FLOAT
       )
    {
        post( "pianoroll : wrong arguments" );
        return;
    }
    pianoroll_draw_erase(x, x->x_glist);
    x->x_name = argv[0].a_w.w_symbol;
    x->x_width = (int)argv[1].a_w.w_float;
    x->x_height = (int)argv[2].a_w.w_float;
    x->x_pmin = argv[3].a_w.w_float;
    x->x_pmax = argv[4].a_w.w_float;
    x->x_nbgrades = argv[5].a_w.w_float;
    onbsteps = x->x_nbsteps;
    x->x_nbsteps = argv[6].a_w.w_float;
    x->x_defvalue = argv[7].a_w.w_float;
    x->x_save = argv[8].a_w.w_float;

    if ( onbsteps != x->x_nbsteps )
    {
        int cmindex = ( onbsteps > x->x_nbsteps ) ? x->x_nbsteps : onbsteps;

        newpeaches = ( t_float* ) getbytes( x->x_nbsteps*sizeof(t_float) );
        newvolumes = ( t_float* ) getbytes( x->x_nbsteps*sizeof(t_float) );
        for ( si=0; si<cmindex ; si++ )
        {
            newpeaches[si] = x->x_peaches[si];
            newvolumes[si] = x->x_volumes[si];
        }
        freebytes( x->x_peaches, onbsteps*sizeof( t_float ) );
        freebytes( x->x_volumes, onbsteps*sizeof( t_float ) );
        x->x_peaches = newpeaches;
        x->x_volumes = newvolumes;
        for ( si=onbsteps; si<x->x_nbsteps; si++ )
        {
            x->x_peaches[si] = x->x_defvalue;
            x->x_volumes[si] = 1.0;
        }
        freebytes( x->x_ipeaches, onbsteps*sizeof( t_int ) );
        freebytes( x->x_ivolumes, onbsteps*sizeof( t_int ) );
        x->x_ipeaches = ( t_int* ) getbytes( x->x_nbsteps*sizeof(t_int) );
        x->x_ivolumes = ( t_int* ) getbytes( x->x_nbsteps*sizeof(t_int) );
    }
    pianoroll_draw_new(x, x->x_glist);
    pianoroll_draw_update(x, x->x_glist);
}

static void pianoroll_delete(t_gobj *z, t_glist *glist)
{
    canvas_deletelinesfor(glist, (t_text *)z);
}

static void pianoroll_displace(t_gobj *z, t_glist *glist, int dx, int dy)
{
    t_pianoroll *x = (t_pianoroll *)z;
    int xold = text_xpix(&x->x_obj, glist);
    int yold = text_ypix(&x->x_obj, glist);

    // post( "pianoroll_displace dx=%d dy=%d", dx, dy );

    x->x_obj.te_xpix += dx;
    x->x_obj.te_ypix += dy;
    if(xold != x->x_obj.te_xpix || yold != x->x_obj.te_ypix)
    {
        pianoroll_draw_move(x, x->x_glist);
    }
}

static int pianoroll_click(t_gobj *z, struct _glist *glist,
                           int xpix, int ypix, int shift, int alt, int dbl, int doit)
{
    t_pianoroll* x = (t_pianoroll *)z;
    t_canvas *canvas=glist_getcanvas(glist);

    if ( doit)
    {
        // calculate position to update
        {
            int si, gi;
            t_float xgstep = x->x_width/x->x_nbsteps;
            t_float ygstep = x->x_height/x->x_nbgrades;

            si = ( xpix - text_xpix(&x->x_obj, glist) ) / xgstep;
            gi = ( ypix - text_ypix(&x->x_obj, glist) ) / ygstep;

            // post( "pianoroll : step : %d : grade : %d", si, gi );

            if ( ( xpix - text_xpix(&x->x_obj, glist) ) > ( si*xgstep+2*xgstep/3 ) )
            {
                {
                    SYS_VGUI5(".x%lx.c itemconfigure %xVOLUME%.4d%.4d -fill #562663\n", canvas, x, si, x->x_ivolumes[ si ] );
                }

                x->x_volumes[ si ] = (((float)x->x_nbgrades-1-(float)gi))/(float)(x->x_nbgrades-1);
                SYS_VGUI5(".x%lx.c itemconfigure %xVOLUME%.4d%.4d -fill #FF0000\n", canvas, x, si, gi);
                x->x_ivolumes[ si ] = gi;
            }
            else
            {
                {
                    SYS_VGUI5(".x%lx.c itemconfigure %xPITCH%.4d%.4d -fill #761623\n", canvas, x, si, x->x_ipeaches[ si ]);
                }

                x->x_peaches[ si ] = x->x_pmin+(float)(x->x_nbgrades-1-gi)/(float)(x->x_nbgrades-1)*(float)(x->x_pmax-x->x_pmin);
                SYS_VGUI5(".x%lx.c itemconfigure %xPITCH%.4d%.4d -fill #FFFF00\n", canvas, x, si, gi);
                x->x_ipeaches[ si ] = gi;
            }
        }

    }
    return (1);
}

static void pianoroll_transpose(t_pianoroll *x, t_floatarg ftranspose)
{
    x->x_transpose = ftranspose;
}

static void pianoroll_save_file(t_pianoroll *x, t_symbol *ffile)
{
    FILE *tmph;
    t_int si;

    if ( ( tmph = sys_fopen( ffile->s_name, "w" ) ) == NULL )
    {
        post( "pianoroll : could not open file : %s for writing", ffile->s_name );
        return;
    }

    // post( "saving pianoroll : %s", x->x_name->s_name );
    fprintf(tmph, "%d %d %f %f %d %d %d %f %d ",
            x->x_width, x->x_height,
            x->x_pmin, x->x_pmax,
            x->x_nbgrades, x->x_nbsteps,
            x->x_defvalue, x->x_transpose, x->x_save );
    for ( si=0; si<x->x_nbsteps; si++ )
    {
        fprintf(tmph, "%f %f ", x->x_peaches[si], x->x_volumes[si] );
    }

    if ( fclose( tmph ) == -1 )
    {
        post( "pianoroll : could not close file : %s ", ffile->s_name );
        return;
    }
}

static void pianoroll_load(t_pianoroll *x, t_symbol *ffile)
{
    FILE *tmph;
    t_int si;

    if ( ( tmph = sys_fopen( ffile->s_name, "r" ) ) == NULL )
    {
        post( "pianoroll : could not open file : %s for reading", ffile->s_name );
        return;
    }

    pianoroll_draw_erase(x, x->x_glist);
    freebytes( x->x_peaches, x->x_nbsteps*sizeof( t_float ) );
    freebytes( x->x_volumes, x->x_nbsteps*sizeof( t_float ) );
    freebytes( x->x_ipeaches, x->x_nbsteps*sizeof( t_int ) );
    freebytes( x->x_ivolumes, x->x_nbsteps*sizeof( t_int ) );

    if ( fscanf(tmph, "%d %d %f %f %d %d %d %f %d",
                &x->x_width, &x->x_height,
                &x->x_pmin, &x->x_pmax,
                &x->x_nbgrades, &x->x_nbsteps,
                &x->x_defvalue, &x->x_transpose, &x->x_save ) != 9 )
    {
        post( "pianoroll : could not restore data from file : %s", ffile->s_name );
        return;
    }

    x->x_peaches = ( t_float* ) getbytes( x->x_nbsteps*sizeof(t_float) );
    x->x_volumes = ( t_float* ) getbytes( x->x_nbsteps*sizeof(t_float) );
    x->x_ipeaches = ( t_int* ) getbytes( x->x_nbsteps*sizeof(t_int) );
    x->x_ivolumes = ( t_int* ) getbytes( x->x_nbsteps*sizeof(t_int) );
    for ( si=0; si<x->x_nbsteps ; si++ )
    {
        fscanf( tmph, "%f", &x->x_peaches[si] );
        fscanf( tmph, "%f", &x->x_volumes[si] );
    }

    pianoroll_draw_new(x, x->x_glist);
    pianoroll_draw_update(x, x->x_glist);

    if ( fclose( tmph ) == -1 )
    {
        post( "pianoroll : could not close file : %s ", ffile->s_name );
        return;
    }
}

static void pianoroll_init(t_pianoroll *x)
{
    t_int si;

    for ( si=0; si<x->x_nbsteps; si++ )
    {
        x->x_peaches[si] = x->x_defvalue;
        x->x_volumes[si] = 1.0;
    }
    pianoroll_draw_erase(x, x->x_glist );
    pianoroll_draw_new(x, x->x_glist );
    pianoroll_draw_update(x, x->x_glist );
}

static void pianoroll_pitch(t_pianoroll *x, t_floatarg fpos, t_floatarg fpitch)
{
    t_canvas *canvas=glist_getcanvas(x->x_glist);
    t_int ipos;

    if ( ( ( (t_int) fpos ) < 0 ) || ( ( (t_int) fpos ) >= x->x_nbsteps ) )
    {
        post( "pianoroll : wrong pitch position : %d", fpos );
        return;
    }
    ipos = (t_int) fpos;
    if ( ( ( (t_int) fpitch ) < x->x_pmin ) || ( ( (t_int) fpitch ) > x->x_pmax ) )
    {
        post( "pianoroll : wrong pitch value : %d", fpitch );
        return;
    }

    {
        SYS_VGUI5(".x%lx.c itemconfigure %xPITCH%.4d%.4d -fill #761623\n", canvas, x, ipos, x->x_ipeaches[ ipos ]);

        x->x_ipeaches[ ipos ] = (t_int) ( ( ( x->x_pmax - fpitch ) / ( x->x_pmax - x->x_pmin ) ) * ( x->x_nbgrades - 1 ) );
        x->x_peaches[ ipos ] = x->x_pmin+(float)(x->x_nbgrades-1-x->x_ipeaches[ ipos ])/(float)(x->x_nbgrades-1)*(float)(x->x_pmax-x->x_pmin);
        SYS_VGUI5(".x%lx.c itemconfigure %xPITCH%.4d%.4d -fill #FFFF00\n", canvas, x, ipos, x->x_ipeaches[ ipos ]);
    }
}

static void pianoroll_volume(t_pianoroll *x, t_floatarg fpos, t_floatarg fvol)
{
    t_canvas *canvas=glist_getcanvas(x->x_glist);
    t_int ipos;

    if ( ( ( (t_int) fpos ) < 0 ) || ( ( (t_int) fpos ) >= x->x_nbsteps ) )
    {
        post( "pianoroll : wrong volume position : %d", fpos );
        return;
    }
    ipos = (t_int) fpos;
    if ( ( ( (t_int) fvol ) < 0.0 ) || ( ( (t_int) fvol ) > 1.0 ) )
    {
        post( "pianoroll : wrong volume value : %d", fvol );
        return;
    }

    {
        SYS_VGUI5(".x%lx.c itemconfigure %xVOLUME%.4d%.4d -fill #562663\n", canvas, x, ipos, x->x_ivolumes[ ipos ] );

        x->x_ivolumes[ ipos ] = (t_int) ( ( 1 - fvol ) * (x->x_nbgrades-1) );
        x->x_volumes[ ipos ] = (((float)x->x_nbgrades-1-(float)x->x_ivolumes[ ipos ]))/(float)(x->x_nbgrades-1);
        SYS_VGUI5(".x%lx.c itemconfigure %xVOLUME%.4d%.4d -fill #FF0000\n", canvas, x, ipos, x->x_ivolumes[ ipos ] );
    }
}

static void pianoroll_float(t_pianoroll *x, t_floatarg fposition)
{
    t_int pposition, rposition, rrposition;
    t_float fpart;
    t_int pi;
    t_canvas *canvas;

    pposition = ( (int)fposition - 1 ) % x->x_nbsteps;
    if ( pposition < 0 ) pposition += x->x_nbsteps;
    rposition = ( (int)fposition ) % x->x_nbsteps;
    if ( rposition < 0 ) rposition += x->x_nbsteps;
    rrposition = ( (int)fposition +1 ) % x->x_nbsteps;
    if ( rrposition < 0 ) rrposition += x->x_nbsteps;

    // post ( "pposition=%d rposition=%d rrposition=%d", pposition, rposition, rrposition );
    fpart = fposition - (int)fposition;
    pi=-1;
    canvas=glist_getcanvas(x->x_glist);

    outlet_float( x->x_pitch, x->x_peaches[ rposition ] +
                  fpart*(x->x_peaches[ rrposition ] - x->x_peaches[ rposition ] ) +
                  x->x_transpose );
    outlet_float( x->x_volume, x->x_volumes[ rposition ] +
                  fpart*(x->x_volumes[ rrposition ] - x->x_volumes[ rposition ] ) );

    // graphical update
    {
        if ( x->x_scurrent != -1 )
        {
            SYS_VGUI5(".x%lx.c itemconfigure %xPITCH%.4d%.4d -fill #FFFF00\n", canvas, x,
                      x->x_scurrent, x->x_ipeaches[ x->x_scurrent  ]);
        }
        x->x_scurrent = rposition;
        SYS_VGUI5(".x%lx.c itemconfigure %xPITCH%.4d%.4d -fill #00FF00\n", canvas, x,
                  x->x_scurrent, x->x_ipeaches[ x->x_scurrent ]);
    }
}

static t_pianoroll *pianoroll_new(t_symbol *s, int argc, t_atom *argv)
{
    int si, i, zz;
    t_pianoroll *x;
    t_pd *x2;
    char *str;

    // post( "pianoroll_new : create : %s argc =%d", s->s_name, argc );

    x = (t_pianoroll *)pd_new(pianoroll_class);
    // new pianoroll created from the gui
    if ( argc != 0 )
    {
        if ( argc < 10 )
        {
            post( "pianoroll : error in the number of arguments ( %d )", argc );
            return NULL;
        }
        if ( ( argv[5].a_w.w_float <= 0 ) || ( argv[5].a_w.w_float <= 0 ) )
        {
            post( "pianoroll : error : wrong number of steps or grades" );
            return NULL;
        }
        if ( ( argv[3].a_w.w_float >= argv[4].a_w.w_float ) )
        {
            post( "pianoroll : error : min pitch is > to max pitch" );
            return NULL;
        }
        if ( argv[0].a_type != A_SYMBOL || argv[1].a_type != A_FLOAT ||
                argv[2].a_type != A_FLOAT || argv[3].a_type != A_FLOAT ||
                argv[4].a_type != A_FLOAT || argv[5].a_type != A_FLOAT ||
                argv[6].a_type != A_FLOAT || argv[7].a_type != A_FLOAT ||
                argv[8].a_type != A_FLOAT || argv[9].a_type != A_FLOAT )
        {
            post( "pianoroll : wrong arguments" );
            return NULL;
        }

        // update pianoroll count
        if (!strncmp((str = argv[0].a_w.w_symbol->s_name), "pianoroll", 9)
                && (zz = atoi(str + 9)) > pianorollcount)
        {
            // post( "pianoroll : already %d objects", pianorollcount );
            pianorollcount = zz;
        }
        x->x_name = argv[0].a_w.w_symbol;
        pd_bind(&x->x_obj.ob_pd, x->x_name);
        x->x_width = argv[1].a_w.w_float;
        x->x_height = argv[2].a_w.w_float;
        x->x_pmin = argv[3].a_w.w_float;
        x->x_pmax = argv[4].a_w.w_float;
        x->x_nbgrades = argv[5].a_w.w_float;
        x->x_nbsteps = argv[6].a_w.w_float;
        x->x_peaches = (t_float*) getbytes( x->x_nbsteps*sizeof(t_float) );
        x->x_ipeaches = (t_int*) getbytes( x->x_nbsteps*sizeof(t_int) );
        x->x_volumes = (t_float*) getbytes( x->x_nbsteps*sizeof(t_float) );
        x->x_ivolumes = (t_int*) getbytes( x->x_nbsteps*sizeof(t_int) );
        x->x_defvalue = argv[7].a_w.w_float;
        x->x_transpose = argv[8].a_w.w_float;
        x->x_save = argv[9].a_w.w_float;
    }
    else
    {
        char buf[40];

        sprintf(buf, "pianoroll%d", ++pianorollcount);
        s = gensym(buf);

        x->x_name = s;
        pd_bind(&x->x_obj.ob_pd, x->x_name);

        x->x_width = DEFAULT_SEQUENCER_WIDTH;
        x->x_height = DEFAULT_SEQUENCER_HEIGHT;
        x->x_pmin = DEFAULT_SEQUENCER_PITCH_MIN;
        x->x_pmax = DEFAULT_SEQUENCER_PITCH_MAX;
        x->x_nbgrades = DEFAULT_SEQUENCER_NBGRADES;
        x->x_nbsteps = DEFAULT_SEQUENCER_STEPS;
        x->x_peaches = (t_float*) getbytes( x->x_nbsteps*sizeof(t_float) );
        x->x_ipeaches = (t_int*) getbytes( x->x_nbsteps*sizeof(t_int) );
        x->x_volumes = (t_float*) getbytes( x->x_nbsteps*sizeof(t_float) );
        x->x_ivolumes = (t_int*) getbytes( x->x_nbsteps*sizeof(t_int) );
        x->x_defvalue = 0;
        x->x_transpose = 0;
        x->x_save = 1;

    }

    // common fields for new and restored pianorolls
    x->x_selected = 0;
    x->x_scurrent = -1;
    x->x_glist = (t_glist *) canvas_getcurrent();
    x->x_pitch = outlet_new(&x->x_obj, &s_float );
    x->x_volume = outlet_new(&x->x_obj, &s_float );
    // post( "pianoroll : argc : %d", argc );
    if ( ( argc != 0 ) && ( x->x_save ) )
    {
        int ai = 10;
        int si = 0;

        while ( ai < argc )
        {
            x->x_peaches[si] = argv[ai++].a_w.w_float;
            if ( ai >= argc ) break;
            x->x_volumes[si++] = argv[ai++].a_w.w_float;
        }
    }
    else // following arguments are the values of pitch, volumes
    {
        for ( si=0; si<x->x_nbsteps; si++ )
        {
            x->x_peaches[si] = x->x_defvalue;
            x->x_volumes[si] = 1.0;
        }
    }

    // post( "pianoroll_new name : %s width: %d height : %d", x->x_name->s_name, x->x_width, x->x_height );

    return (x);
}

static void pianoroll_free(t_pianoroll *x)
{
    // post( "pianoroll~: pianoroll_free" );
    if ( x->x_peaches )
    {
        freebytes( x->x_peaches, x->x_nbsteps*sizeof(t_float) );
    }
    if ( x->x_ipeaches )
    {
        freebytes( x->x_ipeaches, x->x_nbsteps*sizeof(t_int) );
    }
    if ( x->x_volumes )
    {
        freebytes( x->x_volumes, x->x_nbsteps*sizeof(t_float) );
    }
    if ( x->x_ivolumes )
    {
        freebytes( x->x_ivolumes, x->x_nbsteps*sizeof(t_int) );
    }
}

void pianoroll_setup(void)
{
    logpost(NULL, 4,  pianoroll_version );
    pianoroll_class = class_new(gensym("pianoroll"), (t_newmethod)pianoroll_new,
                                (t_method)pianoroll_free, sizeof(t_pianoroll), 0, A_GIMME, 0);
    class_addmethod(pianoroll_class, (t_method)pianoroll_float, &s_float, A_FLOAT, 0);
    class_addmethod(pianoroll_class, (t_method)pianoroll_transpose, gensym("transpose"),
                    A_FLOAT, 0);
    class_addmethod(pianoroll_class, (t_method)pianoroll_init, gensym("init"), 0);
    class_addmethod(pianoroll_class, (t_method)pianoroll_dialog, gensym("dialog"), A_GIMME, 0);
    class_addmethod(pianoroll_class, (t_method)pianoroll_save_file, gensym("save"), A_SYMBOL, 0);
    class_addmethod(pianoroll_class, (t_method)pianoroll_load, gensym("load"), A_SYMBOL, 0);
    class_addmethod(pianoroll_class, (t_method)pianoroll_pitch, gensym("pitch"), A_FLOAT, A_FLOAT, 0);
    class_addmethod(pianoroll_class, (t_method)pianoroll_volume, gensym("volume"), A_FLOAT, A_FLOAT, 0);


    pianoroll_widgetbehavior.w_getrectfn =    pianoroll_getrect;
    pianoroll_widgetbehavior.w_displacefn =   pianoroll_displace;
    pianoroll_widgetbehavior.w_selectfn =     pianoroll_select;
    pianoroll_widgetbehavior.w_activatefn =   NULL;
    pianoroll_widgetbehavior.w_deletefn =     pianoroll_delete;
    pianoroll_widgetbehavior.w_visfn =        pianoroll_vis;
    pianoroll_widgetbehavior.w_clickfn =      pianoroll_click;

#if PD_MINOR_VERSION >= 37
    class_setpropertiesfn(pianoroll_class, pianoroll_properties);
    class_setsavefn(pianoroll_class, pianoroll_save);
#else
    pianoroll_widgetbehavior.w_propertiesfn = pianoroll_properties;
    pianoroll_widgetbehavior.w_savefn =       pianoroll_save;
#endif

    class_setwidget(pianoroll_class, &pianoroll_widgetbehavior);

    sys_vgui("eval [read [open {%s/%s.tcl}]]\n",
             pianoroll_class->c_externdir->s_name,
             pianoroll_class->c_name->s_name);
}
