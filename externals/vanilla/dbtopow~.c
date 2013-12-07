/* Copyright (c) 1997-2001 Miller Puckette and others.
* For information on usage and redistribution, and for a DISCLAIMER OF ALL
* WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

#include "e_sqrt.h"

typedef struct dbtopow_tilde
{
    t_object x_obj;
    t_float x_f;
} t_dbtopow_tilde;

t_class *dbtopow_tilde_class;

static void *dbtopow_tilde_new(void)
{
    t_dbtopow_tilde *x = (t_dbtopow_tilde *)pd_new(dbtopow_tilde_class);
    outlet_new(&x->x_obj, gensym("signal"));
    x->x_f = 0;
    return (x);
}

static t_int *dbtopow_tilde_perform(t_int *w)
{
    t_sample *in = *(t_sample **)(w+1), *out = *(t_sample **)(w+2);
    t_int n = *(t_int *)(w+3);
    for (; n--; in++, out++)
    {
        t_sample f = *in;
        if (f <= 0) *out = 0;
        else
        {
            if (f > 870)
                f = 870;
            *out = exp((LOGTEN * 0.1) * (f-100.));
        }
    }
    return (w + 4);
}

static void dbtopow_tilde_dsp(t_dbtopow_tilde *x, t_signal **sp)
{
    dsp_add(dbtopow_tilde_perform, 3, sp[0]->s_vec, sp[1]->s_vec, sp[0]->s_n);
}

void dbtopow_tilde_setup(void)
{
    dbtopow_tilde_class = class_new(gensym("dbtopow~"), (t_newmethod)dbtopow_tilde_new, 0,
        sizeof(t_dbtopow_tilde), 0, 0);
    CLASS_MAINSIGNALIN(dbtopow_tilde_class, t_dbtopow_tilde, x_f);
    class_addmethod(dbtopow_tilde_class, (t_method)dbtopow_tilde_dsp, gensym("dsp"), 0);
}
