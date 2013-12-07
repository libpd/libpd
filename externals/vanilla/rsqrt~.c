/* Copyright (c) 1997-2001 Miller Puckette and others.
* For information on usage and redistribution, and for a DISCLAIMER OF ALL
* WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

#include "e_sqrt.h"
#include "m_pd.h"
#include <math.h>

/* sigrsqrt - reciprocal square root good to 8 mantissa bits  */

static void init_rsqrt(void)
{
    int i;
    for (i = 0; i < DUMTAB1SIZE; i++)
    {
        float f;
        int32 l = (i ? (i == DUMTAB1SIZE-1 ? DUMTAB1SIZE-2 : i) : 1)<< 23;
        *(int32 *)(&f) = l;
        rsqrt_exptab[i] = 1./sqrt(f);   
    }
    for (i = 0; i < DUMTAB2SIZE; i++)
    {
        float f = 1 + (1./DUMTAB2SIZE) * i;
        rsqrt_mantissatab[i] = 1./sqrt(f);      
    }
}

typedef struct sigrsqrt
{
    t_object x_obj;
    t_float x_f;
} t_sigrsqrt;

static t_class *sigrsqrt_class;

static void *sigrsqrt_new(void)
{
    t_sigrsqrt *x = (t_sigrsqrt *)pd_new(sigrsqrt_class);
    outlet_new(&x->x_obj, gensym("signal"));
    x->x_f = 0;
    return (x);
}

static t_int *sigrsqrt_perform(t_int *w)
{
    t_sample *in = *(t_sample **)(w+1), *out = *(t_sample **)(w+2);
    t_int n = *(t_int *)(w+3);
    while (n--)
    {   
        t_sample f = *in;
        long l = *(long *)(in++);
        if (f < 0) *out++ = 0;
        else
        {
            t_sample g = rsqrt_exptab[(l >> 23) & 0xff] *
                rsqrt_mantissatab[(l >> 13) & 0x3ff];
            *out++ = 1.5 * g - 0.5 * g * g * g * f;
        }
    }
    return (w + 4);
}

static void sigrsqrt_dsp(t_sigrsqrt *x, t_signal **sp)
{
    dsp_add(sigrsqrt_perform, 3, sp[0]->s_vec, sp[1]->s_vec, sp[0]->s_n);
}

void rsqrt_tilde_setup(void)
{
    init_rsqrt();
    sigrsqrt_class = class_new(gensym("rsqrt~"), (t_newmethod)sigrsqrt_new, 0,
        sizeof(t_sigrsqrt), 0, 0);
            /* an old name for it: */
    class_addcreator(sigrsqrt_new, gensym("q8_rsqrt~"), 0);
    CLASS_MAINSIGNALIN(sigrsqrt_class, t_sigrsqrt, x_f);
    class_addmethod(sigrsqrt_class, (t_method)sigrsqrt_dsp, gensym("dsp"), 0);
}

