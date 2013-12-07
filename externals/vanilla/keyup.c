/* Copyright (c) 1997-2000 Miller Puckette.
* For information on usage and redistribution, and for a DISCLAIMER OF ALL
* WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

#include "m_pd.h"
#include <stdio.h>
#include <string.h>

static t_symbol *keyup_sym;
static t_class *keyup_class;

typedef struct _keyup
{
    t_object x_obj;
} t_keyup;

static void *keyup_new( void)
{
    t_keyup *x = (t_keyup *)pd_new(keyup_class);
    outlet_new(&x->x_obj, &s_float);
    pd_bind(&x->x_obj.ob_pd, keyup_sym);
    return (x);
}

static void keyup_float(t_keyup *x, t_floatarg f)
{
    outlet_float(x->x_obj.ob_outlet, f);
}

static void keyup_free(t_keyup *x)
{
    pd_unbind(&x->x_obj.ob_pd, keyup_sym);
}

void keyup_setup(void)
{
    keyup_class = class_new(gensym("keyup"),
        (t_newmethod)keyup_new, (t_method)keyup_free,
        sizeof(t_keyup), CLASS_NOINLET, 0);
    class_addfloat(keyup_class, keyup_float);
    keyup_sym = gensym("#keyup");
    class_sethelpsymbol(keyup_class, gensym("key"));
}
