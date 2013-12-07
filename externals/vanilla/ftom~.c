/* Copyright (c) 1997-2001 Miller Puckette and others.
* For information on usage and redistribution, and for a DISCLAIMER OF ALL
* WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

#include "m_pd.h"
#include <math.h>

typedef struct ftom_tilde
{
    t_object x_obj;
    t_float x_f;
} t_ftom_tilde;

t_class *ftom_tilde_class;

static void *ftom_tilde_new(void)
{
    t_ftom_tilde *x = (t_ftom_tilde *)pd_new(ftom_tilde_class);
    outlet_new(&x->x_obj, gensym("signal"));
    x->x_f = 0;
    return (x);
}

static t_int *ftom_tilde_perform(t_int *w)
{
    t_sample *in = *(t_sample **)(w+1), *out = *(t_sample **)(w+2);
    t_int n = *(t_int *)(w+3);
    for (; n--; in++, out++)
    {
        t_sample f = *in;
        *out = (f > 0 ? 17.3123405046 * log(.12231220585 * f) : -1500);
    }
    return (w + 4);
}

static void ftom_tilde_dsp(t_ftom_tilde *x, t_signal **sp)
{
    dsp_add(ftom_tilde_perform, 3, sp[0]->s_vec, sp[1]->s_vec, sp[0]->s_n);
}

void ftom_tilde_setup(void)
{
    ftom_tilde_class = class_new(gensym("ftom~"), (t_newmethod)ftom_tilde_new, 0,
        sizeof(t_ftom_tilde), 0, 0);
    CLASS_MAINSIGNALIN(ftom_tilde_class, t_ftom_tilde, x_f);
    class_addmethod(ftom_tilde_class, (t_method)ftom_tilde_dsp, gensym("dsp"), 0);
}
