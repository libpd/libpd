/*------------------------ playlist~ ------------------------------------------ */
/*                                                                              */
/* playlist~ : lets you choose a file with 1 click                              */
/*             or by sending a 'seek #' message                                 */
/* constructor : playlist <extension> <width> <height>                          */
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
/* "If a man's made of blood and iron"                                          */
/* "Doctor, doctor, what's in my chest ????"                                    */
/* Gang Of Four -- Guns Before Butter                                           */
/* ---------------------------------------------------------------------------- */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <ctype.h>
#include <regex.h>
#include <time.h>
#include <sys/time.h>
#include <dirent.h>
#include "m_pd.h"
#include "m_imp.h"
#include "g_canvas.h"
#include <sys/types.h>

#ifdef _WIN32
#include <io.h>
#include <pthread.h>
#include <direct.h>

int scandir(const char *dir, struct dirent ***namelist,
            int (*select)(const struct dirent *),
            int (*compar)(const struct dirent **, const struct dirent **))
{
    DIR *d;
    struct dirent *entry;
    register int i=0;
    size_t entrysize;

    if ((d=opendir(dir)) == NULL)
        return(-1);

    *namelist=NULL;
    while ((entry=readdir(d)) != NULL)
    {
        if (select == NULL || (select != NULL && (*select)(entry)))
        {
            *namelist=(struct dirent **)realloc((void *)(*namelist),
                                                (size_t)((i+1)*sizeof(struct dirent *)));
            if (*namelist == NULL) return(-1);
            entrysize=sizeof(struct dirent)-sizeof(entry->d_name)+strlen(entry->d_name)+1;
            (*namelist)[i]=(struct dirent *)malloc(entrysize);
            if ((*namelist)[i] == NULL) return(-1);
            memcpy((*namelist)[i], entry, entrysize);
            i++;
        }
    }
    if (closedir(d)) return(-1);
    if (i == 0) return(-1);
    if (compar != NULL)
        qsort((void *)(*namelist), (size_t)i, sizeof(struct dirent *), compar);

    return(i);
}

int alphasort(const struct dirent **a, const struct dirent **b)
{
    return(strcmp((*a)->d_name, (*b)->d_name));
}

int scandir(const char *, struct dirent ***, int (*)(const struct dirent *), int (*)(const struct dirent **, const struct dirent **));
int alphasort(const struct dirent **, const struct dirent **);

#else
#include <unistd.h>
#endif

t_widgetbehavior playlist_widgetbehavior;
static t_class *playlist_class;

static char   *playlist_version = "playlist: 1 click file chooser : version 0.12, written by Yves Degoyon (ydegoyon@free.fr)";

#define MIN(a,b) (a>b?b:a)

typedef struct _playlist
{
    t_object x_obj;
    t_glist *x_glist;
    t_outlet *x_fullpath;
    t_outlet *x_file;
    t_outlet *x_dir;
    char *x_extension;          /* extension to selected files               */
    t_int x_height;             /* height of the playlist                    */
    t_int x_width;              /* width of the playlist                     */
    t_int x_itemselected;       /* index of the selected item                */
    t_int x_selected;           /* stores selected state                     */
    t_int x_graphics;           /* flag to draw graphics or not              */
    char **x_dentries;          /* directory entries                         */
    t_int x_nentries;           /* number of entries in the current dir      */
    t_int x_pnentries;          /* previous size of entries list             */
    t_int x_firstseen;          /* first displayed entry                     */
    t_int x_lastseen;           /* last displayed entry                      */
    t_int x_cdy;                /* cumulated y drag                          */
    t_int x_sort;               /* sorting option flag                       */
    char  x_curdir[MAXPDSTRING];/* current directory informations            */
    char   *x_font;             /* font used for entries                     */
    t_int  x_charheight;        /* height of characters                      */
    t_int  x_charwidth;         /* width of characters                       */
    char   *x_bgcolor;          /* background color                          */
    char   *x_sbcolor;          /* scrollbar color                           */
    char   *x_fgcolor;          /* foreground color                          */
    char   *x_secolor;          /* selection color                           */
} t_playlist;


static void playlist_update_dir(t_playlist *x, t_glist *glist)
{
    t_canvas *canvas=glist_getcanvas(glist);
    t_int i;
    char wrappedname[ MAXPDSTRING ];
    struct timespec tv;

    tv.tv_sec = 0;
    tv.tv_nsec = 10000000;

    // set title
    sys_vgui(".x%lx.c delete %xTITLE\n", canvas, x);
    if ( x->x_graphics )
    {
        sys_vgui(".x%lx.c create text %d %d -width %d -text \"%s\" -anchor w -font {%s} -tags %xTITLE\n",
                  canvas,
                  text_xpix(&x->x_obj, glist)+5,
                  text_ypix(&x->x_obj, glist)-10,
                  x->x_width,
                  x->x_curdir,
                  x->x_font,
                  x );
    }

    // delete previous entries
    for ( i=x->x_firstseen; i<=x->x_lastseen; i++ )
    {
        sys_vgui(".x%lx.c delete %xENTRY%d\n", canvas, x, i);
    }

    // display the content of current directory
    {
        t_int nentries, i;
        struct dirent** dentries;      /* all directory entries                         */

        // post( "playlist : scandir : %s", x->x_curdir );
        if ( ( nentries = scandir(x->x_curdir, &dentries, NULL, (x->x_sort==1)?alphasort:NULL ) ) == -1 )
        {
            pd_error(x, "playlist : could not scan current directory ( where the hell are you ??? )" );
            perror( "scandir" );
            return;
        }

        x->x_firstseen = 0;
        if ( x->x_dentries )
        {
            for ( i=0; i<x->x_nentries; i++ )
            {
                // post( "playlist : freeing entry %d size=%d : %s", i, strlen( x->x_dentries[i] ) + 1, x->x_dentries[i] );
                freebytes( x->x_dentries[i], strlen( x->x_dentries[i] ) + 1 );
            }
        }
        if ( x->x_pnentries != -1 )
        {
            freebytes( x->x_dentries, x->x_pnentries*sizeof(char**) );
        }

        x->x_nentries = 0;
        // post( "playlist : allocating dentries %d", nentries );
        x->x_dentries = (char **) getbytes( nentries*sizeof(char**) ) ;
        x->x_pnentries = nentries;
        for ( i=0; i<nentries; i++ )
        {
            DIR* tmpdir;

            // ckeck if that entry should be displayed
            if ( ( ( ( tmpdir = opendir( dentries[i]->d_name ) ) != NULL ) ) ||
                    ( strstr( dentries[i]->d_name, x->x_extension ) ) ||
                    ( !strcmp( x->x_extension, "all" ) )
               )
            {
                // close temporarily opened dir
                if ( tmpdir )
                {
                    if ( closedir( tmpdir ) < 0 )
                    {
                        post( "playlist : could not close directory %s", dentries[i]->d_name );
                    }
                }

                // post( "playlist : allocating entry %d %d : %s", x->x_nentries, strlen( dentries[i]->d_name ) + 1, dentries[i]->d_name );
                x->x_dentries[x->x_nentries] = ( char * ) getbytes( strlen( dentries[i]->d_name ) + 1 );
                strcpy( x->x_dentries[x->x_nentries],  dentries[i]->d_name );

                // display the entry if displayable
                if ( x->x_nentries*x->x_charheight+5 < x->x_height )
                {
                    // nanosleep( &tv, NULL );
                    x->x_lastseen = x->x_nentries;
                    strncpy( wrappedname, x->x_dentries[x->x_nentries],  MIN(x->x_width/x->x_charwidth, MAXPDSTRING) );
                    wrappedname[ x->x_width/x->x_charwidth ] = '\0';
                    sys_vgui(".x%lx.c create text %d %d -fill %s -activefill %s -width %d -text \"%s\" -anchor w -font {%s} -tags %xENTRY%d\n",
                               canvas,
                               text_xpix(&x->x_obj, glist)+5,
                               text_ypix(&x->x_obj, glist)+5+(x->x_nentries-x->x_firstseen)*x->x_charheight,
                               x->x_fgcolor,
                               x->x_secolor,
                               x->x_width,
                               wrappedname,
                               x->x_font,
                               x, x->x_nentries );
                }
                x->x_nentries++;
            }

        }

    }
}

static void playlist_output_current(t_playlist* x)
{
    // output the selected dir+file
    // check that it's not a directory
    if ( chdir( x->x_dentries[x->x_itemselected] ) == 0 )
    {
        chdir( x->x_curdir );
        return;
    }

    if ( x->x_dentries && x->x_itemselected < x->x_nentries && x->x_itemselected >= 0 )
    {
        char* tmpstring = (char*) getbytes( strlen( x->x_curdir ) + strlen( x->x_dentries[x->x_itemselected]) + 2 );

        sprintf( tmpstring, "%s/%s", x->x_curdir, x->x_dentries[x->x_itemselected] );
        outlet_symbol( x->x_dir, gensym( x->x_curdir ) );
        outlet_symbol( x->x_file, gensym( x->x_dentries[x->x_itemselected] ) );
        outlet_symbol( x->x_fullpath, gensym( tmpstring ) );
        freebytes( tmpstring, strlen( x->x_curdir ) + strlen( x->x_dentries[x->x_itemselected]) + 2 );
    }
}

static void playlist_sort(t_playlist* x, t_floatarg fsort)
{
    if ( ( (t_int)fsort != 0 ) && ( (t_int)fsort != 1 ) )
    {
        post( "playlist : wrong argument to playlist message : %d", (t_int)fsort );
        return;
    }

    x->x_sort = (t_int) fsort;
    playlist_update_dir( x, x->x_glist );
}

static void playlist_font(t_playlist* x, t_symbol *fname, t_symbol *fcase, t_floatarg fsize)
{
    if ( (t_int)fsize <= 4 )
    {
        post( "playlist : wrong font size in font message : %d", (t_int)fsize );
        return;
    }
    sprintf( x->x_font, "{%s %d %s}", fname->s_name, (int)fsize, fcase->s_name );
    x->x_charheight = (t_int)fsize;
    x->x_charwidth = (2*x->x_charheight)/3;
    post( "playlist : setting font to : %s", x->x_font );
    playlist_update_dir( x, x->x_glist );
}

static void playlist_draw_new(t_playlist *x, t_glist *glist)
{
    t_canvas *canvas=glist_getcanvas(glist);

    x->x_glist = glist;
    if ( x->x_graphics )
    {
        sys_vgui(".x%lx.c create rectangle %d %d %d %d -fill %s -tags %xPLAYLIST\n",
                  canvas, text_xpix(&x->x_obj, glist), text_ypix(&x->x_obj, glist),
                  text_xpix(&x->x_obj, glist) + x->x_width, text_ypix(&x->x_obj, glist) + x->x_height,
                  x->x_bgcolor, x);
        sys_vgui(".x%lx.c create rectangle %d %d %d %d -fill %s -tags %xSCROLLLIST\n",
                  canvas, text_xpix(&x->x_obj, glist)+4*x->x_width/5, text_ypix(&x->x_obj, glist),
                  text_xpix(&x->x_obj, glist) + x->x_width, text_ypix(&x->x_obj, glist) + x->x_height,
                  x->x_sbcolor, x);
    }
    playlist_update_dir( x, glist );

}

static void playlist_draw_move(t_playlist *x, t_glist *glist)
{
    t_canvas *canvas=glist_getcanvas(glist);
    t_int i;
    struct timespec tv;

    tv.tv_sec = 0;
    tv.tv_nsec = 10000000;

    if ( x->x_graphics )
    {
        sys_vgui(".x%lx.c coords %xPLAYLIST %d %d %d %d\n",
                  canvas, x,
                  text_xpix(&x->x_obj, glist), text_ypix(&x->x_obj, glist),
                  text_xpix(&x->x_obj, glist)+x->x_width,
                  text_ypix(&x->x_obj, glist)+x->x_height);
        sys_vgui(".x%lx.c coords %xSCROLLLIST %d %d %d %d\n",
                  canvas, x,
                  text_xpix(&x->x_obj, glist)+4*x->x_width/5, text_ypix(&x->x_obj, glist),
                  text_xpix(&x->x_obj, glist)+x->x_width,
                  text_ypix(&x->x_obj, glist)+x->x_height);
        sys_vgui(".x%lx.c coords %xTITLE %d %d\n",
                  canvas, x,
                  text_xpix(&x->x_obj, glist)+5, text_ypix(&x->x_obj, glist)-10 );
    }
    for ( i=x->x_firstseen; i<=x->x_lastseen; i++ )
    {
        // nanosleep( &tv, NULL );
        sys_vgui(".x%lx.c coords %xENTRY%d %d %d\n",
                  canvas, x, i,
                  text_xpix(&x->x_obj, glist)+5,
                  text_ypix(&x->x_obj, glist)+5+(i-x->x_firstseen)*x->x_charheight);
    }

    canvas_fixlinesfor( canvas, (t_text*)x );
}

static void playlist_draw_erase(t_playlist* x, t_glist* glist)
{
    t_canvas *canvas=glist_getcanvas(glist);
    t_int i;

    if ( x->x_graphics )
    {
        sys_vgui(".x%lx.c delete %xPLAYLIST\n", canvas, x);
        sys_vgui(".x%lx.c delete %xSCROLLLIST\n", canvas, x);
        sys_vgui(".x%lx.c delete %xTITLE\n", canvas, x);
    }
    for ( i=x->x_firstseen; i<=x->x_lastseen; i++ )
    {
        sys_vgui(".x%lx.c delete %xENTRY%d\n", canvas, x, i);
    }
}

static void playlist_draw_select(t_playlist* x, t_glist* glist)
{
    t_canvas *canvas=glist_getcanvas(glist);

    // post( "playlist : select" );
    if(x->x_selected)
    {
        /* sets the item in blue */
        if (x->x_graphics) sys_vgui(".x%lx.c itemconfigure %xPLAYLIST -outline #0000FF\n", canvas, x);
    }
    else
    {
        if (x->x_graphics) sys_vgui(".x%lx.c itemconfigure %xPLAYLIST -outline #000000\n", canvas, x);
    }
}

/* ------------------------ playlist widgetbehaviour----------------------------- */


static void playlist_getrect(t_gobj *z, t_glist *owner,
                             int *xp1, int *yp1, int *xp2, int *yp2)
{
    t_playlist* x = (t_playlist*)z;

    *xp1 = text_xpix(&x->x_obj, owner);
    *yp1 = text_ypix(&x->x_obj, owner);
    *xp2 = text_xpix(&x->x_obj, owner)+x->x_width;
    *yp2 = text_ypix(&x->x_obj, owner)+x->x_height;
}

static void playlist_save(t_gobj *z, t_binbuf *b)
{
    t_playlist *x = (t_playlist *)z;

    // post( "saving playlist : %s", x->x_extension );
    binbuf_addv(b, "ssiissiisssss", gensym("#X"), gensym("obj"),
                (t_int)x->x_obj.te_xpix, (t_int)x->x_obj.te_ypix,
                gensym("playlist"), gensym(x->x_extension), x->x_width, x->x_height,
                gensym(x->x_font), gensym(x->x_bgcolor), gensym(x->x_sbcolor),
                gensym(x->x_fgcolor), gensym(x->x_secolor) );
    binbuf_addv(b, ";");
}

static void playlist_select(t_gobj *z, t_glist *glist, int selected)
{
    t_playlist *x = (t_playlist *)z;

    x->x_selected = selected;

    playlist_draw_select( x, glist );
}

static void playlist_vis(t_gobj *z, t_glist *glist, int vis)
{
    t_playlist *x = (t_playlist *)z;

    x->x_glist = glist;
    if (vis)
    {
        playlist_draw_new( x, glist );
    }
    else
    {
        playlist_draw_erase( x, glist );
    }
}

static void playlist_delete(t_gobj *z, t_glist *glist)
{
    canvas_deletelinesfor(glist, (t_text *)z);
}

static void playlist_displace(t_gobj *z, t_glist *glist, int dx, int dy)
{
    t_playlist *x = (t_playlist *)z;
    t_int xold = text_xpix(&x->x_obj, glist);
    t_int yold = text_ypix(&x->x_obj, glist);

    // post( "playlist_displace dx=%d dy=%d", dx, dy );

    x->x_obj.te_xpix += dx;
    x->x_obj.te_ypix += dy;
    if(xold != text_xpix(&x->x_obj, glist) || yold != text_ypix(&x->x_obj, glist))
    {
        playlist_draw_move(x, glist);
    }
}

static void playlist_motion(t_playlist *x, t_floatarg dx, t_floatarg dy)
{
    t_int i;
    t_canvas *canvas=glist_getcanvas(x->x_glist);

    x->x_cdy+=dy;

    // check if we need to scroll
    if (  ( x->x_lastseen < x->x_nentries ) )
    {
        // eventually, move down
        if ( x->x_cdy >= x->x_charheight )
        {
            x->x_cdy = 0;
            if ( x->x_firstseen < x->x_nentries - ( x->x_height/x->x_charheight ) )
            {
                if ( x->x_firstseen + 1 < x->x_nentries )
                {
                    for ( i=x->x_firstseen; i<=x->x_lastseen; i++ )
                    {
                        sys_vgui(".x%lx.c delete %xENTRY%d\n", canvas, x, i);
                    }
                    x->x_firstseen++;
                    for ( i=x->x_firstseen; i< x->x_nentries; i++ )
                    {
                        char *wrappedname = (char *) getbytes( x->x_width );

                        if ( (i-x->x_firstseen)*x->x_charheight < x->x_height )
                        {
                            x->x_lastseen = i;
                            strncpy( wrappedname, x->x_dentries[i],  x->x_width/x->x_charwidth );
                            wrappedname[ x->x_width/x->x_charwidth ] = '\0';
                            sys_vgui(".x%lx.c create text %d %d -fill %s -activefill %s -width %d -text \"%s\" -anchor w -font {%s} -tags %xENTRY%d\n",
                                       canvas,
                                       text_xpix(&x->x_obj, x->x_glist)+5,
                                       text_ypix(&x->x_obj, x->x_glist)+5+(i-x->x_firstseen)*x->x_charheight,
                                       x->x_fgcolor,
                                       x->x_secolor,
                                       x->x_width,
                                       wrappedname,
                                       x->x_font,
                                       x, i );
                        }
                        else break;
                    }
                    sys_vgui(".x%lx.c itemconfigure %xENTRY%d -fill %s\n",
                              canvas, x, x->x_itemselected, x->x_secolor);
                    // post( "playlist : moved down first=%d last=%d", x->x_firstseen, x->x_lastseen );
                }
            }
        }
        // eventually, move up
        if ( x->x_cdy <= -x->x_charheight )
        {
            x->x_cdy = 0;
            if ( x->x_lastseen >= ( x->x_height/x->x_charheight ) )
            {
                if ( x->x_firstseen - 1 >= 0 )
                {
                    for ( i=x->x_firstseen; i<=x->x_lastseen; i++ )
                    {
                        sys_vgui(".x%lx.c delete %xENTRY%d\n", canvas, x, i);
                    }
                    x->x_firstseen--;
                    for ( i=x->x_firstseen; i< x->x_nentries; i++ )
                    {
                        char *wrappedname = (char *) getbytes( x->x_width );

                        if ( (i-x->x_firstseen)*x->x_charheight < x->x_height )
                        {
                            x->x_lastseen = i;
                            strncpy( wrappedname, x->x_dentries[i],  x->x_width/x->x_charwidth );
                            wrappedname[ x->x_width/x->x_charwidth ] = '\0';
                            sys_vgui(".x%lx.c create text %d %d -fill %s -activefill %s -width %d -text \"%s\"  \
                          -anchor w -font {%s} -tags %xENTRY%d\n",
                                       canvas,
                                       text_xpix(&x->x_obj, x->x_glist)+5,
                                       text_ypix(&x->x_obj, x->x_glist)+5+(i-x->x_firstseen)*x->x_charheight,
                                       x->x_fgcolor,
                                       x->x_secolor,
                                       x->x_width,
                                       wrappedname,
                                       x->x_font,
                                       x, i );
                        }
                        else break;
                    }
                    sys_vgui(".x%lx.c itemconfigure %xENTRY%d -fill %s\n",
                              canvas, x, x->x_itemselected, x->x_secolor);
                    // post( "playlist : moved up first=%d last=%d", x->x_firstseen, x->x_lastseen );
                }
            }
        }
    } // scroll test
}

static void playlist_scroll(t_playlist *x, t_floatarg fdy)
{
    t_int nbsteps, si;

    nbsteps = (t_int)abs(fdy/x->x_charheight);
    // post( "playlist : iterations %d", nbsteps );

    for (si=0; si<nbsteps; si++ )
    {
        playlist_motion(x, 0, (fdy/abs(fdy))*x->x_charheight);
    }
}

static void playlist_graphics(t_playlist *x, t_floatarg fgraphics)
{
    if ( ( (t_int)fgraphics == 0 ) || ( (t_int)fgraphics == 1 ) )
    {
        playlist_draw_erase(x, x->x_glist);
        x->x_graphics = (t_int) fgraphics;
        playlist_draw_new(x, x->x_glist);
    }
}

static int playlist_click(t_gobj *z, struct _glist *glist,
                          int xpix, int ypix, int shift, int alt, int dbl, int doit)
{
    t_playlist* x = (t_playlist *)z;
    t_canvas *canvas=glist_getcanvas(x->x_glist);

    if (doit)
    {
        // leave a margin for scrolling without selection
        if ( (xpix-text_xpix(&x->x_obj, glist)) < 4*x->x_width/5 )
        {
            // deselect previously selected item
            sys_vgui(".x%lx.c itemconfigure %xENTRY%d -fill %s\n",
                      canvas, x, x->x_itemselected, x->x_fgcolor);
            x->x_itemselected = x->x_firstseen + (ypix-text_ypix(&x->x_obj, glist))/x->x_charheight;
            sys_vgui(".x%lx.c itemconfigure %xENTRY%d -fill %s\n",
                      canvas, x, x->x_itemselected, x->x_secolor);
            // post( "playlist : selected item : %d", x->x_itemselected );
            if ( x->x_dentries && ( x->x_itemselected < x->x_nentries ) )
            {
                char *tmpstring = (char *) getbytes( strlen( x->x_curdir ) + strlen( x->x_dentries[x->x_itemselected] ) + 2 );
                sprintf( tmpstring, "%s/%s", x->x_curdir, x->x_dentries[x->x_itemselected] );
                // post( "playlist : chdir : %s", tmpstring );
                if ( chdir( tmpstring ) < 0 )
                {
                    playlist_output_current(x);
                }
                else
                {
                    if ( !strcmp(  x->x_dentries[ x->x_itemselected ], ".." ) )
                    {
                        char *iamthelastslash;

                        iamthelastslash = strrchr( x->x_curdir, '/' );
                        *iamthelastslash = '\0';

                        if ( !strcmp( x->x_curdir, "" ) )
                        {
                            strcpy( x->x_curdir, "/" );
                        }
                    }
                    else if ( !strcmp(  x->x_dentries[ x->x_itemselected ], "." ) )
                    {
                        // nothing
                    }
                    else
                    {
                        if ( strlen( x->x_curdir ) + strlen( x->x_dentries[x->x_itemselected] ) + 2 > MAXPDSTRING )
                        {
                            pd_error(x, "playlist : maximum dir length reached : cannot change directory" );
                            return -1;
                        }
                        if ( strcmp( x->x_curdir, "/" ) )
                        {
                            sprintf( x->x_curdir, "%s/%s", x->x_curdir, x->x_dentries[x->x_itemselected] );
                        }
                        else
                        {
                            sprintf( x->x_curdir, "/%s", x->x_dentries[x->x_itemselected] );
                        }
                    }

                    playlist_update_dir( x, glist );
                }
            }
        }
        x->x_glist = glist;
        glist_grab( glist, &x->x_obj.te_g, (t_glistmotionfn)playlist_motion,
                    NULL, xpix, ypix );
    }
    return (1);
}

static void playlist_properties(t_gobj *z, t_glist *owner)
{
    char buf[800];
    t_playlist *x=(t_playlist *)z;

    sprintf(buf, "pdtk_playlist_dialog %%s %s %d %d %s %s %s %s %s\n",
            x->x_extension, (int)x->x_width, (int)x->x_height,
            x->x_font, x->x_bgcolor, x->x_sbcolor,
            x->x_fgcolor, x->x_secolor );
    // post("playlist_properties : %s", buf );
    gfxstub_new(&x->x_obj.ob_pd, x, buf);
}

static void playlist_dialog(t_playlist *x, t_symbol *s, int argc, t_atom *argv)
{
    if ( !x )
    {
        pd_error(x, "playlist : error :tried to set properties on an unexisting object" );
    }
    if ( argc != 10 )
    {
        pd_error(x, "playlist : error in the number of arguments ( %d instead of 10 )", argc );
        return;
    }
    if ( argv[0].a_type != A_SYMBOL || argv[1].a_type != A_FLOAT ||
            argv[2].a_type != A_FLOAT  || argv[3].a_type != A_SYMBOL ||
            argv[4].a_type != A_FLOAT  || argv[5].a_type != A_SYMBOL ||
            argv[6].a_type != A_SYMBOL || argv[7].a_type != A_SYMBOL ||
            argv[8].a_type != A_SYMBOL || argv[9].a_type != A_SYMBOL )
    {
        pd_error(x, "playlist : wrong arguments" );
        return;
    }
    x->x_extension = argv[0].a_w.w_symbol->s_name;
    x->x_width = (int)argv[1].a_w.w_float;
    x->x_height = (int)argv[2].a_w.w_float;
    sprintf( x->x_font, "{%s %d %s}", argv[3].a_w.w_symbol->s_name,
             (int)argv[4].a_w.w_float, argv[5].a_w.w_symbol->s_name );
    x->x_charheight = (t_int)argv[4].a_w.w_float;
    strcpy( x->x_bgcolor, argv[6].a_w.w_symbol->s_name );
    strcpy( x->x_sbcolor, argv[7].a_w.w_symbol->s_name );
    strcpy( x->x_fgcolor, argv[8].a_w.w_symbol->s_name );
    strcpy( x->x_secolor, argv[9].a_w.w_symbol->s_name );

    playlist_draw_erase(x, x->x_glist);
    playlist_draw_new(x, x->x_glist);
}


static t_playlist *playlist_new(t_symbol *s, int argc, t_atom *argv )
{
    t_int argoffset=0;
    t_playlist *x;

    x = (t_playlist *)pd_new(playlist_class);

    x->x_extension = ( char * ) getbytes( MAXPDSTRING );
    sprintf( x->x_extension, "all" );
    x->x_width = 400;
    x->x_height = 300;
    x->x_font = ( char * ) getbytes( MAXPDSTRING );
    sprintf( x->x_font, "{Helvetica 10 bold}" );
    x->x_charheight = 10;
    x->x_charwidth = (2*10)/3;
    x->x_bgcolor = ( char * ) getbytes( MAXPDSTRING );
    sprintf( x->x_bgcolor, "#457782" );
    x->x_sbcolor = ( char * ) getbytes( MAXPDSTRING );
    sprintf( x->x_sbcolor, "yellow" );
    x->x_fgcolor = ( char * ) getbytes( MAXPDSTRING );
    sprintf( x->x_fgcolor, "black" );
    x->x_secolor = ( char * ) getbytes( MAXPDSTRING );
    sprintf( x->x_secolor, "red" );

    if ( argc >= 1 )
    {
        if ( argv[0].a_type != A_SYMBOL )
        {
            pd_error(x, "playlist : wrong argument (extension : 1)" );
            return NULL;
        }
        if ( !strcmp( argv[0].a_w.w_symbol->s_name, "" ) )
        {
            pd_error(x, "playlist : no extension specified" );
            pd_error(x, "playlist : usage : playlist <extension> <width> <height>" );
            return NULL;
        }
        strcpy( x->x_extension, argv[0].a_w.w_symbol->s_name );
    }
    if ( argc >= 2 )
    {
        if ( argv[1].a_type != A_FLOAT )
        {
            pd_error(x, "playlist : wrong argument (width : 2)" );
            return NULL;
        }
        if ( (int)argv[1].a_w.w_float <= 0 )
        {
            pd_error(x, "playlist : wrong width (%d)", (int)argv[1].a_w.w_float );
            pd_error(x, "playlist : usage : playlist <extension> <width> <height>" );
            return NULL;
        }
        x->x_width = (int)argv[1].a_w.w_float;
    }
    if ( argc >= 3 )
    {
        if ( argv[2].a_type != A_FLOAT )
        {
            pd_error(x, "playlist : wrong argument (height : 3)" );
            return NULL;
        }
        if ( (int)argv[2].a_w.w_float <= 0 )
        {
            pd_error(x, "playlist : wrong height (%d)", (int)argv[2].a_w.w_float );
            pd_error(x, "playlist : usage : playlist <extension> <width> <height>" );
            return NULL;
        }
        x->x_height = (int)argv[2].a_w.w_float;
    }
    if ( argc >= 6 )
    {
        if ( argv[3].a_type != A_SYMBOL ||
                argv[5].a_type != A_SYMBOL )
        {
            pd_error(x, "playlist : wrong arguments (font : 4,6)" );
            pd_error(x, "argument types : %d %d", argv[3].a_type, argv[5].a_type );
            return NULL;
        }
        if ( argv[4].a_type != A_SYMBOL &&
                argv[4].a_type != A_FLOAT )
        {
            pd_error(x, "playlist : wrong arguments (font size : 5)" );
            pd_error(x, "argument types : %d", argv[4].a_type );
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
        logpost(NULL, 4, "playlist : font : %s, size : %d", x->x_font, (int)x->x_charheight );
    }
    if ( argc >= 7-argoffset )
    {
        if ( argv[6-argoffset].a_type != A_SYMBOL )
        {
            pd_error(x, "playlist : wrong arguments (background color : %d)", 7-(int)argoffset );
            return NULL;
        }
        strcpy( x->x_bgcolor, argv[6-argoffset].a_w.w_symbol->s_name );
    }
    if ( argc >= 8-argoffset )
    {
        if ( argv[7-argoffset].a_type != A_SYMBOL )
        {
            pd_error(x, "playlist : wrong arguments (scrollbar color : %d)", 8-(int)argoffset );
            return NULL;
        }
        strcpy( x->x_sbcolor, argv[7-argoffset].a_w.w_symbol->s_name );
    }
    if ( argc >= 9-argoffset )
    {
        if ( argv[8-argoffset].a_type != A_SYMBOL )
        {
            pd_error(x, "playlist : wrong arguments (foreground color : %d)", 9-(int)argoffset );
            return NULL;
        }
        strcpy( x->x_fgcolor, argv[8-argoffset].a_w.w_symbol->s_name );
    }
    if ( argc >= 10-argoffset )
    {
        if ( argv[9-argoffset].a_type != A_SYMBOL )
        {
            pd_error(x, "playlist : wrong arguments (selection color : %d)", 10-(int)argoffset );
            return NULL;
        }
        strcpy( x->x_secolor, argv[9-argoffset].a_w.w_symbol->s_name );
    }

    x->x_fullpath = outlet_new(&x->x_obj, &s_symbol );
    x->x_file = outlet_new(&x->x_obj, &s_symbol );
    x->x_dir = outlet_new(&x->x_obj, &s_symbol );

    x->x_glist = (t_glist *) canvas_getcurrent();
    x->x_nentries = 0;
    x->x_pnentries = 0;
    x->x_dentries = NULL;

    // get current directory full path
    t_symbol *cwd = canvas_getdir(x->x_glist);
    strncpy( x->x_curdir, cwd->s_name, MAXPDSTRING );

    x->x_selected = 0;
    x->x_itemselected = -1;

    x->x_sort = 1;
    x->x_graphics = 1;

    // post( "playlist : built extension=%s width=%d height=%d", x->x_extension, x->x_width, x->x_height );

    return (x);
}

static void playlist_free(t_playlist *x)
{
    // post( "playlist : playlist_free" );
    if ( x->x_extension )
    {
        freebytes( x->x_extension, MAXPDSTRING );
    }
    if ( x->x_font )
    {
        freebytes( x->x_font, MAXPDSTRING );
    }
    if ( x->x_bgcolor )
    {
        freebytes( x->x_bgcolor, MAXPDSTRING );
    }
    if ( x->x_sbcolor )
    {
        freebytes( x->x_sbcolor, MAXPDSTRING );
    }
    if ( x->x_fgcolor )
    {
        freebytes( x->x_fgcolor, MAXPDSTRING );
    }
    if ( x->x_secolor )
    {
        freebytes( x->x_secolor, MAXPDSTRING );
    }
}

static void playlist_seek(t_playlist *x, t_floatarg fseeked)
{
    t_int iout=0;
    t_canvas *canvas=glist_getcanvas(x->x_glist);

    if ( fseeked < 0 )
    {
        pd_error(x, "playlist : wrong searched file : %f", fseeked );
        return;
    }

    if ( x->x_nentries > 2 )
    {
        // do not select . or ..
        iout = (int)fseeked % (x->x_nentries-2) + 2;
    }
    else
    {
        return;
    }
    sys_vgui(".x%lx.c itemconfigure %xENTRY%d -fill %s\n", canvas, x, x->x_itemselected, x->x_fgcolor);
    x->x_itemselected = iout;
    sys_vgui(".x%lx.c itemconfigure %xENTRY%d -fill %s\n", canvas, x, x->x_itemselected, x->x_secolor);
    playlist_output_current(x);
}

static void playlist_location(t_playlist *x, t_symbol *flocation)
{
    char olddir[ MAXPDSTRING ];           /* remember old location  */

    strcpy( olddir, x->x_curdir );

    if ( !strcmp(  flocation->s_name, ".." ) )
    {
        char *iamthelastslash;

        iamthelastslash = strrchr( x->x_curdir, '/' );
        *iamthelastslash = '\0';

        if ( !strcmp( x->x_curdir, "" ) )
        {
            strcpy( x->x_curdir, "/" );
        }
    }
    else if ( !strncmp( flocation->s_name, "/", 1 ) )
    {
        // absolute path required
        if ( strlen( flocation->s_name ) >= MAXPDSTRING )
        {
            pd_error(x, "playlist : maximum dir length reached : cannot change directory" );
            return;
        }
        strncpy( x->x_curdir, flocation->s_name, MAXPDSTRING );
    }
    else
    {
        // relative path
        if ( strlen( x->x_curdir ) + strlen( flocation->s_name ) + 2 > MAXPDSTRING )
        {
            pd_error(x, "playlist : maximum dir length reached : cannot change directory" );
            return;
        }
        if ( strcmp( x->x_curdir, "/" ) )
        {
            sprintf( x->x_curdir, "%s/%s", x->x_curdir, flocation->s_name );
        }
        else
        {
            sprintf( x->x_curdir, "/%s", flocation->s_name );
        }
    }

    if ( chdir( x->x_curdir ) < 0 )
    {
        pd_error(x, "playlist : requested location '%s' is not a directory", x->x_curdir );
        strcpy( x->x_curdir, olddir );
        return;
    }

    playlist_update_dir( x, x->x_glist );
}

void playlist_setup(void)
{
    logpost(NULL, 4, "%s", playlist_version );
    playlist_class = class_new(gensym("playlist"), (t_newmethod)playlist_new,
                               (t_method)playlist_free, sizeof(t_playlist),
                               CLASS_DEFAULT, A_GIMME, 0);
    class_addmethod(playlist_class, (t_method)playlist_seek, gensym("seek"), A_DEFFLOAT, A_NULL );
    class_addmethod(playlist_class, (t_method)playlist_location, gensym("location"), A_SYMBOL, A_NULL );
    class_addmethod(playlist_class, (t_method)playlist_dialog, gensym("dialog"), A_GIMME, A_NULL );
    class_addmethod(playlist_class, (t_method)playlist_sort, gensym("sort"), A_DEFFLOAT, A_NULL );
    class_addmethod(playlist_class, (t_method)playlist_graphics, gensym("graphics"), A_DEFFLOAT, A_NULL );
    class_addmethod(playlist_class, (t_method)playlist_scroll, gensym("scroll"), A_DEFFLOAT, A_NULL );
    class_addmethod(playlist_class, (t_method)playlist_font, gensym("font"), A_SYMBOL,
                    A_SYMBOL, A_DEFFLOAT, A_NULL );

    playlist_widgetbehavior.w_getrectfn =    playlist_getrect;
    playlist_widgetbehavior.w_displacefn =   playlist_displace;
    playlist_widgetbehavior.w_selectfn =     playlist_select;
    playlist_widgetbehavior.w_activatefn =   NULL;
    playlist_widgetbehavior.w_deletefn =     playlist_delete;
    playlist_widgetbehavior.w_visfn =        playlist_vis;
    playlist_widgetbehavior.w_clickfn =      playlist_click;

#if PD_MINOR_VERSION >= 37
    class_setpropertiesfn(playlist_class, playlist_properties);
    class_setsavefn(playlist_class, playlist_save);
#else
    playlist_widgetbehavior.w_propertiesfn = playlist_properties;
    playlist_widgetbehavior.w_savefn =       playlist_save;
#endif

    class_setwidget(playlist_class, &playlist_widgetbehavior);

    sys_vgui("eval [read [open {%s/%s.tcl}]]\n",
             playlist_class->c_externdir->s_name,
             playlist_class->c_name->s_name);
}
