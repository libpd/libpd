/* Copyright (c) 1997-2001 Miller Puckette and others.
* For information on usage and redistribution, and for a DISCLAIMER OF ALL
* WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

#include "m_pd.h"
#include <math.h>

static t_class *abs_tilde_class;

typedef struct _abs_tilde
{
    t_object x_obj;
    t_float x_f;
} t_abs_tilde;

static void *abs_tilde_new(t_symbol *s, int argc, t_atom *argv)
{
    t_abs_tilde *x = (t_abs_tilde *)pd_new(abs_tilde_class);
    outlet_new(&x->x_obj, &s_signal);
    return (x);
}

t_int *abs_tilde_perform(t_int *w)
{
    t_sample *in1 = (t_sample *)(w[1]);
    t_sample *out = (t_sample *)(w[2]);
    int n = (int)(w[3]);
    while (n--)
    {
        float f = *in1++;
        *out++ = (f >= 0 ? f : -f);
    }
    return (w+4);
}

static void abs_tilde_dsp(t_abs_tilde *x, t_signal **sp)
{
    dsp_add(abs_tilde_perform, 3,
        sp[0]->s_vec, sp[1]->s_vec, sp[0]->s_n);
}

void abs_tilde_setup(void)
{
    abs_tilde_class = class_new(gensym("abs~"), (t_newmethod)abs_tilde_new, 0,
        sizeof(t_abs_tilde), 0, 0);
    CLASS_MAINSIGNALIN(abs_tilde_class, t_abs_tilde, x_f);
    class_addmethod(abs_tilde_class, (t_method)abs_tilde_dsp, gensym("dsp"), 0);
}
