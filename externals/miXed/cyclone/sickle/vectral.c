/* Copyright (c) 2002-2003 krzYszcz and others.
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

#include "m_pd.h"
#include "sickle/sic.h"

#define VECTRAL_DEFSIZE  512

struct _vectral;
typedef void (*t_vectral_perform)(struct _vectral *, int,
				  t_float *, t_float *, t_float *, t_float *);

typedef struct _vectral
{
    t_sic              x_sic;
    t_vectral_perform  x_perform;
    int                x_bufsize;
    t_float           *x_buffer;
    t_float           *x_lastframe;
    /* rampsmooth and slide state */
    double             x_upcoef;
    double             x_downcoef;
    /* deltaclip state */
    float              x_lo;
    float              x_hi;
} t_vectral;

static t_class *vectral_class;

/* LATER after any modification make sure about syncing other variants
   of perform routine to the bypassing version */
/* this is:  for i in [0..nblock) buf[in2[i]] = in3[i], out[i] = buf[in1[i]] */
static void vectral_perform_bypass(t_vectral *x, int nblock,
				   t_float *in1, t_float *in2, t_float *in3,
				   t_float *out)
{
    t_float *buf = x->x_buffer;
    int bufsize = x->x_bufsize;
    t_float *last = x->x_lastframe;
    int blocksize = nblock;
    while (nblock--)
    {
	int indx = (int)*in2++;
	/* CHECKED buffer not zeroed out (the buffer's garbage remains) */
	if (indx >= 0 && indx < bufsize)
	    buf[indx] = *in3;
	in3++;
    }
    while (blocksize--)
    {
	int ondx = (int)*in1++;
	if (ondx >= 0 && ondx < bufsize)
	    *out++ = *last++ = buf[ondx];
	else
	    /* CHECKED garbage in the output vector is cleared */
	    *out++ = *last++ = 0.;
    }
}

/* this one is used for rampsmooth mode as well (see rampsmooth.c)
   LATER recheck */
static void vectral_perform_slide(t_vectral *x, int nblock,
				  t_float *in1, t_float *in2, t_float *in3,
				  t_float *out)
{
    t_float *buf = x->x_buffer;
    int bufsize = x->x_bufsize;
    double upcoef = x->x_upcoef;
    double downcoef = x->x_downcoef;
    t_float *last = x->x_lastframe;
    int blocksize = nblock;
    while (nblock--)
    {
	int indx = (int)*in2++;
	if (indx >= 0 && indx < bufsize)
	    buf[indx] = *in3;
	in3++;
    }
    while (blocksize--)
    {
	int ondx = (int)*in1++;
	if (ondx >= 0 && ondx < bufsize)
	{
	    /* CHECKME what is smoothed, and FIXME */
	    float delta = buf[ondx] - *last;
	    *out++ =
		(*last++ += (delta > 0 ? delta * upcoef : delta * downcoef));
	}
	else *out++ = *last++ = 0.;
    }
}

static void vectral_perform_clip(t_vectral *x, int nblock,
				 t_float *in1, t_float *in2, t_float *in3,
				 t_float *out)
{
    t_float *buf = x->x_buffer;
    int bufsize = x->x_bufsize;
    float lo = x->x_lo;
    float hi = x->x_hi;
    t_float *last = x->x_lastframe;
    int blocksize = nblock;
    while (nblock--)
    {
	int indx = (int)*in2++;
	if (indx >= 0 && indx < bufsize)
	    buf[indx] = *in3;
	in3++;
    }
    while (blocksize--)
    {
	int ondx = (int)*in1++;
	if (ondx >= 0 && ondx < bufsize)
	{
	    /* CHECKME what is smoothed, and FIXME */
	    float delta = buf[ondx] - *last;
	    if (delta < lo)
		*out++ = (*last++ += lo);
	    else if (delta > hi)
		*out++ = (*last++ += hi);
	    else
		*out++ = *last++ = buf[ondx];
	}
	else *out++ = *last++ = 0.;
    }
}

static t_int *vectral_perform(t_int *w)
{
    t_vectral *x = (t_vectral *)(w[1]);
    (*x->x_perform)(x, (int)(w[2]), (t_float *)(w[3]), (t_float *)(w[4]),
		    (t_float *)(w[5]), (t_float *)(w[6]));
    return (w + 7);
}

static void vectral_dsp(t_vectral *x, t_signal **sp)
{
    int nblock = sp[0]->s_n;
    if (nblock > x->x_bufsize)
	nblock = x->x_bufsize;  /* CHECKME */
    dsp_add(vectral_perform, 6, x, nblock,
	    sp[0]->s_vec, sp[1]->s_vec, sp[2]->s_vec, sp[3]->s_vec);
}

static void vectral_rampsmooth(t_vectral *x, t_symbol *s, int ac, t_atom *av)
{
    if (ac && av->a_type == A_FLOAT)
    {
	int i;
	x->x_upcoef = ((i = (int)av->a_w.w_float) > 1 ? 1. / (double)i : 1.);
	ac--; av++;
	if (ac && av->a_type == A_FLOAT)
	    x->x_downcoef =
		((i = (int)av->a_w.w_float) > 1 ? 1. / (double)i : 1.);
	else
	    x->x_downcoef = 1.;  /* CHECKED */
	x->x_perform = vectral_perform_slide;  /* see above */
    }
    else x->x_perform = vectral_perform_bypass;  /* CHECKED */
}

static void vectral_slide(t_vectral *x, t_symbol *s, int ac, t_atom *av)
{
    if (ac && av->a_type == A_FLOAT)
    {
	double d;
	x->x_upcoef = ((d = av->a_w.w_float) > 1. ? 1. / d : 1.);
	ac--; av++;
	if (ac && av->a_type == A_FLOAT)
	    x->x_downcoef = ((d = av->a_w.w_float) > 1. ? 1. / d : 1.);
	else
	    x->x_downcoef = 1.;  /* CHECKED */
	x->x_perform = vectral_perform_slide;
    }
    else x->x_perform = vectral_perform_bypass;  /* CHECKED */
}

/* CHECKED 'deltaclip <hi> <lo>' (deltaclip~'s args are swapped) */
static void vectral_deltaclip(t_vectral *x, t_symbol *s, int ac, t_atom *av)
{
    if (ac && av->a_type == A_FLOAT)
    {
	x->x_hi = av->a_w.w_float;
	ac--; av++;
	if (ac && av->a_type == A_FLOAT)
	    x->x_lo = av->a_w.w_float;
	else
	    x->x_lo = 0.;  /* CHECKED */
    }
    else x->x_lo = x->x_hi = 0.;  /* CHECKED */
    x->x_perform = vectral_perform_clip;
}

static void vectral_free(t_vectral *x)
{
    if (x->x_buffer)
	freebytes(x->x_buffer, x->x_bufsize * sizeof(*x->x_buffer));
    if (x->x_lastframe)
	freebytes(x->x_lastframe, x->x_bufsize * sizeof(*x->x_lastframe));
}

static void *vectral_new(t_floatarg f)
{
    t_vectral *x = (t_vectral *)pd_new(vectral_class);
    int i = (int)f;
    x->x_bufsize = (i > 0 ? i : VECTRAL_DEFSIZE);
    if (!(x->x_buffer = getbytes(x->x_bufsize * sizeof(*x->x_buffer))))
	goto failure;
    if (!(x->x_lastframe = getbytes(x->x_bufsize * sizeof(*x->x_lastframe))))
	goto failure;
    x->x_perform = vectral_perform_bypass;
    inlet_new((t_object *)x, (t_pd *)x, &s_signal, &s_signal);
    inlet_new((t_object *)x, (t_pd *)x, &s_signal, &s_signal);
    outlet_new((t_object *)x, &s_signal);
    return (x);
failure:
    pd_free((t_pd *)x);
    return (0);
}

void vectral_tilde_setup(void)
{
    vectral_class = class_new(gensym("vectral~"),
			      (t_newmethod)vectral_new,
			      (t_method)vectral_free,
			      sizeof(t_vectral), 0, A_DEFFLOAT, 0);
    sic_setup(vectral_class, vectral_dsp, SIC_FLOATTOSIGNAL);
    class_addmethod(vectral_class, (t_method)vectral_rampsmooth,
		    gensym("rampsmooth"), A_GIMME, 0);
    class_addmethod(vectral_class, (t_method)vectral_slide,
		    gensym("slide"), A_GIMME, 0);
    class_addmethod(vectral_class, (t_method)vectral_deltaclip,
		    gensym("deltaclip"), A_GIMME, 0);
}
