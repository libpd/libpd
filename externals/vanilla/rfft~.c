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

static t_class *sigrfft_class;

typedef struct rfft
{
    t_object x_obj;
    t_float x_f;
} t_sigrfft;

static void *sigrfft_new(void)
{
    t_sigrfft *x = (t_sigrfft *)pd_new(sigrfft_class);
    outlet_new(&x->x_obj, gensym("signal"));
    outlet_new(&x->x_obj, gensym("signal"));
    x->x_f = 0;
    return (x);
}

static t_int *sigrfft_perform(t_int *w)
{
    t_sample *in = (t_sample *)(w[1]);
    int n = w[2];
    mayer_realfft(n, in);
    return (w+3);
}

static void sigrfft_dsp(t_sigrfft *x, t_signal **sp)
{
    int n = sp[0]->s_n, n2 = (n>>1);
    t_sample *in1 = sp[0]->s_vec;
    t_sample *out1 = sp[1]->s_vec;
    t_sample *out2 = sp[2]->s_vec;
    if (n < 4)
    {
        error("fft: minimum 4 points");
        return;
    }
    if (in1 != out1)
        dsp_add(copy_perform, 3, in1, out1, n);
    dsp_add(sigrfft_perform, 2, out1, n);
    dsp_add(sigrfft_flip, 3, out1 + (n2+1), out2 + n2, n2-1);
    dsp_add_zero(out1 + (n2+1), ((n2-1)&(~7)));
    dsp_add_zero(out1 + (n2+1) + ((n2-1)&(~7)), ((n2-1)&7));
    dsp_add_zero(out2 + n2, n2);
    dsp_add_zero(out2, 1);
}

void rfft_tilde_setup(void)
{
    sigrfft_class = class_new(gensym("rfft~"), sigrfft_new, 0,
        sizeof(t_sigrfft), 0, 0);
    CLASS_MAINSIGNALIN(sigrfft_class, t_sigrfft, x_f);
    class_addmethod(sigrfft_class, (t_method)sigrfft_dsp, gensym("dsp"), 0);
    class_sethelpsymbol(sigrfft_class, gensym("fft~"));
}
