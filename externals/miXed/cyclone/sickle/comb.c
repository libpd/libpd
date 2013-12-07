/* Copyright (c) 2002-2003 krzYszcz and others.
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

#include <string.h>
#include "m_pd.h"
#include "sickle/sic.h"

typedef struct _comb
{
    t_sic     x_sic;
    float     x_sr;
    float     x_ksr;
    t_float  *x_buf;
    int       x_bufsize;   /* as allocated */
    int       x_maxsize;   /* as used */
    float     x_maxdelay;  /* same in ms */
    int       x_phase;     /* writing head */
} t_comb;

static t_class *comb_class;

/* maximum delay defaults to 50 ms (cycling has 10 ms here) */
#define COMB_DEFMAXDELAY  50.0

/* LATER choose the best way.  From msp help patch:
   no clipping is done on a, b, or c coefficient input */
#define COMB_MAXFEEDBACK  0.999

static void comb_clear(t_comb *x)
{
    memset(x->x_buf, 0, x->x_maxsize * sizeof(*x->x_buf));
    x->x_phase = 0;
}

static void comb_resize(t_comb *x, int newsize)
{
    if (newsize > 0 && newsize != x->x_maxsize)
    {
	if (newsize > x->x_bufsize)
	{
	    x->x_buf = resizebytes(x->x_buf,
				   x->x_bufsize * sizeof(*x->x_buf),
				   newsize * sizeof(*x->x_buf));
	    /* LATER test for failure */
	    x->x_bufsize = newsize;
	}
	x->x_maxsize = newsize;
    }
    comb_clear(x);
}

static t_int *comb_perform(t_int *w)
{
    t_comb *x = (t_comb *)(w[1]);
    int nblock = (int)(w[2]);
    t_float *xin = (t_float *)(w[3]);
    t_float *din = (t_float *)(w[4]);
    t_float *ain = (t_float *)(w[5]);
    t_float *bin = (t_float *)(w[6]);
    t_float *cin = (t_float *)(w[7]);
    t_float *out = (t_float *)(w[8]);
    t_float *buf = x->x_buf;
    int maxsize = x->x_maxsize;
    int guardpoint = maxsize - 1;
    float ksr = x->x_ksr;
    int wph = x->x_phase;
    while (nblock--)
    {  /* TDFII scheme is used.  Do not forget, that any signal value
	  read after writing to out has to be saved beforehand. */
	float xn = *xin++;
	float delsize = ksr * *din++;
	float bgain = *bin++;
	float cgain = *cin++;
	float yn = *ain++ * xn;
	float rph;  /* reading head */
	if (cgain < -COMB_MAXFEEDBACK) cgain = -COMB_MAXFEEDBACK;
	else if (cgain > COMB_MAXFEEDBACK) cgain = COMB_MAXFEEDBACK;
	if (delsize > 1.0)
	{
	    int ndx;
	    float val;
	    rph = wph - (delsize > guardpoint ? guardpoint : delsize);
	    if (rph < 0) rph += guardpoint;
	    ndx = (int)rph;
	    val = buf[ndx];
	    /* ``a cheezy linear interpolation'' ala msp,
	       (vd~ uses 4-point interpolation...) */
	    yn += val + (buf[ndx+1] - val) * (rph - ndx);
	}
	*out++ = yn;
	if (wph == guardpoint)
	{
	    buf[wph] = *buf = bgain * xn + cgain * yn;
	    wph = 1;
	}
	else buf[wph++] = bgain * xn + cgain * yn;
    }
    x->x_phase = wph;
    return (w + 9);
}

static void comb_dsp(t_comb *x, t_signal **sp)
{
    float sr = sp[0]->s_sr;
    if (sr != x->x_sr)
    {
	x->x_sr = sr;
	x->x_ksr = sr * 0.001;
	comb_resize(x, x->x_ksr * x->x_maxdelay);
    }
    else comb_clear(x);
    dsp_add(comb_perform, 8, x, sp[0]->s_n,
	    sp[0]->s_vec, sp[1]->s_vec, sp[2]->s_vec,
	    sp[3]->s_vec, sp[4]->s_vec, sp[5]->s_vec);
}

static void *comb_new(t_floatarg f1, t_floatarg f2,
		      t_floatarg f3, t_floatarg f4, t_floatarg f5)
{
    t_comb *x;
    float maxdelay = (f1 > 0 ? f1 : COMB_DEFMAXDELAY);
    float sr = sys_getsr();
    float ksr = sr * 0.001;
    int bufsize = ksr * maxdelay;
    t_float *buf = (t_float *)getbytes(bufsize * sizeof(*buf));
    if (!buf)
	return (0);
    x = (t_comb *)pd_new(comb_class);
    x->x_maxdelay = maxdelay;
    x->x_sr = sr;
    x->x_ksr = ksr;
    x->x_bufsize = x->x_maxsize = bufsize;
    x->x_buf = buf;
    if (f2 < 0) f2 = 0;
    if (f5 < -COMB_MAXFEEDBACK) f5 = -COMB_MAXFEEDBACK;
    else if (f5 > COMB_MAXFEEDBACK) f5 = COMB_MAXFEEDBACK;
    sic_newinlet((t_sic *)x, f2);
    sic_newinlet((t_sic *)x, f3);
    sic_newinlet((t_sic *)x, f4);
    sic_newinlet((t_sic *)x, f5);
    outlet_new((t_object *)x, &s_signal);
    comb_clear(x);
    return (x);
}

static void comb_free(t_comb *x)
{
    if (x->x_buf) freebytes(x->x_buf, x->x_bufsize * sizeof(*x->x_buf));
}

void comb_tilde_setup(void)
{
    comb_class = class_new(gensym("comb~"),
			   (t_newmethod)comb_new,
			   (t_method)comb_free,
			   sizeof(t_comb), 0,
			   A_DEFFLOAT, A_DEFFLOAT,
			   A_DEFFLOAT, A_DEFFLOAT, A_DEFFLOAT, 0);
    sic_setup(comb_class, comb_dsp, SIC_FLOATTOSIGNAL);
    class_addmethod(comb_class, (t_method)comb_clear, gensym("clear"), 0);
}
