/* Copyright (c) 2005 krzYszcz and others.
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

#include <stdio.h>
#include <string.h>
#include "m_pd.h"
#include "g_canvas.h"
#include "patchvalue.h"

#ifdef KRZYSZCZ
# include "loud.h"
# define PATCHVALUE_DEBUG
#else
# define loudbug_bug(msg)  fprintf(stderr, "BUG: %s\n", msg), bug(msg)
#endif

typedef struct _patchstorage
{
    t_glist               *ps_glist;
    t_patchvalue          *ps_values;
    struct _patchstorage  *ps_next;
} t_patchstorage;

typedef struct _patchboard
{
    t_pd             pb_pd;
    t_symbol        *pb_category;
    t_patchstorage  *pb_contents;
} t_patchboard;

static t_class *patchboard_class = 0;

/* assuming there is no 'name' in the storage */
static t_patchvalue *patchstorage_addvalue(
    t_patchstorage *ps, t_patchvalue *prv, t_class *cls, t_symbol *name)
{
    t_patchvalue *pv = (t_patchvalue *)pd_new(cls);
    pv->pv_name = name;
    pv->pv_refcount = 0;
    if (prv)
    {
	pv->pv_next = prv->pv_next;
	prv->pv_next = pv;
    }
    else
    {
	pv->pv_next = ps->ps_values;
	ps->ps_values = pv;
    }
    return (pv);
}

/* assuming there is no 'glist' on the board */
static t_patchstorage *patchboard_addstorage(
    t_patchboard *pb, t_patchstorage *prv, t_glist *glist)
{
    t_patchstorage *ps = getbytes(sizeof(*ps));
    ps->ps_glist = glist;
    ps->ps_values = 0;
    if (prv)
    {
	ps->ps_next = prv->ps_next;
	prv->ps_next = ps;
    }
    else
    {
	ps->ps_next = pb->pb_contents;
	pb->pb_contents = ps;
    }
    return (ps);
}

/* not used (LATER find a gc scheme) */
static void patchstorage_removevalue(
    t_patchstorage *ps, t_patchvalue *prv, t_patchvalue *pv, int force)
{
    if (force || pv->pv_refcount < 1)
    {
	if (prv)
	    prv->pv_next = pv->pv_next;
	else
	    ps->ps_values = pv->pv_next;
	pd_free((t_pd *)pv);
    }
}

/* not used (LATER find a gc scheme) */
static void patchboard_removestorage(
    t_patchboard *pb, t_patchstorage *prv, t_patchstorage *ps, int force)
{
    if (prv)
	prv->ps_next = ps->ps_next;
    else
	pb->pb_contents = ps->ps_next;
    if (force)
    {
	t_patchvalue *pv, *pvnext = ps->ps_values;
	while (pv = pvnext)
	{
	    pvnext = pv->pv_next;
	    pd_free((t_pd *)pv);
	}
    }
    else if (ps->ps_values)
	return;
    freebytes(ps, sizeof(*ps));
}

static t_patchvalue *patchstorage_findvalue(
    t_patchstorage *ps, t_symbol *name)
{
    t_patchvalue *pv;
    for (pv = ps->ps_values; pv; pv = pv->pv_next)
	if (pv->pv_name == name)
	    break;
    return (pv);
}

static t_patchstorage *patchboard_findstorage(
    t_patchboard *pb, t_glist *glist)
{
    t_patchstorage *ps;
    for (ps = pb->pb_contents; ps; ps = ps->ps_next)
	if (ps->ps_glist == glist)
	    break;
    return (ps);
}

static t_patchboard *patchboard_find(t_symbol *category)
{
    if (!patchboard_class)
	patchboard_class =
	    patchvalue_classnew(gensym("_patchboard"), sizeof(t_patchboard));
    return ((t_patchboard *)pd_findbyclass(category, patchboard_class));
}

static t_patchboard *patchboard_use(t_symbol *category)
{
    if (!patchboard_class)
	patchboard_class =
	    patchvalue_classnew(gensym("_patchboard"), sizeof(t_patchboard));
    if (category && *category->s_name == '#')
    {
	t_patchboard *pb;
	if (!(pb = (t_patchboard *)pd_findbyclass(category, patchboard_class)))
	{
	    pb = (t_patchboard *)pd_new(patchboard_class);
	    pb->pb_category = category;
	    pd_bind((t_pd *)pb, category);  /* never unbound */
	    pb->pb_contents = 0;
	}
	return (pb);
    }
    else
    {
	loudbug_bug("patchboard_use");
	return (0);
    }
}

static t_patchstorage *patchstorage_use(t_symbol *category, t_glist *glist)
{
    t_patchboard *pb;
    if (pb = patchboard_use(category))
    {
	t_patchstorage *ps;
	if (!(ps = patchboard_findstorage(pb, glist)))
	    ps = patchboard_addstorage(pb, 0, glist);
	return (ps);
    }
    else return (0);
}

/* The class might have been created by another dll...
   This is public, because apart from the "_patchboard" class above,
   it is called for the "_raftentry" class too.  LATER rethink. */
t_class *patchvalue_classnew(t_symbol *cname, size_t size)
{
    t_class *cls;
    t_symbol *bindsym;
    char buf[MAXPDSTRING];
    sprintf(buf, "#%s", cname->s_name);
    bindsym = gensym(buf);
    if (bindsym->s_thing)
    {
	t_pd *pd = bindsym->s_thing;
	char *name = class_getname(*pd);
	if (strcmp(name, cname->s_name))
	{
	    /* FIXME handle this properly... */
	    loudbug_bug("patchvalue_classnew");
	}
	else return (*pd);
    }
    cls = class_new(cname, 0, 0, size, CLASS_PD | CLASS_NOINLET, 0);
    pd_bind(pd_new(cls), bindsym);  /* never unbound */
    return (cls);
}

t_patchvalue *patchvalue_use(t_symbol *category, t_glist *glist,
			     t_class *cls, t_symbol *name)
{
    t_patchstorage *ps;
    if (ps = patchstorage_use(category, glist))
    {
	t_patchvalue *pv;
	if (pv = patchstorage_findvalue(ps, name))
	{
	    if (*(t_pd *)pv != cls)
	    {
		loudbug_bug("patchvalue_use");
		return (0);
	    }
	}
	else pv = patchstorage_addvalue(ps, 0, cls, name);
	return (pv);
    }
    else return (0);
}

t_patchvalue *patchvalue_get(t_symbol *category, t_glist *glist,
			     t_class *cls, t_symbol *name)
{
    t_patchboard *pb;
    t_patchstorage *ps;
    t_patchvalue *pv;
    if ((pb = patchboard_find(category)) &&
	(ps = patchboard_findstorage(pb, glist)) &&
	(pv = patchstorage_findvalue(ps, name)))
    {
	if (*(t_pd *)pv == cls)
	    return (pv);
	else
	    loudbug_bug("patchvalue_get");
    }
    return (0);
}

t_patchvalue *patchvalue_resolve(t_symbol *category, t_glist *glist,
				 t_class *cls, t_symbol *name)
{
    t_patchboard *pb;
    if (pb = patchboard_find(category))
    {
	t_patchstorage *ps;
	t_patchvalue *pv;
	while (glist)
	{
	    if ((ps = patchboard_findstorage(pb, glist)) &&
		(pv = patchstorage_findvalue(ps, name)))
	    {
		if (*(t_pd *)pv == cls)
		    return (pv);
		else
		    loudbug_bug("patchvalue_resolve");
	    }
	    else if (canvas_isabstraction(glist))
		break;
	    else
		glist = glist->gl_owner;
	}
    }
    return (0);
}
