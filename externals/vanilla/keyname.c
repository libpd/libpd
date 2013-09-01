/* Copyright (c) 1997-2000 Miller Puckette.
* For information on usage and redistribution, and for a DISCLAIMER OF ALL
* WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

/* dialogs.  LATER, deal with the situation where the object goes 
away before the panel does... */

#include "m_pd.h"
#include <stdio.h>
#include <string.h>

static t_symbol *keyname_sym;
static t_class *keyname_class;

typedef struct _keyname
{
    t_object x_obj;
    t_outlet *x_outlet1;
    t_outlet *x_outlet2;
} t_keyname;

static void *keyname_new( void)
{
    t_keyname *x = (t_keyname *)pd_new(keyname_class);
    x->x_outlet1 = outlet_new(&x->x_obj, &s_float);
    x->x_outlet2 = outlet_new(&x->x_obj, &s_symbol);
    pd_bind(&x->x_obj.ob_pd, keyname_sym);
    return (x);
}

static void keyname_list(t_keyname *x, t_symbol *s, int ac, t_atom *av)
{
    outlet_symbol(x->x_outlet2, atom_getsymbolarg(1, ac, av));
    outlet_float(x->x_outlet1, atom_getfloatarg(0, ac, av));
}

static void keyname_free(t_keyname *x)
{
    pd_unbind(&x->x_obj.ob_pd, keyname_sym);
}

void keyname_setup(void)
{
    keyname_class = class_new(gensym("keyname"),
        (t_newmethod)keyname_new, (t_method)keyname_free,
        sizeof(t_keyname), CLASS_NOINLET, 0);
    class_addlist(keyname_class, keyname_list);
    keyname_sym = gensym("#keyname");
    class_sethelpsymbol(keyname_class, gensym("key"));
}
