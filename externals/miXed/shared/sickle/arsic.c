/* Copyright (c) 2003 krzYszcz and others.
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

/* generic array-based signal class */

#include <stdio.h>
#include <string.h>
#include "m_pd.h"
#include "shared.h"
#include "common/loud.h"
#include "common/vefl.h"
#include "sickle/sic.h"
#include "sickle/arsic.h"

void arsic_clear(t_arsic *x)
{
    x->s_vecsize = 0;
    memset(x->s_vectors, 0, x->s_nchannels * sizeof(*x->s_vectors));
}

void arsic_redraw(t_arsic *x)
{
    if (x->s_mononame)
    {
	t_garray *ap =
	    (t_garray *)pd_findbyclass(x->s_mononame, garray_class);
	if (ap) garray_redraw(ap);
	else if (x->s_vectors[0]) loudbug_bug("arsic_redraw 1");
    }
    else if (*x->s_stub)
    {
	int ch = x->s_nchannels;
	while (ch--)
	{
	    t_garray *ap =
		(t_garray *)pd_findbyclass(x->s_channames[ch], garray_class);
	    if (ap) garray_redraw(ap);
	    else if (x->s_vectors[ch]) loudbug_bug("arsic_redraw 2");
	}
    }
}

void arsic_validate(t_arsic *x, int complain)
{
    arsic_clear(x);
    x->s_vecsize = SHARED_INT_MAX;
    if (x->s_mononame)
    {
	x->s_vectors[0] =
	    vefl_get(x->s_mononame, &x->s_vecsize, 1,
		     (complain ? (t_pd *)x : 0));
    }
    else if (*x->s_stub)
    {
	int ch;
	for (ch = 0; ch < x->s_nchannels ; ch++)
	{
	    int vsz = x->s_vecsize;  /* ignore missing arrays */
	    x->s_vectors[ch] =
		vefl_get(x->s_channames[ch], &vsz, 1,
			 (complain ? (t_pd *)x : 0));
	    if (vsz < x->s_vecsize) x->s_vecsize = vsz;
	}
    }
    if (x->s_vecsize == SHARED_INT_MAX) x->s_vecsize = 0;
}

void arsic_check(t_arsic *x)
{
    x->s_playable = (!((t_sic *)x)->s_disabled && x->s_vecsize >= x->s_minsize);
}

int arsic_getnchannels(t_arsic *x)
{
    return (x->s_nchannels);
}

void arsic_setarray(t_arsic *x, t_symbol *s, int complain)
{
    if (s)
    {
	if (x->s_mononame) x->s_mononame = s;
	else
	{
	    x->s_stub = s->s_name;
	    if (*x->s_stub)
	    {
		char buf[MAXPDSTRING];
		int ch;
		for (ch = 0; ch < x->s_nchannels; ch++)
		{
		    sprintf(buf, "%d-%s", ch, x->s_stub);
		    x->s_channames[ch] = gensym(buf);
		}
	    }
	}
	arsic_validate(x, complain);
    }
    arsic_check(x);
}

void arsic_setminsize(t_arsic *x, int i)
{
    x->s_minsize = i;
}

void arsic_dsp(t_arsic *x, t_signal **sp, t_perfroutine perf, int complain)
{
    t_int *ap = x->s_perfargs;
    if (ap)
    {
	int i, nsigs = x->s_nperfargs - 2;
	x->s_ksr = sp[0]->s_sr * 0.001;
	arsic_validate(x, complain);
	arsic_check(x);

	/* LATER consider glist traversing, and, if we have no feeders,
	   choosing an optimized version of perform routine */

	*ap++ = (t_int)x;
	*ap++ = (t_int)sp[0]->s_n;
	for (i = 0; i < nsigs; i++) *ap++ = (t_int)sp[i]->s_vec;
	dsp_addv(perf, x->s_nperfargs, x->s_perfargs);
    }
    else loudbug_bug("arsic_dsp");
}

void arsic_free(t_arsic *x)
{
    if (x->s_vectors)
	freebytes(x->s_vectors, x->s_nchannels * sizeof(*x->s_vectors));
    if (x->s_channames)
	freebytes(x->s_channames,
		  x->s_nchannels * sizeof(*x->s_channames));
    if (x->s_perfargs)
	freebytes(x->s_perfargs, x->s_nperfargs * sizeof(*x->s_perfargs));
}

/* If nauxsigs is positive, then the number of signals is nchannels + nauxsigs;
   otherwise the channels are not used as signals, and the number of signals is
   nsigs -- provided that nsigs is positive -- or, if it is not, then an arsic
   is not used in dsp (peek~). */
void *arsic_new(t_class *c, t_symbol *s,
		int nchannels, int nsigs, int nauxsigs)
{
    t_arsic *x;
    t_symbol *mononame;
    char *stub;
    t_float **vectors;
    int nperfargs = 0;
    t_int *perfargs = 0;
    t_symbol **channames = 0;
    if (!s) s = &s_;
    if (nchannels < 1)
    {
	nchannels = 1;
	mononame = s;
	stub = 0;
    }
    else
    {
	mononame = 0;
	stub = s->s_name;
    }
    if (!(vectors = (t_float **)getbytes(nchannels * sizeof(*vectors))))
	return (0);
    if (nauxsigs > 0)
	nperfargs = nchannels + nauxsigs + 2;
    else if (nsigs > 0)
	nperfargs = nsigs + 2;
    if (nperfargs
	&& !(perfargs = (t_int *)getbytes(nperfargs * sizeof(*perfargs))))
    {
	freebytes(vectors, nchannels * sizeof(*vectors));
	return (0);
    }
    if (stub &&
	!(channames = (t_symbol **)getbytes(nchannels * sizeof(*channames))))
    {
	freebytes(vectors, nchannels * sizeof(*vectors));
	if (perfargs) freebytes(perfargs, nperfargs * sizeof(*perfargs));
	return (0);
    }
    x = (t_arsic *)pd_new(c);
    x->s_vecsize = 0;
    x->s_nchannels = nchannels;
    x->s_vectors = vectors;
    x->s_channames = channames;
    x->s_nperfargs = nperfargs;
    x->s_perfargs = perfargs;
    x->s_mononame = mononame;
    x->s_stub = stub;
    x->s_ksr = sys_getsr() * 0.001;
    ((t_sic *)x)->s_disabled = 0;
    x->s_playable = 0;
    x->s_minsize = 1;
    arsic_setarray(x, s, 0);
    return (x);
}

static void arsic_enable(t_arsic *x, t_floatarg f)
{
    ((t_sic *)x)->s_disabled = (f == 0);
    arsic_check(x);
}

/* LATER somehow link this to sic_setup() */
void arsic_setup(t_class *c, void *dspfn, void *floatfn)
{
    if (floatfn != SIC_NOMAINSIGNALIN)
    {
	if (floatfn)
	{
	    class_domainsignalin(c, -1);
	    class_addfloat(c, floatfn);
	}
	else CLASS_MAINSIGNALIN(c, t_sic, s_f);
    }
    class_addmethod(c, (t_method)dspfn, gensym("dsp"), 0);
    class_addmethod(c, (t_method)arsic_enable, gensym("enable"), 0);
}
