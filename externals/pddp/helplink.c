/* Copyright (c) 2005 krzYszcz and others.
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

/* This is a prototype of an active comment.  It might be replaced with
   a new core object type, T_LINK (te_type bitfield would have to be
   extended then). */

#include <stdio.h>
#include <string.h>
#ifdef _WIN32
# include <io.h>
#else
# include <unistd.h>
#endif

#include "m_pd.h"
#include "m_imp.h"
#include "g_canvas.h"

/* this isn't in any header, but its declared in s_path.c */
void open_via_helppath(const char *name, const char *dir);

typedef struct _helplink
{
    t_object   x_ob;
    t_glist   *x_glist;
    int        x_isgopvisible;
    char      *x_vistext;
    int        x_vissize;
    int        x_vislength;
    int        x_rtextactive;
    t_symbol  *x_ulink;
} t_helplink;

static t_class *helplink_class;

static void helplink_getrect(t_gobj *z, t_glist *glist,
			     int *xp1, int *yp1, int *xp2, int *yp2)
{
    t_helplink *x = (t_helplink *)z;
    int width, height;
    float x1, y1, x2, y2;
    if (glist->gl_editor && glist->gl_editor->e_rtext)
    {
	if (x->x_rtextactive)
	{
	    t_rtext *y = glist_findrtext(glist, (t_text *)x);
	    width = rtext_width(y);
	    height = rtext_height(y) - 2;
	}
	else
	{
	    int font = glist_getfont(glist);
	    width = x->x_vislength * sys_fontwidth(font) + 2;
	    height = sys_fontheight(font) + 2;
	}
    }
    else width = height = 10;
    x1 = text_xpix((t_text *)x, glist);
    y1 = text_ypix((t_text *)x, glist) + 1;
    x2 = x1 + width;
    y2 = y1 + height + 1;
    *xp1 = x1;
    *yp1 = y1;
    *xp2 = x2;
    *yp2 = y2;
}

static void helplink_displace(t_gobj *z, t_glist *glist, int dx, int dy)
{
    t_text *t = (t_text *)z;
    t->te_xpix += dx;
    t->te_ypix += dy;
    if (glist_isvisible(glist))
    {
        t_rtext *y = glist_findrtext(glist, t);
        rtext_displace(y, dx, dy);
    }
}

static void helplink_select(t_gobj *z, t_glist *glist, int state)
{
    t_helplink *x = (t_helplink *)z;
    t_rtext *y = glist_findrtext(glist, (t_text *)x);
    rtext_select(y, state);
    if (glist_isvisible(glist) && glist->gl_havewindow)
    {
	if (state)
	    sys_vgui(".x%lx.c itemconfigure %s -fill blue\n",
		     glist, rtext_gettag(y));
	else
	    sys_vgui(".x%lx.c itemconfigure %s -text {%s} -fill #0000dd -activefill #e70000\n",
		     glist, rtext_gettag(y), x->x_vistext);
    }
}

static void helplink_activate(t_gobj *z, t_glist *glist, int state)
{
    t_helplink *x = (t_helplink *)z;
    t_rtext *y = glist_findrtext(glist, (t_text *)x);
    rtext_activate(y, state);
    x->x_rtextactive = state;
}

static void helplink_vis(t_gobj *z, t_glist *glist, int vis)
{
    t_helplink *x = (t_helplink *)z;
    t_rtext *y;
    if (vis)
    {
        if ((glist->gl_havewindow || x->x_isgopvisible)
            && (y = glist_findrtext(glist, (t_text *)x)))
        {
            rtext_draw(y);
	    sys_vgui(".x%lx.c itemconfigure %s -text {%s} -fill #0000dd -activefill #e70000\n",
		     glist_getcanvas(glist), rtext_gettag(y), x->x_vistext);
        }
    }
    else
    {
        if ((glist->gl_havewindow || x->x_isgopvisible)
	    && (y = glist_findrtext(glist, (t_text *)x)))
            rtext_erase(y);
    }
}

static void helplink_doclick(t_helplink *x)
{
    char* objectname = x->x_ulink->s_name;
    char dirbuf[MAXPDSTRING], *nameptr;
    int fd = canvas_open(x->x_glist, objectname, "-help.pd",
                         dirbuf, &nameptr, MAXPDSTRING, 0);
    
    if (fd < 0) {
        /* if canvas_open() failed try open_via_helppath() */
        open_via_helppath(objectname, canvas_getdir(x->x_glist)->s_name);
    } else {
        /* if canvas_open() gave us a filehandle, then we have a helppatch to
         * open in dirbuf and nameptr, but we don't need the filehandle */
        close(fd);
        glob_evalfile(NULL, gensym(nameptr), gensym(dirbuf));
    }
}

static int helplink_click(t_gobj *z, t_glist *glist, int xpix, int ypix,
			    int shift, int alt, int dbl, int doit)
{
    t_helplink *x = (t_helplink *)z;
    if (glist->gl_havewindow || x->x_isgopvisible)
    {
        if (doit)
            helplink_doclick(x);
        return (1);
    }
    else
        return (0);
}

static void helplink_free(t_helplink *x)
{
    if (x->x_vistext)
        freebytes(x->x_vistext, x->x_vissize);
}

static void *helplink_new(t_symbol *s)
{
    t_helplink *x = (t_helplink *) pd_new(helplink_class);

    x->x_isgopvisible = 0;
    x->x_vistext = 0;
    x->x_vissize = 0;
    x->x_vislength = (x->x_vistext ? strlen(x->x_vistext) : 0);
    x->x_rtextactive = 0;
    x->x_glist = canvas_getcurrent();
    if (s == &s_)
        x->x_ulink = gensym("helplink"); /* default to helplink help patch */
    else
        x->x_ulink = s;
	/* do we need to set ((t_text *)x)->te_type = T_TEXT; ? */
	if (!x->x_vistext)
	{
	    x->x_vislength = strlen(x->x_ulink->s_name);
	    x->x_vissize = x->x_vislength + 1;
	    x->x_vistext = getbytes(x->x_vissize);
	    strcpy(x->x_vistext, x->x_ulink->s_name);
	}
    return (x);
}

static t_widgetbehavior helplink_widgetbehavior =
{
    helplink_getrect,
    helplink_displace,
    helplink_select,
    helplink_activate,
    0,
    helplink_vis,
    helplink_click,
};

void helplink_setup(void)
{
    helplink_class = class_new(gensym("helplink"),
			       (t_newmethod)helplink_new,
			       (t_method)helplink_free,
			       sizeof(t_helplink),
			       CLASS_PATCHABLE,
			       A_DEFSYMBOL, 0);

    class_addbang(helplink_class, helplink_doclick);
    class_setwidget(helplink_class, &helplink_widgetbehavior);
}
