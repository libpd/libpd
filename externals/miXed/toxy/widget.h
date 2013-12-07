/* Copyright (c) 2003-2005 krzYszcz and others.
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

#ifndef __WIDGET_H__
#define __WIDGET_H__

EXTERN_STRUCT _widgettype;
#define t_widgettype  struct _widgettype

EXTERN_STRUCT _masterwidget;
#define t_masterwidget  struct _masterwidget

EXTERN_STRUCT _widgethandlers;
#define t_widgethandlers  struct _widgethandlers

t_widgettype *widgettype_find(t_symbol *s);
t_widgettype *widgettype_get(t_symbol *s, char *ver, char *opt, t_glist *glist);
t_widgettype *widgettype_reload(t_symbol *s, t_glist *glist);
int widgettype_isdefined(t_widgettype *wt);
t_symbol *widgettype_tkclass(t_widgettype *wt);
t_props *widgettype_getoptions(t_widgettype *wt);
t_props *widgettype_gethandlers(t_widgettype *wt);
t_props *widgettype_getarguments(t_widgettype *wt);
t_widgethandlers *widgettype_getscripts(t_widgettype *wt);
char *widgettype_propname(t_symbol *s);
void widgettype_setup(void);

char *masterwidget_getcontents(int *szp);
void masterwidget_validate(void);

t_widgethandlers *widgethandlers_new(t_scriptlet *generator);
void widgethandlers_free(t_widgethandlers *wh);
void widgethandlers_reset(t_widgethandlers *wh);
void widgethandlers_fill(t_widgethandlers *wh, t_props *pp);
t_scriptlet *widgethandlers_getvis(t_widgethandlers *wh);
t_scriptlet *widgethandlers_getnew(t_widgethandlers *wh);
t_scriptlet *widgethandlers_getfree(t_widgethandlers *wh);
t_scriptlet *widgethandlers_getdata(t_widgethandlers *wh);
t_scriptlet *widgethandlers_getbang(t_widgethandlers *wh);
t_scriptlet *widgethandlers_getfloat(t_widgethandlers *wh);
t_scriptlet *widgethandlers_getsymbol(t_widgethandlers *wh);
t_scriptlet *widgethandlers_getother(t_widgethandlers *wh, t_symbol *selector);
t_scriptlet *widgethandlers_takeany(t_widgethandlers *wh, t_symbol *selector);

#endif
