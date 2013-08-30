/* Copyright (c) 1997-2001 Miller Puckette and others.
* For information on usage and redistribution, and for a DISCLAIMER OF ALL
* WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

/*  sqrt-related functions from pd-vanilla's d_math.c */

#include "e_sqrt.h"

    /* these are used in externs like "bonk" */

t_float q8_rsqrt(t_float f)
{
    long l = *(long *)(&f);
    if (f < 0) return (0);
    else return (rsqrt_exptab[(l >> 23) & 0xff] *
            rsqrt_mantissatab[(l >> 13) & 0x3ff]);
}

t_float q8_sqrt(t_float f)
{
    long l = *(long *)(&f);
    if (f < 0) return (0);
    else return (f * rsqrt_exptab[(l >> 23) & 0xff] *
            rsqrt_mantissatab[(l >> 13) & 0x3ff]);
}

t_int *sigsqrt_perform(t_int *w)    /* used in [sqrt~] and [framp~] */
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
            *out++ = f * (1.5 * g - 0.5 * g * g * g * f);
        }
    }
    return (w + 4);
}

    /* the old names are OK unless we're in IRIX N32 */

#ifndef N32
t_float qsqrt(t_float f) {return (q8_sqrt(f)); }
t_float qrsqrt(t_float f) {return (q8_rsqrt(f)); }
#endif
