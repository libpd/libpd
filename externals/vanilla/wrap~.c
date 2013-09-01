/* Copyright (c) 1997-2001 Miller Puckette and others.
* For information on usage and redistribution, and for a DISCLAIMER OF ALL
* WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

/*  mathematical functions and other transfer functions, including tilde
    versions of stuff from x_acoustics.c.
*/

#include "m_pd.h"
#include <math.h>

typedef struct wrap
{
    t_object x_obj;
    t_float x_f;
} t_sigwrap;

t_class *sigwrap_class;

static void *sigwrap_new(void)
{
    t_sigwrap *x = (t_sigwrap *)pd_new(sigwrap_class);
    outlet_new(&x->x_obj, gensym("signal"));
    x->x_f = 0;
    return (x);
}

static t_int *sigwrap_perform(t_int *w)
{
    t_sample *in = *(t_sample **)(w+1), *out = *(t_sample **)(w+2);
    t_int n = *(t_int *)(w+3);
    while (n--)
    {   
        t_sample f = *in++;
        int k = f;
        if (f > 0) *out++ = f-k;
        else *out++ = f - (k-1);
    }
    return (w + 4);
}

static void sigwrap_dsp(t_sigwrap *x, t_signal **sp)
{
    dsp_add(sigwrap_perform, 3, sp[0]->s_vec, sp[1]->s_vec, sp[0]->s_n);
}

void wrap_tilde_setup(void)
{
    sigwrap_class = class_new(gensym("wrap~"), (t_newmethod)sigwrap_new, 0,
        sizeof(t_sigwrap), 0, 0);
    CLASS_MAINSIGNALIN(sigwrap_class, t_sigwrap, x_f);
    class_addmethod(sigwrap_class, (t_method)sigwrap_dsp, gensym("dsp"), 0);
}
