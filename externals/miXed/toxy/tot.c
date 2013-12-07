/* Copyright (c) 2003-2005 krzYszcz and others.
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

/* LATER handle stcriptlet's named arguments */

#include <stdio.h>
#include <string.h>
#include "m_pd.h"
#include "g_canvas.h"
#include "common/loud.h"
#include "unstable/forky.h"
#include "hammer/file.h"
#include "hammer/gui.h"
#include "common/props.h"
#include "toxy/scriptlet.h"
#include "build_counter"

#ifdef KRZYSZCZ
//#define TOT_DEBUG
#endif

/* probably much more than needed for canvas messages from gui */
#define TOTSPY_MAXSIZE  32

typedef struct _tot
{
    t_object   x_ob;
    t_glist   *x_glist;     /* containing glist */
    t_symbol  *x_dotname;   /* "." (if explicit), ".parent", ".root", etc. */
    t_symbol  *x_cvname;    /* destination name (if literal), or 0 */
    t_symbol  *x_cvremote;  /* bindsym of the above */
    t_symbol  *x_cvpathname;     /* see tot_getpathname() */
    t_symbol  *x_visedpathname;  /* see tot__vised() */
    t_symbol  *x_target;
    int        x_warned;
    t_scriptlet     *x_persistent;
    t_scriptlet     *x_transient;
    t_outlet        *x_out2;
    t_outlet        *x_out4;
    t_symbol        *x_defname;  /* file name (if given as a creation arg) */
    t_hammerfile    *x_filehandle;
    t_pd            *x_guidetached;
    t_pd            *x_guisink;
    struct _totspy  *x_spy;
} t_tot;

typedef struct _totspy
{
    t_pd       ts_pd;
    int        ts_on;
    t_canvas  *ts_cv;
    t_symbol  *ts_target;
    t_symbol  *ts_qsym;
    int        ts_gotmotion;
    t_atom     ts_lastmotion[3];
    double     ts_lasttime;
    t_symbol  *ts_selector;
    t_atom     ts_outbuf[TOTSPY_MAXSIZE];
    t_outlet  *ts_out3;
    t_clock   *ts_cleanupclock;  /* also a tot-is-gone flag */
} t_totspy;

static t_class *tot_class;
static t_class *totspy_class;
static t_class *totsink_class;
static t_class *tot_guiconnect_class = 0;

static t_symbol *totps_motion;
static t_symbol *totps_qpush;
static t_symbol *totps_query;

static t_symbol *totps_dotparent;  /* holder of our containing glist's box */
static t_symbol *totps_dotroot;    /* our document's root canvas */
static t_symbol *totps_dotowner;   /* parent of .root */
static t_symbol *totps_dottop;     /* top-level canvas */

static t_glist *tot_getglist(t_tot *x)
{
    t_glist *glist;
    if (x->x_cvremote)
	glist = (t_glist *)pd_findbyclass(x->x_cvremote, canvas_class);
    else if (x->x_dotname == totps_dotparent)
	glist = x->x_glist->gl_owner;
    else if (x->x_dotname == totps_dotroot)
	glist = canvas_getrootfor(x->x_glist);
    else if (x->x_dotname == totps_dotowner)
    {
	if (glist = canvas_getrootfor(x->x_glist))
	    glist = glist->gl_owner;
    }
    else if (x->x_dotname == totps_dottop)
    {
	glist = x->x_glist;
	while (glist->gl_owner) glist = glist->gl_owner;
    }
    else
	glist = x->x_glist;
    return (glist);	
}

static t_symbol *tot_getcvname(t_tot *x)
{
    t_glist *glist = tot_getglist(x);
    t_symbol *cvname = (glist ? glist->gl_name : x->x_cvname);
    if (cvname)
	return (cvname);
    else if (x->x_dotname)
	return (x->x_dotname);
    else
    {
	loudbug_bug("tot_getcvname");
	return (gensym("???"));
    }
}

static t_canvas *tot_getcanvas(t_tot *x, int complain)
{
    t_canvas *cv = 0;
    t_glist *glist = tot_getglist(x);
    if (glist)
	cv = glist_getcanvas(glist);
    else if (complain)
    {
	if (x->x_dotname && *x->x_dotname->s_name)
	    loud_error((t_pd *)x, "%s canvas does not exist",
		       &x->x_dotname->s_name[1]);
	else
	    loud_error((t_pd *)x, "bad canvas name '%s'",
		       tot_getcvname(x)->s_name);
    }
    if (!x->x_warned)
    {
	x->x_warned = 1;
	if (!cv) cv = x->x_glist;  /* redundant */
	loud_warning((t_pd *)x, 0, "using containing canvas ('%s')",
		     cv->gl_name->s_name);
    }
    return (cv);
}

static t_canvas *tot_cvhook(t_pd *z)
{
    return (tot_getcanvas((t_tot *)z, 1));
}

static t_symbol *tot_dogetpathname(t_tot *x, int visedonly, int complain)
{
    t_canvas *cv = tot_getcanvas(x, complain);
    if (cv)
    {
	if (visedonly && !glist_isvisible(cv))
	    return (0);
	else if (cv == x->x_glist)
	    /* containing glist is our destination, and we are not in a gop */
	    return (x->x_cvpathname);
	else
	{
	    char buf[32];
	    sprintf(buf, ".x%lx.c", (int)cv);
	    return (gensym(buf));
	}
    }
    else return (0);
}

static t_symbol *tot_getpathname(t_tot *x, int complain)
{
    return (tot_dogetpathname(x, 0, complain));
}

static t_symbol *tot_getvisedpathname(t_tot *x, int complain)
{
    return (tot_dogetpathname(x, 1, complain));
}

static void tot_reset(t_tot *x)
{
    scriptlet_reset(x->x_persistent);
}

static void tot_prealloc(t_tot *x, t_floatarg f)
{
    int reqsize = (int)f;
    scriptlet_prealloc(x->x_persistent, reqsize, 1);
    scriptlet_prealloc(x->x_transient, reqsize, 1);  /* LATER rethink */
}

static void tot_add(t_tot *x, t_symbol *s, int ac, t_atom *av)
{
    scriptlet_add(x->x_persistent, 0, 0, ac, av);
}

static void tot_addnext(t_tot *x, t_symbol *s, int ac, t_atom *av)
{
    scriptlet_setseparator(x->x_persistent, '\n');
    scriptlet_add(x->x_persistent, 0, 0, ac, av);
}

static void tot_push(t_tot *x, t_symbol *s, int ac, t_atom *av)
{
    if (scriptlet_evaluate(x->x_persistent, x->x_transient, 1, ac, av, 0))
    {
	if (s == totps_qpush)
	    scriptlet_qpush(x->x_transient);
	else
	    scriptlet_push(x->x_transient);
    }
}

static void tot_tot(t_tot *x, t_symbol *s, int ac, t_atom *av)
{
    if (ac)
    {
	t_scriptlet *sp = x->x_transient;
	scriptlet_reset(sp);
	scriptlet_add(sp, 1, 1, ac, av);
	if (s == totps_query)
	    scriptlet_qpush(sp);
	else
	    scriptlet_push(sp);
    }
}

static void tot_dooutput(t_tot *x, t_outlet *op,
			 t_symbol *s, int ac, t_atom *av)
{
    if (ac == 1)
    {
	if (av->a_type == A_FLOAT)
	    outlet_float(op, av->a_w.w_float);
	else if (av->a_type == A_SYMBOL)
	    outlet_symbol(op, av->a_w.w_symbol);
    }
    else if (ac)
    {
	if (av->a_type == A_FLOAT)
	    outlet_list(op, &s_list, ac, av);
	else if (av->a_type == A_SYMBOL)
	    outlet_anything(op, av->a_w.w_symbol, ac - 1, av + 1);
    }
    else outlet_bang(op);
}

static void tot__reply(t_tot *x, t_symbol *s, int ac, t_atom *av)
{
    tot_dooutput(x, ((t_object *)x)->ob_outlet, s, ac, av);
}

static void tot__callback(t_tot *x, t_symbol *s, int ac, t_atom *av)
{
    tot_dooutput(x, x->x_out2, s, ac, av);
}

static void tot_properties(t_gobj *z, t_glist *glist)
{
    t_tot *x = (t_tot *)z;
    int nleft;
    char *head = scriptlet_getcontents(x->x_persistent, &nleft);
    hammereditor_open(x->x_filehandle, "scriptlet editor", 0);
    if (nleft)
    {
	char buf[MAXPDSTRING + 1], *lastptr = buf + MAXPDSTRING;
	*lastptr = 0;
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
		lastptr = buf + nleft;
		*lastptr = 0;
		nleft = 0;
	    }
	    hammereditor_append(x->x_filehandle, buf);
	}
	hammereditor_append(x->x_filehandle, "\n");
    }
    hammereditor_setdirty(x->x_filehandle, 0);
}

static void tot_editorhook(t_pd *z, t_symbol *s, int ac, t_atom *av)
{
    t_tot *x = (t_tot *)z;
    scriptlet_reset(x->x_persistent);
    scriptlet_add(x->x_persistent, 0, 0, ac, av);
}

static void tot_readhook(t_pd *z, t_symbol *fn, int ac, t_atom *av)
{
    scriptlet_read(((t_tot *)z)->x_persistent, fn);
}

static void tot_writehook(t_pd *z, t_symbol *fn, int ac, t_atom *av)
{
    scriptlet_write(((t_tot *)z)->x_persistent, fn);
}

static void tot_read(t_tot *x, t_symbol *s)
{
    if (s && s != &s_)
	scriptlet_read(x->x_persistent, s);
    else
	hammerpanel_open(x->x_filehandle, 0);
}

static void tot_write(t_tot *x, t_symbol *s)
{
    if (s && s != &s_)
	scriptlet_write(x->x_persistent, s);
    else
	hammerpanel_save(x->x_filehandle,
			 canvas_getdir(x->x_glist), x->x_defname);
}

/* This is called for all Map (f==1) and all Destroy (f==0) events,
   comming from any canvas.  If visedpathname is zero, we assume our
   canvas does not exist.  So we ignore everything, waiting for a Map
   event that fits tot_getpathname().  Once we spot it, we set
   visedpathname, and ignore everything, waiting for a Destroy event
   that fits visedpathname.  Then we clear visedpathname, etc... */
static void tot__vised(t_tot *x, t_symbol *s, t_floatarg f)
{
    int flag = f != 0.;
#ifdef TOT_DEBUG
    t_symbol *pn = tot_getpathname(x, 0);
    loudbug_post("tot__vised %s %g (pathname %s) ", s->s_name, f,
		 (pn ? pn->s_name : "unknown"));
#endif
    if (!x->x_visedpathname)
    {
	if (flag && s == tot_getpathname(x, 0))
	{
	    x->x_visedpathname = s;
	    outlet_bang(x->x_out4);
	}
    }
    else if (!flag && s == x->x_visedpathname)
	x->x_visedpathname = 0;  /* LATER reconsider reporting this */
}

#ifdef TOT_DEBUG
static void tot_postscriptlet(t_scriptlet *sp, char *message)
{
    int nleft;
    char *head = scriptlet_getbuffer(sp, &nleft);
    loudbug_startpost("*** %s (size %d)", message, nleft);
    if (nleft)
    {
	char buf[MAXPDSTRING + 1], *lastptr = buf + MAXPDSTRING;
	*lastptr = 0;
	loudbug_stringpost(" ***\n\"");
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
		lastptr = buf + nleft;
		*lastptr = 0;
		nleft = 0;
	    }
	    loudbug_stringpost(buf);
	}
	loudbug_stringpost("\"\n---------------\n");
    }
    else loudbug_stringpost(": \"\" ***\n");
}

static void tot_debug(t_tot *x)
{
    t_symbol *pn = tot_getpathname(x, 0);
    int sz;
    char *bp;
    loudbug_post("containing glist: %x", x->x_glist);
    loudbug_post("destination: %s", tot_getcvname(x)->s_name);
    loudbug_post("pathname%s %s", (pn ? ":" : ""),
		 (pn ? pn->s_name : "unknown"));
    tot_postscriptlet(x->x_transient, "transient buffer");
    tot_postscriptlet(x->x_persistent, "persistent buffer");
}
#endif

static void tot_detach(t_tot *x)
{
    t_canvas *cv = tot_getcanvas(x, 1);
    if (cv && glist_isvisible(cv))
    {
	t_pd *gc;
	t_symbol *target;
	char buf[64];
	sprintf(buf, ".x%lx", (int)cv);
	target = gensym(buf);
	if (!tot_guiconnect_class)
	{
	    gc = (t_pd *)guiconnect_new(0, gensym("tot"));
	    tot_guiconnect_class = *gc;
	    typedmess(gc, gensym("signoff"), 0, 0);
	}
	if (gc = pd_findbyclass(target, tot_guiconnect_class))
	{
	    x->x_guidetached = gc;
	    pd_unbind(gc, target);
	    pd_bind(x->x_guisink, target);
	}
    }
}

static void tot_attach(t_tot *x)
{
    t_canvas *cv = tot_getcanvas(x, 1);
    if (cv && glist_isvisible(cv) && x->x_guidetached)
    {
	if (tot_guiconnect_class)
	{
	    t_pd *gc;
	    t_symbol *target;
	    char buf[64];
	    sprintf(buf, ".x%lx", (int)cv);
	    target = gensym(buf);
	    if (gc = pd_findbyclass(target, tot_guiconnect_class))
	    {
	    }
	    else
	    {  /* assuming nobody else detached it in the meantime... */
		pd_unbind(x->x_guisink, target);
		pd_bind(x->x_guidetached, target);
		x->x_guidetached = 0;
	    }
	}
	else loudbug_bug("tot_attach");
    }
}

static void tot_capture(t_tot *x, t_symbol *s, t_floatarg f)
{
    t_totspy *ts = x->x_spy;
    if ((int)f)
    {
	t_canvas *cv = tot_getcanvas(x, 1);
	ts->ts_qsym = (s == &s_ ? 0 : s);
	if (cv != ts->ts_cv)
	{
	    if (ts->ts_target)
	    {
		pd_unbind((t_pd *)ts, ts->ts_target);
		ts->ts_cv = 0;
		ts->ts_target = 0;
	    }
	    if (cv)
	    {
		char buf[64];
		ts->ts_cv = cv;
		sprintf(buf, ".x%lx", (int)cv);
		pd_bind((t_pd *)ts, ts->ts_target = gensym(buf));
	    }
	}
	ts->ts_on = (ts->ts_target != 0);
	if (ts->ts_on && ts->ts_qsym)
	{
	    ts->ts_lasttime = clock_getlogicaltime();
	    ts->ts_selector = gensym("add");
	    SETFLOAT(&ts->ts_outbuf[0], 0);
	    SETSYMBOL(&ts->ts_outbuf[1], ts->ts_qsym);
	    SETSYMBOL(&ts->ts_outbuf[2], &s_);
	    outlet_anything(ts->ts_out3, gensym("clear"), 0, 0);
	}
    }
    else ts->ts_on = 0;
}

/* this is needed to overcome glist_getnextxy()-related troubles */
static void tot_lastmotion(t_tot *x, t_symbol *s)
{
    t_totspy *ts = x->x_spy;
    if (ts->ts_gotmotion)
    {
	if (s == &s_)
	    s = ts->ts_target;
	if (s && s->s_thing)
	    typedmess(s->s_thing, totps_motion, 3, ts->ts_lastmotion);
    }
}

static void totspy_anything(t_totspy *ts, t_symbol *s, int ac, t_atom *av)
{
    if (ts->ts_cleanupclock)
	return;
    if (s == totps_motion)
    {
	if (ac == 3)
	{
	    ts->ts_lastmotion[0] = av[0];
	    ts->ts_lastmotion[1] = av[1];
	    ts->ts_lastmotion[2] = av[2];
	    ts->ts_gotmotion = 1;
	}
	else loudbug_bug("totspy_anything");
    }
    if (ts->ts_on)
    {
	if (ts->ts_qsym)
	{
	    int cnt = ac + 3;
	    if (cnt < TOTSPY_MAXSIZE)
	    {
		t_atom *ap = ts->ts_outbuf;
		ap++->a_w.w_float = (float)clock_gettimesince(ts->ts_lasttime);
		ap++;
		ap++->a_w.w_symbol = s;
		while (ac--) *ap++ = *av++;
		outlet_anything(ts->ts_out3,
				ts->ts_selector, cnt, ts->ts_outbuf);
		ts->ts_lasttime = clock_getlogicaltime();
	    }
	    else loud_warning((t_pd *)ts, 0,
			      "unexpectedly long message (\"%s...\"), ignored",
			      s->s_name);
	}
	else outlet_anything(ts->ts_out3, s, ac, av);
    }
}

static void totspy_cleanuptick(t_totspy *ts)
{
    if (ts->ts_target)
	pd_unbind((t_pd *)ts, ts->ts_target);
    if (ts->ts_cleanupclock)
	clock_free(ts->ts_cleanupclock);
    pd_free((t_pd *)ts);
}

static void totsink_anything(t_pd *x, t_symbol *s, int ac, t_atom *av)
{
    /* nop */
}

static void tot_free(t_tot *x)
{
    pd_unbind((t_pd *)x, x->x_target);
    hammergui_unbindvised((t_pd *)x);
    hammerfile_free(x->x_filehandle);
    scriptlet_free(x->x_persistent);
    scriptlet_free(x->x_transient);
    if (x->x_spy->ts_target)
    {
	/* postpone unbinding, due to a danger of being deleted by
	   a message to the canvas we spy on... */
	x->x_spy->ts_cleanupclock =
	    clock_new(x->x_spy, (t_method)totspy_cleanuptick);
	clock_delay(x->x_spy->ts_cleanupclock, 0);
    }
    else pd_free((t_pd *)x->x_spy);
    pd_free(x->x_guisink);
}

static void *tot_new(t_symbol *s1, t_symbol *s2)
{
    t_tot *x = (t_tot *)pd_new(tot_class);
    char buf[64];
    sprintf(buf, "tot%x", (int)x);
    pd_bind((t_pd *)x, x->x_target = gensym(buf));
    x->x_glist = canvas_getcurrent();
    x->x_transient = scriptlet_new((t_pd *)x, x->x_target, x->x_target,
				   0, x->x_glist, tot_cvhook);
    x->x_persistent = scriptlet_new((t_pd *)x, x->x_target, x->x_target,
				    0, x->x_glist, tot_cvhook);
    if (s1 && s1 != &s_ && *s1->s_name != '.')
    {
	x->x_dotname = 0;
	x->x_warned = 1;
	x->x_cvremote = canvas_makebindsym(x->x_cvname = s1);
	x->x_cvpathname = 0;
    }
    else
    {
	t_glist *glist;
	x->x_dotname = (s1 && *s1->s_name == '.' ? s1 : 0);
	x->x_warned = (x->x_dotname != 0);  /* do not warn if explicit */
	x->x_cvname = 0;
	x->x_cvremote = 0;
	glist = tot_getglist(x);
	if (glist == x->x_glist)
	{
	    sprintf(buf, ".x%lx.c", (int)glist);
	    x->x_cvpathname = gensym(buf);
	}
	else x->x_cvpathname = 0;
    }
    outlet_new((t_object *)x, &s_anything);
    x->x_out2 = outlet_new((t_object *)x, &s_anything);
    x->x_spy = (t_totspy *)pd_new(totspy_class);
    x->x_spy->ts_on = 0;
    x->x_spy->ts_cv = 0;
    x->x_spy->ts_target = 0;
    x->x_spy->ts_qsym = 0;
    x->x_spy->ts_gotmotion = 0;
    x->x_spy->ts_out3 = outlet_new((t_object *)x, &s_anything);
    x->x_out4 = outlet_new((t_object *)x, &s_bang);
    if (s2 && s2 != &s_)
    {
	x->x_defname = s2;
	scriptlet_read(x->x_persistent, s2);
    }
    else x->x_defname = &s_;
    x->x_filehandle = hammerfile_new((t_pd *)x, 0,
				     tot_readhook, tot_writehook,
				     tot_editorhook);
    hammergui_bindvised((t_pd *)x);
    x->x_visedpathname = tot_getvisedpathname(x, 0);
    x->x_guidetached = 0;
    x->x_guisink = pd_new(totsink_class);
    return (x);
}

void tot_setup(void)
{
    post("beware! this is tot %s, %s %s build...",
	 TOXY_VERSION, loud_ordinal(TOXY_BUILD), TOXY_RELEASE);
    totps_motion = gensym("motion");
    totps_qpush = gensym("qpush");
    totps_query = gensym("query");
    totps_dotparent = gensym(".parent");
    totps_dotroot = gensym(".root");
    totps_dotowner = gensym(".owner");
    totps_dottop = gensym(".top");
    tot_class = class_new(gensym("tot"),
			  (t_newmethod)tot_new,
			  (t_method)tot_free,
			  sizeof(t_tot), 0, A_DEFSYM, A_DEFSYM, 0);
    class_addmethod(tot_class, (t_method)tot_prealloc,
		    gensym("prealloc"), A_FLOAT, 0);
    class_addmethod(tot_class, (t_method)tot_read,
		    gensym("read"), A_DEFSYM, 0);
    class_addmethod(tot_class, (t_method)tot_write,
		    gensym("write"), A_DEFSYM, 0);
    class_addmethod(tot_class, (t_method)tot_reset,
		    gensym("reset"), 0);
    class_addmethod(tot_class, (t_method)tot_push,
		    gensym("push"), A_GIMME, 0);
    class_addmethod(tot_class, (t_method)tot_push,
		    gensym("qpush"), A_GIMME, 0);
    class_addmethod(tot_class, (t_method)tot_add,
		    gensym("add"), A_GIMME, 0);
    class_addmethod(tot_class, (t_method)tot_addnext,
		    gensym("addnext"), A_GIMME, 0);
    class_addmethod(tot_class, (t_method)tot_tot,
		    gensym("tot"), A_GIMME, 0);
    class_addmethod(tot_class, (t_method)tot_tot,
		    gensym("query"), A_GIMME, 0);
    class_addmethod(tot_class, (t_method)tot_detach,
		    gensym("detach"), 0);
    class_addmethod(tot_class, (t_method)tot_attach,
		    gensym("attach"), 0);
    class_addmethod(tot_class, (t_method)tot_capture,
		    gensym("capture"), A_FLOAT, A_DEFSYM, 0);
    class_addmethod(tot_class, (t_method)tot_lastmotion,
		    gensym("lastmotion"), A_DEFSYM, 0);
    class_addmethod(tot_class, (t_method)tot__reply,
		    gensym("_rp"), A_GIMME, 0);
    class_addmethod(tot_class, (t_method)tot__callback,
		    gensym("_cb"), A_GIMME, 0);
    class_addmethod(tot_class, (t_method)tot__vised,
		    gensym("_vised"), A_SYMBOL, A_FLOAT, 0);
#ifdef TOT_DEBUG
    class_addmethod(tot_class, (t_method)tot_debug,
		    gensym("debug"), 0);
#endif
    forky_setpropertiesfn(tot_class, tot_properties);
    hammerfile_setup(tot_class, 0);
    totspy_class = class_new(gensym("tot spy"), 0, 0,
			     sizeof(t_totspy), CLASS_PD, 0);
    class_addanything(totspy_class, totspy_anything);
    totsink_class = class_new(gensym("tot sink"), 0, 0,
			      sizeof(t_pd), CLASS_PD, 0);
    class_addanything(totsink_class, totsink_anything);
}
