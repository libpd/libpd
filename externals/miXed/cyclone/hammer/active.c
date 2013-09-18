/* Copyright (c) 2002-2003 krzYszcz and others.
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

#include <stdio.h>
#include "m_pd.h"
#include "hammer/gui.h"

typedef struct _active
{
    t_object   x_ob;
    t_symbol  *x_cvname;
    int        x_on;
} t_active;

static t_class *active_class;

static void active_dofocus(t_active *x, t_symbol *s, t_floatarg f)
{
    if ((int)f)
    {
	int on = (s == x->x_cvname);
	if (on != x->x_on)
	    outlet_float(((t_object *)x)->ob_outlet, x->x_on = on);
    }
    else if (x->x_on && s == x->x_cvname)
	outlet_float(((t_object *)x)->ob_outlet, x->x_on = 0);
}

static void active_free(t_active *x)
{
    hammergui_unbindfocus((t_pd *)x);
}

static void *active_new(void)
{
    t_active *x = (t_active *)pd_new(active_class);
    char buf[32];
    sprintf(buf, ".x%lx.c", (int)canvas_getcurrent());
    x->x_cvname = gensym(buf);
    x->x_on = 0;
    outlet_new((t_object *)x, &s_float);
    hammergui_bindfocus((t_pd *)x);
    return (x);
}

void active_setup(void)
{
    active_class = class_new(gensym("active"),
			     (t_newmethod)active_new,
			     (t_method)active_free,
			     sizeof(t_active), CLASS_NOINLET, 0);
    class_addmethod(active_class, (t_method)active_dofocus,
		    gensym("_focus"), A_SYMBOL, A_FLOAT, 0);
}
