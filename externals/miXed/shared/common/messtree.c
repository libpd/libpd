/* Copyright (c) 2005 krzYszcz and others.
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

/* This module covers parsing of a single message received by an object
   or used for creation of an object, as well as parsing of multiple messages
   contained in an imported buffer, etc. */

#include "m_pd.h"
#include "common/loud.h"
#include "messtree.h"

#ifdef KRZYSZCZ
#define MESSTREE_DEBUG
#endif

/* There are two different messtree structures: the compile-time input
   (t_messslot/t_messnode) and the run-time one (t_messtree).  The reasons are:
   to allow plugins to extend a message tree, and to make the compile-time
   input initializer-friendly. */

struct _messtree
{
    t_messslot  *mt_slot;
    t_symbol    *mt_selector;
    t_messcall   mt_method;
    int          mt_nonexclusive;
    struct _messtree  *mt_sublist;
    struct _messtree  *mt_next;
};

t_messtree *messtree_new(t_symbol *selector)
{
    t_messtree *mt = getbytes(sizeof(*mt));
    mt->mt_slot = 0;
    mt->mt_selector = selector;
    mt->mt_method = 0;  /* LATER define a default */
    mt->mt_nonexclusive = 0;
    mt->mt_sublist = 0;
    mt->mt_next = 0;
    return (mt);
}

static void messtree_addnode(t_messtree *mt, t_messnode *mn)
{
    /* LATER respect mn->mn_index */
    t_messslot *ms;
    int i;
    for (i = 0, ms = mn->mn_table + mn->mn_nslots - 1;
	 i < mn->mn_nslots; i++, ms--)
    {
	t_messtree *bch = messtree_new(gensym(ms->ms_name));
	bch->mt_slot = ms;
	bch->mt_method = ms->ms_call;
	bch->mt_nonexclusive = (ms->ms_flags & MESSTREE_NONEXCLUSIVE);
	bch->mt_next = mt->mt_sublist;
	mt->mt_sublist = bch;
	if (ms->ms_subnode)
	    messtree_addnode(bch, ms->ms_subnode);
    }
}

void messtree_add(t_messtree *mt, t_messnode *rootnode)
{
    messtree_addnode(mt, rootnode);
}

t_messtree *messtree_build(t_messslot *rootslot)
{
    t_messtree *mt = messtree_new(gensym(rootslot->ms_name));
    mt->mt_slot = rootslot;
    mt->mt_method = rootslot->ms_call;
    mt->mt_nonexclusive = (rootslot->ms_flags & MESSTREE_NONEXCLUSIVE);
    mt->mt_sublist = 0;
    mt->mt_next = 0;
    if (rootslot->ms_subnode)
	messtree_addnode(mt, rootslot->ms_subnode);
    return (mt);
}

int messtree_doit(t_messtree *mt, t_messslot **msp, int *nargp,
		  t_pd *target, t_symbol *s, int ac, t_atom *av)
{
    int result = MESSTREE_OK, nargpdummy;
    t_messslot *mspdummy;
    if (!msp)
	msp = &mspdummy;
    if (!nargp)
	nargp = &nargpdummy;
    if (s && s != mt->mt_selector)
    {
	loud_warning(target, (target ? 0 : "messtree"),
		     "unexpected selector \"%s\"", s->s_name);
	*msp = 0;
	*nargp = 0;
	return (MESSTREE_CORRUPT);
    }
    if (ac && av->a_type == A_SYMBOL)
    {
	t_messtree *bch;
	for (bch = mt->mt_sublist; bch; bch = bch->mt_next)
	{
	    if (av->a_w.w_symbol == bch->mt_selector)
	    {
		if (bch->mt_sublist)
		    return (messtree_doit(bch, msp, nargp, target,
					  av->a_w.w_symbol, ac - 1, av + 1));
		else
		{
		    if (target && bch->mt_method)
			result = bch->mt_method(target, av->a_w.w_symbol,
						ac - 1, av + 1);
		    *msp = (result == MESSTREE_OK ? bch->mt_slot : 0);
		    *nargp = ac - 1;
		    return (result);
		}
	    }
	}
	if (mt->mt_nonexclusive)
	{
	    if (target && mt->mt_method)
		result = mt->mt_method(target, 0, ac, av);  /* LATER rethink */
	    *msp = (result == MESSTREE_OK ? mt->mt_slot : 0);
	    *nargp = ac;
	    return (result);
	}
	else
	{
	    loud_warning(target, (target ? 0 : "messtree"),
			 "unknown property \"%s\"", av->a_w.w_symbol->s_name);
	    *msp = 0;
	    *nargp = 0;
	    return (MESSTREE_UNKNOWN);
	}
    }
    else
    {
	if (target && mt->mt_method)
	    result = mt->mt_method(target, 0, ac, av);
	*msp = (result == MESSTREE_OK ? mt->mt_slot : 0);
	*nargp = ac;
	return (result);
    }
}
