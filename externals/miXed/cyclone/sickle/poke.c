/* Copyright (c) 2003 krzYszcz and others.
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

/* LATER: 'click' method */

#include "m_pd.h"
#include "unstable/fragile.h"
#include "sickle/sic.h"
#include "sickle/arsic.h"

#define POKE_MAXCHANNELS  4  /* LATER implement arsic resizing feature */

typedef struct _poke
{
    t_arsic    x_arsic;
    int        x_maxchannels;
    int        x_effchannel;  /* effective channel (clipped reqchannel) */
    int        x_reqchannel;  /* requested channel */
    t_sample  *x_indexptr;
    t_clock   *x_clock;
    double     x_clocklasttick;
    int        x_clockset;
} t_poke;

static t_class *poke_class;

static void poke_tick(t_poke *x)
{
    arsic_redraw((t_arsic *)x);  /* LATER redraw only dirty channel(s!) */
    x->x_clockset = 0;
    x->x_clocklasttick = clock_getlogicaltime();
}

static void poke_set(t_poke *x, t_symbol *s)
{
    arsic_setarray((t_arsic *)x, s, 1);
}

static void poke_bang(t_poke *x)
{
    arsic_redraw((t_arsic *)x);
}

/* CHECKED: index 0-based, negative values block input, overflowed are clipped.
   LATER revisit: incompatibly, the code below is nop for any out-of-range index
   (see also peek.c) */
/* CHECKED: value never clipped, 'clip' not understood */
/* CHECKED: no float-to-signal conversion.  'Float' message is ignored
   when dsp is on -- whether a signal is connected to the left inlet, or not
   (if not, current index is set to zero).  Incompatible (revisit LATER) */
static void poke_float(t_poke *x, t_float f)
{
    t_arsic *sic = (t_arsic *)x;
    t_float *vp;
    arsic_validate(sic, 0);  /* LATER rethink (efficiency, and complaining) */
    if (vp = sic->s_vectors[x->x_effchannel])
    {
	int ndx = (int)*x->x_indexptr;
	if (ndx >= 0 && ndx < sic->s_vecsize)
	{
	    double timesince;
	    vp[ndx] = f;
	    timesince = clock_gettimesince(x->x_clocklasttick);
	    if (timesince > 1000) poke_tick(x);
	    else if (!x->x_clockset)
	    {
		clock_delay(x->x_clock, 1000 - timesince);
		x->x_clockset = 1;
	    }
	}
    }
}

static void poke_ft2(t_poke *x, t_floatarg f)
{
    if ((x->x_reqchannel = (f > 1 ? (int)f - 1 : 0)) > x->x_maxchannels)
	x->x_effchannel = x->x_maxchannels - 1;
    else
	x->x_effchannel = x->x_reqchannel;
}

static t_int *poke_perform(t_int *w)
{
    t_arsic *sic = (t_arsic *)(w[1]);
    int nblock = (int)(w[2]);
    t_float *in1 = (t_float *)(w[3]);
    t_float *in2 = (t_float *)(w[4]);
    t_poke *x = (t_poke *)sic;
    t_float *vp = sic->s_vectors[x->x_effchannel];
    if (vp && sic->s_playable)
    {
	int vecsize = sic->s_vecsize;
	while (nblock--)
	{
	    t_float f = *in1++;
	    int ndx = (int)*in2++;
	    if (ndx >= 0 && ndx < vecsize)
		vp[ndx] = f;
	}
    }
    return (w + sic->s_nperfargs + 1);
}

static void poke_dsp(t_poke *x, t_signal **sp)
{
    arsic_dsp((t_arsic *)x, sp, poke_perform, 0);
}

static void poke_free(t_poke *x)
{
    if (x->x_clock) clock_free(x->x_clock);
    arsic_free((t_arsic *)x);
}

static void *poke_new(t_symbol *s, t_floatarg f)
{
    int ch = (f > 0 ? (int)f : 0);
    t_poke *x = (t_poke *)arsic_new(poke_class, s,
				    (ch ? POKE_MAXCHANNELS : 0), 2, 0);
    if (x)
    {
	t_inlet *in2;
	if (ch > POKE_MAXCHANNELS)
	    ch = POKE_MAXCHANNELS;
	x->x_maxchannels = (ch ? POKE_MAXCHANNELS : 1);
	x->x_effchannel = x->x_reqchannel = (ch ? ch - 1 : 0);
	/* CHECKED: no float-to-signal conversion.
	   Floats in 2nd inlet are ignored when dsp is on, but only if a signal
	   is connected to this inlet.  Incompatible (revisit LATER). */
	in2 = inlet_new((t_object *)x, (t_pd *)x, &s_signal, &s_signal);
	x->x_indexptr = fragile_inlet_signalscalar(in2);
	inlet_new((t_object *)x, (t_pd *)x, &s_float, gensym("ft2"));
	x->x_clock = clock_new(x, (t_method)poke_tick);
	x->x_clocklasttick = clock_getlogicaltime();
	x->x_clockset = 0;
    }
    return (x);
}

void poke_tilde_setup(void)
{
    poke_class = class_new(gensym("poke~"),
			   (t_newmethod)poke_new,
			   (t_method)poke_free,
			   sizeof(t_poke), 0,
			   A_DEFSYM, A_DEFFLOAT, 0);
    arsic_setup(poke_class, poke_dsp, poke_float);
    class_addbang(poke_class, poke_bang);  /* LATER rethink */
    class_addfloat(poke_class, poke_float);
    class_addmethod(poke_class, (t_method)poke_set,
		    gensym("set"), A_SYMBOL, 0);
    class_addmethod(poke_class, (t_method)poke_ft2,
		    gensym("ft2"), A_FLOAT, 0);
}
