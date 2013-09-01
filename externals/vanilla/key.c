/* Copyright (c) 1997-2000 Miller Puckette.
* For information on usage and redistribution, and for a DISCLAIMER OF ALL
* WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

#include "m_pd.h"
#include <stdio.h>
#include <string.h>

static t_symbol *key_sym;
static t_class *key_class;

typedef struct _key
{
    t_object x_obj;
} t_key;

static void *key_new( void)
{
    t_key *x = (t_key *)pd_new(key_class);
    outlet_new(&x->x_obj, &s_float);
    pd_bind(&x->x_obj.ob_pd, key_sym);
    return (x);
}

static void key_float(t_key *x, t_floatarg f)
{
    outlet_float(x->x_obj.ob_outlet, f);
}

static void key_free(t_key *x)
{
    pd_unbind(&x->x_obj.ob_pd, key_sym);
}

void key_setup(void)
{
    key_class = class_new(gensym("key"),
        (t_newmethod)key_new, (t_method)key_free,
        sizeof(t_key), CLASS_NOINLET, 0);
    class_addfloat(key_class, key_float);
    key_sym = gensym("#key");
}

