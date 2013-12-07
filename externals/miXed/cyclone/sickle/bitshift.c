/* Copyright (c) 2002-2003 krzYszcz and others.
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

/* When bit-shifting 32-bit values, gcc (intel?) bashes the second operand
   modulo 32.  In msp x << 32 gives 0, x >> 32 gives a propagated sign bit
   (as expected).  Mimicking that is clumsy.  LATER consider making the calcs
   more generic (use long long values?) */

#include "m_pd.h"
#include "common/loud.h"
#include "sickle/sic.h"

#ifdef KRZYSZCZ
//#define BITSHIFT_DEBUG
#endif

typedef struct _bitshift
{
    t_sic  x_sic;
    int    x_convert1;
    int    x_lshift;
    int    x_rshift;
    int    x_lover;
} t_bitshift;

static t_class *bitshift_class;

static t_int *bitshift_perform(t_int *w)
{
    t_bitshift *x = (t_bitshift *)(w[1]);
    int nblock = (int)(w[2]);
    t_float *in = (t_float *)(w[3]);
    t_float *out = (t_float *)(w[4]);
    /* LATER think about performance */
    if (x->x_lshift)
    {
	unsigned int shift = x->x_lshift;
	if (x->x_convert1) while (nblock--)
	{
	    /* CHECKED */
	    t_int i = ((t_int)*in++ << shift);
	    *out++ = (t_float)i;
	}
	else while (nblock--)
	{
	    /* CHECKED */
	    t_int i = (*(t_int *)(t_float *)in++ << shift);
	    *out++ = *(t_float *)&i;
	}
    }
    else if (x->x_rshift)
    {
	unsigned int shift = x->x_rshift;
	if (x->x_convert1) while (nblock--)
	{
	    /* CHECKME */
	    t_int i = ((t_int)*in++ >> shift);
	    *out++ = (t_float)i;
	}
	else while (nblock--)
	{
	    /* CHECKME */
	    t_int i = (*(t_int *)(t_float *)in++ >> shift);
	    *out++ = *(t_float *)&i;
	}
    }
    else if (x->x_lover)
	while (nblock--) *out++ = 0;  /* CHECKED both modes */
    else
	while (nblock--) *out++ = *in++;  /* CHECKED both modes */
    return (w + 5);
}

static void bitshift_dsp(t_bitshift *x, t_signal **sp)
{
    dsp_add(bitshift_perform, 4, x, sp[0]->s_n, sp[0]->s_vec, sp[1]->s_vec);
}

static void bitshift_mode(t_bitshift *x, t_floatarg f)
{
    int i = (int)f;
    x->x_convert1 = (i > 0);  /* CHECKED */
}

static void bitshift_shift(t_bitshift *x, t_floatarg f)
{
    int i = (int)f;
    int nbits = sizeof(t_int) * 8;
    x->x_lshift = x->x_rshift = 0;
    x->x_lover = 0;
    if (i > 0)
    {
#ifdef BITSHIFT_DEBUG
	loudbug_post("%.8x << %d == %.8x, %.8x << %d == %.8x",
		     1, i, 1 << i, -1, i, -1 << i);
#endif
	if (i < nbits)
	    x->x_lshift = i;
	else
	    x->x_lover = 1;
    }
    else if (i < 0)
    {
#ifdef BITSHIFT_DEBUG
	loudbug_post("%.8x >> %d == %.8x, %.8x >> %d == %.8x",
		     0x7fffffff, -i, 0x7fffffff >> -i, -1, -i, -1 >> -i);
#endif
	x->x_rshift = (i <= -nbits ? nbits - 1 : -i);
    }
}

static void *bitshift_new(t_floatarg f1, t_floatarg f2)
{
    t_bitshift *x = (t_bitshift *)pd_new(bitshift_class);
    outlet_new((t_object *)x, &s_signal);
    bitshift_shift(x, f1);
    bitshift_mode(x, f2);
    return (x);
}

void bitshift_tilde_setup(void)
{
    bitshift_class = class_new(gensym("bitshift~"),
			       (t_newmethod)bitshift_new, 0,
			       sizeof(t_bitshift), 0,
			       A_DEFFLOAT, A_DEFFLOAT, 0);
    sic_setup(bitshift_class, bitshift_dsp, SIC_FLOATTOSIGNAL);
    class_addmethod(bitshift_class, (t_method)bitshift_mode,
		    gensym("mode"), A_FLOAT, 0);
    class_addmethod(bitshift_class, (t_method)bitshift_shift,
		    gensym("shift"), A_FLOAT, 0);
}
