/* Copyright (c) 2003-2005 krzYszcz and others.
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

#include <stdio.h>
#include <string.h>
#include "m_pd.h"
#include "common/loud.h"
#include "common/grow.h"
#include "common/dict.h"
#include "common/props.h"
#include "toxy/scriptlet.h"
#include "widget.h"

static char masterwidget_builtin[] =
#include "setup.wiq"
;

#define WIDGETTYPE_VERBOSE
#ifdef KRZYSZCZ
//#define WIDGETTYPE_DEBUG
#endif

struct _widgettype
{
    t_pd          wt_pd;
    t_glist      *wt_glist;       /* set by the loading widget */
    t_symbol     *wt_typekey;     /* this is a typemap symbol */
    t_symbol     *wt_tkclass;     /* also 'undefined' flag (gensym symbol) */
    char         *wt_requirements;
    int           wt_isinternal;  /* true if built-in or defined in setup.wid */
    t_props           *wt_options;
    t_props           *wt_handlers;  /* defined inside of #. comments */
    t_props           *wt_arguments;
    t_scriptlet       *wt_auxscript;
    t_widgethandlers  *wt_scripts;   /* multiliners tagged with #@ comments */
};

struct _masterwidget
{
    t_pd           mw_pd;
    t_symbol      *mw_target;
    t_scriptlet   *mw_setupscript;
    t_dict        *mw_typemap;
    t_widgettype  *mw_parsedtype;  /* the type currently parsed, if loading */
    t_binbuf      *mw_bb;          /* auxiliary, LATER remove */
};

static t_class *widgettype_class;
static t_class *masterwidget_class;

static t_masterwidget *masterwidget = 0;

static t_canvas *widgettype_cvhook(t_pd *caller)
{
    return (0);
}

static void widgettype_map(t_widgettype *wt, char *cls, char *req)
{
    wt->wt_tkclass = (cls ? gensym(cls) : 0);
    if (wt->wt_requirements)
	freebytes(wt->wt_requirements, strlen(wt->wt_requirements) + 1);
    if (req && *req)
    {
	char *opt = 0;
	wt->wt_requirements = getbytes(strlen(req) + 1);
	strcpy(wt->wt_requirements, req);
	while (req)
	{
	    char *w1 = scriptlet_nextword(req);
	    opt = (*req == '-' ? req : (w1 && *w1 == '-' ? w1 : 0));
	    if (opt)
	    {
		if (strcmp(opt + 1, "exact"))
		{
		    loud_error
			(0, "unknown option \"%s\" in widget type header", opt);
		    opt = 0;
		}
		if (*req == '-')
		{
		    req = w1;
		    continue;
		}
		else w1 = scriptlet_nextword(w1);
	    }
	    if (*req >= '0' && *req <= '9')
	    {
		loud_error
		    (0, "invalid base widget name \"%s\" in widget type header",
		     req);
		req = w1;
	    }
	    else
	    {
		t_widgettype *base;
		char *ver = (w1 && *w1 >= '0' && *w1 <= '9' ? w1 : 0);
		char *w2 = (ver ? scriptlet_nextword(ver) : w1);
#if 1
		loudbug_post("require %s (version %s %s)",
			     req, (opt ? opt : ">="), (ver ? ver : "any"));
#endif
		base = widgettype_get(gensym(req), ver, opt, wt->wt_glist);
		if (!base->wt_tkclass)
		    loud_error(0, "missing base widget file \"%s.wid\"", req);
		req = w2;
	    }
	}
    }
    else wt->wt_requirements = 0;
}

static void widgettype_clear(t_widgettype *wt)
{
    props_clearall(wt->wt_options);
    widgethandlers_reset(wt->wt_scripts);
}

#if 0
/* only for debugging (never call, unless certain that nobody references wt) */
static void widgettype_free(t_masterwidget *mw, t_widgettype *wt)
{
    loudbug_startpost("widgettype free... ");
    if (wt->wt_requirements)
	freebytes(wt->wt_requirements, strlen(wt->wt_requirements) + 1);
    dict_unbind(mw->mw_typemap, (t_pd *)wt, wt->wt_typekey);
    props_freeall(wt->wt_options);
    scriptlet_free(wt->wt_auxscript);
    widgethandlers_free(wt->wt_scripts);
    pd_free((t_pd *)wt);
    loudbug_post("done");
}
#endif

static t_widgettype *widgettype_new(t_masterwidget *mw, char *typ, char *cls,
				    char *req, t_glist *glist)
{
    t_widgettype *wt = (t_widgettype *)pd_new(widgettype_class);
    wt->wt_glist = glist;
    wt->wt_typekey = dict_key(mw->mw_typemap, typ);
    widgettype_map(wt, cls, req);
    wt->wt_isinternal = 0;
    wt->wt_options = props_new(0, "option", "-", 0, 0);
    wt->wt_handlers = props_new(0, "handler", "@", wt->wt_options, 0);
    wt->wt_arguments = props_new(0, "argument", "#", wt->wt_options, 0);
    wt->wt_auxscript = scriptlet_new((t_pd *)wt, mw->mw_target, mw->mw_target,
				     0, 0, widgettype_cvhook);
    wt->wt_scripts = widgethandlers_new(wt->wt_auxscript);
    dict_bind(mw->mw_typemap, (t_pd *)wt, wt->wt_typekey);
    return (wt);
}

static t_canvas *masterwidget_cvhook(t_pd *caller)
{
    return (0);
}

static t_scriptlet *masterwidget_cmnthook(t_pd *caller, char *rc,
					  char sel, char *buf)
{
    t_masterwidget *mw = masterwidget;
    if (!*buf)
	return (SCRIPTLET_UNLOCK);
    if (sel == '>')
    {
	t_symbol *typekey;
	t_widgettype *typeval;
	char *cls = scriptlet_nextword(buf);
	char *req = (cls ? scriptlet_nextword(cls) : 0);
	mw->mw_parsedtype = 0;
	if (!cls)
	    cls = buf;
	typekey = dict_key(mw->mw_typemap, buf);
	typeval = (t_widgettype *)dict_firstvalue(mw->mw_typemap, typekey, 0);
	if (caller == (t_pd *)mw)
	{  /* setup.wid or built-in defaults */
	    if (typeval)
	    {
		/* LATER may need revisiting, when/if we accept explicit
		   'redefine' requests for internal types */
		loud_warning((t_pd *)mw, 0, "redefinition of '%s'\
 in \"%s.wid\" file, ignored", buf, rc);
		return (SCRIPTLET_LOCK);
	    }
	}
	else
	{  /* <type>.wid */
	    if (caller != (t_pd *)typeval)
	    {
		loud_warning((t_pd *)mw, 0, "alien definition of '%s'\
 in \"%s.wid\" file, ignored", buf, rc);
		return (SCRIPTLET_LOCK);
	    }
	}
	if (typeval)
	    widgettype_map(typeval, cls, req);
	else
	{
	    typeval = widgettype_new(mw, buf, cls, req, 0);
	    typeval->wt_isinternal = (caller == (t_pd *)mw);
	}
	mw->mw_parsedtype = typeval;
#ifdef WIDGETTYPE_DEBUG
	loudbug_post("adding widget type '%s'", typeval->wt_typekey->s_name);
#endif
	widgethandlers_reset(typeval->wt_scripts);

	/* What should follow after the header?  In a cleaner layout, perhaps,
	   the header would be placed at the top, followed by setup.  Any
	   handler would require an explicit #@ tag, and the next statement
	   would return SCRIPTLET_UNLOCK.  LATER revisit -- the change breaks
	   old .wid files, so better wait for a more robust parsing, which
	   notices dot-sequences in the setup part and warns about them.
	   Setup before header will be valid after the change, anyway. */
	return (widgethandlers_getvis(typeval->wt_scripts));
    }
    else if (sel == '.')
    {
	if (mw->mw_parsedtype
	    && (*buf == '-' || *buf == '@' || *buf == '#'))
	{
	    t_symbol *empty;
	    int ac;
	    /* LATER get rid of the binbuf thing */
	    binbuf_text(mw->mw_bb, buf, strlen(buf));
	    if (ac = binbuf_getnatom(mw->mw_bb))
	    {
		t_atom *av = binbuf_getvec(mw->mw_bb);
		t_props *pp;
		if (!(empty = props_add(pp = mw->mw_parsedtype->wt_options,
					0, 0, 0, ac, av)) &&
		    !(empty = props_add(pp = mw->mw_parsedtype->wt_handlers,
					0, 0, 0, ac, av)))
		    empty = props_add(pp = mw->mw_parsedtype->wt_arguments,
				      0, 0, 0, ac, av);
		if (empty)
		    loud_warning((t_pd *)mw, 0,
				 "no value given for %s '%s'\
 of a widget type '%s' in \"%s.wid\" file",
				 props_getname(pp), empty->s_name,
				 mw->mw_parsedtype->wt_typekey->s_name, rc);
	    }
	}
    }
    else if (sel == '@')
    {  /* multiline definition of a handler */
	scriptlet_nextword(buf);
	if (mw->mw_parsedtype)
	    return (widgethandlers_takeany(mw->mw_parsedtype->wt_scripts,
					   gensym(buf)));
    }
    return (SCRIPTLET_UNLOCK);
}

static int widgettype_doload(t_widgettype *wt, t_symbol *s)
{
    int result = 0;
    /* <type>.wid searched in the current patch's dir + pd_path,
       but not in `pwd` */
    t_scriptlet *mwsp =
	scriptlet_new((t_pd *)masterwidget, masterwidget->mw_target,
		      masterwidget->mw_target, 0, wt->wt_glist, 0);
    masterwidget->mw_parsedtype = wt;

    if (scriptlet_rcload(mwsp, (t_pd *)wt,
			 s->s_name, ".wid", 0, masterwidget_cmnthook)
	== SCRIPTLET_OK)
    {
#ifdef WIDGETTYPE_VERBOSE
	loudbug_post("using a separate %s's definition file", s->s_name);
#endif
	if (!scriptlet_isempty(mwsp))
	{
	    t_scriptlet *sp =
		scriptlet_new((t_pd *)masterwidget, masterwidget->mw_target,
			      masterwidget->mw_target, 0, 0, 0);
	    if (scriptlet_evaluate(mwsp, sp, 0, 0, 0, 0))
	    {
		scriptlet_push(sp);
		scriptlet_append(masterwidget->mw_setupscript, mwsp);
	    }
	    else loudbug_bug("widgettype_doload");
	    scriptlet_free(sp);
	}
	result = 1;
    }
    scriptlet_free(mwsp);
    return (result);
}

t_widgettype *widgettype_find(t_symbol *s)
{
    return ((t_widgettype *)dict_firstvalue(masterwidget->mw_typemap,
					    dict_key(masterwidget->mw_typemap,
						     s->s_name), 0));
}

t_widgettype *widgettype_get(t_symbol *s, char *ver, char *opt, t_glist *glist)
{
    t_widgettype *wt = widgettype_find(s);
    /* Design decision: default widget definitions are NOT implicitly
       overridden by <type>.wid (sacrificing flexibility for feature
       stability). */
    if (!wt)
    {
	/* first instance of a type not defined in setup.wid */
	wt = widgettype_new(masterwidget, s->s_name, 0, 0, glist);
	/* LATER use version and option */
	widgettype_doload(wt, s);
    }
    return (wt);
}

t_widgettype *widgettype_reload(t_symbol *s, t_glist *glist)
{
    t_widgettype *wt = widgettype_find(s);
    if (!wt)
	/* first instance of a type not defined in setup.wid */
	wt = widgettype_new(masterwidget, s->s_name, 0, 0, glist);
    if (wt && !wt->wt_isinternal)
    {  /* LATER consider safe-loading through a temporary type */
	widgettype_clear(wt);
	wt->wt_glist = glist;
	if (widgettype_doload(wt, s))
	    return (wt);
    }
    return (0);
}

int widgettype_isdefined(t_widgettype *wt)
{
    return (wt->wt_tkclass != 0);
}

t_symbol *widgettype_tkclass(t_widgettype *wt)
{
    return (wt->wt_tkclass);
}

t_props *widgettype_getoptions(t_widgettype *wt)
{
    return (wt->wt_options);
}

t_props *widgettype_gethandlers(t_widgettype *wt)
{
    return (wt->wt_handlers);
}

t_props *widgettype_getarguments(t_widgettype *wt)
{
    return (wt->wt_arguments);
}

t_widgethandlers *widgettype_getscripts(t_widgettype *wt)
{
    return (wt->wt_scripts);
}

void widgettype_setup(void)
{
    static int done = 0;
    if (!done)
    {
	widgettype_class = class_new(gensym("widget type"), 0, 0,
				     sizeof(t_widgettype), CLASS_PD, 0);
	masterwidget_class = class_new(gensym("Widget"), 0, 0,
				       sizeof(t_masterwidget), CLASS_PD, 0);
	done = 1;
    }
}

char *masterwidget_getcontents(int *szp)
{
    return (scriptlet_getcontents(masterwidget->mw_setupscript, szp));
}

void masterwidget_validate(void)
{
    int rcresult;
    char buf[MAXPDSTRING];
    if (masterwidget)
	return;
    masterwidget = (t_masterwidget *)pd_new(masterwidget_class);
    sprintf(buf, "mw%x", (int)masterwidget);
    /* never unbound, LATER rethink */
    pd_bind((t_pd *)masterwidget, masterwidget->mw_target = gensym(buf));

    masterwidget->mw_typemap = dict_new(0);

    /* setup.wid searched in `pwd` + pd_path, but not in current patch's dir
       (LATER only the pd_path should be searched) */
    masterwidget->mw_setupscript =
	scriptlet_new((t_pd *)masterwidget, masterwidget->mw_target,
		      masterwidget->mw_target, 0, 0, 0);
    masterwidget->mw_bb = binbuf_new();
    masterwidget->mw_parsedtype = 0;

    rcresult =
	scriptlet_rcload(masterwidget->mw_setupscript, 0, "setup", ".wid",
			 masterwidget_builtin, masterwidget_cmnthook);
    if (rcresult == SCRIPTLET_OK)
    {
#ifdef WIDGETTYPE_VERBOSE
	loudbug_post("using file 'setup.wid'");
#endif
    }
    else
    {
	char *msg;
	if (rcresult == SCRIPTLET_NOFILE)
	    msg = "no";
	else if (rcresult == SCRIPTLET_BADFILE)
	    msg = "corrupt";
	else if (rcresult == SCRIPTLET_NOVERSION)
	    msg = "unknown version of";
	else if (rcresult == SCRIPTLET_OLDERVERSION)
	    msg = "obsolete";
	else if (rcresult == SCRIPTLET_NEWERVERSION)
	    msg = "incompatible";
	else
	    msg = "cannot use";
	loud_warning((t_pd *)masterwidget, 0,
		     "%s file 'setup.wid'... using built-in defaults", msg);
    }
    if (!scriptlet_isempty(masterwidget->mw_setupscript))
	rcresult = SCRIPTLET_OK;
    else if (rcresult == SCRIPTLET_OK)
    {
	loud_warning((t_pd *)masterwidget, 0,
		     "missing setup definitions in file 'setup.wid'");
	scriptlet_reset(masterwidget->mw_setupscript);
	rcresult =
	    scriptlet_rcparse(masterwidget->mw_setupscript, 0, "master",
			      masterwidget_builtin, masterwidget_cmnthook);
    }
    else
    {
	loudbug_bug("masterwidget_validate 1");
	rcresult = SCRIPTLET_BADFILE;
    }
    if (rcresult == SCRIPTLET_OK)
    {
	t_scriptlet *sp = scriptlet_newalike(masterwidget->mw_setupscript);
	if (scriptlet_evaluate(masterwidget->mw_setupscript, sp, 0, 0, 0, 0))
	    scriptlet_push(sp);
	else
	    loudbug_bug("masterwidget_validate 2");
	scriptlet_free(sp);
    }
}
