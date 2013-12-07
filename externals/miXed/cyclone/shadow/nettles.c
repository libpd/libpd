/* Copyright (c) 2003 krzYszcz and others.
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

#include <math.h>
#include "m_pd.h"
#include "shared.h"
#include "sickle/sic.h"
#include "shadow.h"

#if defined(NT) || defined(MACOSX)
/* cf pd/src/x_arithmetic.c */
#define fmodf  fmod
#endif

/* Two remaining control binops have their inputs reversed.
   LATER think about float-to-int conversion -- there is no point in making
   the two below compatible, while all the others are not compatible... */

/* CHECKED left inlet causes output (refman's error -- a total rubbish) */

typedef struct _rbinop
{
    t_object  x_ob;
    t_float   x_f1;  /* left inlet value */
    t_float   x_f2;
} t_rbinop;

static t_class *rminus_class;

static void rminus_bang(t_rbinop *x)
{
    outlet_float(((t_object *)x)->ob_outlet, x->x_f2 - x->x_f1);
}

static void rminus_float(t_rbinop *x, t_float f)
{
    outlet_float(((t_object *)x)->ob_outlet, x->x_f2 - (x->x_f1 = f));
}

static void *rminus_new(t_floatarg f)
{
    t_rbinop *x = (t_rbinop *)pd_new(rminus_class);
    floatinlet_new((t_object *)x, &x->x_f2);  /* CHECKED */
    outlet_new((t_object *)x, &s_float);
    x->x_f1 = 0;
    x->x_f2 = f;  /* CHECKED */
    return (x);
}

static t_class *rdiv_class;

static void rdiv_bang(t_rbinop *x)
{
    if (x->x_f1 != 0.)
	outlet_float(((t_object *)x)->ob_outlet, x->x_f2 / x->x_f1);
    else
	/* CHECKED int mode: nonnegative/0 == 0, negative/0 == -1,
	   float mode: positive/0 == INT_MAX, nonpositive/0 == INT_MIN
	   LATER rethink -- why is it INT_MAX, not FLT_MAX? */
	outlet_float(((t_object *)x)->ob_outlet,
		     (x->x_f2 > 0 ? SHARED_INT_MAX : SHARED_INT_MIN));
}

static void rdiv_float(t_rbinop *x, t_float f)
{
    x->x_f1 = f;
    rdiv_bang(x);
}

static void *rdiv_new(t_floatarg f)
{
    t_rbinop *x = (t_rbinop *)pd_new(rdiv_class);
    floatinlet_new((t_object *)x, &x->x_f2);
    outlet_new((t_object *)x, &s_float);
    x->x_f1 = 0;
    x->x_f2 = f;  /* CHECKED (refman's error) */
    return (x);
}

/* The implementation of signal relational operators below has been tuned
   somewhat, mostly in order to get rid of costly int->float conversions.
   Loops are not hand-unrolled, because these have proven to be slower
   in all the tests performed so far.  LATER find a good soul willing to
   make a serious profiling research... */

typedef struct _sigeq
{
    t_sic  x_sic;
    int    x_algo;
} t_sigeq;

static t_class *sigeq_class;

static t_int *sigeq_perform0(t_int *w)
{
    int nblock = (int)(w[1]);
    t_float *in1 = (t_float *)(w[2]);
    t_float *in2 = (t_float *)(w[3]);
    t_float *out = (t_float *)(w[4]);
    t_shared_floatint fi;
#ifdef NETTLES_SAFE
    int32 truebits;
    fi.fi_f = 1.;
    truebits = fi.fi_i;
#endif
    while (nblock--)
    {
#ifdef NETTLES_SAFE
	fi.fi_i = ~((*in1++ == *in2++) - 1) & truebits;
#else
	fi.fi_i = ~((*in1++ == *in2++) - 1) & SHARED_TRUEBITS;
#endif
	*out++ = fi.fi_f;
    }
    return (w + 5);
}

static t_int *sigeq_perform1(t_int *w)
{
    int nblock = (int)(w[1]);
    t_float *in1 = (t_float *)(w[2]);
    t_float *in2 = (t_float *)(w[3]);
    t_float *out = (t_float *)(w[4]);
    while (nblock--) *out++ = (*in1++ == *in2++);
    return (w + 5);
}

static t_int *sigeq_perform2(t_int *w)
{
    int nblock = (int)(w[1]);
    t_float *in1 = (t_float *)(w[2]);
    t_float *in2 = (t_float *)(w[3]);
    t_float *out = (t_float *)(w[4]);
    for (; nblock; nblock -= 8, in1 += 8, in2 += 8, out += 8)
    {
	float f0 = in1[0], f1 = in1[1], f2 = in1[2], f3 = in1[3];
	float f4 = in1[4], f5 = in1[5], f6 = in1[6], f7 = in1[7];
	float g0 = in2[0], g1 = in2[1], g2 = in2[2], g3 = in2[3];
	float g4 = in2[4], g5 = in2[5], g6 = in2[6], g7 = in2[7];
	out[0] = f0 == g0; out[1] = f1 == g1;
	out[2] = f2 == g2; out[3] = f3 == g3;
	out[4] = f4 == g4; out[5] = f5 == g5;
	out[6] = f6 == g6; out[7] = f7 == g7;
    }
    return (w + 5);
}

static void sigeq_dsp(t_sigeq *x, t_signal **sp)
{
    switch (x->x_algo)
    {
    case 1:
	dsp_add(sigeq_perform1, 4, sp[0]->s_n,
		sp[0]->s_vec, sp[1]->s_vec, sp[2]->s_vec);
	break;
    case 2:
	dsp_add(sigeq_perform2, 4, sp[0]->s_n,
		sp[0]->s_vec, sp[1]->s_vec, sp[2]->s_vec);
	break;
    default:
	dsp_add(sigeq_perform0, 4, sp[0]->s_n,
		sp[0]->s_vec, sp[1]->s_vec, sp[2]->s_vec);
    }
}

static void sigeq__algo(t_sigeq *x, t_floatarg f)
{
    x->x_algo = f;
}

static void *sigeq_new(t_symbol *s, int ac, t_atom *av)
{
    t_sigeq *x = (t_sigeq *)pd_new(sigeq_class);
    if (s == gensym("_==1~"))
	x->x_algo = 1;
    else if (s == gensym("_==2~"))
	x->x_algo = 2;
    else
	x->x_algo = 0;
    sic_inlet((t_sic *)x, 1, 0, 0, ac, av);
    outlet_new((t_object *)x, &s_signal);
    return (x);
}

typedef t_sic t_signeq;
static t_class *signeq_class;

static t_int *signeq_perform(t_int *w)
{
    int nblock = (int)(w[1]);
    t_float *in1 = (t_float *)(w[2]);
    t_float *in2 = (t_float *)(w[3]);
    t_float *out = (t_float *)(w[4]);
    t_shared_floatint fi;
    while (nblock--)
    {
	fi.fi_i = ~((*in1++ != *in2++) - 1) & SHARED_TRUEBITS;
	*out++ = fi.fi_f;
    }
    return (w + 5);
}

static void signeq_dsp(t_signeq *x, t_signal **sp)
{
    dsp_add(signeq_perform, 4, sp[0]->s_n,
	    sp[0]->s_vec, sp[1]->s_vec, sp[2]->s_vec);
}

static void *signeq_new(t_symbol *s, int ac, t_atom *av)
{
    t_signeq *x = (t_signeq *)pd_new(signeq_class);
    sic_inlet((t_sic *)x, 1, 0, 0, ac, av);
    outlet_new((t_object *)x, &s_signal);
    return (x);
}

typedef t_sic t_siglt;
static t_class *siglt_class;

static t_int *siglt_perform(t_int *w)
{
    int nblock = (int)(w[1]);
    t_float *in1 = (t_float *)(w[2]);
    t_float *in2 = (t_float *)(w[3]);
    t_float *out = (t_float *)(w[4]);
    t_shared_floatint fi;
    while (nblock--)
    {
	fi.fi_i = ~((*in1++ < *in2++) - 1) & SHARED_TRUEBITS;
	*out++ = fi.fi_f;
    }
    return (w + 5);
}

static void siglt_dsp(t_siglt *x, t_signal **sp)
{
    dsp_add(siglt_perform, 4, sp[0]->s_n,
	    sp[0]->s_vec, sp[1]->s_vec, sp[2]->s_vec);
}

static void *siglt_new(t_symbol *s, int ac, t_atom *av)
{
    t_siglt *x = (t_siglt *)pd_new(siglt_class);
    sic_inlet((t_sic *)x, 1, 0, 0, ac, av);
    outlet_new((t_object *)x, &s_signal);
    return (x);
}

typedef t_sic t_siggt;
static t_class *siggt_class;

static t_int *siggt_perform(t_int *w)
{
    int nblock = (int)(w[1]);
    t_float *in1 = (t_float *)(w[2]);
    t_float *in2 = (t_float *)(w[3]);
    t_float *out = (t_float *)(w[4]);
    t_shared_floatint fi;
    while (nblock--)
    {
	fi.fi_i = ~((*in1++ > *in2++) - 1) & SHARED_TRUEBITS;
	*out++ = fi.fi_f;
    }
    return (w + 5);
}

static void siggt_dsp(t_siggt *x, t_signal **sp)
{
    dsp_add(siggt_perform, 4, sp[0]->s_n,
	    sp[0]->s_vec, sp[1]->s_vec, sp[2]->s_vec);
}

static void *siggt_new(t_symbol *s, int ac, t_atom *av)
{
    t_siggt *x = (t_siggt *)pd_new(siggt_class);
    sic_inlet((t_sic *)x, 1, 0, 0, ac, av);
    outlet_new((t_object *)x, &s_signal);
    return (x);
}

typedef t_sic t_sigleq;
static t_class *sigleq_class;

static t_int *sigleq_perform(t_int *w)
{
    int nblock = (int)(w[1]);
    t_float *in1 = (t_float *)(w[2]);
    t_float *in2 = (t_float *)(w[3]);
    t_float *out = (t_float *)(w[4]);
    t_shared_floatint fi;
    while (nblock--)
    {
	fi.fi_i = ~((*in1++ <= *in2++) - 1) & SHARED_TRUEBITS;
	*out++ = fi.fi_f;
    }
    return (w + 5);
}

static void sigleq_dsp(t_sigleq *x, t_signal **sp)
{
    dsp_add(sigleq_perform, 4, sp[0]->s_n,
	    sp[0]->s_vec, sp[1]->s_vec, sp[2]->s_vec);
}

static void *sigleq_new(t_symbol *s, int ac, t_atom *av)
{
    t_sigleq *x = (t_sigleq *)pd_new(sigleq_class);
    sic_inlet((t_sic *)x, 1, 0, 0, ac, av);
    outlet_new((t_object *)x, &s_signal);
    return (x);
}

typedef t_sic t_siggeq;
static t_class *siggeq_class;

static t_int *siggeq_perform(t_int *w)
{
    int nblock = (int)(w[1]);
    t_float *in1 = (t_float *)(w[2]);
    t_float *in2 = (t_float *)(w[3]);
    t_float *out = (t_float *)(w[4]);
    t_shared_floatint fi;
    while (nblock--)
    {
	fi.fi_i = ~((*in1++ >= *in2++) - 1) & SHARED_TRUEBITS;
	*out++ = fi.fi_f;
    }
    return (w + 5);
}

static void siggeq_dsp(t_siggeq *x, t_signal **sp)
{
    dsp_add(siggeq_perform, 4, sp[0]->s_n,
	    sp[0]->s_vec, sp[1]->s_vec, sp[2]->s_vec);
}

static void *siggeq_new(t_symbol *s, int ac, t_atom *av)
{
    t_siggeq *x = (t_siggeq *)pd_new(siggeq_class);
    sic_inlet((t_sic *)x, 1, 0, 0, ac, av);
    outlet_new((t_object *)x, &s_signal);
    return (x);
}

typedef t_sic t_sigrminus;
static t_class *sigrminus_class;

static t_int *sigrminus_perform(t_int *w)
{
    int nblock = (int)(w[1]);
    t_float *in1 = (t_float *)(w[2]);
    t_float *in2 = (t_float *)(w[3]);
    t_float *out = (t_float *)(w[4]);
    while (nblock--) *out++ = *in2++ - *in1++;
    return (w + 5);
}

static void sigrminus_dsp(t_sigrminus *x, t_signal **sp)
{
    dsp_add(sigrminus_perform, 4, sp[0]->s_n,
	    sp[0]->s_vec, sp[1]->s_vec, sp[2]->s_vec);
}

static void *sigrminus_new(t_symbol *s, int ac, t_atom *av)
{
    t_sigrminus *x = (t_sigrminus *)pd_new(sigrminus_class);
    sic_inlet((t_sic *)x, 1, 0, 0, ac, av);
    outlet_new((t_object *)x, &s_signal);
    return (x);
}

typedef t_sic t_sigrover;
static t_class *sigrover_class;

static t_int *sigrover_perform(t_int *w)
{
    int nblock = (int)(w[1]);
    t_float *in1 = (t_float *)(w[2]);
    t_float *in2 = (t_float *)(w[3]);
    t_float *out = (t_float *)(w[4]);
    while (nblock--)
    {
	t_float f1 = *in1++;
	/* CHECKED incompatible: c74 outputs NaNs.
	   The line below is consistent with Pd's /~, LATER rethink. */
	/* LATER multiply by reciprocal if in1 has no signal feeders */
	*out++ = (f1 == 0. ? 0. : *in2++ / f1);
    }
    return (w + 5);
}

static void sigrover_dsp(t_sigrover *x, t_signal **sp)
{
    dsp_add(sigrover_perform, 4, sp[0]->s_n,
	    sp[0]->s_vec, sp[1]->s_vec, sp[2]->s_vec);
}

static void *sigrover_new(t_symbol *s, int ac, t_atom *av)
{
    t_sigrover *x = (t_sigrover *)pd_new(sigrover_class);
    /* CHECKED default 0 (refman's error), LATER rethink */
    sic_inlet((t_sic *)x, 1, 0, 0, ac, av);
    outlet_new((t_object *)x, &s_signal);
    return (x);
}

typedef t_sic t_sigmod;
static t_class *sigmod_class;

static t_int *sigmod_perform(t_int *w)
{
    int nblock = (int)(w[1]);
    t_float *in1 = (t_float *)(w[2]);
    t_float *in2 = (t_float *)(w[3]);
    t_float *out = (t_float *)(w[4]);
    while (nblock--)
    {
	t_float f1 = *in1++;
	t_float f2 = *in2++;
	/* LATER think about using ieee-754 normalization tricks */
	*out++ = (f2 == 0. ? 0.  /* CHECKED */
		  : fmod(f1, f2));
    }
    return (w + 5);
}

static void sigmod_dsp(t_sigmod *x, t_signal **sp)
{
    dsp_add(sigmod_perform, 4, sp[0]->s_n,
	    sp[0]->s_vec, sp[1]->s_vec, sp[2]->s_vec);
}

static void *sigmod_new(t_symbol *s, int ac, t_atom *av)
{
    t_sigmod *x = (t_sigmod *)pd_new(sigmod_class);
    /* CHECKED default 0 (refman's error), LATER rethink */
    sic_inlet((t_sic *)x, 1, 0, 0, ac, av);
    outlet_new((t_object *)x, &s_signal);
    return (x);
}

typedef struct _sigaccum
{
    t_sic    x_sic;
    t_float  x_sum;
} t_sigaccum;

static t_class *sigaccum_class;

static t_int *sigaccum_perform(t_int *w)
{
    t_sigaccum *x = (t_sigaccum *)(w[1]);
    int nblock = (int)(w[2]);
    t_float *in = (t_float *)(w[3]);
    t_float *out = (t_float *)(w[4]);
    t_float sum = x->x_sum;
    while (nblock--) *out++ = (sum += *in++);
    x->x_sum = sum;
    return (w + 5);
}

static void sigaccum_dsp(t_sigaccum *x, t_signal **sp)
{
    dsp_add(sigaccum_perform, 4, x, sp[0]->s_n, sp[0]->s_vec, sp[1]->s_vec);
}

static void sigaccum_bang(t_sigaccum *x)
{
    x->x_sum = 0;
}

static void sigaccum_set(t_sigaccum *x, t_floatarg f)
{
    x->x_sum = f;
}

static void *sigaccum_new(t_floatarg f)
{
    t_sigaccum *x = (t_sigaccum *)pd_new(sigaccum_class);
    x->x_sum = f;
    outlet_new((t_object *)x, &s_signal);
    return (x);
}

void allnettles_setup(void)
{
    rminus_class = class_new(gensym("!-"),
			     (t_newmethod)rminus_new, 0,
			     sizeof(t_rbinop), 0, A_DEFFLOAT, 0);
    class_addbang(rminus_class, rminus_bang);
    class_addfloat(rminus_class, rminus_float);
    rdiv_class = class_new(gensym("!/"),
			   (t_newmethod)rdiv_new, 0,
			   sizeof(t_rbinop), 0, A_DEFFLOAT, 0);
    class_addbang(rdiv_class, rdiv_bang);
    class_addfloat(rdiv_class, rdiv_float);

    sigeq_class = class_new(gensym("==~"),
			    (t_newmethod)sigeq_new, 0,
			    sizeof(t_sigeq), 0, A_GIMME, 0);
    class_addcreator((t_newmethod)sigeq_new,
		     gensym("_==1~"), A_GIMME, 0);
    class_addcreator((t_newmethod)sigeq_new,
		     gensym("_==2~"), A_GIMME, 0);
    sic_setup(sigeq_class, sigeq_dsp, SIC_FLOATTOSIGNAL);
    class_addmethod(sigeq_class, (t_method)sigeq__algo,
		    gensym("_algo"), A_FLOAT, 0);

    signeq_class = class_new(gensym("!=~"),
			     (t_newmethod)signeq_new, 0,
			     sizeof(t_signeq), 0, A_GIMME, 0);
    sic_setup(signeq_class, signeq_dsp, SIC_FLOATTOSIGNAL);
    siglt_class = class_new(gensym("<~"),
			    (t_newmethod)siglt_new, 0,
			    sizeof(t_siglt), 0, A_GIMME, 0);
    sic_setup(siglt_class, siglt_dsp, SIC_FLOATTOSIGNAL);
    siggt_class = class_new(gensym(">~"),
			    (t_newmethod)siggt_new, 0,
			    sizeof(t_siggt), 0, A_GIMME, 0);
    sic_setup(siggt_class, siggt_dsp, SIC_FLOATTOSIGNAL);
    sigleq_class = class_new(gensym("<=~"),
			     (t_newmethod)sigleq_new, 0,
			     sizeof(t_sigleq), 0, A_GIMME, 0);
    sic_setup(sigleq_class, sigleq_dsp, SIC_FLOATTOSIGNAL);
    siggeq_class = class_new(gensym(">=~"),
			     (t_newmethod)siggeq_new, 0,
			     sizeof(t_siggeq), 0, A_GIMME, 0);
    sic_setup(siggeq_class, siggeq_dsp, SIC_FLOATTOSIGNAL);
    sigrminus_class = class_new(gensym("!-~"),
				(t_newmethod)sigrminus_new, 0,
				sizeof(t_sigrminus), 0, A_GIMME, 0);
    sic_setup(sigrminus_class, sigrminus_dsp, SIC_FLOATTOSIGNAL);
    sigrover_class = class_new(gensym("!/~"),
			       (t_newmethod)sigrover_new, 0,
			       sizeof(t_sigrover), 0, A_GIMME, 0);
    sic_setup(sigrover_class, sigrover_dsp, SIC_FLOATTOSIGNAL);
    sigmod_class = class_new(gensym("%~"),
			     (t_newmethod)sigmod_new, 0,
			     sizeof(t_sigmod), 0, A_GIMME, 0);
    sic_setup(sigmod_class, sigmod_dsp, SIC_FLOATTOSIGNAL);
    sigaccum_class = class_new(gensym("+=~"),
			       (t_newmethod)sigaccum_new, 0,
			       sizeof(t_sigaccum), 0, A_DEFFLOAT, 0);
    sic_setup(sigaccum_class, sigaccum_dsp, SIC_FLOATTOSIGNAL);
    class_addbang(sigaccum_class, sigaccum_bang);
    class_addmethod(sigaccum_class, (t_method)sigaccum_set,
		    gensym("set"), A_FLOAT, 0);
}
