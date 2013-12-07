/* Copyright (c) 2003 krzYszcz and others.
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

/* Put here compilation conditionals supporting older Pd versions. */

#include "m_pd.h"
#include "g_canvas.h"
#include "shared.h"
#include "unstable/forky.h"

#if FORKY_VERSION < 37
/* need this for t_class::c_wb field access */
#include "unstable/pd_imp.h"
#endif

#ifdef KRZYSZCZ
//#define FORKY_DEBUG
#endif

t_pd *forky_newobject(t_symbol *s, int ac, t_atom *av)
{
#if FORKY_VERSION >= 37
    typedmess(&pd_objectmaker, s, ac, av);
    return (pd_newest());
#else
    return (0);
#endif
}

void forky_setsavefn(t_class *c, t_forkysavefn fn)
{
#if FORKY_VERSION >= 37
    class_setsavefn(c, fn);
#else
    if (c->c_wb->w_savefn)
    {
	/* cloning is necessary, because class_setwidget has not been called */
	t_widgetbehavior *wb = getbytes(sizeof(*wb));  /* never freed */
#ifdef FORKY_DEBUG
	fprintf(stderr, "cloning widgetbehavior...\n");
#endif
	*wb = *c->c_wb;
	wb->w_savefn = fn;
	class_setwidget(c, wb);
    }
    else c->c_wb->w_savefn = fn;
#endif
}

void forky_setpropertiesfn(t_class *c, t_forkypropertiesfn fn)
{
#if FORKY_VERSION >= 37
    class_setpropertiesfn(c, fn);
#else
    /* assuming wb has already been either cloned (in forky_setsavefn),
       or defined from scratch -- it is unlikely to ever need props without
       a specialized save (always be sure to set props after save, though). */
    c->c_wb->w_propertiesfn = fn;
#endif
}

/* To be called in a 'dsp' method -- e.g. if there are no feeders, the caller
   might use an optimized version of a 'perform' routine.
   LATER think about replacing 'linetraverser' calls with something faster. */
int forky_hasfeeders(t_object *x, t_glist *glist, int inno, t_symbol *outsym)
{
    t_linetraverser t;
    linetraverser_start(&t, glist);
    while (linetraverser_next(&t))
	if (t.tr_ob2 == x && t.tr_inno == inno
#if FORKY_VERSION >= 36
	    && (!outsym || outsym == outlet_getsymbol(t.tr_outlet))
#endif
	    )
	    return (1);
    return (0);
}

/* Not really a forky, just found no better place to put it in.
   Used in sickle's bitwise signal binops (which use forky_hasfeeders() too).
   Checked against msp2. */
t_int forky_getbitmask(int ac, t_atom *av)
{
    t_int result = 0;
    if (sizeof(shared_t_bitmask) >= sizeof(t_int))
    {
	int nbits = sizeof(t_int) * 8;
	shared_t_bitmask bitmask = 1 << (nbits - 1);
	if (ac > nbits)
	    ac = nbits;
	while (ac--)
	{
	    if (av->a_type == A_FLOAT &&
		(int)av->a_w.w_float)  /* CHECKED */
		result |= bitmask;
	    /* CHECKED symbols are zero */
	    bitmask >>= 1;
	    av++;
	}
	/* CHECKED missing are zero */
#ifdef FORKY_DEBUG
	fprintf(stderr, "mask set to %.8x\n", result);
#endif
    }
    else bug("sizeof(shared_t_bitmask)");
    return (result);
}
