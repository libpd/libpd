/* Copyright (c) 1997-2001 Miller Puckette and others.
* For information on usage and redistribution, and for a DISCLAIMER OF ALL
* WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

#include "m_pd.h"
#include <math.h>

static t_class *log_tilde_class;

typedef struct _log_tilde
{
    t_object x_obj;
    t_float x_f;
} t_log_tilde;

static void *log_tilde_new(t_symbol *s, int argc, t_atom *argv)
{
    t_log_tilde *x = (t_log_tilde *)pd_new(log_tilde_class);
    inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_signal, &s_signal);
    outlet_new(&x->x_obj, &s_signal);
    x->x_f = 0;
    return (x);
}

t_int *log_tilde_perform(t_int *w)
{
    t_sample *in1 = (t_sample *)(w[1]);
    t_sample *in2 = (t_sample *)(w[2]);
    t_sample *out = (t_sample *)(w[3]);
    int n = (int)(w[4]);
    while (n--)
    {
        float f = *in1++, g = *in2++;
        if (f <= 0)
            *out = -1000;   /* rather than blow up, output a number << 0 */
        else if (g <= 0)
            *out = log(f);
        else *out = log(f)/log(g);
        out++;
    }
    return (w+5);
}

static void log_tilde_dsp(t_log_tilde *x, t_signal **sp)
{
    dsp_add(log_tilde_perform, 4,
        sp[0]->s_vec, sp[1]->s_vec, sp[2]->s_vec, sp[0]->s_n);
}

void log_tilde_setup(void)
{
    log_tilde_class = class_new(gensym("log~"), (t_newmethod)log_tilde_new, 0,
        sizeof(t_log_tilde), 0, A_DEFFLOAT, 0);
    CLASS_MAINSIGNALIN(log_tilde_class, t_log_tilde, x_f);
    class_addmethod(log_tilde_class, (t_method)log_tilde_dsp, gensym("dsp"), 0);
}
