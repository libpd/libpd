/* Copyright (c) 1997-2003 Miller Puckette, krzYszcz, and others.
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

/* This is a compilation of phasor~ and noise~ code from d_osc.c. */

#include "m_pd.h"
#include "shared.h"
#include "sickle/sic.h"

typedef struct _rand
{
    t_sic    x_sic;
    double   x_lastphase;
    double   x_nextphase;
    float    x_rcpsr;
    int      x_state;
    float    x_target;
    float    x_scaling;  /* LATER use phase increment */
} t_rand;

static t_class *rand_class;

static t_int *rand_perform(t_int *w)
{
    t_rand *x = (t_rand *)(w[1]);
    int nblock = (int)(w[2]);
    t_float *rin = (t_float *)(w[3]);
    t_float *out = (t_float *)(w[4]);
    double lastph = x->x_lastphase;
    double ph = x->x_nextphase;
    double tfph = ph + SHARED_UNITBIT32;
    t_shared_wrappy wrappy;
    int32 normhipart;
    float rcpsr = x->x_rcpsr;
    float target = x->x_target;
    float scaling = x->x_scaling;

    wrappy.w_d = SHARED_UNITBIT32;
    normhipart = wrappy.w_i[SHARED_HIOFFSET];

    while (nblock--)
    {
	float rate = *rin++;
	if (ph > lastph)
	{
	    int state = x->x_state;
	    float newtarget = ((float)((state & 0x7fffffff) - 0x40000000))
		* (float)(1.0 / 0x40000000);
	    x->x_state = state * 435898247 + 382842987;
	    x->x_scaling = scaling = target - newtarget;
	    x->x_target = target = newtarget;
	}
	*out++ = ph * scaling + target;
	lastph = ph;
	if (rate > 0) rate = -rate;
    	tfph += rate * rcpsr;
	wrappy.w_d = tfph;
    	wrappy.w_i[SHARED_HIOFFSET] = normhipart;
	ph = wrappy.w_d - SHARED_UNITBIT32;
    }
    x->x_lastphase = lastph;
    x->x_nextphase = ph;
    return (w + 5);
}

static void rand_dsp(t_rand *x, t_signal **sp)
{
    x->x_rcpsr = 1. / sp[0]->s_sr;
    dsp_add(rand_perform, 4, x, sp[0]->s_n, sp[0]->s_vec, sp[1]->s_vec);
}

static void *rand_new(t_floatarg f)
{
    t_rand *x = (t_rand *)pd_new(rand_class);
    /* borrowed from d_osc.c, LATER rethink */
    static int init = 307;
    x->x_state = (init *= 1319);
    x->x_lastphase = 0.;
    x->x_nextphase = 1.;  /* start from 0, force retargetting */
    x->x_target = x->x_scaling = 0;
    sic_newinlet((t_sic *)x, (f > 0. ? -f : 0.));
    outlet_new((t_object *)x, &s_signal);
    return (x);
}

void rand_tilde_setup(void)
{
    rand_class = class_new(gensym("rand~"),
			   (t_newmethod)rand_new, 0,
			   sizeof(t_rand), 0,
			   A_DEFFLOAT, 0);
    sic_setup(rand_class, rand_dsp, SIC_FLOATTOSIGNAL);
}
