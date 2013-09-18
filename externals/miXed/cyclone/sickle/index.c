/* Copyright (c) 2002-2003 krzYszcz and others.
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

/* LATER: 'click' method */

#include <string.h>
#include "m_pd.h"
#include "sickle/sic.h"
#include "sickle/arsic.h"

#define INDEX_MAXCHANNELS  4  /* LATER implement arsic resizing feature */

typedef struct _index
{
    t_arsic  x_arsic;
    int      x_maxchannels;
    int      x_effchannel;  /* effective channel (clipped reqchannel) */
    int      x_reqchannel;  /* requested channel */
} t_index;

static t_class *index_class;

static void index_set(t_index *x, t_symbol *s)
{
    arsic_setarray((t_arsic *)x, s, 1);
}

static t_int *index_perform(t_int *w)
{
    t_arsic *sic = (t_arsic *)(w[1]);
    int nblock = (int)(w[2]);
    t_float *out = (t_float *)(w[4]);
    if (sic->s_playable)
    {	
	t_index *x = (t_index *)sic;
	t_float *xin = (t_float *)(w[3]);
	int index, maxindex = sic->s_vecsize - 1;
	t_float *vp = sic->s_vectors[x->x_effchannel];
	if (vp)  /* handle array swapping on the fly via ft1 */
	{
	    while (nblock--)
	    {
		index = (int)(*xin++ + 0.5);
		if (index < 0)
		    index = 0;
		else if (index > maxindex)
		    index = maxindex;
		*out++ = vp[index];
	    }
	}
	else while (nblock--) *out++ = 0;
    }
    else while (nblock--) *out++ = 0;
    return (w + 5);
}

static void index_ft1(t_index *x, t_floatarg f)
{
    if ((x->x_reqchannel = (f > 1 ? (int)f - 1 : 0)) > x->x_maxchannels)
	x->x_effchannel = x->x_maxchannels - 1;
    else
	x->x_effchannel = x->x_reqchannel;
}

static void index_dsp(t_index *x, t_signal **sp)
{
    t_arsic *sic = (t_arsic *)x;
    arsic_dsp(sic, sp, index_perform, sic->s_mononame != 0);
}

static void index_free(t_index *x)
{
    arsic_free((t_arsic *)x);
}

static void *index_new(t_symbol *s, t_floatarg f)
{
    int ch = (f > 0 ? (int)f : 0);
    /* two signals:  index input, value output */
    t_index *x = (t_index *)arsic_new(index_class, s,
				      (ch ? INDEX_MAXCHANNELS : 0), 2, 0);
    if (x)
    {
	if (ch > INDEX_MAXCHANNELS)
	    ch = INDEX_MAXCHANNELS;
	x->x_maxchannels = (ch ? INDEX_MAXCHANNELS : 1);
	x->x_effchannel = x->x_reqchannel = (ch ? ch - 1 : 0);
	inlet_new((t_object *)x, (t_pd *)x, &s_float, gensym("ft1"));
	outlet_new((t_object *)x, &s_signal);
    }
    return (x);
}

void index_tilde_setup(void)
{
    index_class = class_new(gensym("index~"),
			    (t_newmethod)index_new,
			    (t_method)index_free,
			    sizeof(t_index), 0,
			    A_DEFSYM, A_DEFFLOAT, 0);
    arsic_setup(index_class, index_dsp, SIC_FLOATTOSIGNAL);
    class_addmethod(index_class, (t_method)index_set,
		    gensym("set"), A_SYMBOL, 0);
    class_addmethod(index_class, (t_method)index_ft1,
		    gensym("ft1"), A_FLOAT, 0);
}
