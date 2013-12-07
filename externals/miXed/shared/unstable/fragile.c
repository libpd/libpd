/* Copyright (c) 1997-2005 Miller Puckette, krzYszcz, and others.
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

/* Put here bits and pieces likely to break with any new Pd version. */

#include <string.h>
#include "m_pd.h"
#include "common/loud.h"
#include "unstable/pd_imp.h"
#include "unstable/fragile.h"

/* this one rather belongs to fringe.c... */
t_symbol *fragile_class_getexterndir(t_class *c)
{
    return (c->c_externdir);
}

int fragile_class_count(void)
{
    return (pd_objectmaker->c_nmethod);
}

int fragile_class_getnames(t_atom *av, int maxnames)
{
    int ac = pd_objectmaker->c_nmethod;
    t_methodentry *mp = pd_objectmaker->c_methods;
    if (ac > maxnames)
	ac = maxnames;
    else
	maxnames = ac;
    while (ac--)
    {
	SETSYMBOL(av, mp->me_name);
	mp++; av++;
    }
    return (maxnames);
}

/* Raising and voluntary mutation is a method of resolving name clashes.
   A raised class hides other equivocal candidates.  A simpler method,
   raising and lowering, works only in global scope, because, currently, Pd
   has only one visibility stack.  Until this is changed, abstraction scope
   will involve some kind of a hack for overriding global visibility stack. */

void fragile_class_raise(t_symbol *cname, t_newmethod thiscall)
{
    t_methodentry *mp = pd_objectmaker->c_methods, *topmp = 0;
    int count = pd_objectmaker->c_nmethod;
    while (count--)
    {
	if (mp->me_name == cname)
	{
	    if (mp->me_fun == (t_gotfn)thiscall)
	    {
		if (topmp)
		{
		    t_methodentry auxmp;
		    /* no linkage there, but anyway... */
		    loud_warning(0, 0, "%s is raising itself...",
				 cname->s_name);
		    memcpy(&auxmp, mp, sizeof(t_methodentry));
		    memcpy(mp, topmp, sizeof(t_methodentry));
		    memcpy(topmp, &auxmp, sizeof(t_methodentry));
		}
		return;
	    }
	    else if (!topmp)
		topmp = mp;
	}
	mp++;
    }
    loudbug_bug("fragile_class_raise");
}

t_pd *fragile_class_mutate(t_symbol *cname, t_newmethod thiscall,
			   int ac, t_atom *av)
{
    t_newmethod fn;
    t_atomtype *argtypes;
    if (fn = fragile_class_getalien(cname, thiscall, &argtypes))
    {
	t_pd *z;
	loud_warning(0, 0, "%s is mutating now...", cname->s_name);
	if (z = fragile_class_createobject(cname, fn, argtypes, ac, av))
	{
	    post("...succeeded");
	    return (z);
	}
	else post("...failed");
    }
    return (0);
}

t_newmethod fragile_class_getalien(t_symbol *cname, t_newmethod thiscall,
				   t_atomtype **argtypesp)
{
    t_methodentry *mp = pd_objectmaker->c_methods;
    int count = pd_objectmaker->c_nmethod;
    while (count--)
    {
	if (mp->me_name == cname && mp->me_fun != (t_gotfn)thiscall)
	{
	    *argtypesp = mp->me_arg;
	    return ((t_newmethod)mp->me_fun);
	}
	mp++;
    }
    return (0);
}

/* A specialized copy of pd_typedmess() from m_class.c,
   somewhat simplified for readability. */

typedef t_pd *(*t_newgimme)(t_symbol *s, int ac, t_atom *av);
typedef t_pd *(*t_new0)(
    t_floatarg, t_floatarg, t_floatarg, t_floatarg, t_floatarg);
typedef t_pd *(*t_new1)(
    t_symbol*,
    t_floatarg, t_floatarg, t_floatarg, t_floatarg, t_floatarg);
typedef t_pd *(*t_new2)(
    t_symbol*, t_symbol*,
    t_floatarg, t_floatarg, t_floatarg, t_floatarg, t_floatarg);
typedef t_pd *(*t_new3)(
    t_symbol*, t_symbol*, t_symbol*,
    t_floatarg, t_floatarg, t_floatarg, t_floatarg, t_floatarg);
typedef t_pd *(*t_new4)(
    t_symbol*, t_symbol*, t_symbol*, t_symbol*,
    t_floatarg, t_floatarg, t_floatarg, t_floatarg, t_floatarg);
typedef t_pd *(*t_new5)(
    t_symbol*, t_symbol*, t_symbol*, t_symbol*, t_symbol*,
    t_floatarg, t_floatarg, t_floatarg, t_floatarg, t_floatarg);
typedef t_pd *(*t_new6)(
    t_symbol*, t_symbol*, t_symbol*, t_symbol*, t_symbol*, t_symbol*,
    t_floatarg, t_floatarg, t_floatarg, t_floatarg, t_floatarg);

t_pd *fragile_class_createobject(t_symbol *cname, t_newmethod callthis,
				 t_atomtype *argtypes, int ac, t_atom *av)
{
    t_floatarg ff[MAXPDARG+1], *fp = ff;
    t_symbol *ss[MAXPDARG+1], **sp = ss;
    int nsymbols = 0;
    t_atomtype wanttype;
    if (*argtypes == A_GIMME)
	return ((*((t_newgimme)(callthis)))(cname, ac, av));
    if (ac > MAXPDARG)
	ac = MAXPDARG;
    while (wanttype = *argtypes++)
    {
	switch (wanttype)
	{
	case A_POINTER:
	    goto badarg;
	case A_FLOAT:
	    if (!ac) goto badarg;
	case A_DEFFLOAT:
	    if (!ac) *fp = 0;
	    else
	    {
		if (av->a_type == A_FLOAT)
		    *fp = av->a_w.w_float;
		else goto badarg;
		ac--; av++;
	    }
	    fp++;
	    break;
	case A_SYMBOL:
	    if (!ac) goto badarg;
	case A_DEFSYM:
	    if (!ac) *sp = &s_;
	    else
	    {
		if (av->a_type == A_SYMBOL)
		    *sp = av->a_w.w_symbol;
		else if (av->a_type == A_FLOAT && av->a_w.w_float == 0)
		    *sp = &s_;
		else goto badarg;
		ac--; av++;
	    }
	    nsymbols++;
	    sp++;
	}
    }
    switch (nsymbols)
    {
    case 0: return ((*(t_new0)(callthis))
		    (ff[0], ff[1], ff[2], ff[3], ff[4]));
    case 1: return ((*(t_new1)(callthis))
		    (ss[0],
		     ff[0], ff[1], ff[2], ff[3], ff[4]));
    case 2: return ((*(t_new2)(callthis))
		    (ss[0], ss[1],
		     ff[0], ff[1], ff[2], ff[3], ff[4]));
    case 3: return ((*(t_new3)(callthis))
		    (ss[0], ss[1], ss[2],
		     ff[0], ff[1], ff[2], ff[3], ff[4]));
    case 4: return ((*(t_new4)(callthis))
		    (ss[0], ss[1], ss[2], ss[3],
		     ff[0], ff[1], ff[2], ff[3], ff[4]));
    case 5: return ((*(t_new5)(callthis))
		    (ss[0], ss[1], ss[2], ss[3], ss[4],
		     ff[0], ff[1], ff[2], ff[3], ff[4]));
    case 6: return ((*(t_new6)(callthis))
		    (ss[0], ss[1], ss[2], ss[3], ss[4], ss[5],
		     ff[0], ff[1], ff[2], ff[3], ff[4]));
    }
badarg:
    loud_error(0, "bad creation arguments for class '%s'", cname->s_name);
    return (0);
}

void fragile_class_printnames(char *msg, int firstndx, int lastndx)
{
    t_methodentry *mp = pd_objectmaker->c_methods;
    int ndx, len = strlen(msg);
    startpost(msg);
    for (ndx = firstndx, mp += ndx; ndx <= lastndx; ndx++, mp++)
    {
	t_symbol *s = mp->me_name;
	if (s && s->s_name[0] != '_')
	{
	    int l = 1 + strlen(s->s_name);
	    if ((len += l) > 66)
	    {
		endpost();
		startpost("   ");
		len = 3 + l;
	    }
	    poststring(s->s_name);
	}
    }
    endpost();
}

/* This structure is local to g_array.c.  We need it,
   because there is no other way to get into array's graph. */
struct _garray
{
    t_gobj x_gobj;
    t_glist *x_glist;
    /* ... */
};

t_glist *fragile_garray_glist(void *arr)
{
    return (((struct _garray *)arr)->x_glist);
}

/* This is local to m_obj.c.
   LATER export write access to o_connections field ('grab' class).
   LATER encapsulate 'traverseoutlet' routines (not in the stable API yet). */
struct _outlet
{
    t_object *o_owner;
    struct _outlet *o_next;
    t_outconnect *o_connections;
    t_symbol *o_sym;
};

/* obj_starttraverseoutlet() replacement */
t_outconnect *fragile_outlet_connections(t_outlet *o)
{
    return (o ? o->o_connections : 0);
}

t_outconnect *fragile_outlet_nextconnection(t_outconnect *last,
					    t_object **destp, int *innop)
{
    t_inlet *dummy;
    return (obj_nexttraverseoutlet(last, destp, &dummy, innop));
}

/* silent, if caller is empty */
t_object *fragile_outlet_destination(t_outlet *op,
				     int ntypes, t_symbol **types,
				     t_pd *caller, char *errand)
{
    t_object *booty = 0;
    t_symbol *badtype = 0;
    int count = 0;
    t_outconnect *tobooty = fragile_outlet_connections(op);
    while (tobooty)
    {
	t_object *ob;
	int inno;
	count++;
	tobooty = fragile_outlet_nextconnection(tobooty, &ob, &inno);
	if (ob && inno == 0)
	{
	    /* LATER ask for class_getname()'s symbol version */
	    t_symbol **tp, *dsttype = gensym(class_getname(*(t_pd *)ob));
	    int i;
	    for (i = 0, tp = types; i < ntypes; i++, tp++)
	    {
		if (*tp == dsttype)
		{
		    booty = ob;
		    break;
		}
		else badtype = dsttype;
	    }
	}
    }
    if (booty)
    {
	if (count > 1 && caller)
	    loud_warning(caller, 0, "multiple targets");
    }
    else if (caller)
    {
	if (badtype)
	    loud_error(caller, "bad target type '%s'", badtype->s_name);
	else
	    loud_error(caller, "no target");
	if (errand)
	    loud_errand(caller, errand);
    }
    return (booty);
}

/* These are local to m_obj.c. */
union inletunion
{
    t_symbol *iu_symto;
    t_gpointer *iu_pointerslot;
    t_float *iu_floatslot;
    t_symbol **iu_symslot;
    t_sample iu_floatsignalvalue;
};

struct _inlet
{
    t_pd i_pd;
    struct _inlet *i_next;
    t_object *i_owner;
    t_pd *i_dest;
    t_symbol *i_symfrom;
    union inletunion i_un;
};

/* simplified obj_findsignalscalar(), works for non-left inlets */
t_sample *fragile_inlet_signalscalar(t_inlet *i)
{
    return (&i->i_un.iu_floatsignalvalue);
}
