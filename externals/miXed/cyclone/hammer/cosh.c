/* Copyright (c) 2002-2003 krzYszcz and others.
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

#include <math.h>
#include "m_pd.h"

#if defined(NT) || defined(MACOSX)
/* cf pd/src/x_arithmetic.c */
#define coshf  cosh
#endif

typedef struct _cosh
{
    t_object  x_ob;
    float     x_value;
} t_cosh;

static t_class *cosh_class;

static void cosh_bang(t_cosh *x)
{
    outlet_float(((t_object *)x)->ob_outlet, x->x_value);
}

static void cosh_float(t_cosh *x, t_float f)
{
    /* CHECKME large values */
    outlet_float(((t_object *)x)->ob_outlet, x->x_value = coshf(f));
}

static void *cosh_new(t_floatarg f)
{
    t_cosh *x = (t_cosh *)pd_new(cosh_class);
    /* CHECKME large values */
    x->x_value = coshf(f);
    outlet_new((t_object *)x, &s_float);
    return (x);
}

void cosh_setup(void)
{
    cosh_class = class_new(gensym("cosh"),
			   (t_newmethod)cosh_new, 0,
			   sizeof(t_cosh), 0, A_DEFFLOAT, 0);
    class_addbang(cosh_class, cosh_bang);
    class_addfloat(cosh_class, cosh_float);
}
