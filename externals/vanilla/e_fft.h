/* Copyright (c) 1997- Miller Puckette and others.
* For information on usage and redistribution, and for a DISCLAIMER OF ALL
* WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

#include "m_pd.h"

typedef struct fft
{
    t_object x_obj;
    t_float x_f;
} t_sigfft;

t_int *sigfft_swap(t_int *w); /* swap two arrays */
    /* take array1 (supply a pointer to beginning) and copy it,
    into decreasing addresses, into array 2 (supply a pointer one past the
    end), and negate the sign. */
t_int *sigrfft_flip(t_int *w);
void sigfft_dspx(t_sigfft *x, t_signal **sp, t_int *(*f)(t_int *w));
