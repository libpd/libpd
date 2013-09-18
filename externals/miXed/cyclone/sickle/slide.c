/* Copyright (c) 2002-2003 krzYszcz and others.
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

#include "m_pd.h"
#include "shared.h"
#include "sickle/sic.h"

#define SLIDE_DEFUP  1.
#define SLIDE_DEFDN  1.

typedef struct _slide
{
    t_sic    x_sic;
    t_float  x_last;
} t_slide;

static t_class *slide_class;

static t_int *slide_perform(t_int *w)
{
    t_slide *x = (t_slide *)(w[1]);
    int nblock = (int)(w[2]);
    t_float *in1 = (t_float *)(w[3]);
    t_float *in2 = (t_float *)(w[4]);
    t_float *in3 = (t_float *)(w[5]);
    t_float *out = (t_float *)(w[6]);
    t_float last = x->x_last;
    while (nblock--)
    {
    	float f = *in1++;
	if (f > last)
	{
	    float up = *in2++;
	    if (up > 1.)  /* CHECKED */
		last += (f - last) / up;
	    else
		last = f;
	    in3++;
	}
	else if (f < last)
	{
	    float dn = *in3++;
	    if (dn > 1.)  /* CHECKED */
		last += (f - last) / dn;
	    else
		last = f;
	    in2++;
	}
	*out++ = last;
    }
    x->x_last = (PD_BIGORSMALL(last) ? 0. : last);
    return (w + 7);
}

static void slide_dsp(t_slide *x, t_signal **sp)
{
    dsp_add(slide_perform, 6, x, sp[0]->s_n,
	    sp[0]->s_vec, sp[1]->s_vec, sp[2]->s_vec, sp[3]->s_vec);
}

static void *slide_new(t_symbol *s, int ac, t_atom *av)
{
    t_slide *x = (t_slide *)pd_new(slide_class);
    sic_inlet((t_sic *)x, 1, SLIDE_DEFUP, 0, ac, av);
    sic_inlet((t_sic *)x, 2, SLIDE_DEFDN, 1, ac, av);
    outlet_new((t_object *)x, &s_signal);
    x->x_last = 0;
    return (x);
}

void slide_tilde_setup(void)
{
    slide_class = class_new(gensym("slide~"),
			    (t_newmethod)slide_new, 0,
			    sizeof(t_slide), 0, A_GIMME, 0);
    sic_setup(slide_class, slide_dsp, SIC_FLOATTOSIGNAL);
}
