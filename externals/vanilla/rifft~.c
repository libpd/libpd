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

static t_class *sigrifft_class;

typedef struct rifft
{
    t_object x_obj;
    t_float x_f;
} t_sigrifft;

static void *sigrifft_new(void)
{
    t_sigrifft *x = (t_sigrifft *)pd_new(sigrifft_class);
    inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_signal, &s_signal);
    outlet_new(&x->x_obj, gensym("signal"));
    x->x_f = 0;
    return (x);
}

static t_int *sigrifft_perform(t_int *w)
{
    t_sample *in = (t_sample *)(w[1]);
    int n = w[2];
    mayer_realifft(n, in);
    return (w+3);
}

static void sigrifft_dsp(t_sigrifft *x, t_signal **sp)
{
    int n = sp[0]->s_n, n2 = (n>>1);
    t_sample *in1 = sp[0]->s_vec;
    t_sample *in2 = sp[1]->s_vec;
    t_sample *out1 = sp[2]->s_vec;
    if (n < 4)
    {
        error("fft: minimum 4 points");
        return;
    }
    if (in2 == out1)
    {
        dsp_add(sigrfft_flip, 3, out1+1, out1 + n, n2-1);
        dsp_add(copy_perform, 3, in1, out1, n2+1);
    }
    else
    {
        if (in1 != out1) dsp_add(copy_perform, 3, in1, out1, n2+1);
        dsp_add(sigrfft_flip, 3, in2+1, out1 + n, n2-1);
    }
    dsp_add(sigrifft_perform, 2, out1, n);
}

void rifft_tilde_setup(void)
{
    sigrifft_class = class_new(gensym("rifft~"), sigrifft_new, 0,
        sizeof(t_sigrifft), 0, 0);
    CLASS_MAINSIGNALIN(sigrifft_class, t_sigrifft, x_f);
    class_addmethod(sigrifft_class, (t_method)sigrifft_dsp, gensym("dsp"), 0);
    class_sethelpsymbol(sigrifft_class, gensym("fft~"));
}
