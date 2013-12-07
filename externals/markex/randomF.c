/*
 * Copyright (c) 1997-1999 Mark Danks.
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt" in this distribution.
 */

#include "m_pd.h"
#include <stdlib.h>

#ifndef __APPLE__
#include <math.h>
#endif

#ifdef _WIN32
int random(void)
{
    static int foo = 1489853723;
    foo = foo * (int)435898247 + (int)9382842987;
    return (foo & 0x7fffffff);
}
#endif

/* -------------------------- randomF  ------------------------------ */

/* instance structure */

static t_class *randomF_class;

typedef struct _randomF
{
    t_object	x_obj;
    t_float		x_range;
	t_outlet    *t_out1;	    /* the outlet */
} t_randomF;

static void *randomF_new(t_floatarg n)
{
    t_randomF *x = (t_randomF *)pd_new(randomF_class);
    x->x_range = (float)n;
    floatinlet_new(&x->x_obj, &x->x_range);
    x->t_out1 = outlet_new(&x->x_obj, 0);
    return (x);
}

static void randomF_bang(t_randomF *x)
{
    double range = (x->x_range < 0 ? 0 : x->x_range);
    double n = (double)range * (double)random() * (1. / 2147483648.);
    if (n >= range) n = range - 1;
    outlet_float(x->t_out1, (float)n);
}

void randomF_setup(void)
{
    randomF_class = class_new(gensym("randomF"), (t_newmethod)randomF_new, 0,
    	sizeof(t_randomF), 0, A_DEFFLOAT, 0);

    class_addbang(randomF_class, (t_method)randomF_bang);

#if PD_MINOR_VERSION < 37 
	class_sethelpsymbol(randomF_class, gensym("randomF-help.pd"));
#endif
}
