/* Copyright (c) 2002-2003 krzYszcz and others.
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

/* LATER: 'click' method */

#include <string.h>
#include "m_pd.h"
#include "sickle/sic.h"
#include "sickle/arsic.h"

#define PEEK_MAXCHANNELS  4  /* LATER implement arsic resizing feature */

typedef struct _peek
{
    t_arsic   x_arsic;
    int       x_maxchannels;
    int       x_effchannel;  /* effective channel (clipped reqchannel) */
    int       x_reqchannel;  /* requested channel */
    int       x_clipmode;
    int       x_pokemode;
    t_float   x_value;
    t_clock  *x_clock;
    double    x_clocklasttick;
    int       x_clockset;
} t_peek;

static t_class *peek_class;

static void peek_tick(t_peek *x)
{
    arsic_redraw((t_arsic *)x);  /* LATER redraw only dirty channel(s!) */
    x->x_clockset = 0;
    x->x_clocklasttick = clock_getlogicaltime();
}

static void peek_set(t_peek *x, t_symbol *s)
{
    arsic_setarray((t_arsic *)x, s, 1);
}

#define peek_doclip(f)  (f < -1. ? -1. : (f > 1. ? 1. : f))

/* CHECKED refman's error: ``if the number received in the left inlet
   specifies a sample index that does not exist in the buffer~ object's
   currently allocated memory, nothing happens.''  This is plainly wrong,
   at least for max/msp 4.0.7 bundle: the index is clipped (just like
   in tabread/tabwrite).   As a kind of an experiment, lets make this
   the refman's way... */
static void peek_float(t_peek *x, t_float f)
{
    t_arsic *sic = (t_arsic *)x;
    t_float *vp;
    arsic_validate(sic, 0);  /* LATER rethink (efficiency, and complaining) */
    if (vp = sic->s_vectors[x->x_effchannel])
    {
	int ndx = (int)f;
	if (ndx >= 0 && ndx < sic->s_vecsize)
	{
	    if (x->x_pokemode)
	    {
		double timesince;
		t_float f = x->x_value;
		vp[ndx] = (x->x_clipmode ? peek_doclip(f) : f);
		x->x_pokemode = 0;
		timesince = clock_gettimesince(x->x_clocklasttick);
		if (timesince > 1000) peek_tick(x);
		else if (!x->x_clockset)
		{
		    clock_delay(x->x_clock, 1000 - timesince);
		    x->x_clockset = 1;
		}
	    }
	    /* CHECKED: output not clipped */
	    else outlet_float(((t_object *)x)->ob_outlet, vp[ndx]);
	}
    }
}

static void peek_ft1(t_peek *x, t_floatarg f)
{
    x->x_value = f;
    x->x_pokemode = 1;
    /* CHECKED: poke-mode is reset only after receiving left inlet input
       -- it is kept across 'ft2', 'clip', and 'set' inputs. */
}

static void peek_ft2(t_peek *x, t_floatarg f)
{
    if ((x->x_reqchannel = (f > 1 ? (int)f - 1 : 0)) > x->x_maxchannels)
	x->x_effchannel = x->x_maxchannels - 1;
    else
	x->x_effchannel = x->x_reqchannel;
}

static void peek_clip(t_peek *x, t_floatarg f)
{
    x->x_clipmode = ((int)f != 0);
}

static void peek_free(t_peek *x)
{
    if (x->x_clock) clock_free(x->x_clock);
    arsic_free((t_arsic *)x);
}

static void *peek_new(t_symbol *s, t_floatarg f1, t_floatarg f2)
{
    int ch = (f1 > 0 ? (int)f1 : 0);
    t_peek *x = (t_peek *)arsic_new(peek_class, s,
				    (ch ? PEEK_MAXCHANNELS : 0), 0, 0);
    if (x)
    {
	if (ch > PEEK_MAXCHANNELS)
	    ch = PEEK_MAXCHANNELS;
	x->x_maxchannels = (ch ? PEEK_MAXCHANNELS : 1);
	x->x_effchannel = x->x_reqchannel = (ch ? ch - 1 : 0);
	/* CHECKED (refman's error) clipping is disabled by default */
	x->x_clipmode = ((int)f2 != 0);
	x->x_pokemode = 0;
	inlet_new((t_object *)x, (t_pd *)x, &s_float, gensym("ft1"));
	inlet_new((t_object *)x, (t_pd *)x, &s_float, gensym("ft2"));
	outlet_new((t_object *)x, &s_float);
	x->x_clock = clock_new(x, (t_method)peek_tick);
	x->x_clocklasttick = clock_getlogicaltime();
	x->x_clockset = 0;
    }
    return (x);
}

void peek_tilde_setup(void)
{
    peek_class = class_new(gensym("peek~"),
			   (t_newmethod)peek_new,
			   (t_method)peek_free,
			   sizeof(t_peek), 0,
			   A_DEFSYM, A_DEFFLOAT, A_DEFFLOAT, 0);
    class_addfloat(peek_class, peek_float);
    class_addmethod(peek_class, (t_method)peek_set,
		    gensym("set"), A_SYMBOL, 0);
    class_addmethod(peek_class, (t_method)peek_ft1,
		    gensym("ft1"), A_FLOAT, 0);
    class_addmethod(peek_class, (t_method)peek_ft2,
		    gensym("ft2"), A_FLOAT, 0);
    class_addmethod(peek_class, (t_method)peek_clip,
		    gensym("clip"), A_FLOAT, 0);
}
