/* Copyright (c) 2002-2004 krzYszcz and others.
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

#include "m_pd.h"
#include "common/loud.h"
#include "hammer/tree.h"

/* As a class `derived' from the common hammertree code (also in funbuff),
   offer maintains the auxiliary list, the main purpose of which is faster
   traversal (not needed here).  As a side-effect, there is a bonus of a small
   speedup of deletion, and a penalty of a small slowdown of insertion. */

typedef struct _offer
{
    t_object      x_ob;
    t_float       x_value;
    int           x_valueset;
    t_hammertree  x_tree;
} t_offer;

static t_class *offer_class;

static void offer_float(t_offer *x, t_float f)
{
    int ndx;
    if (loud_checkint((t_pd *)x, f, &ndx, &s_float))  /* CHECKED */
    {
	t_hammernode *np;
	if (x->x_valueset)
	{
	    hammertree_insertfloat(&x->x_tree, ndx, x->x_value, 1);
	    x->x_valueset = 0;
	}
	else if (np = hammertree_search(&x->x_tree, ndx))
	{
	    outlet_float(((t_object *)x)->ob_outlet, HAMMERNODE_GETFLOAT(np));
	    hammertree_delete(&x->x_tree, np);
	}
    }
}

static void offer_ft1(t_offer *x, t_floatarg f)
{
    /* this is incompatible -- CHECKED float is silently truncated */
    x->x_value = f;
    x->x_valueset = 1;
}

static void offer_clear(t_offer *x)
{
    hammertree_clear(&x->x_tree, 0);
    /* CHECKED valueset is not cleared */
}

#ifdef HAMMERTREE_DEBUG
static void offer_debug(t_offer *x, t_floatarg f)
{
    hammertree_debug(&x->x_tree, (int)f, 0);
}
#endif

static void offer_free(t_offer *x)
{
    hammertree_clear(&x->x_tree, 0);
}

static void *offer_new(void)
{
    t_offer *x = (t_offer *)pd_new(offer_class);
    x->x_valueset = 0;
    hammertree_inittyped(&x->x_tree, HAMMERTYPE_FLOAT, 0);
    inlet_new((t_object *)x, (t_pd *)x, &s_float, gensym("ft1"));
    outlet_new((t_object *)x, &s_float);
    return (x);
}

void offer_setup(void)
{
    offer_class = class_new(gensym("offer"),
			    (t_newmethod)offer_new,
			    (t_method)offer_free,
			    sizeof(t_offer), 0, 0);
    class_addfloat(offer_class, offer_float);
    class_addmethod(offer_class, (t_method)offer_ft1,
		    gensym("ft1"), A_FLOAT, 0);
    class_addmethod(offer_class, (t_method)offer_clear,
		    gensym("clear"), 0);
#ifdef HAMMERTREE_DEBUG
    class_addmethod(offer_class, (t_method)offer_debug,
		    gensym("debug"), A_DEFFLOAT, 0);
#endif
}
