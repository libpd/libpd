/* Copyright (c) 2005 krzYszcz and others.
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

/* This is a prototype of an active comment.  It might be replaced with
   a new core object type, T_LINK (te_type bitfield would have to be
   extended then). */

#include <stdio.h>
#include <string.h>
#include "m_pd.h"
#include "m_imp.h"  /* FIXME need access to c_externdir... */
#include "g_canvas.h"
#include "common/loud.h"
#include "build_counter"

#ifdef KRZYSZCZ
//#define PDDPLINK_DEBUG
#endif

enum { PDDPLINK_PD, PDDPLINK_HTML };  /* LATER add others */

typedef struct _pddplink
{
    t_object   x_ob;
    t_glist   *x_glist;
    int        x_isboxed;
    int        x_isgopvisible;
    char      *x_vistext;
    int        x_vissize;
    int        x_vislength;
    int        x_rtextactive;
    t_symbol  *x_dirsym;
    t_symbol  *x_ulink;
    t_atom     x_openargs[2];
    int        x_linktype;
    int        x_ishit;
} t_pddplink;

static t_class *pddplink_class;
static t_class *pddplinkbox_class;

/* Code that might be merged back to g_text.c starts here: */

static void pddplink_getrect(t_gobj *z, t_glist *glist,
			     int *xp1, int *yp1, int *xp2, int *yp2)
{
    t_pddplink *x = (t_pddplink *)z;
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
    y1 = text_ypix((t_text *)x, glist);
    x2 = x1 + width;
    y2 = y1 + height;
    y1 += 1;
    *xp1 = x1;
    *yp1 = y1;
    *xp2 = x2;
    *yp2 = y2;
}

static void pddplink_displace(t_gobj *z, t_glist *glist, int dx, int dy)
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

static void pddplink_select(t_gobj *z, t_glist *glist, int state)
{
    t_pddplink *x = (t_pddplink *)z;
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

static void pddplink_activate(t_gobj *z, t_glist *glist, int state)
{
    t_pddplink *x = (t_pddplink *)z;
    t_rtext *y = glist_findrtext(glist, (t_text *)x);
    rtext_activate(y, state);
    x->x_rtextactive = state;
}

static void pddplink_vis(t_gobj *z, t_glist *glist, int vis)
{
    t_pddplink *x = (t_pddplink *)z;
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

static int pddplink_wbclick(t_gobj *z, t_glist *glist, int xpix, int ypix,
			    int shift, int alt, int dbl, int doit);

static t_widgetbehavior pddplink_widgetbehavior =
{
    pddplink_getrect,
    pddplink_displace,
    pddplink_select,
    pddplink_activate,
    0,
    pddplink_vis,
    pddplink_wbclick,
};

/* Code that might be merged back to g_text.c ends here. */

/* FIXME need access to glob_pdobject... */
static t_pd *pddplink_pdtarget(t_pddplink *x)
{
    t_pd *pdtarget = gensym("pd")->s_thing;
    if (pdtarget && !strcmp(class_getname(*pdtarget), "pd"))
	return (pdtarget);
    else
	return ((t_pd *)x);  /* internal error */
}

static void pddplink_anything(t_pddplink *x, t_symbol *s, int ac, t_atom *av)
{
    if (x->x_ishit)
    {
	startpost("pddplink: internal error (%s", (s ? s->s_name : ""));
	postatom(ac, av);
	post(")");
    }
}

static void pddplink_click(t_pddplink *x, t_floatarg xpos, t_floatarg ypos,
			   t_floatarg shift, t_floatarg ctrl, t_floatarg alt)
{
    x->x_ishit = 1;
    switch (x->x_linktype)
    {
    case PDDPLINK_PD:
	typedmess(pddplink_pdtarget(x), gensym("open"), 2, x->x_openargs);
	break;
    case PDDPLINK_HTML:
	sys_vgui("after 0 {::pddp::cliOpen {%s}}\n", x->x_ulink->s_name);
	break;
    }
    x->x_ishit = 0;
}

static int pddplink_wbclick(t_gobj *z, t_glist *glist, int xpix, int ypix,
			    int shift, int alt, int dbl, int doit)
{
    t_pddplink *x = (t_pddplink *)z;
    if (glist->gl_havewindow || x->x_isgopvisible)
    {
	if (doit)
	    pddplink_click(x, (t_floatarg)xpix, (t_floatarg)ypix,
			   (t_floatarg)shift, 0, (t_floatarg)alt);
	return (1);
    }
    else return (0);
}

static int pddplink_isoption(char *name)
{
    if (*name == '-')
    {
	char c = name[1];
	return ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'));
    }
    else return (0);
}

static t_symbol *pddplink_nextsymbol(int ac, t_atom *av, int opt, int *skipp)
{
    int ndx;
    for (ndx = 0; ndx < ac; ndx++, av++)
    {
	if (av->a_type == A_SYMBOL &&
	    (!opt || pddplink_isoption(av->a_w.w_symbol->s_name)))
	{
	    *skipp = ++ndx;
	    return (av->a_w.w_symbol);
	}
    }
    return (0);
}

static int pddplink_dooptext(char *dst, int maxsize, int ac, t_atom *av)
{
    int i, sz, sep, len;
    char buf[32], *src;
    for (i = 0, sz = 0, sep = 0; i < ac; i++, av++)
    {
	if (sep)
	{
	    sz++;
	    if (sz >= maxsize)
		break;
	    else if (dst)
	    {
		*dst++ = ' ';
		*dst = 0;
	    }
	}
	else sep = 1;
	if (av->a_type == A_SYMBOL)
	    src = av->a_w.w_symbol->s_name;
	else if (av->a_type == A_FLOAT)
	{
	    src = buf;
	    sprintf(src, "%g", av->a_w.w_float);
	}
	else
	{
	    sep = 0;
	    continue;
	}
	len = strlen(src);
	sz += len;
	if (sz >= maxsize)
	    break;
	else if (dst)
	{
	    strcpy(dst, src);
	    dst += len;
	}
    }
    return (sz);
}

static char *pddplink_optext(int *sizep, int ac, t_atom *av)
{
    char *result;
    int sz = pddplink_dooptext(0, MAXPDSTRING, ac, av);
    *sizep = sz + (sz >= MAXPDSTRING ? 4 : 1);
    result = getbytes(*sizep);
    pddplink_dooptext(result, sz + 1, ac, av);
    if (sz >= MAXPDSTRING)
    {
	sz = strlen(result);
	strcpy(result + sz, "...");
    }
    return (result);
}

#ifdef PDDPLINK_DEBUG
static void pddplink_debug(t_pddplink *x)
{
}
#endif

static void pddplink_free(t_pddplink *x)
{
    if (x->x_vistext)
	freebytes(x->x_vistext, x->x_vissize);
}

static void *pddplink_new(t_symbol *s, int ac, t_atom *av)
{
    t_pddplink xgen, *x;
    int skip;
    xgen.x_isboxed = 0;
    xgen.x_isgopvisible = 0;
    xgen.x_vistext = 0;
    xgen.x_vissize = 0;
    if (xgen.x_ulink = pddplink_nextsymbol(ac, av, 0, &skip))
    {
	t_symbol *opt;
	ac -= skip;
	av += skip;
	while (opt = pddplink_nextsymbol(ac, av, 1, &skip))
	{
	    ac -= skip;
	    av += skip;
	    if (opt == gensym("-box"))
		xgen.x_isboxed = 1;
	    else if (opt == gensym("-gop"))
		xgen.x_isgopvisible = 1;
	    else if (opt == gensym("-text"))
	    {
		t_symbol *nextsym = pddplink_nextsymbol(ac, av, 1, &skip);
		int natoms = (nextsym ? skip - 1 : ac);
		if (natoms)
		    xgen.x_vistext =
			pddplink_optext(&xgen.x_vissize, natoms, av);
	    }
	}
    }
    x = (t_pddplink *)
	pd_new(xgen.x_isboxed ? pddplinkbox_class : pddplink_class);
    x->x_glist = canvas_getcurrent();
    x->x_dirsym = canvas_getdir(x->x_glist);  /* FIXME */

    x->x_isboxed = xgen.x_isboxed;
    x->x_isgopvisible = xgen.x_isgopvisible;
    x->x_vistext = xgen.x_vistext;
    x->x_vissize = xgen.x_vissize;
    x->x_vislength = (x->x_vistext ? strlen(x->x_vistext) : 0);
    x->x_rtextactive = 0;
    if (xgen.x_ulink)
    {
	int len = strlen(xgen.x_ulink->s_name);
	if (len > 3 && !strcmp(xgen.x_ulink->s_name + len - 3, ".pd"))
	    x->x_linktype = PDDPLINK_PD;
	else
	    x->x_linktype = PDDPLINK_HTML;
	x->x_ulink = xgen.x_ulink;
    }
    else
    {
	x->x_linktype = PDDPLINK_HTML;
	x->x_ulink = gensym("index.html");
    }
    SETSYMBOL(&x->x_openargs[0], x->x_ulink);
    SETSYMBOL(&x->x_openargs[1], x->x_dirsym);
    x->x_ishit = 0;
    if (x->x_isboxed)
	outlet_new((t_object *)x, &s_anything);
    else
    {
	/* do we need to set ((t_text *)x)->te_type = T_TEXT; ? */
	if (!x->x_vistext)
	{
	    x->x_vislength = strlen(x->x_ulink->s_name);
	    x->x_vissize = x->x_vislength + 1;
	    x->x_vistext = getbytes(x->x_vissize);
	    strcpy(x->x_vistext, x->x_ulink->s_name);
	}
    }
    if (x->x_linktype == PDDPLINK_HTML)
	sys_vgui("after 0 {::pddp::srvUse {%s}}\n", x->x_dirsym->s_name);
    return (x);
}

void pddplink_setup(void)
{
    t_symbol *dirsym;
    post("this is pddplink %s, %s %s build...",
	 PDDP_VERSION, loud_ordinal(PDDP_BUILD), PDDP_RELEASE);

    pddplink_class = class_new(gensym("pddplink"),
			       (t_newmethod)pddplink_new,
			       (t_method)pddplink_free,
			       sizeof(t_pddplink),
			       CLASS_NOINLET | CLASS_PATCHABLE,
			       A_GIMME, 0);
    class_addanything(pddplink_class, pddplink_anything);
    class_setwidget(pddplink_class, &pddplink_widgetbehavior);

    pddplinkbox_class = class_new(gensym("pddplink"), 0,
				  (t_method)pddplink_free,
				  sizeof(t_pddplink), 0, A_GIMME, 0);
    class_addanything(pddplinkbox_class, pddplink_anything);
    class_addmethod(pddplinkbox_class, (t_method)pddplink_click,
		    gensym("click"),
		    A_FLOAT, A_FLOAT, A_FLOAT, A_FLOAT, A_FLOAT, 0);

#ifdef PDDPLINK_DEBUG
    class_addmethod(pddplink_class, (t_method)pddplink_debug,
		    gensym("debug"), 0);
    class_addmethod(pddplinkbox_class, (t_method)pddplink_debug,
		    gensym("debug"), 0);
#endif

    dirsym = pddplink_class->c_externdir;  /* FIXME */
    sys_vgui(
	"if {[lsearch $auto_path \"%s\"] < 0} {lappend auto_path \"%s\"}\n",
	dirsym->s_name, dirsym->s_name);
    sys_gui("after 0 {package require pddp}\n");
}
