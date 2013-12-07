/* Copyright (c) 2003-2005 krzYszcz and others.
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

#include "m_pd.h"
#include "common/loud.h"
#include "toxy/plusbob.h"
#include "plustot.h"

typedef struct _plusproxy_var
{
    t_pd        pp_pd;
    t_plusvar  *pp_var;
} t_plusproxy_var;

typedef struct _plustot_var
{
    t_plusobject      x_plusobject;
    t_glist          *x_glist;
    t_plusvar        *x_var;
    t_plusproxy_var  *x_proxy;
} t_plustot_var;

static t_class *plusproxy_var_class;
static t_class *plustot_var_class;

static t_plusproxy_var *plusproxy_var_new(t_pd *master)
{
    t_plusproxy_var *pp = (t_plusproxy_var *)pd_new(plusproxy_var_class);
    pp->pp_var = ((t_plustot_var *)master)->x_var;
    return (pp);
}

static void plusproxy_var_float(t_plusproxy_var *pp, t_float f)
{
    plusvar_setfloat(pp->pp_var, f, 1);
}

static void plusproxy_var_symbol(t_plusproxy_var *pp, t_symbol *s)
{
    plusvar_setsymbol(pp->pp_var, s, 1);
}

static void plusproxy_var_list(t_plusproxy_var *pp,
			       t_symbol *s, int ac, t_atom *av)
{
    plusvar_setlist(pp->pp_var, ac, av, 1);
}

static void plustot_var_bang(t_plustot_var *x)
{
    if (plusvar_pull(x->x_var))
	outlet_plusbob(((t_object *)x)->ob_outlet, (t_plusbob *)x->x_var);
}

static void plustot_var_float(t_plustot_var *x, t_float f)
{
    if (plusvar_setfloat(x->x_var, f, 1))
	outlet_plusbob(((t_object *)x)->ob_outlet, (t_plusbob *)x->x_var);
}

static void plustot_var_symbol(t_plustot_var *x, t_symbol *s)
{
    if (plusvar_setsymbol(x->x_var, s, 1))
	outlet_plusbob(((t_object *)x)->ob_outlet, (t_plusbob *)x->x_var);
}

static void plustot_var_list(t_plustot_var *x, t_symbol *s, int ac, t_atom *av)
{
    if (plusvar_setlist(x->x_var, ac, av, 1))
	outlet_plusbob(((t_object *)x)->ob_outlet, (t_plusbob *)x->x_var);
}

static void plustot_var_free(t_plustot_var *x)
{
    plusbob_release((t_plusbob *)x->x_var);
    if (x->x_proxy) pd_free((t_pd *)x->x_proxy);
    plusobject_free(&x->x_plusobject);
}

void *plustot_var_new(t_symbol *s, int ac, t_atom *av)
{
    t_plustot_var *x = 0;
    t_glist *glist = canvas_getcurrent();
    t_plustin *tin = 0;
    t_plusvar *var = 0;
    if (ac && av->a_type == A_SYMBOL &&
	(tin = plustin_glistprovide(glist, PLUSTIN_GLIST_ANY, 0)) &&
	(var = plusvar_new(av->a_w.w_symbol->s_name, 0, tin)))
    {
	x = (t_plustot_var *)plusobject_new(plustot_var_class, s, ac, av, 0);
	plusbob_preserve((t_plusbob *)var);
	plusbob_setowner((t_plusbob *)var, (t_pd *)x);
	plusvar_setlist(var, ac - 1, av + 1, 1);
	x->x_glist = glist;
	x->x_var = var;
	x->x_proxy = plusproxy_var_new((t_pd *)x);
	plusinlet_new(&x->x_plusobject, (t_pd *)x->x_proxy, 0, 0);
	plusoutlet_new(&x->x_plusobject, &s_symbol);
    }
    else
    {
	if (!ac || av->a_type != A_SYMBOL)
	    loud_error(0, "+var: missing name of a variable");
	else
	    loud_error(0, "+var: cannot initialize");
	if (tin)
	{
	    plusbob_preserve((t_plusbob *)tin);
	    plusbob_release((t_plusbob *)tin);
	}
    }
    return (x);
}

void plustot_var_setup(void)
{
    plustot_var_class = class_new(gensym("+var"), 0,
				  (t_method)plustot_var_free,
				  sizeof(t_plustot_var), 0, 0);
    plusclass_inherit(plustot_var_class, gensym("+var"));
    class_addbang(plustot_var_class, plustot_var_bang);
    class_addfloat(plustot_var_class, plustot_var_float);
    class_addsymbol(plustot_var_class, plustot_var_symbol);
    class_addlist(plustot_var_class, plustot_var_list);

    plusproxy_var_class = class_new(gensym("+var proxy"), 0, 0,
				    sizeof(t_plusproxy_var), CLASS_PD, 0);
    class_addfloat(plusproxy_var_class, plusproxy_var_float);
    class_addsymbol(plusproxy_var_class, plusproxy_var_symbol);
    class_addlist(plusproxy_var_class, plusproxy_var_list);
}
