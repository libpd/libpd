/* Copyright (c) 2002-2003 krzYszcz and others.
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

/* LATER reconsider making float/int conversions
   more compatible (and less useful). */

#include "m_pd.h"
#include "sickle/sic.h"
#include "sickle/arsic.h"

typedef t_arsic t_lookup;
static t_class *lookup_class;

#define LOOKUP_DEFSIZE  512

static void lookup_set(t_lookup *x, t_symbol *s)
{
    arsic_setarray((t_arsic *)x, s, 1);
}

static t_int *lookup_perform(t_int *w)
{
    t_arsic *sic = (t_arsic *)(w[1]);
    int nblock = (int)(w[2]);
    t_float *out = (t_float *)(w[6]);
    if (sic->s_playable)
    {	
	t_float *xin = (t_float *)(w[3]);
	t_float *oin = (t_float *)(w[4]);
	t_float *sin = (t_float *)(w[5]);
	int vecsize = sic->s_vecsize;
	t_float *vec = sic->s_vectors[0];  /* playable implies nonzero (mono) */
	while (nblock--)
	{
	    float off = *oin++;  /* msp: converted to int (if not a signal) */
	    int siz = (int)*sin++ - 1;  /* msp: converted to int (signal too) */
	    float pos = (siz > 0 ? off + siz * (*xin++ + 1.0) * 0.5 : off);
	    int ndx = (int)pos;
	    int ndx1 = ndx + 1;
	    if (ndx1 > 0 && ndx1 < vecsize)
	    {
		float val = vec[ndx];
		*out++ = val + (vec[ndx1] - val) * (pos - ndx);
	    }
	    /* CHECKED: */
	    else if (ndx1 == 0) *out++ = *vec * (pos + 1.0);
	    else if (ndx1 == vecsize) *out++ = vec[ndx] * (ndx1 - pos);
	    else *out++ = 0;
	}
    }
    else while (nblock--) *out++ = 0;
    return (w + 7);
}

static void lookup_dsp(t_lookup *x, t_signal **sp)
{
    arsic_dsp((t_arsic *)x, sp, lookup_perform, 1);
}

static void lookup_free(t_lookup *x)
{
    arsic_free((t_arsic *)x);
}

static void *lookup_new(t_symbol *s, t_floatarg f1, t_floatarg f2)
{
    /* CHECKED: lookup~ always uses the first channel in a multi-channel buffer~
       (as the refman says). */
    /* three auxiliary signals: amplitude, offset and size inputs */
    t_lookup *x = (t_lookup *)arsic_new(lookup_class, s, 0, 0, 3);
    if (x)
    {
	arsic_setminsize((t_arsic *)x, 2);
	if (f1 < 0) f1 = 0;
	if (f2 <= 0) f2 = LOOKUP_DEFSIZE;
	sic_newinlet((t_sic *)x, f1);
	sic_newinlet((t_sic *)x, f2);
	outlet_new((t_object *)x, &s_signal);
    }
    return (x);
}

void lookup_tilde_setup(void)
{
    lookup_class = class_new(gensym("lookup~"),
			     (t_newmethod)lookup_new,
			     (t_method)lookup_free,
			     sizeof(t_lookup), 0,
			     A_DEFSYM, A_DEFFLOAT, A_DEFFLOAT, 0);
    arsic_setup(lookup_class, lookup_dsp, SIC_FLOATTOSIGNAL);
    class_addmethod(lookup_class, (t_method)lookup_set,
		    gensym("set"), A_SYMBOL, 0);
}
