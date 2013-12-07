/* Copyright (c) 1997-1999 Miller Puckette.
* For information on usage and redistribution, and for a DISCLAIMER OF ALL
* WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

/* clock objects */

#include "m_pd.h"
#include <stdio.h>

static t_class *metro_class;

typedef struct _metro
{
    t_object x_obj;
    t_clock *x_clock;
    double x_deltime;
    int x_hit;
} t_metro;

static void metro_tick(t_metro *x)
{
    x->x_hit = 0;
    outlet_bang(x->x_obj.ob_outlet);
    if (!x->x_hit) clock_delay(x->x_clock, x->x_deltime);
}

static void metro_float(t_metro *x, t_float f)
{
    if (f != 0) metro_tick(x);
    else clock_unset(x->x_clock);
    x->x_hit = 1;
}

static void metro_bang(t_metro *x)
{
    metro_float(x, 1);
}

static void metro_stop(t_metro *x)
{
    metro_float(x, 0);
}

static void metro_ft1(t_metro *x, t_floatarg g)
{
    if (g < 1) g = 1;
    x->x_deltime = g;
}

static void metro_free(t_metro *x)
{
    clock_free(x->x_clock);
}

static void *metro_new(t_floatarg f)
{
    t_metro *x = (t_metro *)pd_new(metro_class);
    metro_ft1(x, f);
    x->x_hit = 0;
    x->x_clock = clock_new(x, (t_method)metro_tick);
    outlet_new(&x->x_obj, gensym("bang"));
    inlet_new(&x->x_obj, &x->x_obj.ob_pd, gensym("float"), gensym("ft1"));
    return (x);
}

void metro_setup(void)
{
    metro_class = class_new(gensym("metro"), (t_newmethod)metro_new,
        (t_method)metro_free, sizeof(t_metro), 0, A_DEFFLOAT, 0);
    class_addbang(metro_class, metro_bang);
    class_addmethod(metro_class, (t_method)metro_stop, gensym("stop"), 0);
    class_addmethod(metro_class, (t_method)metro_ft1, gensym("ft1"),
        A_FLOAT, 0);
    class_addfloat(metro_class, (t_method)metro_float);
}
