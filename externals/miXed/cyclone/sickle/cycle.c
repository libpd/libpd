/* Copyright (c) 2002-2003 krzYszcz and others.
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

#include <string.h>
#include "m_pd.h"
#include "shared.h"
#include "common/loud.h"
#include "common/vefl.h"
#include "sickle/sic.h"

#define CYCLE_TABSIZE  512

typedef struct _cycle
{
    t_sic      x_sic;
    double     x_phase;
    double     x_conv;
    t_symbol  *x_name;
    int        x_offset;
    t_float   *x_table;
    t_float   *x_costable;
    t_float    x_usertable[CYCLE_TABSIZE + 1];
} t_cycle;

static t_class *cycle_class;

static void cycle_gettable(t_cycle *x)
{
    x->x_table = 0;
    if (x->x_name)
    {
	int tabsize = 0;
	t_float *table = vefl_get(x->x_name, &tabsize, 1, (t_pd *)x);
	/* CHECKED buffer is copied */
	if (table)
	{
	    int indx = x->x_offset + CYCLE_TABSIZE;
	    t_float *ptr = x->x_usertable + CYCLE_TABSIZE;
	    if (indx == tabsize)
	    {
		*ptr-- = *table;
		indx--;
	    }
	    if (indx < tabsize)
	    {
		table += indx;
		indx -= x->x_offset;
		while (indx--) *ptr-- = *table--;
		x->x_table = x->x_usertable;
	    }
	    /* CHECKED else no complaint */
	}
    }
    else x->x_table = x->x_costable;
    if (!x->x_table)
    {
	/* CHECKED (incompatible) cycle~ is disabled -- garbage is output */
	x->x_table = x->x_usertable;
	memset(x->x_table, 0, (CYCLE_TABSIZE + 1) * sizeof(*x->x_table));
    }
}

static void cycle_set(t_cycle *x, t_symbol *s, t_floatarg f)
{
    if (s && s != &s_)
    {
	x->x_name = s;
	if ((x->x_offset = (int)f) < 0)
	    x->x_offset = 0;
    }
    else x->x_name = 0;
    cycle_gettable(x);
}

static t_int *cycle_perform(t_int *w)
{
    t_cycle *x = (t_cycle *)(w[1]);
    int nblock = (int)(w[2]);
    t_float *in1 = (t_float *)(w[3]);
    t_float *in2 = (t_float *)(w[4]);
    t_float *out = (t_float *)(w[5]);
    t_float *tab = x->x_table;
    t_float *addr, f1, f2, frac;
    double dphase = x->x_phase + SHARED_UNITBIT32;
    double conv = x->x_conv;
    int32 normhipart;
    t_shared_wrappy wrappy;
    
    wrappy.w_d = SHARED_UNITBIT32;
    normhipart = wrappy.w_i[SHARED_HIOFFSET];

    wrappy.w_d = dphase + CYCLE_TABSIZE * *in2++;  /* CHECKED */
    dphase += *in1++ * conv;
    addr = tab + (wrappy.w_i[SHARED_HIOFFSET] & (CYCLE_TABSIZE-1));
    wrappy.w_i[SHARED_HIOFFSET] = normhipart;
    frac = wrappy.w_d - SHARED_UNITBIT32;

    while (--nblock)
    {
	wrappy.w_d = dphase + CYCLE_TABSIZE * *in2++;  /* CHECKED */
    	dphase += *in1++ * conv;
	f1 = addr[0];
	f2 = addr[1];
	addr = tab + (wrappy.w_i[SHARED_HIOFFSET] & (CYCLE_TABSIZE-1));
	wrappy.w_i[SHARED_HIOFFSET] = normhipart;
	*out++ = f1 + frac * (f2 - f1);
	frac = wrappy.w_d - SHARED_UNITBIT32;
    }
    f1 = addr[0];
    f2 = addr[1];
    *out++ = f1 + frac * (f2 - f1);

    wrappy.w_d = SHARED_UNITBIT32 * CYCLE_TABSIZE;
    normhipart = wrappy.w_i[SHARED_HIOFFSET];
    wrappy.w_d = dphase + (SHARED_UNITBIT32 * CYCLE_TABSIZE - SHARED_UNITBIT32);
    wrappy.w_i[SHARED_HIOFFSET] = normhipart;
    x->x_phase = wrappy.w_d - (SHARED_UNITBIT32 * CYCLE_TABSIZE);
    return (w + 6);
}

static void cycle_dsp(t_cycle *x, t_signal **sp)
{
    cycle_gettable(x);
    x->x_conv = CYCLE_TABSIZE / sp[0]->s_sr;
    dsp_add(cycle_perform, 5, x, sp[0]->s_n,
	    sp[0]->s_vec, sp[1]->s_vec, sp[2]->s_vec);
}

static void *cycle_new(t_symbol *s, int ac, t_atom *av)
{
    t_cycle *x = (t_cycle *)pd_new(cycle_class);
    int i = (ac && av->a_type == A_FLOAT ? 1 : 0);
    int tabsize = CYCLE_TABSIZE;
    x->x_costable = sic_makecostable(&tabsize);
    if (tabsize != CYCLE_TABSIZE)
    {
	loudbug_bug("cycle_new");
	pd_free((t_pd *)x);
	return (0);
    }
    if (ac && av->a_type == A_FLOAT)
    {
	sic_inlet((t_sic *)x, 0, 0, 0, ac, av);
	ac--, av++;
    }
    sic_newinlet((t_sic *)x, 0);
    outlet_new((t_object *)x, &s_signal);
    x->x_offset = 0;
    if (ac && av->a_type == A_SYMBOL)
    {
	x->x_name = av->a_w.w_symbol;
	ac--, av++;
	if (ac && av->a_type == A_FLOAT)
	    if ((x->x_offset = (int)av->a_w.w_float) < 0)
		x->x_offset = 0;
    }
    else x->x_name = 0;
    x->x_table = 0;
    x->x_phase = 0.;
    x->x_conv = 0.;
    return (x);
}

void cycle_tilde_setup(void)
{    
    cycle_class = class_new(gensym("cycle~"),
			    (t_newmethod)cycle_new, 0,
			    sizeof(t_cycle), 0, A_GIMME, 0);
    sic_setup(cycle_class, cycle_dsp, SIC_FLOATTOSIGNAL);
    class_addmethod(cycle_class, (t_method)cycle_set,
		    gensym("set"), A_DEFSYMBOL, A_DEFFLOAT, 0);
}
