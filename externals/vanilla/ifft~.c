/* Copyright (c) 1997- Miller Puckette and others.
* For information on usage and redistribution, and for a DISCLAIMER OF ALL
* WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

#include "m_pd.h"
#include "e_fft.h"

/* This file interfaces to one of the Mayer, Ooura, or fftw FFT packages
to implement the "fft~", etc, Pd objects.  If using Mayer, also compile
d_fft_mayer.c; if ooura, use d_fft_fftsg.c instead; if fftw, use d_fft_fftw.c
and also link in the fftw library.  You can only have one of these three
linked in.  The configure script can be used to select which one.
*/

static t_class *sigifft_class;

static void *sigifft_new(void)
{
    t_sigfft *x = (t_sigfft *)pd_new(sigifft_class);
    outlet_new(&x->x_obj, gensym("signal"));
    outlet_new(&x->x_obj, gensym("signal"));
    inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_signal, &s_signal);
    x->x_f = 0;
    return (x);
}

static t_int *sigifft_perform(t_int *w)
{
    t_sample *in1 = (t_sample *)(w[1]);
    t_sample *in2 = (t_sample *)(w[2]);
    int n = w[3];
    mayer_ifft(n, in1, in2);
    return (w+4);
}

static void sigifft_dsp(t_sigfft *x, t_signal **sp)
{
    sigfft_dspx(x, sp, sigifft_perform);
}

void ifft_tilde_setup(void)
{
    sigifft_class = class_new(gensym("ifft~"), sigifft_new, 0,
        sizeof(t_sigfft), 0, 0);
    CLASS_MAINSIGNALIN(sigifft_class, t_sigfft, x_f);
    class_addmethod(sigifft_class, (t_method)sigifft_dsp, gensym("dsp"), 0);
    class_sethelpsymbol(sigifft_class, gensym("fft~"));
}
