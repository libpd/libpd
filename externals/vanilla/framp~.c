/* Copyright (c) 1997- Miller Puckette and others.
* For information on usage and redistribution, and for a DISCLAIMER OF ALL
* WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

#include "m_pd.h"

/* This file interfaces to one of the Mayer, Ooura, or fftw FFT packages
to implement the "fft~", etc, Pd objects.  If using Mayer, also compile
d_fft_mayer.c; if ooura, use d_fft_fftsg.c instead; if fftw, use d_fft_fftw.c
and also link in the fftw library.  You can only have one of these three
linked in.  The configure script can be used to select which one.
*/

static t_class *sigframp_class;

typedef struct framp
{
    t_object x_obj;
    t_float x_f;
} t_sigframp;

static void *sigframp_new(void)
{
    t_sigframp *x = (t_sigframp *)pd_new(sigframp_class);
    inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_signal, &s_signal);
    outlet_new(&x->x_obj, gensym("signal"));
    outlet_new(&x->x_obj, gensym("signal"));
    x->x_f = 0;
    return (x);
}

static t_int *sigframp_perform(t_int *w)
{
    t_sample *inreal = (t_sample *)(w[1]);
    t_sample *inimag = (t_sample *)(w[2]);
    t_sample *outfreq = (t_sample *)(w[3]);
    t_sample *outamp = (t_sample *)(w[4]);
    t_sample lastreal = 0, currentreal = inreal[0], nextreal = inreal[1];
    t_sample lastimag = 0, currentimag = inimag[0], nextimag = inimag[1];
    int n = w[5];
    int m = n + 1;
    t_sample fbin = 1, oneovern2 = 1.f/((t_sample)n * (t_sample)n);
    
    inreal += 2;
    inimag += 2;
    *outamp++ = *outfreq++ = 0;
    n -= 2;
    while (n--)
    {
        t_sample re, im, pow, freq;
        lastreal = currentreal;
        currentreal = nextreal;
        nextreal = *inreal++;
        lastimag = currentimag;
        currentimag = nextimag;
        nextimag = *inimag++;
        re = currentreal - 0.5f * (lastreal + nextreal);
        im = currentimag - 0.5f * (lastimag + nextimag);
        pow = re * re + im * im;
        if (pow > 1e-19)
        {
            t_sample detune = ((lastreal - nextreal) * re +
                    (lastimag - nextimag) * im) / (2.0f * pow);
            if (detune > 2 || detune < -2) freq = pow = 0;
            else freq = fbin + detune;
        }
        else freq = pow = 0;
        *outfreq++ = freq;
        *outamp++ = oneovern2 * pow;
        fbin += 1.0f;
    }
    while (m--) *outamp++ = *outfreq++ = 0;
    return (w+6);
}

t_int *sigsqrt_perform(t_int *w);

static void sigframp_dsp(t_sigframp *x, t_signal **sp)
{
    int n = sp[0]->s_n, n2 = (n>>1);
    if (n < 4)
    {
        error("framp: minimum 4 points");
        return;
    }
    dsp_add(sigframp_perform, 5, sp[0]->s_vec, sp[1]->s_vec,
        sp[2]->s_vec, sp[3]->s_vec, n2);
    dsp_add(sigsqrt_perform, 3, sp[3]->s_vec, sp[3]->s_vec, n2);
}

void framp_tilde_setup(void)
{
    sigframp_class = class_new(gensym("framp~"), sigframp_new, 0,
        sizeof(t_sigframp), 0, 0);
    CLASS_MAINSIGNALIN(sigframp_class, t_sigframp, x_f);
    class_addmethod(sigframp_class, (t_method)sigframp_dsp, gensym("dsp"), 0);
}
