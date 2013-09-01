/* Copyright (c) 1997-2001 Miller Puckette and others.
* For information on usage and redistribution, and for a DISCLAIMER OF ALL
* WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

/* MIDI. */

#include "m_pd.h"

static t_symbol *sysexin_sym;

static t_class *sysexin_class;

typedef struct _midiin
{
    t_object x_obj;
    t_outlet *x_outlet1;
    t_outlet *x_outlet2;
} t_midiin;

static void sysexin_list(t_midiin *x, t_symbol *s, int ac, t_atom *av)
{
    outlet_float(x->x_outlet2, atom_getfloatarg(1, ac, av) + 1);
    outlet_float(x->x_outlet1, atom_getfloatarg(0, ac, av));
}

static void *sysexin_new( void)
{
    t_midiin *x = (t_midiin *)pd_new(sysexin_class);
    x->x_outlet1 = outlet_new(&x->x_obj, &s_float);
    x->x_outlet2 = outlet_new(&x->x_obj, &s_float);
    pd_bind(&x->x_obj.ob_pd, sysexin_sym);
#ifdef WIN32
    pd_error(x, "sysexin: windows: not supported");
#endif
    return (x);
}

static void sysexin_free(t_midiin *x)
{
    pd_unbind(&x->x_obj.ob_pd, sysexin_sym);
}

void sysexin_setup(void)
{
    sysexin_class = class_new(gensym("sysexin"), (t_newmethod)sysexin_new,
        (t_method)sysexin_free, sizeof(t_midiin),
            CLASS_NOINLET, A_DEFFLOAT, 0);
    class_addlist(sysexin_class, sysexin_list);

    sysexin_sym = gensym("#sysexin");
}
