/*---------------------- scrolllist~ ------------------------------------------ */
/*                                                                              */
/* scrolllist~ : scrolling list of text items                                   */
/* constructor : scrolllist | scrolllist <capacity> <width> <height>            */
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
/* "I tried your cat's name, it tried your favorite band"                       */
/* "I have the password to your ... shell account"                              */
/* Barcelona - Shell Account                                                    */
/* ---------------------------------------------------------------------------- */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <ctype.h>
#include <regex.h>
#include <time.h>
#include <sys/time.h>
#include "m_pd.h"
#include "m_imp.h"
#include "g_canvas.h"


#ifdef _WIN32
#include <io.h>
#include <pthread.h>
#else
#include <unistd.h>
#include <sys/types.h>
#include <dirent.h>
#endif

t_widgetbehavior scrolllist_widgetbehavior;
static t_class *scrolllist_class;

static int guidebug=0;

static char   *scrolllist_version = "scrolllist: scrolling list of text items : version 0.3, written by Yves Degoyon (ydegoyon@free.fr)";

#define MIN(a,b) (a>b?b:a)

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
                         post(a,b,c,d,e,f,g,h,i );\
                         sys_vgui(a,b,c,d,e,f,g,h,i)

#define SYS_VGUI10(a,b,c,d,e,f,g,h,i,j) if (guidebug) \
                         post(a,b,c,d,e,f,g,h,i,j );\
                         sys_vgui(a,b,c,d,e,f,g,h,i,j)

#define SYS_VGUI11(a,b,c,d,e,f,g,h,i,j,k) if (guidebug) \
                         post(a,b,c,d,e,f,g,h,i,j,k );\
                         sys_vgui(a,b,c,d,e,f,g,h,i,j,k)

#define MAX_STRING_LENGTH 256

typedef struct _scrolllist
{
    t_object x_obj;
    t_glist *x_glist;
    t_outlet *x_item;           /* outlet to output current selected item    */
    t_int x_capacity;           /* number of text items                      */
    t_int x_height;             /* height of the scrolllist                  */
    t_int x_width;              /* width of the scrolllist                   */
    t_int x_itemselected;       /* index of the selected item                */
    t_int x_selected;           /* stores selected state                     */
    t_int x_graphics;           /* flag to draw graphics or not              */
    char **x_items;             /* text items                                */
    t_int x_nitems;             /* number of current items                   */
    t_int x_ndisplayed;         /* number of displayed items                 */
    t_int x_firstseen;          /* first displayed item                      */
    t_int x_lastseen;           /* last displayed item                       */
    t_int x_cdy;                /* cumulated y drag                          */
    char   *x_font;             /* font used for entries                     */
    t_int  x_charheight;        /* height of characters                      */
    t_int  x_charwidth;         /* width of characters                       */
    char   *x_bgcolor;          /* background color                          */
    char   *x_fgcolor;          /* foreground color                          */
    char   *x_secolor;          /* selection color                           */
} t_scrolllist;

static void scrolllist_erase(t_scrolllist *x, t_glist *glist)
{
    t_canvas *canvas=glist_getcanvas(glist);
    t_int i;

    // just in case we got confused
    if ( x->x_firstseen < 0 ) x->x_firstseen=0;
    if ( x->x_lastseen > x->x_capacity-1 ) x->x_lastseen=x->x_capacity-1;

    // delete previous entries
    for ( i=x->x_firstseen; i<=x->x_lastseen; i++ )
    {
        SYS_VGUI4(".x%lx.c delete %xITEM%d\n", canvas, x, i);
    }
}

static void scrolllist_update(t_scrolllist *x, t_glist *glist)
{
    t_canvas *canvas=glist_getcanvas(glist);
    t_int i;

    // just in case we got confused
    if ( x->x_firstseen < 0 ) x->x_firstseen=0;
    if ( x->x_lastseen > x->x_capacity-1 ) x->x_lastseen=x->x_capacity-1;

    // display the content of text items
    for ( i=x->x_firstseen; i<=x->x_lastseen; i++ )
    {
        // display the entry if displayable
        if ( ( (i-x->x_firstseen)*x->x_charheight < x->x_height ) && ( x->x_items[i] != NULL ) )
        {
            SYS_VGUI11(".x%lx.c create text %d %d -fill %s -activefill %s -width %d -text \"%s\" -anchor w -font {%s} -tags %xITEM%d\n",
                       canvas,
                       text_xpix(&x->x_obj, glist)+5,
                       text_ypix(&x->x_obj, glist)+5+(i-x->x_firstseen)*x->x_charheight,
                       x->x_fgcolor,
                       x->x_secolor,
                       x->x_width,
                       x->x_items[i],
                       x->x_font,
                       x, i );
        }
        if ( ( x->x_itemselected >= x->x_firstseen ) && ( x->x_itemselected <= x->x_lastseen ) )
        {
            SYS_VGUI5(".x%lx.c itemconfigure %xITEM%d -fill %s\n",
                      canvas, x, x->x_itemselected, x->x_secolor);
        }
    }
}

static void scrolllist_output_current(t_scrolllist* x)
{
    if ( x->x_items && x->x_itemselected < x->x_nitems && x->x_itemselected >= 0 )
    {
        outlet_symbol( x->x_item, gensym( x->x_items[x->x_itemselected] ) );
    }
}

static void scrolllist_draw_new(t_scrolllist *x, t_glist *glist)
{
    t_canvas *canvas=glist_getcanvas(glist);

    x->x_glist = glist;
    if ( x->x_graphics )
    {
        SYS_VGUI8(".x%lx.c create rectangle %d %d %d %d -fill %s -tags %xTEXTLIST\n",
                  canvas, text_xpix(&x->x_obj, glist), text_ypix(&x->x_obj, glist),
                  text_xpix(&x->x_obj, glist) + x->x_width, text_ypix(&x->x_obj, glist) + x->x_height,
                  x->x_bgcolor, x);
    }
    else
    {
        SYS_VGUI7(".x%lx.c create rectangle %d %d %d %d -outline white -fill white -tags %xTEXTLIST\n",
                  canvas, text_xpix(&x->x_obj, glist), text_ypix(&x->x_obj, glist),
                  text_xpix(&x->x_obj, glist) + x->x_width, text_ypix(&x->x_obj, glist) + x->x_height, x);
    }
    scrolllist_erase( x, glist );
    scrolllist_update( x, glist );
}

static void scrolllist_draw_move(t_scrolllist *x, t_glist *glist)
{
    t_canvas *canvas=glist_getcanvas(glist);
    t_int i;
    struct timespec tv;

    tv.tv_sec = 0;
    tv.tv_nsec = 10000000;

    SYS_VGUI7(".x%lx.c coords %xTEXTLIST %d %d %d %d\n",
              canvas, x,
              text_xpix(&x->x_obj, glist), text_ypix(&x->x_obj, glist),
              text_xpix(&x->x_obj, glist)+x->x_width,
              text_ypix(&x->x_obj, glist)+x->x_height);
    for ( i=x->x_firstseen; i<=x->x_lastseen; i++ )
    {
        // nanosleep( &tv, NULL );
        SYS_VGUI6(".x%lx.c coords %xITEM%d %d %d\n",
                  canvas, x, i,
                  text_xpix(&x->x_obj, glist)+5,
                  text_ypix(&x->x_obj, glist)+5+(i-x->x_firstseen)*x->x_charheight);
    }

    canvas_fixlinesfor( canvas, (t_text*)x );
}

static void scrolllist_draw_erase(t_scrolllist* x, t_glist* glist)
{
    t_canvas *canvas=glist_getcanvas(glist);
    t_int i;

    SYS_VGUI3(".x%lx.c delete %xTEXTLIST\n", canvas, x);
    for ( i=x->x_firstseen; i<=x->x_lastseen; i++ )
    {
        SYS_VGUI4(".x%lx.c delete %xITEM%d\n", canvas, x, i);
    }
}

static void scrolllist_draw_select(t_scrolllist* x, t_glist* glist)
{
    t_canvas *canvas=glist_getcanvas(glist);

    // post( "scrolllist : select" );
    if(x->x_selected)
    {
        /* sets the item in blue */
        SYS_VGUI3(".x%lx.c itemconfigure %xTEXTLIST -outline #0000FF\n", canvas, x);
    }
    else
    {
        SYS_VGUI3(".x%lx.c itemconfigure %xTEXTLIST -outline #000000\n", canvas, x);
    }
}

/* ------------------------ scrolllist widgetbehaviour----------------------------- */


static void scrolllist_getrect(t_gobj *z, t_glist *owner,
                               int *xp1, int *yp1, int *xp2, int *yp2)
{
    t_scrolllist* x = (t_scrolllist*)z;

    *xp1 = text_xpix(&x->x_obj, owner);
    *yp1 = text_ypix(&x->x_obj, owner);
    *xp2 = text_xpix(&x->x_obj, owner)+x->x_width;
    *yp2 = text_ypix(&x->x_obj, owner)+x->x_height;
}

static void scrolllist_save(t_gobj *z, t_binbuf *b)
{
    t_scrolllist *x = (t_scrolllist *)z;

    // post( "saving scrolllist : %d", x->x_capacity );
    binbuf_addv(b, "ssiisiiissss", gensym("#X"), gensym("obj"),
                (t_int)x->x_obj.te_xpix, (t_int)x->x_obj.te_ypix,
                atom_getsymbol(binbuf_getvec(x->x_obj.te_binbuf)),
                x->x_capacity, x->x_width, x->x_height,
                gensym(x->x_font), gensym(x->x_bgcolor),
                gensym(x->x_fgcolor), gensym(x->x_secolor) );
    binbuf_addv(b, ";");
}

static void scrolllist_select(t_gobj *z, t_glist *glist, int selected)
{
    t_scrolllist *x = (t_scrolllist *)z;

    x->x_selected = selected;

    scrolllist_draw_select( x, glist );
}

static void scrolllist_vis(t_gobj *z, t_glist *glist, int vis)
{
    t_scrolllist *x = (t_scrolllist *)z;
    t_rtext *y;

    // post( "scrolllist : vis (%d)", vis );
    x->x_glist = glist;
    if (vis)
    {
        scrolllist_draw_erase(x, x->x_glist);
        scrolllist_draw_new( x, glist );
    }
    else
    {
        scrolllist_draw_erase( x, glist );
    }
}

static void scrolllist_deleteobj(t_gobj *z, t_glist *glist)
{
    t_scrolllist *x = (t_scrolllist *)z;

    canvas_deletelinesfor(glist, (t_text *)z);
}

static void scrolllist_displace(t_gobj *z, t_glist *glist, int dx, int dy)
{
    t_scrolllist *x = (t_scrolllist *)z;
    t_int xold = text_xpix(&x->x_obj, glist);
    t_int yold = text_ypix(&x->x_obj, glist);

    // post( "scrolllist_displace dx=%d dy=%d", dx, dy );

    x->x_obj.te_xpix += dx;
    x->x_obj.te_ypix += dy;
    if(xold != text_xpix(&x->x_obj, glist) || yold != text_ypix(&x->x_obj, glist))
    {
        scrolllist_draw_move(x, glist);
    }
}

static void scrolllist_motion(t_scrolllist *x, t_floatarg dx, t_floatarg dy)
{
    t_int i;
    t_canvas *canvas=glist_getcanvas(x->x_glist);

    x->x_cdy+=dy;
    scrolllist_erase( x, x->x_glist );

    // check if we need to scroll
    // eventually, move down
    if ( x->x_cdy >= x->x_charheight )
    {
        if ( x->x_firstseen < x->x_nitems - x->x_ndisplayed )
        {
            x->x_firstseen++;
            x->x_lastseen++;
            // post( "scrolllist : moved down first=%d last=%d", x->x_firstseen, x->x_lastseen );
        }
    }
    // eventually, move up
    if ( x->x_cdy <= -x->x_charheight )
    {
        if ( x->x_firstseen-1 >= 0 )
        {
            x->x_firstseen--;
            x->x_lastseen--;
            // post( "scrolllist : moved up first=%d last=%d", x->x_firstseen, x->x_lastseen );
        }
    }
    scrolllist_update(x, x->x_glist);
    if ( ( x->x_cdy >= x->x_charheight ) || ( x->x_cdy <= -x->x_charheight ) ) x->x_cdy = 0;
}

static void scrolllist_scroll(t_scrolllist *x, t_floatarg fdy)
{
    t_int nbsteps, si;

    x->x_cdy += (t_int)fdy;
    nbsteps = (t_int)abs(x->x_cdy/x->x_charheight);
    // post( "scrolllist : iterations %d", nbsteps );
    for (si=0; si<nbsteps; si++ )
    {
        scrolllist_motion(x, 0, (fdy/abs(fdy))*x->x_charheight);
    }
}

static void scrolllist_graphics(t_scrolllist *x, t_floatarg fgraphics)
{
    if ( ( (t_int)fgraphics == 0 ) || ( (t_int)fgraphics == 1 ) )
    {
        scrolllist_draw_erase(x, x->x_glist);
        x->x_graphics = (t_int) fgraphics;
        scrolllist_draw_new(x, x->x_glist);
    }
}

static int scrolllist_click(t_gobj *z, struct _glist *glist,
                            int xpix, int ypix, int shift, int alt, int dbl, int doit)
{
    t_scrolllist* x = (t_scrolllist *)z;
    t_canvas *canvas=glist_getcanvas(x->x_glist);
    t_int xoffset;

    if (doit)
    {
        // deselect previously selected item
        SYS_VGUI5(".x%lx.c itemconfigure %xITEM%d -fill %s\n",
                  canvas, x, x->x_itemselected, x->x_fgcolor);
        x->x_itemselected = x->x_firstseen + (ypix-text_ypix(&x->x_obj, glist))/x->x_charheight;
        SYS_VGUI5(".x%lx.c itemconfigure %xITEM%d -fill %s\n",
                  canvas, x, x->x_itemselected, x->x_secolor);
        // post( "scrolllist : selected item : %d", x->x_itemselected );
        if ( x->x_items && ( x->x_itemselected < x->x_nitems ) )
        {
            xoffset=(xpix-text_xpix(&x->x_obj, glist));
            if ( xoffset <= (t_int)( x->x_width*4/5 ) )
            {
                scrolllist_output_current(x);
                scrolllist_erase( x, x->x_glist );
                scrolllist_update( x, glist );
            }
            else
            {
                x->x_itemselected=-1;
            }
        }
        x->x_glist = glist;
        glist_grab( glist, &x->x_obj.te_g, (t_glistmotionfn)scrolllist_motion,
                    NULL, xpix, ypix );
    }
    return (1);
}

static void scrolllist_properties(t_gobj *z, t_glist *owner)
{
    char buf[800];
    t_scrolllist *x=(t_scrolllist *)z;

    sprintf(buf, "pdtk_scrolllist_dialog %%s %d %d %d %s %s %s %s\n",
            (int)x->x_capacity, (int)x->x_width, (int)x->x_height,
            x->x_font, x->x_bgcolor,
            x->x_fgcolor, x->x_secolor );
    // post("scrolllist_properties : %s", buf );
    gfxstub_new(&x->x_obj.ob_pd, x, buf);
}

static void scrolllist_dialog(t_scrolllist *x, t_symbol *s, int argc, t_atom *argv)
{
    char **titems;
    t_int ncapacity, i, ccapacity;

    scrolllist_erase( x, x->x_glist );
    scrolllist_draw_erase(x, x->x_glist);

    if ( !x )
    {
        post( "scrolllist : error :tried to set properties on an unexisting object" );
    }
    if ( argc != 9 )
    {
        post( "scrolllist : error in the number of arguments ( %d instead of 10 )", argc );
        return;
    }
    if ( argv[0].a_type != A_FLOAT  || argv[1].a_type != A_FLOAT ||
            argv[2].a_type != A_FLOAT  || argv[3].a_type != A_SYMBOL ||
            argv[4].a_type != A_FLOAT  || argv[5].a_type != A_SYMBOL ||
            argv[6].a_type != A_SYMBOL || argv[7].a_type != A_SYMBOL ||
            argv[8].a_type != A_SYMBOL )
    {
        post( "scrolllist : wrong arguments" );
        return;
    }

    ncapacity = (t_int)argv[0].a_w.w_float;
    titems = (char**) malloc( ncapacity*sizeof(char*) );

    if ( ncapacity < x->x_nitems )
    {
        post( "scrolllist : new size is too small : texts lost !!" );
        ccapacity = ncapacity;
    }
    else
    {
        ccapacity = x->x_nitems;
    }
    for ( i=0; i<ccapacity; i++ )
    {
        if ( x->x_items[i] != NULL )
        {
            titems[i] = (char*) malloc( strlen( x->x_items[i] ) + 1 );
            memcpy( titems[i], x->x_items[i], strlen( x->x_items[i] ) );
            titems[i][strlen( x->x_items[i] )]='\0';
            free( x->x_items[i] );
            x->x_items[i] = NULL;
        }
    }
    if ( x->x_items )
    {
        free( x->x_items );
        x->x_items = NULL;
    }
    x->x_items = titems;
    x->x_nitems = ccapacity;
    x->x_capacity = ncapacity;

    x->x_width = (int)argv[1].a_w.w_float;
    x->x_height = (int)argv[2].a_w.w_float;
    sprintf( x->x_font, "{%s %d %s}", argv[3].a_w.w_symbol->s_name,
             (int)argv[4].a_w.w_float, argv[5].a_w.w_symbol->s_name );
    x->x_charheight = (t_int)argv[4].a_w.w_float;
    strcpy( x->x_bgcolor, argv[6].a_w.w_symbol->s_name );
    strcpy( x->x_fgcolor, argv[7].a_w.w_symbol->s_name );
    strcpy( x->x_secolor, argv[8].a_w.w_symbol->s_name );

    x->x_firstseen = 0;
    x->x_ndisplayed = (t_int)(x->x_height/x->x_charheight );
    if ( x->x_nitems >= x->x_ndisplayed )
    {
        x->x_lastseen = x->x_ndisplayed-1;
    }
    else
    {
        x->x_lastseen = x->x_nitems-1;
    }

    scrolllist_draw_new(x, x->x_glist);
    scrolllist_update(x, x->x_glist);
}

static void scrolllist_bgcolor(t_scrolllist *x, t_symbol *s)
{
    scrolllist_erase( x, x->x_glist );
    scrolllist_draw_erase(x, x->x_glist);

    strcpy( x->x_bgcolor, s->s_name );

    scrolllist_draw_new(x, x->x_glist);
    scrolllist_update(x, x->x_glist);
}

static void scrolllist_fgcolor(t_scrolllist *x, t_symbol *s)
{
    scrolllist_erase( x, x->x_glist );
    scrolllist_draw_erase(x, x->x_glist);

    strcpy( x->x_fgcolor, s->s_name );

    scrolllist_draw_new(x, x->x_glist);
    scrolllist_update(x, x->x_glist);
}

static void scrolllist_secolor(t_scrolllist *x, t_symbol *s)
{
    scrolllist_erase( x, x->x_glist );
    scrolllist_draw_erase(x, x->x_glist);

    strcpy( x->x_secolor, s->s_name );

    scrolllist_draw_new(x, x->x_glist);
    scrolllist_update(x, x->x_glist);
}

static void scrolllist_sort(t_scrolllist* x)
{
    char **titems;
    t_int i, j, k, irank, indest;

    scrolllist_erase( x, x->x_glist );

    // sort text items
    titems = (char**) malloc( x->x_capacity*sizeof(char*) );
    for ( i=0; i<x->x_capacity; i++ )
    {
        titems[i]=NULL;
    }
    indest=0;
    for ( i=0; i<x->x_nitems; i++ )
    {
        if ( x->x_items[i] != NULL )
        {
            irank=0;
            for ( j=0; j<x->x_capacity; j++ )
            {
                if ( titems[j] != NULL )
                {
                    // post( "scrollist : comparing >%s< to >%s<", titems[j], x->x_items[i] );
                    if ( strcasecmp( titems[j], x->x_items[i] ) > 0 )
                    {
                        irank=j;
                        break;
                    }
                    irank=j+1;
                }
            }
            // insert in irank
            // post( "scrollist : inserting %s at %d", x->x_items[i], irank );
            for ( k=indest-1; k>=irank; k-- )
            {
                if ( (k+1) < x->x_capacity )
                {
                    if ( titems[k+1] != NULL )
                    {
                        free( titems[k+1] );
                        titems[k+1] = NULL;
                    }
                    titems[k+1] = (char*) malloc( strlen( titems[k] ) + 1 );
                    memcpy( titems[k+1], titems[k], strlen( titems[k] ) );
                    titems[k+1][strlen( titems[k] )]='\0';
                    // post( "scrollist : copying %d to %d", k, k+1 );
                }
            }
            titems[irank] = (char*) malloc( strlen( x->x_items[i] ) + 1 );
            memcpy( titems[irank], x->x_items[i], strlen( x->x_items[i] ) );
            titems[irank][strlen( x->x_items[i] )]='\0';
            free( x->x_items[i] );
            x->x_items[i]=NULL;
            indest++;
        }
    }
    if ( x->x_items )
    {
        free( x->x_items );
        x->x_items = NULL;
    }
    x->x_items = titems;

    scrolllist_update( x, x->x_glist );
}

static void scrolllist_font(t_scrolllist* x, t_symbol *fname, t_symbol *fcase, t_floatarg fsize)
{
    if ( (t_int)fsize <= 4 )
    {
        post( "scrolllist : wrong font size in font message : %d", (t_int)fsize );
        return;
    }
    sprintf( x->x_font, "{%s %d %s}", fname->s_name, (int)fsize, fcase->s_name );
    x->x_charheight = (t_int)fsize;
    x->x_charwidth = (2*x->x_charheight)/3;
    // post( "scrolllist : setting font to : %s", x->x_font );
    scrolllist_erase( x, x->x_glist );
    scrolllist_update( x, x->x_glist );
}

static void scrolllist_add(t_scrolllist* x, t_symbol *fnewtext)
{
    t_int i;

    // post( "scrollist : add : nitems = %d", x->x_nitems );
    if ( x->x_nitems >= x->x_capacity )
    {
        // post( "scrolllist : warning : list is full, erasing first line" );
        for ( i=0; i<(x->x_nitems-1); i++ )
        {
            if ( x->x_items[i] != NULL )
            {
                free( x->x_items[i] );
                x->x_items[i] = NULL;
            }
            x->x_items[i] = (char*) malloc( strlen( x->x_items[i+1] ) + 1 );
            memcpy( x->x_items[i], x->x_items[i+1], strlen( x->x_items[i+1] ) );
            x->x_items[i][strlen( x->x_items[i+1] )]='\0';
            // post( "scrollist : copying %d to %d", i+1, i );
        }
        x->x_items[x->x_nitems-1] = (char*) malloc( strlen( fnewtext->s_name ) + 1 );
        memcpy( x->x_items[x->x_nitems-1], fnewtext->s_name, strlen( fnewtext->s_name ) );
        x->x_items[x->x_nitems-1][strlen( fnewtext->s_name )]='\0';
    }
    else
    {
        // post( "scrolllist : item #%d : %x", x->x_nitems, &x->x_items[x->x_nitems] );
        // post( "scrolllist : allocating : %d", strlen( fnewtext->s_name ) + 1 );
        x->x_items[x->x_nitems] = (char*) malloc( strlen( fnewtext->s_name ) + 1 );
        memcpy( x->x_items[x->x_nitems], fnewtext->s_name, strlen( fnewtext->s_name ) );
        x->x_items[x->x_nitems][strlen( fnewtext->s_name )]='\0';
        x->x_nitems++;
        if ( (x->x_nitems-x->x_firstseen)*x->x_charheight+5 < x->x_height )
        {
            x->x_lastseen = x->x_nitems-1;
        }
    }
    scrolllist_erase( x, x->x_glist );
    scrolllist_update( x, x->x_glist );
}

static void scrolllist_insert(t_scrolllist* x, t_symbol *ftext, t_floatarg frank)
{
    t_int rank, i;

    if ( (t_int)frank > x->x_capacity )
    {
        post( "scrolllist : error : incorrect rank in insert message (%d), over capacity", (t_int)frank );
        return;
    }
    rank = (t_int)frank-1;
    if ( rank < 0 )
    {
        rank=0;
    }
    if ( rank > x->x_nitems-1 )
    {
        rank=x->x_nitems-1;
    }
    for ( i=x->x_nitems-1; i>=rank; i-- )
    {
        if ( (i+1) < x->x_capacity )
        {
            if ( x->x_items[i+1] != NULL )
            {
                free( x->x_items[i+1] );
                x->x_items[i+1] = NULL;
            }
            x->x_items[i+1] = (char*) malloc( strlen( x->x_items[i] ) + 1 );
            memcpy( x->x_items[i+1], x->x_items[i], strlen( x->x_items[i] ) );
            x->x_items[i+1][strlen( x->x_items[i] )]='\0';
            // post( "scrollist : copying %d to %d", i, i+1 );
        }
    }
    // post( "scrollist : inserting at %d", rank );
    x->x_items[rank] = (char*) malloc( strlen( ftext->s_name ) + 1 );
    memcpy( x->x_items[rank], ftext->s_name, strlen( ftext->s_name ) );
    x->x_items[rank][strlen( ftext->s_name )]='\0';
    if ( x->x_nitems < x->x_capacity ) x->x_nitems++;
    if ( (x->x_nitems-x->x_firstseen)*x->x_charheight+5 < x->x_height )
    {
        x->x_lastseen = x->x_nitems-1;
    }
    scrolllist_erase( x, x->x_glist );
    scrolllist_update( x, x->x_glist );
}

static void scrolllist_replace(t_scrolllist* x, t_symbol *ftext, t_floatarg frank)
{
    t_int rank;

    if ( ( (t_int)frank <= 0 ) || ( (t_int)frank > x->x_nitems ) )
    {
        post( "scrolllist : error : incorrect rank in replace message (%d), no such text", (t_int)frank );
        return;
    }

    rank = (t_int) frank-1;
    if ( x->x_items[rank] != NULL )
    {
        free( x->x_items[rank] );
        x->x_items[rank] = NULL;
    }
    x->x_items[rank] = (char*) malloc( strlen( ftext->s_name ) + 1 );
    memcpy( x->x_items[rank], ftext->s_name, strlen( ftext->s_name ) );
    x->x_items[rank][strlen( ftext->s_name )]='\0';
    scrolllist_erase( x, x->x_glist );
    scrolllist_update( x, x->x_glist );
}

static void scrolllist_delete(t_scrolllist* x, t_floatarg frank)
{
    t_int rank, i;

    if ( ( (t_int)frank <= 0 ) || ( (t_int)frank > x->x_nitems ) )
    {
        post( "scrolllist : error : incorrect rank in delete message (%d), no such text (%d)", (t_int)frank, x->x_nitems );
        return;
    }

    rank = (t_int) frank-1;
    for ( i=rank; i<x->x_nitems-1; i++ )
    {
        if ( x->x_items[i] != NULL )
        {
            free( x->x_items[i] );
            x->x_items[i] = NULL;
        }
        x->x_items[i] = (char*) malloc( strlen( x->x_items[i+1] ) + 1 );
        memcpy( x->x_items[i], x->x_items[i+1], strlen( x->x_items[i+1] ) );
        x->x_items[i][strlen( x->x_items[i+1] )]='\0';
        // post( "scrollist : copying %d to %d", i+1, i );
    }
    free( x->x_items[x->x_nitems-1] );
    x->x_items[x->x_nitems-1] = NULL;
    if ( x->x_lastseen == x->x_nitems-2 ) x->x_lastseen--;
    x->x_nitems--;
    scrolllist_erase( x, x->x_glist );
    scrolllist_update( x, x->x_glist );
}

static void scrolllist_clear(t_scrolllist* x)
{
    t_int i;

    scrolllist_erase( x, x->x_glist );
    for ( i=0; i<x->x_capacity; i++ )
    {
        x->x_items[i]=NULL;
    }
    x->x_nitems = 0;
    x->x_selected = 0;
    x->x_itemselected = -1;
    x->x_firstseen = 0;
    x->x_lastseen = -1;
    scrolllist_update( x, x->x_glist );
}

static void scrolllist_seek(t_scrolllist *x, t_floatarg fseeked)
{
    t_int iout=0;
    t_canvas *canvas=glist_getcanvas(x->x_glist);

    if ( fseeked < 0 )
    {
        post( "scrolllist : wrong searched file : %f", fseeked );
        return;
    }
    if ( x->x_nitems == 0 ) return;

    iout = (t_int)fseeked % (x->x_nitems);
    SYS_VGUI5(".x%lx.c itemconfigure %xITEM%d -fill %s\n", canvas, x, x->x_itemselected, x->x_fgcolor);
    x->x_itemselected = iout;
    SYS_VGUI5(".x%lx.c itemconfigure %xITEM%d -fill %s\n", canvas, x, x->x_itemselected, x->x_secolor);
    scrolllist_output_current(x);
}

static t_scrolllist *scrolllist_new(t_symbol *s, int argc, t_atom *argv )
{
    t_int i, argoffset=0;
    t_scrolllist *x;

    x = (t_scrolllist *)pd_new(scrolllist_class);

    x->x_capacity = 100;
    x->x_width = 400;
    x->x_height = 200;
    x->x_font = ( char * ) malloc( MAX_STRING_LENGTH );
    sprintf( x->x_font, "{Helvetica 10 bold}" );
    x->x_charheight = 10;
    x->x_charwidth = (2*10)/3;
    x->x_bgcolor = ( char * ) malloc( MAX_STRING_LENGTH );
    sprintf( x->x_bgcolor, "#457782" );
    x->x_fgcolor = ( char * ) malloc( MAX_STRING_LENGTH );
    sprintf( x->x_fgcolor, "black" );
    x->x_secolor = ( char * ) malloc( MAX_STRING_LENGTH );
    sprintf( x->x_secolor, "red" );

    if ( argc >= 1 )
    {
        if ( argv[0].a_type != A_FLOAT )
        {
            error( "scrolllist : wrong argument (capacity : 1)" );
            return NULL;
        }
        x->x_capacity = (int)argv[0].a_w.w_float;
    }
    if ( argc >= 2 )
    {
        if ( argv[1].a_type != A_FLOAT )
        {
            error( "scrolllist : wrong argument (width : 2)" );
            return NULL;
        }
        if ( (int)argv[1].a_w.w_float <= 0 )
        {
            error( "scrolllist : wrong width (%d)", (t_int)(int)argv[1].a_w.w_float );
            error( "scrolllist : usage : scrolllist <capacity> <width> <height>" );
            return NULL;
        }
        x->x_width = (int)argv[1].a_w.w_float;
    }
    if ( argc >= 3 )
    {
        if ( argv[2].a_type != A_FLOAT )
        {
            error( "scrolllist : wrong argument (height : 3)" );
            return NULL;
        }
        if ( (int)argv[2].a_w.w_float <= 0 )
        {
            error( "scrolllist : wrong height (%d)", (t_int)(int)argv[2].a_w.w_float );
            error( "scrolllist : usage : scrolllist <capacity> <width> <height>" );
            return NULL;
        }
        x->x_height = (int)argv[2].a_w.w_float;
    }
    if ( argc >= 6 )
    {
        if ( argv[3].a_type != A_SYMBOL ||
                argv[5].a_type != A_SYMBOL )
        {
            error( "scrolllist : wrong arguments (font : 4,6)" );
            error( "argument types : %d %d", argv[3].a_type, argv[5].a_type );
            return NULL;
        }
        if ( argv[4].a_type != A_SYMBOL &&
                argv[4].a_type != A_FLOAT )
        {
            error( "scrolllist : wrong arguments (font size : 5)" );
            error( "argument types : %d", argv[4].a_type );
            return NULL;
        }
        if ( argv[4].a_type == A_SYMBOL )
        {
            sprintf( x->x_font, "%s", argv[3].a_w.w_symbol->s_name );
            x->x_charheight = (t_int)atoi( strstr( argv[3].a_w.w_symbol->s_name, " ") );
            argoffset=2;
        }
        if ( argv[4].a_type == A_FLOAT )
        {
            x->x_charheight = (t_int)argv[4].a_w.w_float;
            sprintf( x->x_font, "%s %d %s", argv[3].a_w.w_symbol->s_name,
                     (int)x->x_charheight, argv[5].a_w.w_symbol->s_name );
            argoffset=0;
        }
        post( "scrolllist : font : %s, size : %d", x->x_font, x->x_charheight );
    }
    if ( argc >= 7-argoffset )
    {
        if ( argv[6-argoffset].a_type != A_SYMBOL )
        {
            error( "scrolllist : wrong arguments (background color : %d)", 7-argoffset );
            return NULL;
        }
        strcpy( x->x_bgcolor, argv[6-argoffset].a_w.w_symbol->s_name );
    }
    if ( argc >= 8-argoffset )
    {
        if ( argv[7-argoffset].a_type != A_SYMBOL )
        {
            error( "scrolllist : wrong arguments (foreground color : %d)", 8-argoffset );
            return NULL;
        }
        strcpy( x->x_fgcolor, argv[7-argoffset].a_w.w_symbol->s_name );
    }
    if ( argc >= 9-argoffset )
    {
        if ( argv[8-argoffset].a_type != A_SYMBOL )
        {
            error( "scrolllist : wrong arguments (selection color : %d)", 9-argoffset );
            return NULL;
        }
        strcpy( x->x_secolor, argv[8-argoffset].a_w.w_symbol->s_name );
    }

    x->x_item = outlet_new(&x->x_obj, &s_symbol );

    x->x_glist = (t_glist *) canvas_getcurrent();
    x->x_nitems = 0;
    x->x_items = (char **) malloc( x->x_capacity*sizeof(char*) );
    for ( i=0; i<x->x_capacity; i++ )
    {
        x->x_items[i]=NULL;
    }

    x->x_selected = 0;
    x->x_itemselected = -1;
    x->x_firstseen = 0;
    x->x_lastseen = -1;
    x->x_ndisplayed = (t_int)(x->x_height/x->x_charheight );

    x->x_graphics = 1;

    post( "scrolllist : capacity=%d width=%d height=%d", x->x_capacity, x->x_width, x->x_height );

    return (x);
}

static void scrolllist_free(t_scrolllist *x)
{
    t_int i;

    // post( "scrolllist : scrolllist_free" );

    // free text items list
    if ( x->x_nitems )
    {
        for ( i=0; i<x->x_nitems; i++ )
        {
            if ( x->x_items[i] != NULL )
            {
                // post( "scrolllist : freeing entry %d size=%d : %s", i, strlen( x->x_items[i] ) + 1, x->x_items[i] );
                free( x->x_items[i] );
            }
        }
    }
    free( x->x_items );
    if ( x->x_font )
    {
        free( x->x_font );
    }
    if ( x->x_bgcolor )
    {
        free( x->x_bgcolor );
    }
    if ( x->x_fgcolor )
    {
        free( x->x_fgcolor );
    }
    if ( x->x_secolor )
    {
        free( x->x_secolor );
    }
}

void scrolllist_setup(void)
{
    logpost(NULL, 4,  scrolllist_version );
    scrolllist_class = class_new(gensym("scrolllist"), (t_newmethod)scrolllist_new,
                                 (t_method)scrolllist_free, sizeof(t_scrolllist),
                                 CLASS_DEFAULT, A_GIMME, 0);
    class_addmethod(scrolllist_class, (t_method)scrolllist_seek, gensym("seek"), A_DEFFLOAT, A_NULL );
    class_addmethod(scrolllist_class, (t_method)scrolllist_dialog, gensym("dialog"), A_GIMME, A_NULL );
    class_addmethod(scrolllist_class, (t_method)scrolllist_sort, gensym("sort"), A_NULL );
    class_addmethod(scrolllist_class, (t_method)scrolllist_graphics, gensym("graphics"), A_DEFFLOAT, A_NULL );
    class_addmethod(scrolllist_class, (t_method)scrolllist_scroll, gensym("scroll"), A_DEFFLOAT, A_NULL );
    class_addmethod(scrolllist_class, (t_method)scrolllist_font, gensym("font"), A_SYMBOL, A_SYMBOL, A_DEFFLOAT, A_NULL );
    class_addmethod(scrolllist_class, (t_method)scrolllist_insert, gensym("insert"), A_SYMBOL, A_DEFFLOAT, A_NULL );
    class_addmethod(scrolllist_class, (t_method)scrolllist_replace, gensym("replace"), A_SYMBOL, A_DEFFLOAT, A_NULL );
    class_addmethod(scrolllist_class, (t_method)scrolllist_add, gensym("add"), A_SYMBOL, A_NULL );
    class_addmethod(scrolllist_class, (t_method)scrolllist_bgcolor, gensym("bgcolor"), A_SYMBOL, A_NULL );
    class_addmethod(scrolllist_class, (t_method)scrolllist_fgcolor, gensym("fgcolor"), A_SYMBOL, A_NULL );
    class_addmethod(scrolllist_class, (t_method)scrolllist_secolor, gensym("secolor"), A_SYMBOL, A_NULL );
    class_addmethod(scrolllist_class, (t_method)scrolllist_delete, gensym("delete"), A_DEFFLOAT, A_NULL );
    class_addmethod(scrolllist_class, (t_method)scrolllist_clear, gensym("clear"), A_NULL );

    scrolllist_widgetbehavior.w_getrectfn =    scrolllist_getrect;
    scrolllist_widgetbehavior.w_displacefn =   scrolllist_displace;
    scrolllist_widgetbehavior.w_selectfn =     scrolllist_select;
    scrolllist_widgetbehavior.w_activatefn =   NULL;
    scrolllist_widgetbehavior.w_deletefn =     scrolllist_deleteobj;
    scrolllist_widgetbehavior.w_visfn =        scrolllist_vis;
    scrolllist_widgetbehavior.w_clickfn =      scrolllist_click;

#if PD_MINOR_VERSION >= 37
    class_setpropertiesfn(scrolllist_class, scrolllist_properties);
    class_setsavefn(scrolllist_class, scrolllist_save);
#else
    scrolllist_widgetbehavior.w_propertiesfn = scrolllist_properties;
    scrolllist_widgetbehavior.w_savefn =       scrolllist_save;
#endif

    class_setwidget(scrolllist_class, &scrolllist_widgetbehavior);

    sys_vgui("eval [read [open {%s/%s.tcl}]]\n",
             scrolllist_class->c_externdir->s_name,
             scrolllist_class->c_name->s_name);
}
