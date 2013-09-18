/* Copyright (c) 2003-2005 krzYszcz and others.
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

#include <string.h>
#include "m_pd.h"
#include "common/loud.h"
#include "plusbob.h"

#ifdef KRZYSZCZ
//#define PLUSBOB_DEBUG
#endif

/* The main failure of the current implementation is when a foreign object
   stores a faked symbol beyond lifetime of a wrappee.  There is no obvious
   way of protecting against stale pointers, other than leaking small
   portions of memory (four words) with every new faked symbol.  In case of
   plustot, this is not a very big deal, since for each [+tot] object the
   number of wrapped tcl objects is small and constant.

   Another failure is when a foreign object binds something to a faked
   symbol (for example, when a faked symbol is passed to an array's rename
   method).  This should not happen in usual contexts, and even if it does,
   it will unlikely cause any real harm.

   LATER let there be a choice of using either fake-symbols, or gpointers.
   The gpointer layout would be such:  gs_un points to a plusbob-like
   structure (without the bob_stub field), a unique integer code has to be
   reserved for gs_which, the fields gp_un and gp_valid are ignored.
   Using bob_refcount instead of gs_refcount is likely to simplify code. */

typedef struct _plusstub
{
    t_symbol    sb_tag;  /* common value for all bob types */
    t_plusbob  *sb_bob;
} t_plusstub;

/* Currently, objects of all +bob types are tagged with the same name: */
static char plustag_name[] = "+bob";

static void plustag_init(t_symbol *tag)
{
    tag->s_name = plustag_name;
    tag->s_thing = 0;
    tag->s_next = 0;
}

/* returns tagged +bob if valid, null otherwise (silent if caller is empty) */
t_plusbob *plustag_isvalid(t_symbol *tag, t_pd *caller)
{
    if (tag->s_name == plustag_name)
	return (((t_plusstub *)tag)->sb_bob);
    else if (caller)
    {
	if (strcmp(tag->s_name, plustag_name))
	    loud_error((caller == PLUSBOB_OWNER ? 0 : caller),
 "does not understand '%s' (check object connections)", tag->s_name);
	else
	    loud_error((caller == PLUSBOB_OWNER ? 0 : caller), "confused...");
    }
    return (0);
}

static t_plusstub *plusstub_create(t_plusbob *bob)
{
    t_plusstub *stub = getbytes(sizeof(*stub));
    plustag_init(&stub->sb_tag);
    stub->sb_bob = bob;
    return (stub);
}

/* +bob is an object tossed around, a bobbing object.  Currently, this is
   a wrapping for Tcl_Interp, Tcl_Obj, or a tcl variable, but the +bob
   interface is abstract enough to be suitable for other types of objects.
   The t_plusbob is kind of a virtual base. */

struct _plustype
{
    t_plustype   *tp_base;  /* empty, if directly derived from t_plusbob */
    t_symbol     *tp_name;
    size_t        tp_size;
    /* constructor is to be called explicitly, from derived constructors,
       or from a public wrapper. */
    t_plustypefn  tp_deletefn;  /* destructor */
    t_plustypefn  tp_preservefn;
    t_plustypefn  tp_releasefn;
    t_plustypefn  tp_attachfn;
};

static t_plustype *plustype_default = 0;

t_plustype *plustype_new(t_plustype *base, t_symbol *name, size_t sz,
			 t_plustypefn deletefn,
			 t_plustypefn preservefn, t_plustypefn releasefn,
			 t_plustypefn attachfn)
{
    t_plustype *tp = getbytes(sizeof(*tp));
    tp->tp_base = base;
    tp->tp_name = name;
    tp->tp_size = sz;
    tp->tp_deletefn = deletefn;
    tp->tp_preservefn = preservefn;
    tp->tp_releasefn = releasefn;
    tp->tp_attachfn = attachfn;
    return (tp);
}

static void plusbob_doattach(t_plusbob *bob, t_plusbob *parent)
{
    if (bob->bob_parent = parent)
    {
	/* become the youngest child: */
	bob->bob_prev = 0;
	if (bob->bob_next = parent->bob_children)
	{
	    if (parent->bob_children->bob_prev)
		loudbug_bug("plusbob_doattach 1");
	    parent->bob_children->bob_prev = bob;
	}
	parent->bob_children = bob;
    }
    else loudbug_bug("plusbob_doattach 2");
}

static void plusbob_dodetach(t_plusbob *bob)
{
    if (bob->bob_parent)
    {
	if (bob->bob_prev)
	{
	    if (bob == bob->bob_parent->bob_children)
		loudbug_bug("plusbob_dodetach 1");
	    bob->bob_prev->bob_next = bob->bob_next;
	}
	if (bob->bob_next)
	    bob->bob_next->bob_prev = bob->bob_prev;
	if (bob == bob->bob_parent->bob_children)
	    bob->bob_parent->bob_children = bob->bob_next;
    }
    else loudbug_bug("plusbob_dodetach 2");
}

/* To be called from derived constructors.
   Preserving is caller's responsibility. */
t_plusbob *plusbob_create(t_plustype *tp, t_plusbob *parent)
{
    t_plusbob *bob;
    if (!tp)
    {
	if (!plustype_default)
	    plustype_default = plustype_new(0, 0, sizeof(t_plusbob),
					    0, 0, 0, 0);
	tp = plustype_default;
    }
    if (bob = getbytes(tp->tp_size))
    {
	bob->bob_stub = (t_symbol *)plusstub_create(bob);
	bob->bob_type = tp;
	while (tp->tp_base) tp = tp->tp_base;
	bob->bob_root = tp;
	bob->bob_owner = 0;
	bob->bob_refcount = 0;
	bob->bob_dorefcount = 1;
	bob->bob_children = 0;
	if (parent)
	    plusbob_doattach(bob, parent);
	else
	    bob->bob_parent = 0;
    }
    return (bob);
}

/* Should never be called, but from plusbob_release().
   Calling from a derived destructor is illegal. */
static void plusbob_free(t_plusbob *bob)
{
    t_plustype *tp;
    if (bob->bob_parent)
	plusbob_dodetach(bob);
    for (tp = bob->bob_type; tp; tp = tp->tp_base)
	if (tp->tp_deletefn) (*tp->tp_deletefn)(bob);
    freebytes(bob, (bob->bob_type ? bob->bob_type->tp_size : sizeof(*bob)));
    /* the stub remains... */
}

void plusbob_preserve(t_plusbob *bob)
{
    if (bob->bob_dorefcount)
    {
	t_plustype *tp;
	for (tp = bob->bob_type; tp; tp = tp->tp_base)
	    if (tp->tp_preservefn) (*tp->tp_preservefn)(bob);
	bob->bob_refcount++;
    }
}

void plusbob_release(t_plusbob *bob)
{
    if (bob->bob_dorefcount)
    {
	t_plustype *tp;
	for (tp = bob->bob_type; tp; tp = tp->tp_base)
	    if (tp->tp_releasefn) (*tp->tp_releasefn)(bob);
	if (--bob->bob_refcount <= 0)
	{
	    if (bob->bob_refcount == 0)
		plusbob_free(bob);
	    else
		loudbug_bug("plusbob_release");
	}
    }
}

t_plusbob *plusbob_getparent(t_plusbob *bob)
{
    return (bob->bob_parent);
}

/* To be called for redirection only.  Bobs created as orphans are a special
   case, and cannot be attached later on.  Likewise, changing non-orphan bobs
   to orphans is illegal. */
void plusbob_attach(t_plusbob *bob, t_plusbob *newparent)
{
    if (bob->bob_parent && newparent)
    {
	t_plustype *tp;
	plusbob_dodetach(bob);
	plusbob_doattach(bob, newparent);
	for (tp = bob->bob_type; tp; tp = tp->tp_base)
	    if (tp->tp_attachfn) (*tp->tp_attachfn)(bob);
    }
    else if (newparent)
	loudbug_bug("plusbob_attach 1");
    else
	loudbug_bug("plusbob_attach 2");
}

t_plusbob *plusbob_getnext(t_plusbob *bob)
{
    return (bob->bob_next);
}

t_plusbob *plusbob_getchildren(t_plusbob *bob)
{
    return (bob->bob_children);
}

/* Redirect all bobs to a replacement parent.
   Assuming replacement exists. */
void plusbob_detachchildren(t_plusbob *bob, t_plusbob *newparent)
{
    while (bob->bob_children)
	plusbob_attach(bob->bob_children, newparent);
}

void plusbob_detachownedchildren(t_plusbob *bob, t_plusbob *newparent,
				 t_pd *owner)
{
    t_plusbob *child = bob->bob_children, *next;
    while (child)
    {
	next = child->bob_next;
	if (child->bob_owner == owner)
	    plusbob_attach(child, newparent);
	child = next;
    }
}

void plusbob_setowner(t_plusbob *bob, t_pd *owner)
{
    bob->bob_owner = owner;
}

t_pd *plusbob_getowner(t_plusbob *bob)
{
    return (bob->bob_owner);
}

void outlet_plusbob(t_outlet *o, t_plusbob *bob)
{
    outlet_symbol(o, bob->bob_stub);
}

/* returns tagged +bob if valid, null otherwise (silent if caller is empty) */
t_plusbob *plustag_validtype(t_symbol *tag, t_symbol *tname, t_pd *caller)
{
    if (tag->s_name == plustag_name)
    {
	t_plusbob *bob = ((t_plusstub *)tag)->sb_bob;
	if (bob->bob_type->tp_name == tname)
	    return (bob);
	else if (caller)
	{
	    t_symbol *s = bob->bob_type->tp_name;
	    loud_error((caller == PLUSBOB_OWNER ? bob->bob_owner : caller),
		       "invalid type '%s' ('%s' expected)",
		       (s ? s->s_name : "<unknown>"),
		       (tname ? tname->s_name : "<unknown>"));
	}
    }
    else if (plustag_isvalid(tag, caller))  /* print the error there */
	loudbug_bug("plustag_validtype");
    return (0);
}

/* returns tagged +bob if valid, null otherwise (silent if caller is empty) */
t_plusbob *plustag_validroot(t_symbol *tag, t_symbol *rname, t_pd *caller)
{
    if (tag->s_name == plustag_name)
    {
	t_plusbob *bob = ((t_plusstub *)tag)->sb_bob;
	if (bob->bob_root->tp_name == rname)
	    return (bob);
	else if (caller)
	{
	    t_symbol *s = bob->bob_root->tp_name;
	    loud_error((caller == PLUSBOB_OWNER ? bob->bob_owner : caller),
		       "invalid base type '%s' ('%s' expected)",
		       (s ? s->s_name : "<unknown>"),
		       (rname ? rname->s_name : "<unknown>"));
	}
    }
    else if (plustag_isvalid(tag, caller))  /* print the error there */
	loudbug_bug("plustag_validroot");
    return (0);
}

t_symbol *plustag_typename(t_symbol *tag, int validate, t_pd *caller)
{
    if (!validate || tag->s_name == plustag_name)
	return (((t_plusstub *)tag)->sb_bob->bob_type->tp_name);
    else if (plustag_isvalid(tag, caller))  /* print the error there */
	loudbug_bug("plustag_typename");
    return (0);
}

t_symbol *plustag_rootname(t_symbol *tag, int validate, t_pd *caller)
{
    if (!validate || tag->s_name == plustag_name)
	return (((t_plusstub *)tag)->sb_bob->bob_root->tp_name);
    else if (plustag_isvalid(tag, caller))  /* print the error there */
	loudbug_bug("plustag_rootname");
    return (0);
}

/* Plusenv (aka +env) is the base for an `environment' +bob.  Environment
   encapsulates data common for a collection of +bobs.  This is the standard
   way of grouping +bobs, according to a parent/children relationship. */

static t_plustype *plusenv_type = 0;
static t_plusbob *plusenv_parent = 0;  /* the parent of all environments */

/* To be called from derived constructors (or, LATER, plusenv's provider). */
t_plusenv *plusenv_create(t_plustype *tp, t_plusbob *parent, t_symbol *id)
{
    t_plusenv *env = 0;
    if (env = (t_plusenv *)plusbob_create(tp, parent))
    {
	if (!id)
	    /* LATER design a public interface for bob_dorefcount */
	    ((t_plusbob *)env)->bob_dorefcount = 0;
	env->env_id = id;  /* LATER rethink */
    }
    return (env);
}

t_plusenv *plusenv_find(t_symbol *id, t_plusenv *defenv)
{
    if (plusenv_parent && id)
    {
	t_plusbob *bob;
	for (bob = plusenv_parent->bob_children; bob; bob = bob->bob_next)
	    if (((t_plusenv *)bob)->env_id == id)
		break;
	return ((t_plusenv *)bob);
    }
    else return (defenv);
}

t_symbol *plusenv_getid(t_plusenv *env)
{
    return (env->env_id);
}

/* Type ignored, LATER rethink. */
t_plusbob *plusenv_getparent(t_plustype *tp)
{
    if (!plusenv_parent) plusenv_parent = plusbob_create(0, 0);
    return (plusenv_parent);
}

t_plustype *plusenv_setup(void)
{
    if (!plusenv_type)
    {
	plusenv_type = plustype_new(0, gensym("+env"),
				    sizeof(t_plusenv), 0, 0, 0, 0);
    }
    return (plusenv_type);
}
