/* Copyright (c) 2002-2003 krzYszcz and others.
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

#include <math.h>
#include "m_pd.h"

#if defined(NT) || defined(MACOSX)
/* cf pd/src/x_arithmetic.c */
#define sinhf  sinh
#endif

typedef struct _sinh
{
    t_object  x_ob;
    float     x_value;
} t_sinh;

static t_class *sinh_class;

static void sinh_bang(t_sinh *x)
{
    outlet_float(((t_object *)x)->ob_outlet, x->x_value);
}

static void sinh_float(t_sinh *x, t_float f)
{
    /* CHECKME large values */
    outlet_float(((t_object *)x)->ob_outlet, x->x_value = sinhf(f));
}

static void *sinh_new(t_floatarg f)
{
    t_sinh *x = (t_sinh *)pd_new(sinh_class);
    /* CHECKME large values */
    x->x_value = sinhf(f);
    outlet_new((t_object *)x, &s_float);
    return (x);
}

void sinh_setup(void)
{
    sinh_class = class_new(gensym("sinh"),
			   (t_newmethod)sinh_new, 0,
			   sizeof(t_sinh), 0, A_DEFFLOAT, 0);
    class_addbang(sinh_class, sinh_bang);
    class_addfloat(sinh_class, sinh_float);
}
