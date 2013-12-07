/* Copyright (c) 2002-2003 krzYszcz and others.
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

#include <math.h>
#include "m_pd.h"

#if defined(NT) || defined(MACOSX)
/* cf pd/src/x_arithmetic.c */
#define atan2f  atan2
#define hypotf  hypot
#endif

typedef struct _cartopol
{
    t_object   x_ob;
    t_float    x_imag;
    t_outlet  *x_out2;
} t_cartopol;

static t_class *cartopol_class;

static void cartopol_float(t_cartopol *x, t_float f)
{
    outlet_float(x->x_out2, atan2f(x->x_imag, f));
    outlet_float(((t_object *)x)->ob_outlet, hypotf(f, x->x_imag));
}

static void *cartopol_new(void)
{
    t_cartopol *x = (t_cartopol *)pd_new(cartopol_class);
    floatinlet_new((t_object *)x, &x->x_imag);
    outlet_new((t_object *)x, &s_float);
    x->x_out2 = outlet_new((t_object *)x, &s_float);
    return (x);
}

void cartopol_setup(void)
{
    cartopol_class = class_new(gensym("cartopol"),
			       (t_newmethod)cartopol_new, 0,
			       sizeof(t_cartopol), 0, 0);
    class_addfloat(cartopol_class, cartopol_float);
}
