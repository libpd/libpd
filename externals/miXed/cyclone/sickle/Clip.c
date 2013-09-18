/* Copyright (c) 2002-2003 krzYszcz and others.
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

/* Clip~ substitution is needed to handle signal input for lo and hi */

#include "m_pd.h"
#include "sickle/sic.h"

#define CLIP_DEFLO  0.
#define CLIP_DEFHI  0.

typedef t_sic t_clip;
static t_class *clip_class;

static t_int *Clip_perform(t_int *w)
{
    int nblock = (int)(w[1]);
    t_float *in1 = (t_float *)(w[2]);
    t_float *in2 = (t_float *)(w[3]);
    t_float *in3 = (t_float *)(w[4]);
    t_float *out = (t_float *)(w[5]);
    while (nblock--)
    {
    	float f = *in1++;
    	float lo = *in2++;
    	float hi = *in3++;
    	if (f < lo)
	    *out++ = lo;
    	else if (f > hi)
	    *out++ = hi;
	else
	    *out++ = f;
    }
    return (w + 6);
}

static void clip_dsp(t_clip *x, t_signal **sp)
{
    dsp_add(Clip_perform, 5, sp[0]->s_n,
	    sp[0]->s_vec, sp[1]->s_vec, sp[2]->s_vec, sp[3]->s_vec);
}

static void *clip_new(t_symbol *s, int ac, t_atom *av)
{
    t_clip *x = (t_clip *)pd_new(clip_class);
    sic_inlet((t_sic *)x, 1, CLIP_DEFLO, 0, ac, av);
    sic_inlet((t_sic *)x, 2, CLIP_DEFHI, 1, ac, av);
    outlet_new((t_object *)x, &s_signal);
    return (x);
}

void Clip_tilde_setup(void)
{
    clip_class = class_new(gensym("Clip~"),
			   (t_newmethod)clip_new, 0,
			   sizeof(t_clip), 0, A_GIMME, 0);
    class_addcreator((t_newmethod)clip_new, gensym("clip~"), A_GIMME, 0);
    class_addcreator((t_newmethod)clip_new, gensym("cyclone/clip~"), A_GIMME, 0);
    sic_setup(clip_class, clip_dsp, SIC_FLOATTOSIGNAL);
}

void clip_tilde_setup(void)
{
    Clip_tilde_setup();
}
