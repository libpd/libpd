/* Copyright (c) 2002-2005 krzYszcz and others.
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

#include <math.h>
#include "m_pd.h"
#include "sickle/sic.h"

/* some random tests (average percentage in load meter and top):
   gcc 3.3.5 -O6, p4 2.66GHz, 4500 copies: perform 56, perf8 57, perf0 94
   gcc 3.3.5 -O6, p4 2.66GHz, 9000 copies: perform 118, perf8 123, perf0 194
   vc 6.0 /O2, p3 800Mhz, 750 copies: perform 61, perf8 56, perf0 82 */
#ifdef KRZYSZCZ
//#define ABS_TEST
#endif

#ifdef ABS_TEST
#include "common/fitter.h"
#endif

typedef t_sic t_abs;
static t_class *abs_class;

static t_int *abs_perform(t_int *w)
{
    int nblock = (int)(w[1]);
    t_float *in = (t_float *)(w[2]);
    t_float *out = (t_float *)(w[3]);
    while (nblock--)
	*out++ = fabsf(*in++);
    return (w + 4);
}

#ifdef ABS_TEST
static t_int *abs_perf0(t_int *w)
{
    int nblock = (int)(w[1]);
    t_float *in = (t_float *)(w[2]);
    t_float *out = (t_float *)(w[3]);
    while (nblock--)
    {
	float f = *in++;
    	*out++ = (f >= 0 ? f : -f);
    }
    return (w + 4);
}

static t_int *abs_perf8(t_int *w)
{
    int nblock = (int)(w[1])>>3;
    t_float *in = (t_float *)(w[2]);
    t_float *out = (t_float *)(w[3]);
    while (nblock--)
    {
	*out++ = fabsf(*in++);
	*out++ = fabsf(*in++);
	*out++ = fabsf(*in++);
	*out++ = fabsf(*in++);
	*out++ = fabsf(*in++);
	*out++ = fabsf(*in++);
	*out++ = fabsf(*in++);
	*out++ = fabsf(*in++);
    }
    return (w + 4);
}
#endif

static void abs_dsp(t_abs *x, t_signal **sp)
{
#ifdef ABS_TEST
    t_symbol *tst = fitter_getsymbol(gensym("test"));
    if (tst == gensym("unroll"))
	dsp_add(abs_perf8, 3, sp[0]->s_n, sp[0]->s_vec, sp[1]->s_vec);
    else if (tst == gensym("branch"))
	dsp_add(abs_perf0, 3, sp[0]->s_n, sp[0]->s_vec, sp[1]->s_vec);
    else
#endif
	dsp_add(abs_perform, 3, sp[0]->s_n, sp[0]->s_vec, sp[1]->s_vec);
}

static void *abs_new(void)
{
    t_abs *x = (t_abs *)pd_new(abs_class);
    outlet_new((t_object *)x, &s_signal);
    return (x);
}

void abs_tilde_setup(void)
{
    abs_class = class_new(gensym("abs~"),
			  (t_newmethod)abs_new, 0,
			  sizeof(t_abs), 0, 0);
    sic_setup(abs_class, abs_dsp, SIC_FLOATTOSIGNAL);
#ifdef ABS_TEST
    fitter_setup(abs_class, 0);
#endif
}
