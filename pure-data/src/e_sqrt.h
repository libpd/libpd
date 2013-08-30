/* Copyright (c) 1997-2001 Miller Puckette and others.
* For information on usage and redistribution, and for a DISCLAIMER OF ALL
* WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

/* these are things used in a number of objectclasses in d_math.c and d_fft.c */

#include "m_pd.h"
#include <math.h>

#define LOGTEN 2.302585092994
#define DUMTAB1SIZE 256
#define DUMTAB2SIZE 1024

#ifdef _WIN32
#define int32 long
#else
#include <stdint.h>
#define int32 int32_t
#endif

t_float q8_sqrt(t_float f);
t_float q8_rsqrt(t_float f);
t_int *sigsqrt_perform(t_int *w);    /* from d_math.c; also used in d_fft.c */

float rsqrt_exptab[DUMTAB1SIZE], rsqrt_mantissatab[DUMTAB2SIZE];
