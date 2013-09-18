/* Copyright (c) 2003 krzYszcz and others.
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

/* generic signal class */

#include <math.h>
#include "m_pd.h"
#include "shared.h"
#include "common/loud.h"
#include "sickle/sic.h"

#ifdef KRZYSZCZ
//#define SIC_DEBUG
#endif

#if defined(NT) || defined(MACOSX)
/* cf pd/src/x_arithmetic.c */
#define cosf  cos
#endif

t_inlet *sic_inlet(t_sic *x, int ix, t_float df, int ax, int ac, t_atom *av)
{
    t_inlet *in = 0;
    if (ax < ac)
    {
	if (av[ax].a_type == A_FLOAT)
	    df = av[ax].a_w.w_float;
	else
	    loud_error((t_pd *)x, "bad argument %d (float expected)", ax + 1);
    }
    if (ix)
    {
	in = inlet_new((t_object *)x, (t_pd *)x, &s_signal, &s_signal);
	/* this is persistent (in->i_un.iu_floatsignalvalue = df) */
	pd_float((t_pd *)in, df);
    }
    else
    {
	in = ((t_object *)x)->ob_inlet;
	pd_float((t_pd *)x, df);
    }
    return (in);
}

t_inlet *sic_newinlet(t_sic *x, t_float f)
{
    return (sic_inlet(x, 1, f, 0, 0, 0));
}

t_float *sic_makecostable(int *sizep)
{
    /* permanent cache (tables are never freed); LATER rethink */
    static t_float *sic_costables[SIC_NCOSTABLES];
    int ndx, sz;
    /* round upwards -- always return at least requested number of elements,
       unless the maximum of 2^SIC_NCOSTABLES is exceeded; LATER rethink */
    /* (the minimum, at ndx 0, is 2^1) */
    for (ndx = 0, sz = 2; ndx < (SIC_NCOSTABLES - 1); ndx++, sz <<= 1)
	if (sz >= *sizep)
	    break;
#ifdef SIC_DEBUG
    loudbug_post("request for a costable of %d points (effective %d, ndx %d)",
		 *sizep, sz, ndx);
#endif
    *sizep = sz;
    if (sic_costables[ndx])
	return (sic_costables[ndx]);
    else if (sz == COSTABSIZE && cos_table)
	return (sic_costables[ndx] = cos_table);
    else
    {
	int cnt = sz + 1;
	float phase = 0, phsinc = SHARED_2PI / sz;
	t_float *table = (t_float *)getbytes(cnt * sizeof(*table)), *tp = table;
	if (table)
	{
#ifdef SIC_DEBUG
	    loudbug_post("got %d points of a costable", cnt);
#endif
	    while (cnt--)
	    {
		*tp++ = cosf(phase);
		phase += phsinc;
	    }
	}
	return (sic_costables[ndx] = table);
    }
}

static void sic_enable(t_sic *x, t_floatarg f)
{
    x->s_disabled = (f == 0);
}

void sic_setup(t_class *c, void *dspfn, void *floatfn)
{
    static int checked = 0;
    if (!checked)
    {
	/* MSP: here we check at startup whether the byte alignment
	   is as we declared it.  If not, the code has to be
	   recompiled the other way. */
	t_shared_wrappy wrappy;
	wrappy.w_d = SHARED_UNITBIT32 + 0.5;
	if ((unsigned)wrappy.w_i[SHARED_LOWOFFSET] != 0x80000000)
	    loudbug_bug("sic_setup: unexpected machine alignment");
	checked = 1;
    }
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
    class_addmethod(c, (t_method)sic_enable, gensym("enable"), 0);
}
