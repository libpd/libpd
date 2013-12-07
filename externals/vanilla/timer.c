/* Copyright (c) 1997-1999 Miller Puckette.
* For information on usage and redistribution, and for a DISCLAIMER OF ALL
* WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

/* clock objects */

#include "m_pd.h"
#include <stdio.h>

static t_class *timer_class;

typedef struct _timer
{
    t_object x_obj;
    double x_settime;
} t_timer;

static void timer_bang(t_timer *x)
{
    x->x_settime = clock_getsystime();
}

static void timer_bang2(t_timer *x)
{
    outlet_float(x->x_obj.ob_outlet, clock_gettimesince(x->x_settime));
}

static void *timer_new(t_floatarg f)
{
    t_timer *x = (t_timer *)pd_new(timer_class);
    timer_bang(x);
    outlet_new(&x->x_obj, gensym("float"));
    inlet_new(&x->x_obj, &x->x_obj.ob_pd, gensym("bang"), gensym("bang2"));
    return (x);
}

void timer_setup(void)
{
    timer_class = class_new(gensym("timer"), (t_newmethod)timer_new, 0,
        sizeof(t_timer), 0, A_DEFFLOAT, 0);
    class_addbang(timer_class, timer_bang);
    class_addmethod(timer_class, (t_method)timer_bang2, gensym("bang2"), 0);
}
