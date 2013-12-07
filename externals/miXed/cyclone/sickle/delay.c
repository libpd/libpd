/* Copyright (c) 2002-2003 krzYszcz and others.
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

#include <string.h>
#include "m_pd.h"
#include "sickle/sic.h"

typedef struct _delay
{
    t_sic     x_sic;
    t_float  *x_buf;
    t_float  *x_bufend;
    t_float  *x_whead;
    int	      x_maxsize;
    int       x_delsize;
} t_delay;

static t_class *delay_class;

#define DELAY_DEFMAXSIZE  512

static void delay_ft1(t_delay *x, t_floatarg f)
{
    x->x_delsize = (f > 0 ? (int)f : 0);
    if (x->x_delsize > x->x_maxsize)
	x->x_delsize = x->x_maxsize;  /* CHECKED */
    /* CHECKED: all buffered values should be available */
}

static t_int *delay_perform(t_int *w)
{
    t_delay *x = (t_delay *)(w[1]);
    int nblock = (int)(w[2]);
    t_float *in = (t_float *)(w[3]);
    t_float *out = (t_float *)(w[4]);
    t_float *buf = x->x_buf;
    t_float *ep = x->x_bufend;
    t_float *wp = x->x_whead;
    if (x->x_delsize)
    {
	t_float *rp = wp - x->x_delsize;
	if (rp < buf) rp += x->x_maxsize;
	while (nblock--)
	{
	    float f = *in++;
	    *out++ = *rp;
	    if (rp++ == ep) rp = buf;
	    *wp = f;
	    if (wp++ == ep) wp = buf;
	}
    }
    else while (nblock--)
    {
	*out++ = *wp = *in++;
	if (wp++ == ep) wp = buf;
    }
    x->x_whead = wp;
    return (w + 5);
}

static void delay_dsp(t_delay *x, t_signal **sp)
{
    memset(x->x_buf, 0, x->x_maxsize * sizeof(*x->x_buf));  /* CHECKED */
    x->x_whead = x->x_buf;
    dsp_add(delay_perform, 4, x, sp[0]->s_n, sp[0]->s_vec, sp[1]->s_vec);
}

static void *delay_new(t_floatarg f1, t_floatarg f2)
{
    t_delay *x;
    int maxsize = (f1 > 0 ? (int)f1 : DELAY_DEFMAXSIZE);
    t_float *buf = (t_float *)getbytes(maxsize * sizeof(*buf));
    if (!buf)
	return (0);
    x = (t_delay *)pd_new(delay_class);
    x->x_maxsize = maxsize;
    x->x_buf = x->x_whead = buf;
    x->x_bufend = buf + maxsize - 1;
    x->x_delsize = (f2 > 0 ? (int)f2 : 0);
    if (x->x_delsize > maxsize)
	x->x_delsize = maxsize;
    inlet_new((t_object *)x, (t_pd *)x, &s_float, gensym("ft1"));
    outlet_new((t_object *)x, &s_signal);
    return (x);
}

static void delay_free(t_delay *x)
{
    if (x->x_buf) freebytes(x->x_buf, x->x_maxsize * sizeof(*x->x_buf));
}

void delay_tilde_setup(void)
{
    delay_class = class_new(gensym("delay~"),
			    (t_newmethod)delay_new, (t_method)delay_free,
			    sizeof(t_delay), 0, A_DEFFLOAT, A_DEFFLOAT, 0);
    sic_setup(delay_class, delay_dsp, SIC_FLOATTOSIGNAL);
    class_addmethod(delay_class, (t_method)delay_ft1,
		    gensym("ft1"), A_FLOAT, 0);
}
