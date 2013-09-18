/* Copyright (c) 2003-2005 krzYszcz and others.
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

#include "m_pd.h"
#include "common/loud.h"
#include "toxy/plusbob.h"
#include "plustot.h"

typedef struct _plusproxy_ar
{
    t_pd        pp_pd;
    t_plustob  *pp_tob;
} t_plusproxy_ar;

typedef struct _plustot_ar
{
    t_plusobject     x_plusobject;
    t_glist         *x_glist;
    t_plustob       *x_tob;
    t_plusproxy_ar  *x_proxy;
} t_plustot_ar;

static t_class *plusproxy_ar_class;
static t_class *plustot_ar_class;

static t_plusproxy_ar *plusproxy_ar_new(t_pd *master)
{
    t_plusproxy_ar *pp = (t_plusproxy_ar *)pd_new(plusproxy_ar_class);
    pp->pp_tob = ((t_plustot_ar *)master)->x_tob;
    return (pp);
}

static void plusproxy_ar_float(t_plusproxy_ar *pp, t_float f)
{
    plustob_setfloat(pp->pp_tob, f);
}

static void plusproxy_ar_symbol(t_plusproxy_ar *pp, t_symbol *s)
{
    plustob_setsymbol(pp->pp_tob, s);
}

static void plusproxy_ar_list(t_plusproxy_ar *pp,
			      t_symbol *s, int ac, t_atom *av)
{
    plustob_setlist(pp->pp_tob, ac, av);
}

static void plustot_ar_bang(t_plustot_ar *x)
{
    if (plustob_getvalue(x->x_tob))
	outlet_plusbob(((t_object *)x)->ob_outlet, (t_plusbob *)x->x_tob);
}

static void plustot_ar_float(t_plustot_ar *x, t_float f)
{
    if (plustob_setfloat(x->x_tob, f))
	outlet_plusbob(((t_object *)x)->ob_outlet, (t_plusbob *)x->x_tob);
}

static void plustot_ar_symbol(t_plustot_ar *x, t_symbol *s)
{
    if (plustob_setsymbol(x->x_tob, s))
	outlet_plusbob(((t_object *)x)->ob_outlet, (t_plusbob *)x->x_tob);
}

static void plustot_ar_list(t_plustot_ar *x, t_symbol *s, int ac, t_atom *av)
{
    if (plustob_setlist(x->x_tob, ac, av))
	outlet_plusbob(((t_object *)x)->ob_outlet, (t_plusbob *)x->x_tob);
}

static void plustot_ar_free(t_plustot_ar *x)
{
    plusbob_release((t_plusbob *)x->x_tob);
    if (x->x_proxy) pd_free((t_pd *)x->x_proxy);
    plusobject_free(&x->x_plusobject);
}

void *plustot_ar_new(t_symbol *s, int ac, t_atom *av)
{
    t_plustot_ar *x = 0;
    t_glist *glist = canvas_getcurrent();
    t_plustin *tin = 0;
    t_plustob *tob = 0;
    if ((tin = plustin_glistprovide(glist, PLUSTIN_GLIST_ANY, 0)) &&
	(tob = plustob_new(tin, 0)))
    {
	x = (t_plustot_ar *)plusobject_new(plustot_ar_class, s, ac, av, 0);
	plusbob_preserve((t_plusbob *)tob);
	plusbob_setowner((t_plusbob *)tob, (t_pd *)x);
	plustob_setlist(tob, ac, av);
	x->x_glist = glist;
	x->x_tob = tob;
	x->x_proxy = plusproxy_ar_new((t_pd *)x);
	plusinlet_new(&x->x_plusobject, (t_pd *)x->x_proxy, 0, 0);
	plusoutlet_new(&x->x_plusobject, &s_symbol);
    }
    else
    {
	loud_error(0, "+ar: cannot initialize");
	if (tin)
	{
	    plusbob_preserve((t_plusbob *)tin);
	    plusbob_release((t_plusbob *)tin);
	}
    }
    return (x);
}

void plustot_ar_setup(void)
{
    plustot_ar_class = class_new(gensym("+ar"), 0,
				 (t_method)plustot_ar_free,
				 sizeof(t_plustot_ar), 0, 0);
    plusclass_inherit(plustot_ar_class, gensym("+ar"));
    class_addbang(plustot_ar_class, plustot_ar_bang);
    class_addfloat(plustot_ar_class, plustot_ar_float);
    class_addsymbol(plustot_ar_class, plustot_ar_symbol);
    class_addlist(plustot_ar_class, plustot_ar_list);

    plusproxy_ar_class = class_new(gensym("+ar proxy"), 0, 0,
				   sizeof(t_plusproxy_ar), CLASS_PD, 0);
    class_addfloat(plusproxy_ar_class, plusproxy_ar_float);
    class_addsymbol(plusproxy_ar_class, plusproxy_ar_symbol);
    class_addlist(plusproxy_ar_class, plusproxy_ar_list);
}
