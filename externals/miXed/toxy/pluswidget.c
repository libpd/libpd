/* Copyright (c) 2005 krzYszcz and others.
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

/* This is a prototype of a custom object box.  It might be replaced with
   a new core object type, T_CUSTOM (te_type bitfield would have to be
   extended then). */

#include <stdio.h>
#include <string.h>
#include "m_pd.h"
#include "g_canvas.h"
#include "common/loud.h"
#include "toxy/plusbob.h"
#include "plustot.h"

#ifdef KRZYSZCZ
//#define PLUSWIDGET_DEBUG
#endif

struct _pluswidget
{
    t_plusstring  *pw_visstring;
    char          *pw_visbuf;  /* binbuf_gettext()-style: no null termination */
    int            pw_vissize;
    int            pw_rtextactive;
    int            pw_ishit;
};

/* Code that might be merged back to g_text.c starts here: */

static void pluswidget_getrect(t_gobj *z, t_glist *glist,
			       int *xp1, int *yp1, int *xp2, int *yp2)
{
    t_pluswidget *pw = ((t_plusobject *)z)->po_widget;
    int width, height;
    float x1, y1, x2, y2;
    if (glist->gl_editor && glist->gl_editor->e_rtext)
    {
	if (pw->pw_rtextactive)
	{
	    t_rtext *y = glist_findrtext(glist, (t_text *)z);
	    width = rtext_width(y);
	    height = rtext_height(y);
	}
	else
	{
	    int font = glist_getfont(glist);
	    width = pw->pw_vissize * sys_fontwidth(font) + 2;
	    height = sys_fontheight(font) + 4;  /* 2-pixel top/bottom margins */
	}
    }
    else width = height = 10;
    x1 = text_xpix((t_text *)z, glist);
    y1 = text_ypix((t_text *)z, glist);
    x2 = x1 + width;
    y2 = y1 + height;
    y1 += 1;
    *xp1 = x1;
    *yp1 = y1;
    *xp2 = x2;
    *yp2 = y2;
}

static void pluswidget_drawiofor(t_glist *glist, t_plusobject *po,
				 int firsttime,
				 char *tag, int x1, int y1, int x2, int y2)
{
    int n, nplus, i, width = x2 - x1;
    for (n = po->po_noutlets, nplus = (n == 1 ? 1 : n-1), i = 0; i < n; i++)
    {
        int onset = x1 + (width - IOWIDTH) * i / nplus;
        if (firsttime)
            sys_vgui(".x%lx.c create rectangle %d %d %d %d -tags %so%d\
 -outline brown -fill lightgrey\n",
                glist_getcanvas(glist),
                onset, y2 - 3,
                onset + IOWIDTH, y2 + 2,
                tag, i);
        else
            sys_vgui(".x%lx.c coords %so%d %d %d %d %d\n",
                glist_getcanvas(glist), tag, i,
                onset, y2 - 3,
                onset + IOWIDTH, y2 + 2);
    }
    for (n = po->po_ninlets, nplus = (n == 1 ? 1 : n-1), i = 0; i < n; i++)
    {
        int onset = x1 + (width - IOWIDTH) * i / nplus;
        if (firsttime)
            sys_vgui(".x%lx.c create rectangle %d %d %d %d -tags %si%d\
 -outline brown -fill lightgrey\n",
                glist_getcanvas(glist),
                onset, y1 - 3,
                onset + IOWIDTH, y1 + 2,
                tag, i);
        else
            sys_vgui(".x%lx.c coords %si%d %d %d %d %d\n",
                glist_getcanvas(glist), tag, i,
                onset, y1 - 3,
                onset + IOWIDTH, y1 + 2);
    }
}

static void pluswidget_drawborder(t_text *t, t_glist *glist,
				  char *tag, int firsttime)
{
    int x1, y1, x2, y2;
    pluswidget_getrect(&t->te_g, glist, &x1, &y1, &x2, &y2);
    if (firsttime)
	sys_vgui(".x%lx.c create line\
 %d %d %d %d %d %d %d %d %d %d -width 2 -fill brown -tags %sR\n",
		 glist_getcanvas(glist),
		 x1, y1,  x2, y1,  x2, y2,  x1, y2,  x1, y1, tag);
    else
	sys_vgui(".x%lx.c coords %sR\
 %d %d %d %d %d %d %d %d %d %d\n",
		 glist_getcanvas(glist), tag,
		 x1, y1,  x2, y1,  x2, y2,  x1, y2,  x1, y1);
    pluswidget_drawiofor(glist, (t_plusobject *)t, firsttime,
			 tag, x1, y1, x2, y2);
}

static void pluswidget_displace(t_gobj *z, t_glist *glist, int dx, int dy)
{
    t_text *t = (t_text *)z;
    t->te_xpix += dx;
    t->te_ypix += dy;
    if (glist_isvisible(glist))
    {
        t_rtext *y = glist_findrtext(glist, t);
        rtext_displace(y, dx, dy);
        pluswidget_drawborder(t, glist, rtext_gettag(y), 0);
        canvas_fixlinesfor(glist, t);
    }
}

static void pluswidget_select(t_gobj *z, t_glist *glist, int state)
{
    t_pluswidget *pw = ((t_plusobject *)z)->po_widget;
    t_rtext *y = glist_findrtext(glist, (t_text *)z);
    rtext_select(y, state);
    if (glist_isvisible(glist) && glist->gl_havewindow)
    {
	if (state)
	    sys_vgui(".x%lx.c itemconfigure %s -fill blue\n",
		     glist, rtext_gettag(y));
	else
	    sys_vgui(".x%lx.c itemconfigure %s -text {%.*s} -fill brown\n",
		     glist, rtext_gettag(y), pw->pw_vissize, pw->pw_visbuf);
    }
}

static void pluswidget_activate(t_gobj *z, t_glist *glist, int state)
{
    t_pluswidget *pw = ((t_plusobject *)z)->po_widget;
    t_rtext *y = glist_findrtext(glist, (t_text *)z);
    rtext_activate(y, state);
    pw->pw_rtextactive = state;
    pluswidget_drawborder((t_text *)z, glist, rtext_gettag(y), 0);
}

static void pluswidget_delete(t_gobj *z, t_glist *glist)
{
    canvas_deletelinesfor(glist, (t_text *)z);
}

static void pluswidget_vis(t_gobj *z, t_glist *glist, int vis)
{
    t_pluswidget *pw = ((t_plusobject *)z)->po_widget;
    if (vis)
    {
        if (glist->gl_havewindow)
        {
            t_rtext *y = glist_findrtext(glist, (t_text *)z);
            pluswidget_drawborder((t_text *)z, glist, rtext_gettag(y), 1);
            rtext_draw(y);
	    sys_vgui(".x%lx.c itemconfigure %s -text {%.*s} -fill brown\n",
		     glist, rtext_gettag(y), pw->pw_vissize, pw->pw_visbuf);
        }
    }
    else
    {
        if (glist->gl_havewindow)
	{
	    t_rtext *y = glist_findrtext(glist, (t_text *)z);
            text_eraseborder((t_text *)z, glist, rtext_gettag(y));
            rtext_erase(y);
	}
    }
}

static int pluswidget_click(t_gobj *z, t_glist *glist, int xpix, int ypix,
			    int shift, int alt, int dbl, int doit)
{
    if (glist->gl_havewindow)
    {
	if (doit)
	    pd_bang((t_pd *)z);
	return (1);
    }
    else return (0);
}

static t_widgetbehavior pluswidget_widgetbehavior =
{
    pluswidget_getrect,
    pluswidget_displace,
    pluswidget_select,
    pluswidget_activate,
    pluswidget_delete,
    pluswidget_vis,
    pluswidget_click,
};

/* Code that might be merged back to g_text.c ends here. */

void plusobject_widgetfree(t_plusobject *po)
{
    t_pluswidget *pw = po->po_widget;
    if (pw)
    {
	if (pw->pw_visstring)
	    plusstring_release(pw->pw_visstring);
	else if (pw->pw_visbuf)
	    freebytes(pw->pw_visbuf, pw->pw_vissize);
	freebytes(pw, sizeof(*pw));
    }
}

/* assuming non-null ps will remain constant, LATER rethink */
void plusobject_widgetcreate(t_plusobject *po, t_symbol *s, int ac, t_atom *av,
			     t_plusstring *ps)
{
    t_pluswidget *pw = getbytes(sizeof(*pw));
    pw->pw_visstring = 0;
    if (ps)
    {
	plusstring_preserve(ps);
	pw->pw_visbuf = plusstring_get(ps, &pw->pw_vissize);
	if (pw->pw_vissize > 0)
	    pw->pw_visstring = ps;
	else
	    plusstring_release(ps);
    }
    if (pw->pw_visstring == 0)
    {
	t_binbuf *inbb = binbuf_new();
	if (!s || s == &s_)
	    s = plusps_tot;
	if ((s != totps_plustot && s != plusps_tot) || ac == 0)
	{
	    t_atom at;
	    if (s == totps_plustot)
		s = plusps_tot;
	    SETSYMBOL (&at, s);
	    binbuf_add(inbb, 1, &at);
	}
	if (ac > 0)
	    binbuf_add(inbb, ac, av);
	binbuf_gettext(inbb, &pw->pw_visbuf, &pw->pw_vissize);
	binbuf_free(inbb);
    }
    pw->pw_rtextactive = 0;
    pw->pw_ishit = 0;
    po->po_widget = pw;
}

void plusclass_widgetsetup(t_class *c)
{
    class_setwidget(c, &pluswidget_widgetbehavior);
}
