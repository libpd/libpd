/* Copyright (c) 2002-2003 krzYszcz and others.
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

#include <string.h>
#include "m_pd.h"
#include "common/grow.h"
#include "sickle/sic.h"

#define FRAMEACCUM_INISIZE  512

typedef struct _frameaccum
{
    t_sic     x_sic;
    int       x_size;
    t_float  *x_frame;
    t_float   x_frameini[FRAMEACCUM_INISIZE];
} t_frameaccum;

static t_class *frameaccum_class;

static t_int *frameaccum_perform(t_int *w)
{
    int nblock = (int)(w[1]);
    t_float *frame = (t_float *)(w[2]);
    t_float *in = (t_float *)(w[3]);
    t_float *out = (t_float *)(w[4]);
    while (nblock--) *out++ = (*frame++ += *in++);
    return (w + 5);
}

static void frameaccum_dsp(t_frameaccum *x, t_signal **sp)
{
    int nblock = sp[0]->s_n;
    if (nblock > x->x_size)
	x->x_frame = grow_nodata(&nblock, &x->x_size, x->x_frame,
				 FRAMEACCUM_INISIZE, x->x_frameini,
				 sizeof(*x->x_frame));
    memset(x->x_frame, 0, nblock * sizeof(*x->x_frame));  /* CHECKED */
    dsp_add(frameaccum_perform, 4, nblock, x->x_frame,
	    sp[0]->s_vec, sp[1]->s_vec);
}

static void frameaccum_free(t_frameaccum *x)
{
    if (x->x_frame != x->x_frameini)
	freebytes(x->x_frame, x->x_size * sizeof(*x->x_frame));
}

static void *frameaccum_new(t_symbol *s, int ac, t_atom *av)
{
    t_frameaccum *x = (t_frameaccum *)pd_new(frameaccum_class);
    int size;
    x->x_size = FRAMEACCUM_INISIZE;
    x->x_frame = x->x_frameini;
    if ((size = sys_getblksize()) > FRAMEACCUM_INISIZE)
	x->x_frame = grow_nodata(&size, &x->x_size, x->x_frame,
				 FRAMEACCUM_INISIZE, x->x_frameini,
				 sizeof(*x->x_frame));
    outlet_new((t_object *)x, &s_signal);
    return (x);
}

void frameaccum_tilde_setup(void)
{
    frameaccum_class = class_new(gensym("frameaccum~"),
				 (t_newmethod)frameaccum_new,
				 (t_method)frameaccum_free,
				 sizeof(t_frameaccum), 0, 0);
    sic_setup(frameaccum_class, frameaccum_dsp, SIC_FLOATTOSIGNAL);
}
