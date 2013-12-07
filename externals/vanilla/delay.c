/* Copyright (c) 1997-1999 Miller Puckette.
* For information on usage and redistribution, and for a DISCLAIMER OF ALL
* WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

/* clock objects */

#include "m_pd.h"
#include <stdio.h>
/* -------------------------- delay ------------------------------ */
static t_class *delay_class;

typedef struct _delay
{
    t_object x_obj;
    t_clock *x_clock;
    double x_deltime;
} t_delay;

static void delay_bang(t_delay *x)
{
    clock_delay(x->x_clock, x->x_deltime);
}

static void delay_stop(t_delay *x)
{
    clock_unset(x->x_clock);
}

static void delay_ft1(t_delay *x, t_floatarg g)
{
    if (g < 0) g = 0;
    x->x_deltime = g;
}

static void delay_float(t_delay *x, t_float f)
{
    delay_ft1(x, f);
    delay_bang(x);
}

static void delay_tick(t_delay *x)
{
    outlet_bang(x->x_obj.ob_outlet);
}

static void delay_free(t_delay *x)
{
    clock_free(x->x_clock);
}

static void *delay_new(t_floatarg f)
{
    t_delay *x = (t_delay *)pd_new(delay_class);
    delay_ft1(x, f);
    x->x_clock = clock_new(x, (t_method)delay_tick);
    outlet_new(&x->x_obj, gensym("bang"));
    inlet_new(&x->x_obj, &x->x_obj.ob_pd, gensym("float"), gensym("ft1"));
    return (x);
}

void delay_setup(void)
{
    delay_class = class_new(gensym("delay"), (t_newmethod)delay_new,
        (t_method)delay_free, sizeof(t_delay), 0, A_DEFFLOAT, 0);
    class_addcreator((t_newmethod)delay_new, gensym("del"), A_DEFFLOAT, 0);
    class_addbang(delay_class, delay_bang);
    class_addmethod(delay_class, (t_method)delay_stop, gensym("stop"), 0);
    class_addmethod(delay_class, (t_method)delay_ft1,
        gensym("ft1"), A_FLOAT, 0);
    class_addfloat(delay_class, (t_method)delay_float);
}
