/* Copyright (c) 2002-2003 krzYszcz and others.
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

#include <string.h>
#include "m_pd.h"
#include "common/grow.h"

#define FUNNEL_MINSLOTS   2
#define FUNNEL_INISIZE   32  /* LATER rethink */
#define FUNNEL_MAXSIZE  256

typedef struct _funnel
{
    t_object    x_ob;
    int         x_nslots;
    int         x_nproxies;  /* as requested (and allocated) */
    t_pd      **x_proxies;
} t_funnel;

typedef struct _funnel_proxy
{
    t_object   p_ob;
    t_outlet  *p_out;   /* master outlet (the same value for each slot) */
    int        p_id;    /* adjusted according to an offset argument */
    t_float    p_value;
    int        p_size;  /* as allocated */
    t_atom    *p_message;
    t_atom     p_messini[FUNNEL_INISIZE];
    int        p_entered;
} t_funnel_proxy;

static t_class *funnel_class;
static t_class *funnel_proxy_class;

static void funnel_proxy_bang(t_funnel_proxy *x)
{
    t_atom at[2];
    SETFLOAT(&at[0], x->p_id);
    SETFLOAT(&at[1], x->p_value);
    outlet_list(x->p_out, &s_list, 2, at);
}

static void funnel_proxy_float(t_funnel_proxy *x, t_float f)
{
    x->p_value = f;
    funnel_proxy_bang(x);
}

static void funnel_proxy_list(t_funnel_proxy *x,
			      t_symbol *s, int ac, t_atom *av)
{
    int reentered = x->p_entered;
    int prealloc = !reentered;
    int ntotal = ac + 1;
    x->p_entered = 1;
    if (prealloc && ntotal > x->p_size)
    {
	if (ntotal > FUNNEL_MAXSIZE)
	    prealloc = 0;
	else
	{
	    x->p_message = grow_nodata(&ntotal, &x->p_size, x->p_message,
				       FUNNEL_INISIZE, x->p_messini,
				       sizeof(*x->p_message));
	    ac = ntotal - 1;
	}
    }
    /* LATER consider a compatibility warning if av->a_type != A_FLOAT */
    x->p_value = ((ac && av->a_type == A_FLOAT) ? av->a_w.w_float : 0);
    if (prealloc)
    {
	SETFLOAT(x->p_message, x->p_id);
	if (ac)
	    memcpy(x->p_message + 1, av, ac * sizeof(*x->p_message));
	outlet_list(x->p_out, &s_list, ntotal, x->p_message);
    }
    else
    {
	/* LATER consider using the stack if ntotal <= MAXSTACK */
	t_atom *buf = getbytes(ntotal * sizeof(*buf));
	if (buf)
	{
	    SETFLOAT(buf, x->p_id);
	    if (ac)
		memcpy(buf + 1, av, ac * sizeof(*buf));
	    outlet_list(x->p_out, &s_list, ntotal, buf);
	    freebytes(buf, ntotal * sizeof(*buf));
	}
    }
    if (!reentered) x->p_entered = 0;
}

static void funnel_bang(t_funnel *x)
{
    funnel_proxy_bang((t_funnel_proxy *)x->x_proxies[0]);
}

static void funnel_float(t_funnel *x, t_float f)
{
    funnel_proxy_float((t_funnel_proxy *)x->x_proxies[0], f);
}

static void funnel_list(t_funnel *x, t_symbol *s, int ac, t_atom *av)
{
    funnel_proxy_list((t_funnel_proxy *)x->x_proxies[0], s, ac, av);
}

static void funnel_free(t_funnel *x)
{
    if (x->x_proxies)
    {
	int i = x->x_nslots;
	while (i--)
	{
	    t_funnel_proxy *y = (t_funnel_proxy *)x->x_proxies[i];
	    if (y->p_message != y->p_messini)
		freebytes(y->p_message, y->p_size * sizeof(*y->p_message));
	    pd_free((t_pd *)y);
	}
	freebytes(x->x_proxies, x->x_nproxies * sizeof(*x->x_proxies));
    }
}

static void *funnel_new(t_floatarg f1, t_floatarg f2)
{
    t_funnel *x;
    int i, nslots, nproxies = (int)f1;
    int offset = (int)f2;
    t_outlet *out;
    t_pd **proxies;
    if (nproxies < 1)  /* CHECKED: one-slot funnel may be created */
	nproxies = FUNNEL_MINSLOTS;
    if (!(proxies = (t_pd **)getbytes(nproxies * sizeof(*proxies))))
	return (0);
    for (nslots = 0; nslots < nproxies; nslots++)
	if (!(proxies[nslots] = pd_new(funnel_proxy_class))) break;
    if (!nslots)
    {
	freebytes(proxies, nproxies * sizeof(*proxies));
	return (0);
    }
    x = (t_funnel *)pd_new(funnel_class);
    x->x_nslots = nslots;
    x->x_nproxies = nproxies;
    x->x_proxies = proxies;
    out = outlet_new((t_object *)x, &s_list);
    for (i = 0; i < nslots; i++)
    {
	t_funnel_proxy *y = (t_funnel_proxy *)proxies[i];
	y->p_out = out;
	y->p_id = offset++;
	y->p_value = 0;
	y->p_size = FUNNEL_INISIZE;
	y->p_message = y->p_messini;
	y->p_entered = 0;
	if (i) inlet_new((t_object *)x, (t_pd *)y, 0, 0);
    }
    return (x);
}

void funnel_setup(void)
{
    funnel_class = class_new(gensym("funnel"),
			     (t_newmethod)funnel_new,
			     (t_method)funnel_free,
			     sizeof(t_funnel), 0, A_DEFFLOAT, A_DEFFLOAT, 0);
    class_addbang(funnel_class, funnel_bang);
    class_addfloat(funnel_class, funnel_float);
    class_addlist(funnel_class, funnel_list);
    /* CHECKED: funnel doesn't understand symbol, anything */
    funnel_proxy_class = class_new(gensym("_funnel_proxy"), 0, 0,
				   sizeof(t_funnel_proxy),
				   CLASS_PD | CLASS_NOINLET, 0);
    class_addbang(funnel_proxy_class, funnel_proxy_bang);
    class_addfloat(funnel_proxy_class, funnel_proxy_float);
    class_addlist(funnel_proxy_class, funnel_proxy_list);
    /* CHECKED: funnel doesn't understand symbol, anything */
}
