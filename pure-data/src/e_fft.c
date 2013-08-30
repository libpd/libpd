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

/* ---------------- utility functions for DSP chains ---------------------- */

    /* swap two arrays */
t_int *sigfft_swap(t_int *w)
{
    t_sample *in1 = (t_sample *)(w[1]);
    t_sample *in2 = (t_sample *)(w[2]);
    int n = w[3];
    for (;n--; in1++, in2++)
    {   
        t_sample f = *in1;
        *in1 = *in2;
        *in2 = f;
    }
    return (w+4);    
}

    /* take array1 (supply a pointer to beginning) and copy it,
    into decreasing addresses, into array 2 (supply a pointer one past the
    end), and negate the sign. */

t_int *sigrfft_flip(t_int *w)
{
    t_sample *in = (t_sample *)(w[1]);
    t_sample *out = (t_sample *)(w[2]);
    int n = w[3];
    while (n--)
        *(--out) = - *in++;
    return (w+4);
}

void sigfft_dspx(t_sigfft *x, t_signal **sp, t_int *(*f)(t_int *w))
{
    int n = sp[0]->s_n;
    t_sample *in1 = sp[0]->s_vec;
    t_sample *in2 = sp[1]->s_vec;
    t_sample *out1 = sp[2]->s_vec;
    t_sample *out2 = sp[3]->s_vec;
    if (out1 == in2 && out2 == in1)
        dsp_add(sigfft_swap, 3, out1, out2, n);
    else if (out1 == in2)
    {
        dsp_add(copy_perform, 3, in2, out2, n);
        dsp_add(copy_perform, 3, in1, out1, n);
    }
    else
    {
        if (out1 != in1) dsp_add(copy_perform, 3, in1, out1, n);
        if (out2 != in2) dsp_add(copy_perform, 3, in2, out2, n);
    }
    dsp_add(f, 3, sp[2]->s_vec, sp[3]->s_vec, n);
}
