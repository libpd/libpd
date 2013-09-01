/* Copyright (c) 1997-2001 Miller Puckette and others.
* For information on usage and redistribution, and for a DISCLAIMER OF ALL
* WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

#include "e_sqrt.h"

typedef struct sigsqrt
{
    t_object x_obj;
    t_float x_f;
} t_sigsqrt;

static t_class *sigsqrt_class;

static void *sigsqrt_new(void)
{
    t_sigsqrt *x = (t_sigsqrt *)pd_new(sigsqrt_class);
    outlet_new(&x->x_obj, gensym("signal"));
    x->x_f = 0;
    return (x);
}

static void sigsqrt_dsp(t_sigsqrt *x, t_signal **sp)
{
    dsp_add(sigsqrt_perform, 3, sp[0]->s_vec, sp[1]->s_vec, sp[0]->s_n);
}

void sqrt_tilde_setup(void)
{
    sigsqrt_class = class_new(gensym("sqrt~"), (t_newmethod)sigsqrt_new, 0,
        sizeof(t_sigsqrt), 0, 0);
    class_addcreator(sigsqrt_new, gensym("q8_sqrt~"), 0);   /* old name */
    CLASS_MAINSIGNALIN(sigsqrt_class, t_sigsqrt, x_f);
    class_addmethod(sigsqrt_class, (t_method)sigsqrt_dsp, gensym("dsp"), 0);
}
