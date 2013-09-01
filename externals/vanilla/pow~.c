/* Copyright (c) 1997-2001 Miller Puckette and others.
* For information on usage and redistribution, and for a DISCLAIMER OF ALL
* WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

#include "m_pd.h"
#include <math.h>

static t_class *pow_tilde_class;

typedef struct _pow_tilde
{
    t_object x_obj;
    t_float x_f;
} t_pow_tilde;

static void *pow_tilde_new(t_symbol *s, int argc, t_atom *argv)
{
    t_pow_tilde *x = (t_pow_tilde *)pd_new(pow_tilde_class);
    inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_signal, &s_signal);
    outlet_new(&x->x_obj, &s_signal);
    x->x_f = 0;
    return (x);
}

t_int *pow_tilde_perform(t_int *w)
{
    t_sample *in1 = (t_sample *)(w[1]);
    t_sample *in2 = (t_sample *)(w[2]);
    t_sample *out = (t_sample *)(w[3]);
    int n = (int)(w[4]);
    while (n--)
    {
        float f = *in1++;
        if (f > 0)
            *out = pow(f, *in2);
        else *out = 0;
        out++;
        in2++;
    }
    return (w+5);
}

static void pow_tilde_dsp(t_pow_tilde *x, t_signal **sp)
{
    dsp_add(pow_tilde_perform, 4,
        sp[0]->s_vec, sp[1]->s_vec, sp[2]->s_vec, sp[0]->s_n);
}

void pow_tilde_setup(void)
{
    pow_tilde_class = class_new(gensym("pow~"), (t_newmethod)pow_tilde_new, 0,
        sizeof(t_pow_tilde), 0, A_DEFFLOAT, 0);
    CLASS_MAINSIGNALIN(pow_tilde_class, t_pow_tilde, x_f);
    class_addmethod(pow_tilde_class, (t_method)pow_tilde_dsp, gensym("dsp"), 0);
}
