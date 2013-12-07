/* Copyright (c) 2003-2005 krzYszcz and others.
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

#include "m_pd.h"
#include "common/loud.h"
#include "toxy/plusbob.h"
#include "plustot.h"

typedef struct _plusproxy_in
{
    t_pd        pp_pd;
    t_plustob  *pp_tob;
} t_plusproxy_in;

typedef struct _plustot_in
{
    t_plusobject     x_plusobject;
    t_glist         *x_glist;
    t_plustob       *x_tob;
    t_plusproxy_in  *x_proxy;
} t_plustot_in;

static t_class *plusproxy_in_class;
static t_class *plustot_in_class;

static t_plusproxy_in *plusproxy_in_new(t_pd *master)
{
    t_plusproxy_in *pp = (t_plusproxy_in *)pd_new(plusproxy_in_class);
    pp->pp_tob = ((t_plustot_in *)master)->x_tob;
    return (pp);
}

static void plusproxy_in_float(t_plusproxy_in *pp, t_float f)
{
    plustob_setfloat(pp->pp_tob, f);
}

static void plusproxy_in_symbol(t_plusproxy_in *pp, t_symbol *s)
{
    plustob_setsymbol(pp->pp_tob, s);
}

static void plusproxy_in_list(t_plusproxy_in *pp,
			      t_symbol *s, int ac, t_atom *av)
{
    plustob_setlist(pp->pp_tob, ac, av);
}

static void plustot_in_bang(t_plustot_in *x)
{
    if (plustob_getvalue(x->x_tob))
	outlet_plusbob(((t_object *)x)->ob_outlet, (t_plusbob *)x->x_tob);
}

static void plustot_in_float(t_plustot_in *x, t_float f)
{
    if (plustob_setfloat(x->x_tob, f))
	outlet_plusbob(((t_object *)x)->ob_outlet, (t_plusbob *)x->x_tob);
}

static void plustot_in_symbol(t_plustot_in *x, t_symbol *s)
{
    if (plustob_setsymbol(x->x_tob, s))
	outlet_plusbob(((t_object *)x)->ob_outlet, (t_plusbob *)x->x_tob);
}

static void plustot_in_list(t_plustot_in *x, t_symbol *s, int ac, t_atom *av)
{
    if (plustob_setlist(x->x_tob, ac, av))
	outlet_plusbob(((t_object *)x)->ob_outlet, (t_plusbob *)x->x_tob);
}

static void plustot_in_free(t_plustot_in *x)
{
    plusbob_release((t_plusbob *)x->x_tob);
    if (x->x_proxy) pd_free((t_pd *)x->x_proxy);
    plusobject_free(&x->x_plusobject);
}

void *plustot_in_new(t_symbol *s, int ac, t_atom *av)
{
    t_plustot_in *x = 0;
    t_glist *glist = canvas_getcurrent();
    t_plustin *tin = 0;
    t_plustob *tob = 0;
    if ((tin = plustin_glistprovide(glist, PLUSTIN_GLIST_ANY, 0)) &&
	(tob = plustob_new(tin, 0)))
    {
	x = (t_plustot_in *)plusobject_new(plustot_in_class, s, ac, av, 0);
	plusbob_preserve((t_plusbob *)tob);
	plusbob_setowner((t_plusbob *)tob, (t_pd *)x);
	plustob_setlist(tob, ac, av);
	x->x_glist = glist;
	x->x_tob = tob;
	x->x_proxy = plusproxy_in_new((t_pd *)x);
	plusinlet_new(&x->x_plusobject, (t_pd *)x->x_proxy, 0, 0);
	plusoutlet_new(&x->x_plusobject, &s_symbol);
    }
    else
    {
	loud_error(0, "+in: cannot initialize");
	if (tin)
	{
	    plusbob_preserve((t_plusbob *)tin);
	    plusbob_release((t_plusbob *)tin);
	}
    }
    return (x);
}

void plustot_in_setup(void)
{
    plustot_in_class = class_new(gensym("+in"), 0,
				 (t_method)plustot_in_free,
				 sizeof(t_plustot_in), 0, 0);
    plusclass_inherit(plustot_in_class, gensym("+in"));
    class_addbang(plustot_in_class, plustot_in_bang);
    class_addfloat(plustot_in_class, plustot_in_float);
    class_addsymbol(plustot_in_class, plustot_in_symbol);
    class_addlist(plustot_in_class, plustot_in_list);

    plusproxy_in_class = class_new(gensym("+in proxy"), 0, 0,
				   sizeof(t_plusproxy_in), CLASS_PD, 0);
    class_addfloat(plusproxy_in_class, plusproxy_in_float);
    class_addsymbol(plusproxy_in_class, plusproxy_in_symbol);
    class_addlist(plusproxy_in_class, plusproxy_in_list);
}
