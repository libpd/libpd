/* Copyright (c) 2003 krzYszcz and others.
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

/* This is Pd's lop~ with signal-controlled cutoff. */

/* CHECKED scalar case: input preserved (not coefs) after changing mode */
/* CHECKME if creation arg (or a default) restored after signal disconnection */

#include <math.h>
#include "m_pd.h"
#include "shared.h"
#include "sickle/sic.h"

#if defined(NT) || defined(MACOSX)
/* cf pd/src/x_arithmetic.c */
#define sinf  sin
#endif

#define ONEPOLE_HZ        0
#define ONEPOLE_LINEAR    1
#define ONEPOLE_RADIANS   2
#define ONEPOLE_MINB0     .0001  /* CHECKED 1st term of ir for b0=0 */
#define ONEPOLE_MAXB0     .99    /* CHECKED 1st term of ir for b0=1 */
#define ONEPOLE_MINOMEGA  0.     /* CHECKME */
#define ONEPOLE_MAXOMEGA  (SHARED_PI * .5)  /* CHECKME */

typedef struct _onepole
{
    t_sic  x_sic;
    int    x_mode;
    float  x_srcoef;
    float  x_ynm1;
} t_onepole;

static t_class *onepole_class;

static t_symbol *ps_hz;
static t_symbol *ps_linear;
static t_symbol *ps_radians;

static void onepole_clear(t_onepole *x)
{
    x->x_ynm1 = 0.;
}

static void onepole_hz(t_onepole *x)
{
    x->x_mode = ONEPOLE_HZ;
}

static void onepole_linear(t_onepole *x)
{
    x->x_mode = ONEPOLE_LINEAR;
}

static void onepole_radians(t_onepole *x)
{
    x->x_mode = ONEPOLE_RADIANS;
}

/* LATER make ready for optional audio-rate modulation
   (separate scalar case routine, use sic_makecostable(), etc.) */
static t_int *onepole_perform(t_int *w)
{
    t_onepole *x = (t_onepole *)(w[1]);
    int nblock = (int)(w[2]);
    t_float *xin = (t_float *)(w[3]);
    t_float fin0 = *(t_float *)(w[4]);
    t_float *out = (t_float *)(w[5]);
    int mode = x->x_mode;
    float ynm1 = x->x_ynm1;
    /* CHECKME sampled once per block */
    float b0;
    if (mode == ONEPOLE_HZ)
    {
	float omega = fin0 * x->x_srcoef;
	if (omega < ONEPOLE_MINOMEGA)
	    omega = ONEPOLE_MINOMEGA;
	else if (omega > ONEPOLE_MAXOMEGA)
	    omega = ONEPOLE_MAXOMEGA;
	/* The actual solution for a half-power cutoff is:
	   b0 = sqrt(sqr(2-cos(omega))-1) + cos(omega) - 1.
	   The sin(omega) below is only slightly better approximation than
	   Miller's b0=omega, except for the two highest octaves (or so),
	   where it is much better (but far from good). */
	b0 = sinf(omega);
    }
    else if (mode == ONEPOLE_LINEAR)
	b0 = sinf(fin0 * (SHARED_PI * .5));  /* CHECKME actual range of fin0 */
    else
	b0 = fin0;
    if (b0 < ONEPOLE_MINB0)
	b0 = ONEPOLE_MINB0;
    else if (b0 > ONEPOLE_MAXB0)
	b0 = ONEPOLE_MAXB0;
    /* b0 is the standard 1-|a1| (where |a1| is pole's radius),
       specifically: a1=b0-1 => a1 in [-.9999 .. -.01] => lowpass (stable) */
    while (nblock--)
	*out++ = ynm1 = b0 * (*xin++ - ynm1) + ynm1;
    x->x_ynm1 = (PD_BIGORSMALL(ynm1) ? 0. : ynm1);
    return (w + 6);
}

static void onepole_dsp(t_onepole *x, t_signal **sp)
{
    x->x_srcoef = SHARED_2PI / sp[0]->s_sr;
    onepole_clear(x);
    dsp_add(onepole_perform, 5, x, sp[0]->s_n,
	    sp[0]->s_vec, sp[1]->s_vec, sp[2]->s_vec);
}

static void *onepole_new(t_symbol *s, t_floatarg f)
{
    t_onepole *x = (t_onepole *)pd_new(onepole_class);
    x->x_srcoef = SHARED_2PI / sys_getsr();
    /* CHECKED no int-to-float conversion (any int bashed to 0.) */
    sic_newinlet((t_sic *)x, f);
    outlet_new((t_object *)x, &s_signal);
    onepole_clear(x);
    if (s == ps_linear)
	x->x_mode = ONEPOLE_LINEAR;
    else if (s == ps_radians)
	x->x_mode = ONEPOLE_RADIANS;
    else
    {
	x->x_mode = ONEPOLE_HZ;
	if (s && s != &s_ && s != ps_hz && s != gensym("Hz"))
	{
	    /* CHECKED no warning */
	}
    }
    return (x);
}

void onepole_tilde_setup(void)
{
    ps_hz = gensym("hz");
    ps_linear = gensym("linear");
    ps_radians = gensym("radians");
    onepole_class = class_new(gensym("onepole~"),
			      (t_newmethod)onepole_new, 0,
			      sizeof(t_onepole), 0,
			      A_DEFFLOAT, A_DEFSYM, 0);
    sic_setup(onepole_class, onepole_dsp, SIC_FLOATTOSIGNAL);
    class_addmethod(onepole_class, (t_method)onepole_clear, gensym("clear"), 0);
    class_addmethod(onepole_class, (t_method)onepole_hz, ps_hz, 0);
    class_addmethod(onepole_class, (t_method)onepole_hz, gensym("Hz"), 0);
    class_addmethod(onepole_class, (t_method)onepole_linear, ps_linear, 0);
    class_addmethod(onepole_class, (t_method)onepole_radians, ps_radians, 0);
}
