/* Copyright (c) 2005 krzYszcz and others.
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

#include <stdio.h>
#include "m_pd.h"
#include "common/loud.h"
#include "common/props.h"
#include "toxy/scriptlet.h"
#include "widget.h"

#ifdef KRZYSZCZ
//#define WIDGETHANDLERS_DEBUG
#endif

typedef struct _widgetscript
{
    t_symbol              *ws_selector;
    t_scriptlet           *ws_script;
    struct _widgetscript  *ws_next;
} t_widgetscript;

struct _widgethandlers
{
    t_scriptlet      *wh_vis;
    t_scriptlet      *wh_new;
    t_scriptlet      *wh_free;
    t_scriptlet      *wh_data;
    t_scriptlet      *wh_bang;
    t_scriptlet      *wh_float;
    t_scriptlet      *wh_symbol;
    t_widgetscript   *wh_others;
};

static t_symbol *widgetps_vis = 0;
static t_symbol *widgetps_new;
static t_symbol *widgetps_free;
static t_symbol *widgetps_data;

t_widgethandlers *widgethandlers_new(t_scriptlet *generator)
{
    t_widgethandlers *wh = getbytes(sizeof(*wh));
    if (!widgetps_vis)
    {
	widgetps_vis = gensym("vis");
	widgetps_new = gensym("new");
	widgetps_free = gensym("free");
	widgetps_data = gensym("data");
    }
    wh->wh_vis = scriptlet_newalike(generator);
    wh->wh_new = scriptlet_newalike(generator);
    wh->wh_free = scriptlet_newalike(generator);
    wh->wh_data = scriptlet_newalike(generator);
    wh->wh_bang = scriptlet_newalike(generator);
    wh->wh_float = scriptlet_newalike(generator);
    wh->wh_symbol = scriptlet_newalike(generator);
    wh->wh_others = 0;
    return (wh);
}

void widgethandlers_free(t_widgethandlers *wh)
{
    t_widgetscript  *ws, *wsnext = wh->wh_others;
    scriptlet_free(wh->wh_vis);
    scriptlet_free(wh->wh_new);
    scriptlet_free(wh->wh_free);
    scriptlet_free(wh->wh_data);
    scriptlet_free(wh->wh_bang);
    scriptlet_free(wh->wh_float);
    scriptlet_free(wh->wh_symbol);
    while (ws = wsnext)
    {
	wsnext = ws->ws_next;
	scriptlet_free(ws->ws_script);
	freebytes(ws, sizeof(*ws));
    }
    freebytes(wh, sizeof(*wh));
}

void widgethandlers_reset(t_widgethandlers *wh)
{
    t_widgetscript  *ws = wh->wh_others;
    scriptlet_reset(wh->wh_vis);
    scriptlet_reset(wh->wh_new);
    scriptlet_reset(wh->wh_free);
    scriptlet_reset(wh->wh_data);
    scriptlet_reset(wh->wh_bang);
    scriptlet_reset(wh->wh_float);
    scriptlet_reset(wh->wh_symbol);
    for (ws = wh->wh_others; ws; ws = ws->ws_next)
	scriptlet_reset(ws->ws_script);
}

static t_widgetscript *widgethandlers_takeotherscript(t_widgethandlers *wh,
						      t_symbol *selector)
{
    t_widgetscript *ws;
    for (ws = wh->wh_others; ws; ws = ws->ws_next)
	if (ws->ws_selector == selector)
	    break;
    if (!ws)
    {
	ws = getbytes(sizeof(*ws));
	ws->ws_selector = selector;
	ws->ws_script = scriptlet_newalike(wh->wh_vis);
	ws->ws_next = wh->wh_others;
	wh->wh_others = ws;
    }
    return (ws);
}

t_scriptlet *widgethandlers_takeany(t_widgethandlers *wh, t_symbol *selector)
{
    t_scriptlet *sp;
    if (selector == widgetps_vis)
	sp = wh->wh_vis;
    else if (selector == widgetps_new)
	sp = wh->wh_new;
    else if (selector == widgetps_free)
	sp = wh->wh_free;
    else if (selector == widgetps_data)
	sp = wh->wh_data;
    else if (selector == &s_bang)
	sp = wh->wh_bang;
    else if (selector == &s_float)
	sp = wh->wh_float;
    else if (selector == &s_symbol)
	sp = wh->wh_symbol;
    else
    {
	t_widgetscript *ws;
	if (ws = widgethandlers_takeotherscript(wh, selector))
	    sp = ws->ws_script;
	else
	{
	    loudbug_bug("widgethandlers_takeany");
	    sp = 0;
	}
    }
    return (sp);
}

void widgethandlers_fill(t_widgethandlers *wh, t_props *pp)
{
    int ac;
    t_atom *ap;
    if (ap = props_getfirst(pp, &ac))
    {
	do
	{
	    if (ac > 1 && ap->a_type == A_SYMBOL &&
		ap->a_w.w_symbol->s_name[0] == '@' &&
		ap->a_w.w_symbol->s_name[1] != 0)
	    {
		t_symbol *sel = gensym(ap->a_w.w_symbol->s_name + 1);
		t_scriptlet *sp;
		if (sp = widgethandlers_takeany(wh, sel))
		{
		    scriptlet_reset(sp);
		    scriptlet_add(sp, 0, 0, ac - 1, ap + 1);
		}
	    }
	    else loudbug_bug("widgethandlers_fill");
	}
	while (ap = props_getnext(pp, &ac));
    }
}

t_scriptlet *widgethandlers_getvis(t_widgethandlers *wh)
{
    return (wh->wh_vis);
}

t_scriptlet *widgethandlers_getnew(t_widgethandlers *wh)
{
    return (wh->wh_new);
}

t_scriptlet *widgethandlers_getfree(t_widgethandlers *wh)
{
    return (wh->wh_free);
}

t_scriptlet *widgethandlers_getdata(t_widgethandlers *wh)
{
    return (wh->wh_data);
}

t_scriptlet *widgethandlers_getbang(t_widgethandlers *wh)
{
    return (wh->wh_bang);
}

t_scriptlet *widgethandlers_getfloat(t_widgethandlers *wh)
{
    return (wh->wh_float);
}

t_scriptlet *widgethandlers_getsymbol(t_widgethandlers *wh)
{
    return (wh->wh_symbol);
}

t_scriptlet *widgethandlers_getother(t_widgethandlers *wh, t_symbol *selector)
{
    t_widgetscript *ws;
    for (ws = wh->wh_others; ws; ws = ws->ws_next)
	if (ws->ws_selector == selector)
	    return (ws->ws_script);
    return (0);
}
