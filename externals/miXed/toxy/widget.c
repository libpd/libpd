/* Copyright (c) 2003-2005 krzYszcz and others.
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

/* LATER think about reloading method for .wid files */

#include <stdio.h>
#include <string.h>
#include "m_pd.h"
#include "g_canvas.h"
#include "common/loud.h"
#include "common/grow.h"
#include "unstable/forky.h"
#include "hammer/file.h"
#include "common/props.h"
#include "toxy/scriptlet.h"
#include "widget.h"
#include "build_counter"

/* our proxy of the text_class (not in the API), LATER do not cheat */
static t_class *makeshift_class;

#ifdef KRZYSZCZ
#define WIDGET_DEBUG
//#define TOW_DEBUG
//#define WIDGET_PROFILE
#endif

enum { WIDGET_NOVIS = 0, WIDGET_PUSHVIS, WIDGET_REVIS };

typedef struct _towentry
{
    struct _tow       *te_tow;
    struct _towentry  *te_next;
} t_towentry;

typedef struct _widgetentry
{
    struct _widget       *we_widget;
    struct _widgetentry  *we_next;
} t_widgetentry;

typedef struct _widget
{
    t_object       x_ob;
    t_glist       *x_glist;        /* containing glist */
    t_widgettype  *x_typedef;
    t_symbol      *x_type;        /* 1st creation arg: our type */
    t_symbol      *x_tkclass;     /* Tk widget class */
    t_symbol      *x_name;        /* 2nd creation arg: our name (common tag) */
    t_symbol      *x_cbtarget;    /* same, mangled (a target, and a tag) */
    t_symbol      *x_rptarget;    /* same, further mangled */
    t_symbol      *x_cvpathname;  /* see widget_getcvpathname() */
    t_symbol      *x_cvtarget;    /* for gui commands to be (re)sent to */
    t_symbol      *x_varname;     /* tcl variable holding our data */
    t_props       *x_options;     /* instance options */
    t_props       *x_handlers;    /* instance handlers */
    t_props       *x_arguments;   /* instance arguments */
    t_props       *x_xargs;       /* type and instance arguments, resolved */
    t_props       *x_diffoptions;    /* type options minus instance options */
    t_props       *x_diffhandlers;   /* same for handlers */
    t_props       *x_diffarguments;  /* same for arguments */
    t_widgethandlers  *x_hooks;   /* actual handlers (short definitions) */
    t_scriptlet   *x_optscript;   /* option scriptlet */
    t_scriptlet   *x_auxscript;   /* auxiliary scriptlet */
    t_scriptlet   *x_transient;   /* output buffer */
    t_hammerfile  *x_filehandle;
    int            x_width;
    int            x_height;
    t_symbol      *x_background;
    int            x_hasstate;    /* no longer used, LATER rethink */
    int            x_disabled;
    int            x_selected;
    int            x_update;      /* see widget_update() */
    int            x_vised;
    int            x_constructed;
    t_clock       *x_transclock;
    t_towentry    *x_towlist;
} t_widget;

typedef struct _tow
{
    t_object   x_ob;
    t_glist   *x_glist;        /* containing glist */
    t_glist   *x_targetglist;  /* containing or parent glist */
    t_symbol  *x_cvremote;     /* null if targetglist is set */
    t_symbol  *x_cvname;
    t_symbol  *x_type;         /* 2nd creation arg: widget's type */
    t_symbol  *x_name;         /* 3rd creation arg: widget's name */
    t_widgetentry  *x_widgetlist;
    struct _tow    *x_next;    /* next in the global towlist */
} t_tow;

static t_class *widget_class;
static t_class *tow_class;

/* Global towlist, searched in widget_attach().  There is no global widgetlist,
   because a destination glist is searched instead in tow_attach(). */
static t_tow *widget_towlist = 0;

static t_symbol *widgetps_mouse;
static t_symbol *widgetps_motion;
static t_symbol *widgetps_vis;
static t_symbol *widgetps_new;
static t_symbol *widgetps_free;
static t_symbol *widgetps_data;
static t_symbol *widgetps_add;
static t_symbol *widgetps_delete;
static t_symbol *widgetps_set;
static t_symbol *widgetps_get;

#ifdef WIDGET_PROFILE
static double widgetprofile_lasttime;

static double widgetprofile_step(void)
{
    double newtime = sys_getrealtime(),
	delta = newtime - widgetprofile_lasttime;
    widgetprofile_lasttime = newtime;
    return (delta);
}

static int widgetprofile_handlerphase = 0;
static double widgetprofile_handlerslice[3];
static double widgetprofile_handlerdelta[2];

static void widgetprofile_handler_enter(void)
{
    widgetprofile_handlerphase = 1;
    widgetprofile_step();
}

static void widgetprofile_handler_eval(void)
{
    widgetprofile_handlerphase = 2;
    widgetprofile_handlerdelta[0] = widgetprofile_step();
}

static void widgetprofile_handler_push(void)
{
    widgetprofile_handlerphase = 3;
    widgetprofile_handlerdelta[1] = widgetprofile_step();
}

static void widgetprofile_handler_quit(void)
{
    if (widgetprofile_handlerphase == 3)
    {
	widgetprofile_handlerslice[2] += widgetprofile_step();
	widgetprofile_handlerslice[0] += widgetprofile_handlerdelta[0];
	widgetprofile_handlerslice[1] += widgetprofile_handlerdelta[1];
    }
    widgetprofile_handlerphase = 0;
}

static void widget_profile(t_widget *x)
{
    loudbug_post("total time in ms:");
    loudbug_post("\thandler get %g", widgetprofile_handlerslice[0] * 1000.);
    loudbug_post("\thandler eval %g", widgetprofile_handlerslice[1] * 1000.);
    loudbug_post("\thandler push %g", widgetprofile_handlerslice[2] * 1000.);
}

#define WIDGETPROFILE_HANDLER_ENTER  widgetprofile_handler_enter()
#define WIDGETPROFILE_HANDLER_EVAL   widgetprofile_handler_eval()
#define WIDGETPROFILE_HANDLER_PUSH   widgetprofile_handler_push()
#define WIDGETPROFILE_HANDLER_QUIT   widgetprofile_handler_quit()
#else
#define WIDGETPROFILE_HANDLER_ENTER
#define WIDGETPROFILE_HANDLER_EVAL
#define WIDGETPROFILE_HANDLER_PUSH
#define WIDGETPROFILE_HANDLER_QUIT
#endif

/* resolving type and instance arguments into x_xargs */
static char *widget_propsresolver(t_pd *owner, int ac, t_atom *av)
{
    t_widget *x = (t_widget *)owner;
    int len;
    scriptlet_reset(x->x_auxscript);
    if (scriptlet_add(x->x_auxscript, 1, 0, ac, av))
	return (scriptlet_getcontents(x->x_auxscript, &len));
    else
	return (0);
}

static t_canvas *widget_cvhook(t_pd *caller)
{
    return (glist_getcanvas(((t_widget *)caller)->x_glist));
}

/* LATER move to scriptlet.c, use the scriptlet interface (.^) */
static t_symbol *widget_getcvpathname(t_widget *x, t_glist *glist)
{
    t_canvas *cv;
    if (glist && glist != x->x_glist)
    {
	loudbug_bug("widget_getcvpathname");
	x->x_glist = glist;
    }
    cv = glist_getcanvas(x->x_glist);
    if (cv == x->x_glist)
	return (x->x_cvpathname);  /* we are not in a gop */
    else
    {
	char buf[32];
	sprintf(buf, ".x%lx.c", (int)cv);
	return (gensym(buf));
    }
}

/* LATER use the scriptlet interface (.-) */
static t_symbol *widget_getmypathname(t_widget *x, t_glist *glist)
{
    char buf[64];
    t_symbol *cvpathname = widget_getcvpathname(x, glist);
    sprintf(buf, "%s.%s%x", cvpathname->s_name, x->x_name->s_name, (int)x);
    return (gensym(buf));
}

/* If Tk widget creation fails, gui will send the '_failure' message
   to the Pd widget object, asking the receiving object to transform
   itself into a regular text object.  Due to the 'bindlist' corruption
   danger, this cannot be done directly from the '_failure' call, but
   has to be scheduled through a 'transclock', instead.  When the clock
   fires, the widget object creates, and glist_adds a 'makeshift' text
   object, then glist_deletes itself. */

/* this lock prevents glist_noselect() from reevaluating failure boxes */
static int widget_transforming = 0;

/* LATER also bind this to F4 or something */
static void widget_transtick(t_widget *x)
{
    t_text *newt, *oldt = (t_text *)x;
    t_binbuf *bb = binbuf_new();
    int nopt, nhnd, narg;
    t_atom *opt = props_getall(x->x_options, &nopt);
    t_atom *hnd = props_getall(x->x_handlers, &nhnd);
    t_atom *arg = props_getall(x->x_arguments, &narg);
    if (widget_transforming++)
	loudbug_bug("widget_transtick");
    binbuf_addv(bb, "sss", gensym("widget"), x->x_type, x->x_name);
    if (narg) binbuf_add(bb, narg, arg);
    if (nopt) binbuf_add(bb, nopt, opt);
    if (nhnd) binbuf_add(bb, nhnd, hnd);
    canvas_setcurrent(x->x_glist);
    newt = (t_text *)pd_new(makeshift_class);
    newt->te_width = 0;
    newt->te_type = T_OBJECT;
    newt->te_binbuf = bb;
    newt->te_xpix = oldt->te_xpix;
    newt->te_ypix = oldt->te_ypix;
    outlet_new(newt, &s_);
    inlet_new(newt, &newt->ob_pd, &s_, &s_);
    /* LATER preserve connections (although connected widget is a bad thing) */
    glist_add(x->x_glist, &newt->te_g);
    if (glist_isvisible(x->x_glist))
    {
	glist_noselect(x->x_glist);
	glist_select(x->x_glist, &newt->te_g);
	gobj_activate(&newt->te_g, x->x_glist, 1);
	x->x_glist->gl_editor->e_textdirty = 1;  /* force evaluation */
    }
    canvas_unsetcurrent(x->x_glist);
    canvas_dirty(x->x_glist, 1);
    glist_delete(x->x_glist, (t_gobj *)x);
    widget_transforming--;
}

/* FIXME x_glist field validation against glist parameter (all handlers) */

static void widget_getrect(t_gobj *z, t_glist *glist,
			   int *xp1, int *yp1, int *xp2, int *yp2)
{
    t_widget *x = (t_widget *)z;
    float x1, y1, x2, y2;
    x1 = text_xpix((t_text *)x, glist);
    y1 = text_ypix((t_text *)x, glist);
    x2 = x1 + x->x_width;
    y2 = y1 + x->x_height;
    *xp1 = x1;
    *yp1 = y1;
    *xp2 = x2;
    *yp2 = y2;
}

static void widget_displace(t_gobj *z, t_glist *glist, int dx, int dy)
{
    t_widget *x = (t_widget *)z;
    t_text *t = (t_text *)z;
#if 0
    loudbug_post("displace %d %d (%d %d -> %d %d)",
		 dx, dy, t->te_xpix, t->te_ypix,
		 t->te_xpix + dx, t->te_ypix + dy);
#endif
    t->te_xpix += dx;
    t->te_ypix += dy;
    if (glist_isvisible(glist))
	sys_vgui("%s move %s %d %d\n", widget_getcvpathname(x, glist)->s_name,
		 x->x_cbtarget->s_name, dx, dy);
    canvas_fixlinesfor(glist, t);
}

/* LATER handle subitems */
static void widget_select(t_gobj *z, t_glist *glist, int flag)
{
    t_widget *x = (t_widget *)z;
    char *mypathname = widget_getmypathname(x, glist)->s_name;
    if (flag)
    {
	sys_vgui("%s config -bg blue\n", mypathname);
	sys_vgui("event generate %s <<disable>>\n", mypathname);
	x->x_selected = 1;
    }
    else
    {
	if (x->x_disabled)
	    sys_vgui("%s config -bg %s\n", mypathname,
		     (x->x_background ? x->x_background->s_name : "gray"));
	else
	    sys_vgui("%s config -bg %s \n", mypathname,
		     (x->x_background ? x->x_background->s_name : "gray"));
	sys_vgui("event generate %s <<enable>>\n", mypathname);
	x->x_selected = 0;
    }
}

static void widget_delete(t_gobj *z, t_glist *glist)
{
    canvas_deletelinesfor(glist, (t_text *)z);
}

static void widget_pushoptions(t_widget *x, int doit)
{
    char *mypathname = widget_getmypathname(x, x->x_glist)->s_name;
    if (scriptlet_evaluate(x->x_optscript, x->x_transient, 0, 0, 0, x->x_xargs))
    {
#ifdef WIDGET_DEBUG
	int sz;
	char *dp = scriptlet_getcontents(x->x_transient, &sz);
	loudbug_post("vis: \"%s\"", dp);
#endif
	if (doit)
	{
	    sys_vgui("%s config ", mypathname);
	    scriptlet_push(x->x_transient);
	}
	else scriptlet_vpush(x->x_transient, "itemoptions");
    }
    else if (!scriptlet_isempty(x->x_optscript))
	loudbug_bug("widget_pushoptions");
}

static void widget_pushonehook(t_widget *x, t_scriptlet *sp, char *vname)
{
    if (scriptlet_evaluate(sp, x->x_transient, 0, 0, 0, x->x_xargs))
	scriptlet_vpush(x->x_transient, vname);
    else if (!scriptlet_isempty(sp))
	loudbug_bug("widget_pushonehook (%s)", vname);
}

static void widget_pushvishooks(t_widget *x)
{
    if (widgettype_isdefined(x->x_typedef))
    {
	t_widgethandlers *wh = widgettype_getscripts(x->x_typedef);
	widget_pushonehook(x, widgethandlers_getvis(wh), "longvishook");
    }
    widget_pushonehook(x, widgethandlers_getvis(x->x_hooks), "shortvishook");
}

static void widget_pushnewhooks(t_widget *x)
{
    /* LATER master constructor */
    if (widgettype_isdefined(x->x_typedef))
    {
	t_widgethandlers *wh = widgettype_getscripts(x->x_typedef);
	widget_pushonehook(x, widgethandlers_getnew(wh), "longnewhook");
    }
    widget_pushonehook(x, widgethandlers_getnew(x->x_hooks), "shortnewhook");
}

static void widget_pushfreehooks(t_widget *x)
{
    /* LATER master destructor */
    if (widgettype_isdefined(x->x_typedef))
    {
	t_widgethandlers *wh = widgettype_getscripts(x->x_typedef);
	widget_pushonehook(x, widgethandlers_getfree(wh), "longfreehook");
    }
    widget_pushonehook(x, widgethandlers_getfree(x->x_hooks), "shortfreehook");
}

static void widget_pushdatahooks(t_widget *x, int ac, t_atom *av)
{
    t_scriptlet *sp;
    WIDGETPROFILE_HANDLER_ENTER;
    if (!widgettype_isdefined(x->x_typedef)
	|| !(sp = widgethandlers_getdata(widgettype_getscripts(x->x_typedef)))
	|| scriptlet_isempty(sp))
	sp = widgethandlers_getdata(x->x_hooks);
    if (sp)
    {
	WIDGETPROFILE_HANDLER_EVAL;
	if (scriptlet_evaluate(sp, x->x_transient, 0, ac, av, x->x_xargs))
	{
	    WIDGETPROFILE_HANDLER_PUSH;
	    scriptlet_push(x->x_transient);
	}
	else if (!scriptlet_isempty(sp))
	    loudbug_bug("widget_pushdatahooks (%s)",
			(sp == widgethandlers_getdata(x->x_hooks) ?
			 "short" : "long"));
    }
    WIDGETPROFILE_HANDLER_QUIT;
}

static void widget_getconfig(t_widget *x)
{
    sys_vgui("::toxy::item_getconfig %s %s\n",
	     widget_getmypathname(x, x->x_glist)->s_name,
	     x->x_cbtarget->s_name);
}

static void widget_vis(t_gobj *z, t_glist *glist, int vis)
{
    t_widget *x = (t_widget *)z;
    t_text *t = (t_text *)z;
    char *cvpathname = widget_getcvpathname(x, glist)->s_name;
    char *mypathname = widget_getmypathname(x, glist)->s_name;
    x->x_update = WIDGET_NOVIS;
    if (vis)
    {
	float px1 = text_xpix((t_text *)x, glist);
	float py1 = text_ypix((t_text *)x, glist);
#ifndef PD_MINOR_VERSION
	rtext_new(glist, t, glist->gl_editor->e_rtext, 0);
#endif
	widget_pushoptions(x, 0);
	widget_pushvishooks(x);
	if (!x->x_constructed)
	{
	    widget_pushnewhooks(x);
	    x->x_constructed = 1;
	}
	sys_vgui("::toxy::item_vis %s %s %s %s %s %s %g %g\n",
		 x->x_tkclass->s_name, mypathname,
		 x->x_cbtarget->s_name, x->x_name->s_name,
		 x->x_varname->s_name, cvpathname, px1, py1);
	x->x_vised = 1;
    }
    else
    {
#ifndef PD_MINOR_VERSION
	t_rtext *rt = glist_findrtext(glist, t);
	if (rt) rtext_free(rt);
#endif
	x->x_vised = 0;
    }
}

static void widget_save(t_gobj *z, t_binbuf *bb)
{
    t_widget *x = (t_widget *)z;
    t_text *t = (t_text *)x;
    int nopt, nhnd, narg;
    t_atom *opt = props_getall(x->x_options, &nopt);
    t_atom *hnd = props_getall(x->x_handlers, &nhnd);
    t_atom *arg = props_getall(x->x_arguments, &narg);
    binbuf_addv(bb, "ssiisss", gensym("#X"), gensym("obj"),
		(int)t->te_xpix, (int)t->te_ypix, 
        atom_getsymbol(binbuf_getvec(t->te_binbuf)),
		x->x_type, x->x_name);
    if (narg) binbuf_add(bb, narg, arg);
    if (nopt) binbuf_add(bb, nopt, opt);
    if (nhnd) binbuf_add(bb, nhnd, hnd);
    binbuf_addsemi(bb);
}

static void widget_editorappend(t_widget *x, t_props *pp)
{
    int ac;
    t_atom *ap;
    if (ap = props_getfirst(pp, &ac))
    {
	if (pp != x->x_diffoptions)
	    hammereditor_append(x->x_filehandle, "\n");
	do
	{
	    int nleft;
	    char buf[MAXPDSTRING + 1], *head;
	    buf[MAXPDSTRING] = 0;
	    scriptlet_reset(x->x_auxscript);
	    scriptlet_add(x->x_auxscript, 0, 0, ac, ap);
	    head = scriptlet_getcontents(x->x_auxscript, &nleft);
	    while (nleft > 0)
	    {
		if (nleft > MAXPDSTRING)
		{
		    strncpy(buf, head, MAXPDSTRING);
		    head += MAXPDSTRING;
		    nleft -= MAXPDSTRING;
		}
		else
		{
		    strncpy(buf, head, nleft);
		    buf[nleft] = 0;
		    nleft = 0;
		}
		hammereditor_append(x->x_filehandle, buf);
	    }
	    hammereditor_append(x->x_filehandle, "\n");
	}
	while (ap = props_getnext(pp, &ac));
    }
}

static void widget_properties(t_gobj *z, t_glist *glist)
{
    t_widget *x = (t_widget *)z;
    char buf[MAXPDSTRING];
    sprintf(buf, "%s %s", x->x_type->s_name, x->x_name->s_name);
    hammereditor_open(x->x_filehandle, buf, 0);
    widget_editorappend(x, x->x_diffoptions);
    widget_editorappend(x, x->x_options);
    widget_editorappend(x, x->x_diffhandlers);
    widget_editorappend(x, x->x_handlers);
    widget_editorappend(x, x->x_diffarguments);
    widget_editorappend(x, x->x_arguments);
    hammereditor_setdirty(x->x_filehandle, 0);
}

static t_widgetbehavior widget_behavior =
{
    widget_getrect,
    widget_displace,
    widget_select,
    0,
    widget_delete,
    widget_vis,
    0,
    FORKY_WIDGETPADDING
};

static void widget_novis(t_widget *x)
{
    sys_vgui("::toxy::item_destroy %s %s\n",
	     widget_getmypathname(x, x->x_glist)->s_name, x->x_varname->s_name);
}

static void widget_update(t_widget *x, t_props *op)
{
    if (op == x->x_options)
    {
	t_atom *ap;
	int ac;
	props_diff(x->x_diffoptions,
		   widgettype_getoptions(x->x_typedef), x->x_options);
	scriptlet_reset(x->x_optscript);
	ap = props_getall(x->x_diffoptions, &ac);
	if (ac) scriptlet_add(x->x_optscript, 0, 0, ac, ap);
	ap = props_getall(x->x_options, &ac);
	if (ac) scriptlet_add(x->x_optscript, 0, 0, ac, ap);
	if (x->x_update &&
	    glist_isvisible(x->x_glist))  /* FIXME the condition */
	{
	    if (x->x_update == WIDGET_REVIS)
	    {
		widget_novis(x);
		widget_vis((t_gobj *)x, x->x_glist, 1);
	    }
	    else if (x->x_update == WIDGET_PUSHVIS)
	    {
		widget_pushoptions(x, 1);
		widget_getconfig(x);
	    }
	    x->x_update = WIDGET_NOVIS;
	}
    }
    else if (op == x->x_handlers)
    {
	props_diff(x->x_diffhandlers,
		   widgettype_gethandlers(x->x_typedef), x->x_handlers);
	/* This is the only point where mirroring of handlers is performed.
	   We get here both during construction, and after any change
	   in our handlers -- the mirror never stales. */
	widgethandlers_reset(x->x_hooks);
	widgethandlers_fill(x->x_hooks, x->x_diffhandlers);
	widgethandlers_fill(x->x_hooks, x->x_handlers);
    }
    else if (op == x->x_arguments)
    {
	props_diff(x->x_diffarguments,
		   widgettype_getarguments(x->x_typedef), x->x_arguments);
	props_clearvalues(x->x_xargs);
	props_clonevalues(x->x_xargs, x->x_diffarguments);
	props_clonevalues(x->x_xargs, x->x_arguments);
    }
}

static t_symbol *widget_addprops(t_widget *x, t_props *op,
				 int single, t_props *filter,
				 t_symbol *s, int ac, t_atom *av)
{
    if (op)
    {
	t_symbol *empty;
	empty = props_add(op, single, filter, s, ac, av);
	if (empty)
	    loud_error((t_pd *)x, "no value given for %s '%s'",
		       props_getname(op), empty->s_name);
	widget_update(x, op);
	return (empty);
    }
    else
    {
	loudbug_bug("widget_addprops");
	return (0);
    }
}

static t_symbol *widget_addmessage(t_widget *x, int unique,
				   t_symbol *s, int ac, t_atom *av)
{
    t_symbol *empty;
    if (s)
    {
	/* FIXME mixed messages */
	if (*s->s_name == '-')
	    x->x_update = WIDGET_PUSHVIS;
	else if (*s->s_name == '#')
	    x->x_update = WIDGET_REVIS;
	else
	    x->x_update = WIDGET_NOVIS;
    }
    /* Instance-type duplicates are not removed, unless 'unique' is set.
       If it is set, we are called from editorhook, so we assume duplicates
       were not specified explicitly.  In other cases we keep duplicates,
       because type may change until next time this widget is created or
       refreshed. */
    if (!(empty = widget_addprops(x, x->x_arguments, 0,
				  (unique ? x->x_diffarguments : 0),
				  s, ac, av)) &&
	!(empty = widget_addprops(x, x->x_handlers, 0,
				  (unique ? x->x_diffhandlers : 0),
				  s, ac, av)))
	empty = widget_addprops(x, x->x_options, 0,
				(unique ? x->x_diffoptions : 0),
				s, ac, av);
    return (empty);
}

static void widget_anything(t_widget *x, t_symbol *s, int ac, t_atom *av)
{
    if (s && s != &s_)
    {
	if (*s->s_name == '-' || *s->s_name == '@' || *s->s_name == '#')
	{
	    t_symbol *empty;
	    if (empty = widget_addmessage(x, 0, s, ac, av))
		loud_errand((t_pd *)x,
			    "(use 'remove %s' if that is what you want).",
			    empty->s_name);
	}
	else
	{
	    /* FIXME use long defs too, cf widget_pushdatahooks() */
	    t_scriptlet *sp;
	    WIDGETPROFILE_HANDLER_ENTER;
	    if (sp = widgethandlers_getother(x->x_hooks, s))
	    {
		WIDGETPROFILE_HANDLER_EVAL;
		if (scriptlet_evaluate(sp, x->x_transient,
				       1, ac, av, x->x_xargs))
		{
		    WIDGETPROFILE_HANDLER_PUSH;
		    scriptlet_push(x->x_transient);
		}
		else if (!scriptlet_isempty(sp))
		    loudbug_bug("widget_anything");
	    }
	    else if (s == widgetps_vis || s == widgetps_new ||
		     s == widgetps_free || s == widgetps_data)
		loud_error((t_pd *)x,
			   "explicit call of the special handler \"%s\"",
			   s->s_name);
	    else
		loud_nomethod((t_pd *)x, s);
	    WIDGETPROFILE_HANDLER_QUIT;
	}
    }
}

/* FIXME use long defs too, cf widget_pushdatahooks() */
static void widget_bang(t_widget *x)
{
    t_scriptlet *sp;
    WIDGETPROFILE_HANDLER_ENTER;
    sp = widgethandlers_getbang(x->x_hooks);
    WIDGETPROFILE_HANDLER_EVAL;
    if (scriptlet_evaluate(sp, x->x_transient, 1, 0, 0, x->x_xargs))
    {
	WIDGETPROFILE_HANDLER_PUSH;
	scriptlet_push(x->x_transient);
    }
    else if (!scriptlet_isempty(sp))
	loudbug_bug("widget_bang");
    WIDGETPROFILE_HANDLER_QUIT;
}

/* FIXME use long defs too, cf widget_pushdatahooks() */
static void widget_float(t_widget *x, t_float f)
{
    t_scriptlet *sp;
    t_atom at;
    WIDGETPROFILE_HANDLER_ENTER;
    sp = widgethandlers_getfloat(x->x_hooks);
    WIDGETPROFILE_HANDLER_EVAL;
    SETFLOAT(&at, f);
    if (scriptlet_evaluate(sp, x->x_transient, 1, 1, &at, x->x_xargs))
    {
	WIDGETPROFILE_HANDLER_PUSH;
	scriptlet_push(x->x_transient);
    }
    else if (!scriptlet_isempty(sp))
	loudbug_bug("widget_float");
    WIDGETPROFILE_HANDLER_QUIT;
}

/* FIXME use long defs too, cf widget_pushdatahooks() */
static void widget_symbol(t_widget *x, t_symbol *s)
{
    t_scriptlet *sp;
    t_atom at;
    WIDGETPROFILE_HANDLER_ENTER;
    sp = widgethandlers_getsymbol(x->x_hooks);
    WIDGETPROFILE_HANDLER_EVAL;
    SETSYMBOL(&at, s);
    if (scriptlet_evaluate(sp, x->x_transient, 1, 1, &at, x->x_xargs))
    {
	WIDGETPROFILE_HANDLER_PUSH;
	scriptlet_push(x->x_transient);
    }
    else if (!scriptlet_isempty(sp))
	loudbug_bug("widget_symbol");
    WIDGETPROFILE_HANDLER_QUIT;
}

static void widget_set(t_widget *x, t_symbol *s, int ac, t_atom *av)
{
    t_symbol *prp;
    if (ac && av->a_type == A_SYMBOL && (prp = av->a_w.w_symbol))
    {
	t_symbol *empty = 0;
	ac--; av++;
	if (*prp->s_name == '-')
	{
	    x->x_update = WIDGET_PUSHVIS;
	    empty = widget_addprops(x, x->x_options, 1, 0, prp, ac, av);
	}
	else if (*prp->s_name == '@')
	    empty = widget_addprops(x, x->x_handlers, 1, 0, prp, ac, av);
	else if (*prp->s_name == '#')
	    empty = widget_addprops(x, x->x_arguments, 1, 0, prp, ac, av);
	if (empty)
	    loud_errand((t_pd *)x,
			"(use 'remove %s' if that is what you want).",
			empty->s_name);
    }
    else loud_messarg((t_pd *)x, s);
}

static void widget_remove(t_widget *x, t_symbol *s)
{
    if (s)
    {
	t_props *op;
	if (*s->s_name == '-')
	    op = x->x_options;
	else if (*s->s_name == '@')
	    op = x->x_handlers;
	else if (*s->s_name == '#')
	    op = x->x_arguments;
	else
	    op = 0;
	if (op && props_remove(op, s))
	{
	    if (op == x->x_options)  /* LATER rethink */
		x->x_update = WIDGET_REVIS;
	    widget_update(x, op);
	}
	else loud_warning((t_pd *)x, 0, "%s %s has not been specified",
			  props_getname(op), s->s_name);
    }
}

static void widget_tot(t_widget *x, t_symbol *s, int ac, t_atom *av)
{
    if (ac)
    {
	scriptlet_reset(x->x_auxscript);
	scriptlet_add(x->x_auxscript, 1, 1, ac, av);
	if (scriptlet_evaluate(x->x_auxscript, x->x_transient, 1,
			       0, 0, x->x_xargs))
	    scriptlet_push(x->x_transient);
    }
}

static void widget_refresh(t_widget *x)
{
    x->x_update = WIDGET_REVIS;
    widget_update(x, x->x_arguments);
    widget_update(x, x->x_handlers);
    widget_update(x, x->x_options);
}

static int widget_resettype(t_widget *x, t_widgettype *wt)
{
    if (!wt ||  /* LATER rethink, cf widgettype_reload() */
	wt == x->x_typedef)
    {
	widget_pushfreehooks(x);
	if (!(x->x_tkclass = widgettype_tkclass(x->x_typedef)))
	    x->x_tkclass = x->x_type;
	x->x_update = WIDGET_REVIS;
	widget_update(x, x->x_arguments);
	widget_pushnewhooks(x);
	widget_update(x, x->x_handlers);
	widget_update(x, x->x_options);
	return (1);
    }
    else
    {
	loudbug_bug("widget_resettype");
	return (0);
    }
}

static void widget_redefine(t_widget *x)
{
    widget_resettype(x, widgettype_reload(x->x_type, x->x_glist));
}

static void widget_editorhook(t_pd *z, t_symbol *s, int ac, t_atom *av)
{
    t_widget *x = (t_widget *)z;
    props_clearall(x->x_options);
    widget_addmessage(x, 1, 0, ac, av);
    widget_refresh(x);
}

static void widget__failure(t_widget *x)
{
    loud_error((t_pd *)x, "creation failure");
    /* details printed at the gui side, in order to support special chars
       in error message */
    loud_errand((t_pd *)x, "see standard error for details");
    x->x_vised = 0;
    clock_delay(x->x_transclock, 0);
}

/* LATER handle subitems */
static void widget__config(t_widget *x, t_symbol *target, t_symbol *bg,
			   t_floatarg fw, t_floatarg fh, t_floatarg fst)
{
#ifdef WIDGET_DEBUG
    loudbug_post("config %x %s \"%s\" %g %g",
		 (int)x, target->s_name, bg->s_name, fw, fh);
#endif
    x->x_width = (int)fw;
    x->x_height = (int)fh;
    if (bg != &s_) x->x_background = bg;
    x->x_hasstate = ((int)fst == 0);
    canvas_fixlinesfor(x->x_glist, (t_text *)x);  /* FIXME */
}

/* FIXME this is only a template */
static void widget__data(t_widget *x, t_symbol *s, int ac, t_atom *av)
{
#ifdef WIDGET_DEBUG
    loudbug_startpost("_data:");
    loudbug_postatom(ac, av);
    loudbug_endpost();
#endif
    if (ac && av->a_type == A_SYMBOL)
    {
	s = av->a_w.w_symbol;
	if (s == widgetps_add)
	{
	    widget_pushdatahooks(x, ac, av);
	}
	else if (s == widgetps_delete)
	{
	    widget_pushdatahooks(x, ac, av);
	}
	else if (s == widgetps_set)
	{
	    widget_pushdatahooks(x, ac, av);
	}
	else if (s == widgetps_get)
	{
	    widget_pushdatahooks(x, ac, av);
	}
	else loud_error((t_pd *)x,
			"invalid \"_data\" subcommand \"%s\"", s->s_name);
    }
    else loud_error((t_pd *)x, "missing \"_data\" subcommand");
}

static void widget__callback(t_widget *x, t_symbol *s, int ac, t_atom *av)
{
    if (ac == 1)
    {
	if (av->a_type == A_FLOAT)
	    outlet_float(((t_object *)x)->ob_outlet, av->a_w.w_float);
	else if (av->a_type == A_SYMBOL)
	    outlet_symbol(((t_object *)x)->ob_outlet, av->a_w.w_symbol);
    }
    else if (ac)
    {
	if (av->a_type == A_FLOAT)
	    outlet_list(((t_object *)x)->ob_outlet, &s_list, ac, av);
	else if (av->a_type == A_SYMBOL)
	    outlet_anything(((t_object *)x)->ob_outlet,
			    av->a_w.w_symbol, ac - 1, av + 1);
    }
    else outlet_bang(((t_object *)x)->ob_outlet);
}

/* see also widget_select() */
/* LATER handle subitems */
static void widget__inout(t_widget *x, t_floatarg f)
{
    int disable = (int)f && x->x_glist->gl_edit;
    if (x->x_disabled)
    {
	if (!disable)
	{
	    if (!x->x_selected)
	    {
		char *mypathname = widget_getmypathname(x, x->x_glist)->s_name;
		sys_vgui("event generate %s <<enable>>\n", mypathname);
	    }
	    x->x_disabled = 0;
	}
    }
    else if (disable)
    {
	if (!x->x_selected)
	{
	    char *mypathname = widget_getmypathname(x, x->x_glist)->s_name;
	    sys_vgui("event generate %s <<disable>>\n", mypathname);
	}
	x->x_disabled = 1;
    }
}

static void widget__click(t_widget *x, t_symbol *s, int ac, t_atom *av)
{
    if (ac != 4)
    {
	loud_error((t_pd *)x, "bad arguments to the '%s' method", s->s_name);
	return;
    }
    if (x->x_glist->gl_havewindow)  /* LATER calculate on-parent coords */
    {
	if (x->x_cvtarget->s_thing)
	    /* LATER rethink */
	    typedmess(x->x_cvtarget->s_thing, widgetps_mouse, ac, av);
	else
	    typedmess((t_pd *)x->x_glist, widgetps_mouse, ac, av);
	widget__inout(x, 2.);
    }
}

/* LATER think how to grab the mouse when dragging */
static void widget__motion(t_widget *x, t_symbol *s, int ac, t_atom *av)
{
    if (ac != 3)
    {
	loud_error((t_pd *)x, "bad arguments to the '%s' method", s->s_name);
	return;
    }
    if (x->x_glist->gl_havewindow)  /* LATER calculate on-parent coords */
    {
#if 0
	loudbug_post("motion %g %g", av[0].a_w.w_float, av[1].a_w.w_float);
#endif
	if (x->x_cvtarget->s_thing)
	    /* LATER rethink */
	    typedmess(x->x_cvtarget->s_thing, widgetps_motion, ac, av);
	else
	    typedmess((t_pd *)x->x_glist, widgetps_motion, ac, av);
    }
}

int widget_iswidget(t_gobj *g, t_symbol *type, t_symbol *name)
{
    if (*(t_pd *)g == widget_class)
    {
	t_widget *x = (t_widget *)g;
	return ((!type || type == x->x_type) &&
		(!name || name == x->x_name));
    }
    else return (0);
}

#ifdef WIDGET_DEBUG
static void widgetbug_postprops(char *msg, t_props *pp)
{
    int ac;
    t_atom *av = props_getall(pp, &ac);
    if (av)
    {
	loudbug_startpost(msg);
	loudbug_postatom(ac, av);
	loudbug_endpost();
    }
}

static void widget_debug(t_widget *x)
{
    t_widgethandlers *wh = widgettype_getscripts(x->x_typedef);
    t_symbol *pn = widget_getcvpathname(x, 0);
    t_symbol *mn = widget_getmypathname(x, 0);
    int sz, i, nopt;
    t_atom *ap;
    static char bempty[] = "<empty>";
    char *bp, *key;
    loudbug_post("containing glist: %x", (int)x->x_glist);
    loudbug_post("cv pathname%s %s",
		 (pn ? ":" : ""), (pn ? pn->s_name : "unknown"));
    loudbug_post("my pathname%s %s",
		 (mn ? ":" : ""), (mn ? mn->s_name : "unknown"));
    widgetbug_postprops("default options:",
			widgettype_getoptions(x->x_typedef));
    widgetbug_postprops("instance options:", x->x_options);
    widgetbug_postprops("diff options:", x->x_diffoptions);
    widgetbug_postprops("default handlers:",
			widgettype_gethandlers(x->x_typedef));
    widgetbug_postprops("instance handlers:", x->x_handlers);
    widgetbug_postprops("diff handlers:", x->x_diffhandlers);
    widgetbug_postprops("default arguments:",
			widgettype_getarguments(x->x_typedef));
    widgetbug_postprops("instance arguments:", x->x_arguments);
    widgetbug_postprops("diff arguments:", x->x_diffarguments);
    loudbug_post("dictionary:");
    bp = props_firstvalue(x->x_xargs, &key);
    while (bp)
    {
	loudbug_post("\t%s: \"%s\"", key, bp);
	bp = props_nextvalue(x->x_xargs, &key);
    }
    bp = scriptlet_getcontents(x->x_transient, &sz);
    loudbug_post("transient buffer (size %d):\n\"%s\"", sz, (bp ? bp : bempty));
    bp = scriptlet_getcontents(x->x_optscript, &sz);
    loudbug_post("option buffer (size %d):\n\"%s\"", sz, (bp ? bp : bempty));

    bp = scriptlet_getcontents(widgethandlers_getnew(wh), &sz);
    loudbug_post("long newhook (size %d):\n\"%s\"", sz, (bp ? bp : bempty));
    bp = scriptlet_getcontents(widgethandlers_getfree(wh), &sz);
    loudbug_post("long freehook (size %d):\n\"%s\"", sz, (bp ? bp : bempty));
    bp = scriptlet_getcontents(widgethandlers_getdata(wh), &sz);
    loudbug_post("long datahook (size %d):\n\"%s\"", sz, (bp ? bp : bempty));

    bp = scriptlet_getcontents(widgethandlers_getvis(wh), &sz);
    loudbug_post("long vishook (size %d):\n\"%s\"", sz, (bp ? bp : bempty));
    bp = scriptlet_getcontents(widgethandlers_getvis(x->x_hooks), &sz);
    loudbug_post("short vishook (size %d):\n\"%s\"",
		 sz, (bp ? bp : bempty));

    bp = masterwidget_getcontents(&sz);
    loudbug_post("setup definitions (size %d):\n\"%s\"",
		 sz, (bp ? bp : bempty));
}
#endif

static void widget_attach(t_widget *x);
static void widget_detach(t_widget *x);

/* FIXME for all gui objects use a single sink (with a single clock) */
static void gui_unbind(t_pd *x, t_symbol *s)
{
    t_guiconnect *gc = guiconnect_new(0, s);
    guiconnect_notarget(gc, 1000.);
    pd_unbind(x, s);
}

static void widget_free(t_widget *x)
{
    widget_novis(x);
    widget_pushfreehooks(x);
    gui_unbind((t_pd *)x, x->x_cbtarget);
    gui_unbind((t_pd *)x, x->x_rptarget);
    widgethandlers_free(x->x_hooks);
    props_freeall(x->x_options);
    props_freeall(x->x_xargs);
    props_freeall(x->x_diffoptions);
    scriptlet_free(x->x_optscript);
    scriptlet_free(x->x_auxscript);
    scriptlet_free(x->x_transient);
    hammerfile_free(x->x_filehandle);
    if (x->x_transclock) clock_free(x->x_transclock);
    widget_detach(x);
}

static void *widget_new(t_symbol *s, int ac, t_atom *av)
{
    t_widget *x;
    t_symbol *type = 0, *name = 0;
    char buf[MAXPDSTRING];
    if (widget_transforming)
	return (0);
    masterwidget_validate();
    if (ac && av->a_type == A_SYMBOL)
    {
	type = av->a_w.w_symbol;
	ac--; av++;
    }
    if (ac && av->a_type == A_SYMBOL)
    {
	name = av->a_w.w_symbol;
	ac--; av++;
    }
    /* LATER think about anonymous widgets (single arg, or '.') */
    if (!type || type == &s_ || !name || name == &s_)
    {
	loud_error(0, "bad arguments for a widget");
	loud_errand(0, "expecting \"widget <type> <name> [properties]\"");
	return (0);
    }

    x = (t_widget *)pd_new(widget_class);
    x->x_type = type;
    x->x_name = name;

    sprintf(buf, "%s%x", x->x_name->s_name, (int)x);
    pd_bind((t_pd *)x, x->x_cbtarget = gensym(buf));
    sprintf(buf, "%s%x.rp", x->x_name->s_name, (int)x);
    pd_bind((t_pd *)x, x->x_rptarget = gensym(buf));

    x->x_glist = canvas_getcurrent();
    x->x_typedef = widgettype_get(x->x_type, 0, 0, x->x_glist);
    if (!(x->x_tkclass = widgettype_tkclass(x->x_typedef)))
	x->x_tkclass = x->x_type;

    sprintf(buf, ".x%lx.c", (int)x->x_glist);
    x->x_cvpathname = gensym(buf);
    sprintf(buf, ".x%lx", (int)x->x_glist);
    x->x_cvtarget = gensym(buf);
    sprintf(buf, "::toxy::v%x", (int)x);
    x->x_varname = gensym(buf);

    x->x_auxscript = scriptlet_new((t_pd *)x, x->x_rptarget, x->x_cbtarget,
				   x->x_name, x->x_glist, widget_cvhook);
    x->x_transient = scriptlet_newalike(x->x_auxscript);
    x->x_optscript = scriptlet_newalike(x->x_auxscript);

    x->x_options = props_new((t_pd *)x, "option", "-", 0, 0);
    x->x_handlers = props_new((t_pd *)x, "handler", "@", x->x_options, 0);
    x->x_arguments = props_new((t_pd *)x, "argument", "#", x->x_options, 0);
    x->x_xargs = props_new((t_pd *)x, "argument", "#", 0, widget_propsresolver);
    x->x_diffoptions = props_new((t_pd *)x, "option", "-", 0, 0);
    x->x_diffhandlers = props_new((t_pd *)x, "handler", "@",
				  x->x_diffoptions, 0);
    x->x_diffarguments = props_new((t_pd *)x, "argument", "#",
				   x->x_diffoptions, 0);

    x->x_hooks = widgethandlers_new(x->x_auxscript);

    outlet_new((t_object *)x, &s_anything);
    /* LATER consider estimating these, based on widget class and options.
       The default used to be 50x50, which confused people wanting widgets
       in small gops, of size exactly as specified by the 'coords' message,
       but finding gops stretched, to accomodate the widget's default area. */
    x->x_width = 5;
    x->x_height = 5;
    x->x_filehandle = hammerfile_new((t_pd *)x, 0, 0, 0, widget_editorhook);
    x->x_transclock = clock_new(x, (t_method)widget_transtick);
    x->x_background = 0;
    x->x_hasstate = 0;
    x->x_update = WIDGET_NOVIS;
    x->x_disabled = 0;
    x->x_vised = 0;
    widget_attach(x);
    widget_addmessage(x, 0, 0, ac, av);
    x->x_constructed = 0;
    return (x);
}

static t_glist *tow_getglist(t_tow *x, int complain)
{
    t_glist *glist = (x->x_cvremote ?
		      (t_glist *)pd_findbyclass(x->x_cvremote, canvas_class) :
		      x->x_targetglist);
    if (!glist && x->x_cvname && complain)
	loud_error((t_pd *)x, "bad canvas name '%s'", x->x_cvname->s_name);
    return (glist);
}

static void tow_widgetattach(t_tow *x, t_widget *w)
{
    t_towentry *te = getbytes(sizeof(*te));
    t_widgetentry *we = getbytes(sizeof(*we));
    te->te_tow = x;
    te->te_next = w->x_towlist;
    w->x_towlist = te;
    we->we_widget = w;
    we->we_next = x->x_widgetlist;
    x->x_widgetlist = we;
    pd_bind((t_pd *)x, w->x_cbtarget);
#ifdef TOW_DEBUG
    loudbug_post("%s widget '%s' attached",
		 w->x_type->s_name, w->x_cbtarget->s_name);
#endif
}

static void tow_widgetdetach(t_tow *x, t_widget *w)
{
    t_widgetentry *we1, *we2;
    for (we1 = 0, we2 = x->x_widgetlist; we2; we2 = we2->we_next)
    {
	if (we2->we_widget == w)
	{
#ifdef TOW_DEBUG
	    loudbug_post("%s widget '%s' detached by widget's destructor",
			 w->x_type->s_name, w->x_cbtarget->s_name);
#endif
	    pd_unbind((t_pd *)x, w->x_cbtarget);
	    if (we1)
		we1->we_next = we2->we_next;
	    else
		x->x_widgetlist = we2->we_next;
	    freebytes(we2, sizeof(*we2));
	    return;
	}
	we1 = we2;
    }
    loudbug_bug("tow_widgetdetach");
}

static void widget_attach(t_widget *x)
{
    t_tow *t;
    for (t = widget_towlist; t; t = t->x_next)
	if (x->x_glist == tow_getglist(t, 0) &&
	    t->x_type == x->x_type && t->x_name == x->x_name)
	    tow_widgetattach(t, x);
}

static void widget_detach(t_widget *x)
{
    t_towentry *te;
    while (te = x->x_towlist)
    {
	x->x_towlist = te->te_next;
	tow_widgetdetach(te->te_tow, x);
	freebytes(te, sizeof(*te));
    }
}

static void tow_attach(t_tow *x)
{
    t_glist *glist = tow_getglist(x, 0);
    if (glist)
    {
	t_gobj *g;
	for (g = glist->gl_list; g; g = g->g_next)
	{
	    if (*(t_pd *)g == widget_class)
	    {
		t_widget *w = (t_widget *)g;
		if (w->x_type == x->x_type && w->x_name == x->x_name)
		    tow_widgetattach(x, w);
	    }
	}
#ifdef TOW_DEBUG
	if (!x->x_widgetlist)
	    loudbug_post("%s widget '%s' not found",
			 x->x_type->s_name, x->x_name->s_name);
#endif
    }
#ifdef TOW_DEBUG
    else if (x->x_cvname)
	loudbug_post("glist '%s' not found", x->x_cvname->s_name);
#endif
}

static void tow_detach(t_tow *x)
{
    t_widgetentry *we;
    while (we = x->x_widgetlist)
    {
	t_widget *w = we->we_widget;
	t_towentry *te1, *te2;
	x->x_widgetlist = we->we_next;
	pd_unbind((t_pd *)x, w->x_cbtarget);
	freebytes(we, sizeof(*we));
	for (te1 = 0, te2 = w->x_towlist; te2; te2 = te2->te_next)
	{
	    if (te2->te_tow == x)
	    {
#ifdef TOW_DEBUG
		loudbug_post("%s widget '%s' detached by tow's destructor",
			     w->x_type->s_name, w->x_cbtarget->s_name);
#endif
		if (te1)
		    te1->te_next = te2->te_next;
		else
		    w->x_towlist = te2->te_next;
		freebytes(te2, sizeof(*te2));
		break;
	    }
	    te1 = te2;
	}
	if (!te2) loudbug_bug("tow_detach");
    }
}

static void tow_bang(t_tow *x)
{
    t_widgetentry *we;
    for (we = x->x_widgetlist; we; we = we->we_next)
	widget_bang(we->we_widget);
}

static void tow_float(t_tow *x, t_float f)
{
    t_widgetentry *we;
    for (we = x->x_widgetlist; we; we = we->we_next)
	widget_float(we->we_widget, f);
}

static void tow_symbol(t_tow *x, t_symbol *s)
{
    t_widgetentry *we;
    for (we = x->x_widgetlist; we; we = we->we_next)
	widget_symbol(we->we_widget, s);
}

static void tow_anything(t_tow *x, t_symbol *s, int ac, t_atom *av)
{
    t_widgetentry *we;
    for (we = x->x_widgetlist; we; we = we->we_next)
	typedmess((t_pd *)we->we_widget, s, ac, av);
}

static void tow_redefine(t_tow *x)
{
    t_widgettype *wt = widgettype_reload(x->x_type, x->x_glist);
    t_widgetentry *we;
    for (we = x->x_widgetlist; we; we = we->we_next)
	if (!widget_resettype(we->we_widget, wt))
	    break;
}

/* LATER broadcasting: canvas-wide or type-on-canvas-wide */
static void tow_settarget(t_tow *x, t_symbol *s1, t_symbol *s2, t_symbol *s3)
{
    char buf[64];
    if (s1 == &s_ || !strcmp(s1->s_name, "."))
	s1 = 0;
    if (s1 && strcmp(s1->s_name, ".parent"))
    {
	x->x_cvremote = canvas_makebindsym(x->x_cvname = s1);
	x->x_targetglist = 0;
    }
    else
    {
	x->x_cvremote = 0;
	if (s1)
	{
	    if (x->x_glist->gl_owner)
		x->x_targetglist = x->x_glist->gl_owner;
	    else
	    {  /* The case of a tow pointing out from an abstraction,
		  targeting its parent, is considered invalid (otherwise,
		  opening an abstraction as a top-level patch should not be
		  flagged as error).  LATER rethink. */
		loud_error((t_pd *)x, "parent of a top level patch requested,");
		loud_errand((t_pd *)x, "this is a dangling tow...");
		x->x_cvname = 0;
		x->x_targetglist = 0;
	    }
	}
	else x->x_targetglist = x->x_glist;
    }
    if (x->x_targetglist)
	x->x_cvname = x->x_targetglist->gl_name;
    x->x_type = s2;
    x->x_name = s3;
    tow_attach(x);
}

static void tow_retarget(t_tow *x, t_symbol *s1, t_symbol *s2, t_symbol *s3)
{
    tow_detach(x);
    tow_settarget(x, s1, s2, s3);
}

static void tow_pwd(t_tow *x, t_symbol *s)
{
    t_glist *glist;
    t_symbol *dir;
    if (s && s->s_thing && (glist = tow_getglist(x, 1)) &&
	(dir = canvas_getdir(glist)))
	pd_symbol(s->s_thing, dir);
}

static void tow__callback(t_tow *x, t_symbol *s, int ac, t_atom *av)
{
    if (ac == 1)
    {
	if (av->a_type == A_FLOAT)
	    outlet_float(((t_object *)x)->ob_outlet, av->a_w.w_float);
	else if (av->a_type == A_SYMBOL)
	    outlet_symbol(((t_object *)x)->ob_outlet, av->a_w.w_symbol);
    }
    else if (ac)
    {
	if (av->a_type == A_FLOAT)
	    outlet_list(((t_object *)x)->ob_outlet, &s_list, ac, av);
	else if (av->a_type == A_SYMBOL)
	    outlet_anything(((t_object *)x)->ob_outlet,
			    av->a_w.w_symbol, ac - 1, av + 1);
    }
    else outlet_bang(((t_object *)x)->ob_outlet);
}

#ifdef TOW_DEBUG
static void tow_debug(t_tow *x)
{
    t_widgetentry *we;
    loudbug_post("attached widgets:");
    for (we = x->x_widgetlist; we; we = we->we_next)
    {
	t_widget *w = we->we_widget;
	t_towentry *te;
	int other = 0, found = 0;
	loudbug_startpost("\t%s %s", w->x_type->s_name, w->x_cbtarget->s_name);
	for (te = w->x_towlist; te; te = te->te_next)
	    if (te->te_tow == x)
		found++;
	    else
		other++;
	loudbug_post(" (%d other tow%s)", other, (other == 1 ? "" : "s"));
	if (found != 1)
	    loudbug_bug("listed %d times in widget's towlist", found);
    }
}
#endif

static void tow_free(t_tow *x)
{
    t_tow *t1, *t2;
#ifdef TOW_DEBUG
    loudbug_startpost("updating towlist...");
#endif
    for (t1 = 0, t2 = widget_towlist; t2; t2 = t2->x_next)
    {
	if (t2 == x)
	{
	    if (t1)
		t1->x_next = t2->x_next;
	    else
		widget_towlist = t2->x_next;
#ifdef TOW_DEBUG
	    loudbug_post("ok");
#endif
	    break;
	}
	t1 = t2;
    }
    tow_detach(x);
}

static void *tow_new(t_symbol *s1, t_symbol *s2, t_symbol *s3)
{
    t_tow *x = (t_tow *)pd_new(tow_class);
    x->x_glist = canvas_getcurrent();
    x->x_targetglist = 0;
    x->x_widgetlist = 0;
    x->x_next = widget_towlist;
    widget_towlist = x;
    outlet_new((t_object *)x, &s_anything);
    tow_settarget(x, s1, s2, s3);
    return (x);
}

void widget_setup(void)
{
    post("beware! this is widget %s, %s %s build...",
	 TOXY_VERSION, loud_ordinal(TOXY_BUILD), TOXY_RELEASE);
    widgetps_mouse = gensym("mouse");
    widgetps_motion = gensym("motion");
    widgetps_vis = gensym("vis");
    widgetps_new = gensym("new");
    widgetps_free = gensym("free");
    widgetps_data = gensym("data");
    widgetps_add = gensym("add");
    widgetps_delete = gensym("delete");
    widgetps_set = gensym("set");
    widgetps_get = gensym("get");
    widgettype_setup();
    widget_class = class_new(gensym("widget"),
			     (t_newmethod)widget_new,
			     (t_method)widget_free,
			     sizeof(t_widget), 0, A_GIMME, 0);
    class_setwidget(widget_class, &widget_behavior);
    forky_setsavefn(widget_class, widget_save);
    forky_setpropertiesfn(widget_class, widget_properties);
    class_addbang(widget_class, widget_bang);
    class_addfloat(widget_class, widget_float);
    class_addsymbol(widget_class, widget_symbol);
    class_addanything(widget_class, widget_anything);
    class_addmethod(widget_class, (t_method)widget_set,
		    gensym("set"), A_GIMME, 0);
    class_addmethod(widget_class, (t_method)widget_remove,
		    gensym("remove"), A_SYMBOL, 0);
    class_addmethod(widget_class, (t_method)widget_tot,
		    gensym("tot"), A_GIMME, 0);
    class_addmethod(widget_class, (t_method)widget_refresh,
		    gensym("refresh"), 0);
    class_addmethod(widget_class, (t_method)widget_redefine,
		    gensym("redefine"), 0);
    class_addmethod(widget_class, (t_method)widget__failure,
		    gensym("_failure"), 0);
    class_addmethod(widget_class, (t_method)widget__config,
		    gensym("_config"),
		    A_SYMBOL, A_SYMBOL, A_FLOAT, A_FLOAT, A_FLOAT, 0);
    class_addmethod(widget_class, (t_method)widget__data,
		    gensym("_data"), A_GIMME, 0);
    class_addmethod(widget_class, (t_method)widget__callback,
		    gensym("_cb"), A_GIMME, 0);
    class_addmethod(widget_class, (t_method)widget__inout,
		    gensym("_inout"), A_FLOAT, 0);
    class_addmethod(widget_class, (t_method)widget__click,
		    gensym("_click"), A_GIMME, 0);
    class_addmethod(widget_class, (t_method)widget__motion,
		    gensym("_motion"), A_GIMME, 0);
#ifdef WIDGET_DEBUG
    class_addmethod(widget_class, (t_method)widget_debug,
		    gensym("debug"), 0);
#endif
#ifdef WIDGET_PROFILE
    class_addmethod(widget_class, (t_method)widget_profile,
		    gensym("profile"), 0);
#endif
    hammerfile_setup(widget_class, 0);

    makeshift_class = class_new(gensym("text"), 0, 0,
				sizeof(t_text),
				/* inlet added explicitly (cf text_class) */
				CLASS_NOINLET | CLASS_PATCHABLE, 0);

    tow_class = class_new(gensym("tow"),
			  (t_newmethod)tow_new,
			  (t_method)tow_free,
			  sizeof(t_tow), 0, A_SYMBOL, A_DEFSYM, A_DEFSYM, 0);
    class_addbang(tow_class, tow_bang);
    class_addfloat(tow_class, tow_float);
    class_addsymbol(tow_class, tow_symbol);
    class_addanything(tow_class, tow_anything);
    class_addmethod(tow_class, (t_method)tow_redefine,
		    gensym("redefine"), 0);
    class_addmethod(tow_class, (t_method)tow_retarget,
		    gensym("retarget"), A_SYMBOL, A_SYMBOL, A_SYMBOL, 0);
    class_addmethod(tow_class, (t_method)tow_pwd,
		    gensym("pwd"), A_SYMBOL, 0);
    class_addmethod(tow_class, (t_method)tow__callback,
		    gensym("_cb"), A_GIMME, 0);
#ifdef TOW_DEBUG
    class_addmethod(tow_class, (t_method)tow_debug,
		    gensym("debug"), 0);
#endif
}
