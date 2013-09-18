/* Copyright (c) 2003-2005 krzYszcz and others.
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

#include <string.h>
#include "m_pd.h"
#include "common/loud.h"
#include "common/grow.h"
#include "props.h"

#ifdef KRZYSZCZ
//#define PROPS_DEBUG
#endif

#define PROPS_INISIZE    32  /* LATER rethink */
#define PROPS_MAXMIXUPS  32

/* return values of props_iskey() */
enum { PROPS_NONE = 0, PROPS_THIS, PROPS_MIXUP };

/* 'mode' argument values of props_iskey() and props_update() */
enum { PROPS_SINGLEMODE = 0, PROPS_MULTIMODE };

typedef struct _propelem
{
    char              *e_key;
    char              *e_value;
    struct _propelem  *e_next;
} t_propelem;

struct _props
{
    char             p_thisescape;
    char            *p_thisinitial;
    char            *p_name;
    int              p_size;    /* as allocated */
    int              p_natoms;  /* as used */
    t_atom          *p_buffer;
    t_atom           p_bufini[PROPS_INISIZE];
    int              p_nextindex;
    t_pd            *p_owner;
    t_propsresolver  p_resolver;
    t_propelem      *p_dict;
    t_propelem      *p_nextelem;
    int              p_badupdate;
    char             p_mixupescapes[PROPS_MAXMIXUPS];
    t_props         *p_firstmixup;  /* points to the props list's head */
    t_props         *p_next;
};

/* Dictionary of properties, p_dict, meant to be nothing more, but an
   optimization detail, is handled implicitly, through its owning t_props.
   This optimization has to be enabled by passing a nonzero 'resolver'
   argument to props_new().
   Since p_dict stores resolved strings, it is a secondary, `shallow' storage,
   which has to be synced to its master, p_buffer of atoms.
   Currently, p_dict is implemented as an unsorted linked list, which should
   be fine in most cases (but might need revisiting LATER). */

static t_propelem *propelem_new(char *key, char *value)
{
    t_propelem *ep = (t_propelem *)getbytes(sizeof(*ep));
    ep->e_key = getbytes(strlen(key) + 1);
    strcpy(ep->e_key, key);
    ep->e_value = getbytes(strlen(value) + 1);
    strcpy(ep->e_value, value);
    ep->e_next = 0;
    return (ep);
}

static void propelem_free(t_propelem *ep)
{
    if (ep->e_key) freebytes(ep->e_key, strlen(ep->e_key) + 1);
    if (ep->e_value) freebytes(ep->e_value, strlen(ep->e_value) + 1);
    freebytes(ep, sizeof(*ep));
}

/* Returns zero if the key was found (and value replaced),
   nonzero if a new element was added. */
static t_propelem *propelem_add(t_propelem *ep, char *key, char *value)
{
    while (ep)
    {
	if (strcmp(ep->e_key, key))
	    ep = ep->e_next;
	else
	    break;
    }
    if (ep)
    {
	if (strcmp(ep->e_value, value))
	{
	    if (ep->e_value)
		ep->e_value = resizebytes(ep->e_value, strlen(ep->e_value) + 1,
					  strlen(value) + 1);
	    else
		ep->e_value = getbytes(strlen(value) + 1);
	    strcpy(ep->e_value, value);
	}
	return (0);
    }
    else return (propelem_new(key, value));
}

static void props_dictadd(t_props *pp, t_symbol *s, int ac, t_atom *av)
{
    if (s && *s->s_name && s->s_name[1] && ac)
    {
	t_propelem *ep;
	char *value = pp->p_resolver(pp->p_owner, ac, av);
	if (value &&
	    (ep = propelem_add(pp->p_dict, s->s_name + 1, value)))
	{
	    ep->e_next = pp->p_dict;
	    pp->p_dict = ep;
	}
    }
}

/* API calls for lookup (getvalue) and traversal (firstvalue, nextvalue), and
   non-api calls (removevalue) of resolved properties.  Only dictionary-enabled
   properties handle these calls.  Plain 'key', without escape, is expected.
   Traversal is not thread-safe (will we need threaded props, LATER?) */

char *props_getvalue(t_props *pp, char *key)
{
    if (pp->p_resolver)
    {
	t_propelem *ep = pp->p_dict;
	while (ep)
	{
	    if (strcmp(ep->e_key, key))
		ep = ep->e_next;
	    else
		return (ep->e_value);
	}
    }
    return (0);
}

char *props_nextvalue(t_props *pp, char **keyp)
{
    if (pp->p_nextelem)
    {
	char *value = pp->p_nextelem->e_value;
	*keyp = pp->p_nextelem->e_key;
	pp->p_nextelem = pp->p_nextelem->e_next;
	return (value);
    }
    return (0);
}

char *props_firstvalue(t_props *pp, char **keyp)
{
    if (pp->p_nextelem)
	loudbug_bug("props_firstvalue");
    if (pp->p_resolver)
	pp->p_nextelem = pp->p_dict;
    return (props_nextvalue(pp, keyp));
}

static void props_removevalue(t_props *pp, char *key)
{
    if (pp->p_resolver && *key)
    {
	t_propelem *ep = pp->p_dict, *epp = 0;
	while (ep)
	{
	    if (strcmp(ep->e_key, key))
	    {
		epp = ep;
		ep = ep->e_next;
	    }
	    else
	    {
		if (epp)
		    epp->e_next = ep->e_next;
		else
		    pp->p_dict = ep->e_next;
		propelem_free(ep);
		break;
	    }
	}
    }
}

void props_clearvalues(t_props *pp)
{
    while (pp->p_dict)
    {
	t_propelem *ep = pp->p_dict->e_next;
	propelem_free(pp->p_dict);
	pp->p_dict = ep;
    }
}

/* LATER think about 'deep' cloning, i.e. propagating source atoms into
   the destination buffer.  Since cloning, unless requested by the user,
   should never be persistent (source atoms should not stick to the
   destination object in a .pd file), deep cloning requires introducing
   a two-buffer scheme.  There is no reason for deep cloning of arguments,
   or handlers, but options could benefit. */

void props_clonevalues(t_props *to, t_props *from)
{
    if (to->p_resolver)
    {
	int ac;
	t_atom *ap = props_getfirst(from, &ac);
	while (ap)
	{
	    props_dictadd(to, ap->a_w.w_symbol, ac - 1, ap + 1);
	    ap = props_getnext(from, &ac);
	}
    }
}

static char *props_mixupinitial(t_props *pp, char c)
{
    t_props *pp1 = pp->p_firstmixup;
    while (pp1)
    {
	if (pp1 != pp && pp1->p_thisescape == c)
	    return (pp1->p_thisinitial);
	pp1 = pp1->p_next;
    }
    loudbug_bug("props_mixupinitial");
    loudbug_post("(%c \"%s\")", c, pp->p_mixupescapes);
    return (0);
}

/* If buf is pp's key, returns PROPS_THIS; otherwise, if PROPS_MULTIMODE
   was passed and buf is a key of one of pp's mixups, returns PROPS_MIXUP;
   otherwise, returns PROPS_NONE. */
static int props_iskey(t_props *pp, int mode, char *buf)
{
    char *mixupinitial;
    if (*buf == pp->p_thisescape)
    {
	char c = buf[1];
	if ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z')
	    || (pp->p_thisinitial && strchr(pp->p_thisinitial, c)))
	    return (PROPS_THIS);
    }
    else if (mode == PROPS_MULTIMODE &&
	     *pp->p_mixupescapes && strchr(pp->p_mixupescapes, *buf))
    {
	char c = buf[1];
	if ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z')
	    || ((mixupinitial = props_mixupinitial(pp, *buf))
		&& *mixupinitial && strchr(mixupinitial, c)))
	    return (PROPS_MIXUP);
    }
    return (PROPS_NONE);
}

/* Lookup (getone) and traversal (getfirst, getnext) of unresolved properties.
   These calls return a pointer to the key atom (the contents follows it),
   unlike the get/first/nextvalue calls, which return the (resolved) value.
   Traversal is not thread-safe (will we need threaded props, LATER?) */

t_atom *props_getone(t_props *pp, t_symbol *keysym, int *npp)
{
    if (keysym &&
	props_iskey(pp, PROPS_SINGLEMODE, keysym->s_name) != PROPS_NONE)
    {
	int ibeg, iend = 0;
	t_atom *ap;
	for (ibeg = 0, ap = pp->p_buffer; ibeg < pp->p_natoms; ibeg++, ap++)
	{
	    if (ap->a_type == A_SYMBOL && ap->a_w.w_symbol == keysym)
	    {
		for (iend = ibeg + 1, ap++; iend < pp->p_natoms; iend++, ap++)
		    if (ap->a_type == A_SYMBOL &&
			props_iskey(pp, PROPS_SINGLEMODE,
				    ap->a_w.w_symbol->s_name) != PROPS_NONE)
			break;
		break;
	    }
	}
	if (iend > ibeg)
	{
	    *npp = iend - ibeg;
	    return (pp->p_buffer + ibeg);
	}
    }
    *npp = 0;
    return (0);
}

t_atom *props_getnext(t_props *pp, int *npp)
{
    if (pp->p_nextindex >= 0)
    {
	int ibeg = pp->p_nextindex;
	t_atom *ap;
	for (ap = pp->p_buffer + ibeg; ibeg < pp->p_natoms; ibeg++, ap++)
	    if (ap->a_type == A_SYMBOL &&
		props_iskey(pp, PROPS_SINGLEMODE,
			    ap->a_w.w_symbol->s_name) != PROPS_NONE)
		break;
	if (ibeg < pp->p_natoms)
	{
	    int iend;
	    for (iend = ibeg + 1, ap++; iend < pp->p_natoms; iend++, ap++)
		if (ap->a_type == A_SYMBOL &&
		    props_iskey(pp, PROPS_SINGLEMODE,
				ap->a_w.w_symbol->s_name) != PROPS_NONE)
		    break;
	    if (iend < pp->p_natoms)
		pp->p_nextindex = iend;
	    else
		pp->p_nextindex = -1;
	    *npp = iend - ibeg;
	    return (pp->p_buffer + ibeg);
	}
	else pp->p_nextindex = -1;
    }
    *npp = 0;
    return (0);
}

t_atom *props_getfirst(t_props *pp, int *npp)
{
    if (pp->p_nextindex >= 0)
	loudbug_bug("props_getfirst");
    pp->p_nextindex = 0;
    return (props_getnext(pp, npp));
}

t_atom *props_getall(t_props *pp, int *npp)
{
    *npp = pp->p_natoms;
    return (pp->p_buffer);
}

char *props_getname(t_props *pp)
{
    return (pp ? pp->p_name : "property");
}

/* Search for a property, replace its value if found, otherwise add.
   If 'filter' contains an exact copy, do nothing.  Assuming 'keysym'
   is valid.  Returning nafter - nbefore. */
static int props_update(t_props *pp, int mode, t_props *filter,
			t_symbol *keysym, int ac, t_atom *av, int doit)
{
    int nadd, ndiff, ibeg, iend = 0;
    t_atom *ap;
    for (nadd = 0, ap = av; nadd < ac; nadd++, ap++)
	if (ap->a_type == A_SYMBOL &&
	    props_iskey(pp, mode, ap->a_w.w_symbol->s_name) != PROPS_NONE)
	    break;
    if (!nadd)
    {
	pp->p_badupdate = 1;
	return (0);
    }
    pp->p_badupdate = 0;
    nadd++;

    if (filter)
    {
	int acf;
	t_atom *apf = props_getone(filter, keysym, &acf);
	if (acf == nadd)
	{
	    int i;
#ifdef PROPS_DEBUG
	    loudbug_startpost("checking %s", keysym->s_name);
	    loudbug_postatom(nadd - 1, av);
#endif
	    for (i = 1, ap = av, apf++; i < nadd; i++, ap++, apf++)
		if (ap->a_type != apf->a_type ||
		    ap->a_w.w_symbol != apf->a_w.w_symbol)
		    break;
	    if (i == nadd)
#ifndef PROPS_DEBUG
		return (0);
#else
	    {
		loudbug_post(" ... filtered");
		return (0);
	    }
	    else loudbug_post(" ... updated");
#endif
	}
    }

    for (ibeg = 0, ap = pp->p_buffer; ibeg < pp->p_natoms; ibeg++, ap++)
    {
	if (ap->a_type == A_SYMBOL && ap->a_w.w_symbol == keysym)
	{
	    for (iend = ibeg + 1, ap++; iend < pp->p_natoms; iend++, ap++)
		if (ap->a_type == A_SYMBOL &&
		    props_iskey(pp, PROPS_SINGLEMODE,
				ap->a_w.w_symbol->s_name) != PROPS_NONE)
		    break;
	    break;
	}
    }
    ndiff = (iend > ibeg ? nadd - (iend - ibeg) : nadd);
    if (doit)
    {
	int i, newnatoms = pp->p_natoms + ndiff;
	if (newnatoms > pp->p_size)
	{
	    loudbug_bug("props_update");
	    return (0);
	}
#ifdef PROPS_DEBUG
	loudbug_post("%s %s, [%d..%d), ndiff %d",
		     (iend > ibeg ? "replacing" : "adding"), keysym->s_name,
		     ibeg, iend, ndiff);
#endif
	if (iend > ibeg)
	{
	    if (ndiff > 0)
	    {
		t_atom *ap2 = pp->p_buffer + newnatoms;
		t_atom *ap1 = ap2 - ndiff;
		for (i = iend; i < pp->p_natoms; i++) *--ap2 = *--ap1;
	    }
	    else if (ndiff < 0)
	    {
		t_atom *ap2 = pp->p_buffer + iend;
		t_atom *ap1 = ap2 + ndiff;
		for (i = iend; i < pp->p_natoms; i++) *ap1++ = *ap2++;
	    }
	    ap = pp->p_buffer + ibeg;
	}
	else
	{
	    ap = pp->p_buffer + pp->p_natoms;
	    SETSYMBOL(ap, keysym);
	}
	ap++;
	nadd--;
	if (pp->p_resolver)
	    props_dictadd(pp, keysym, nadd, av);
	for (i = 0; i < nadd; i++) *ap++ = *av++;
	pp->p_natoms = newnatoms;
    }
    return (ndiff);
}

/* Carve out all properties of a given kind from a message.  If in a single
   mode, ignore `mixup' properties -- their keys are parsed through as values.
   If there is an empty property, which is not to be ignored, do not parse
   beyond.  Return an offending key symbol, if any. */
t_symbol *props_add(t_props *pp, int single,  t_props *filter,
		    t_symbol *s, int ac, t_atom *av)
{
    t_symbol *empty = 0;
    t_atom *av1, *ap;
    int mode = (single ? PROPS_SINGLEMODE : PROPS_MULTIMODE);
    int ac1, i, ngrown = 0;
    if (!s || props_iskey(pp, PROPS_SINGLEMODE, s->s_name) == PROPS_NONE)
    {
	s = 0;
	while (ac)
	{
	    s = (av->a_type == A_SYMBOL ? av->a_w.w_symbol : 0);
	    ac--; av++;
	    if (s &&
		props_iskey(pp, PROPS_SINGLEMODE, s->s_name) != PROPS_NONE)
		break;
	    s = 0;
	}
    }
    if (!s || !ac)
    {
	empty = s;
	goto done;
    }
    ngrown += props_update(pp, mode, filter, s, ac, av, 0);
    if (pp->p_badupdate)
	empty = s;
    else for (i = 0, ap = av; i < ac; i++, ap++)
    {
	if (ap->a_type == A_SYMBOL &&
	    props_iskey(pp, PROPS_SINGLEMODE,
			ap->a_w.w_symbol->s_name) != PROPS_NONE)
	{
	    ngrown += props_update(pp, mode, filter, ap->a_w.w_symbol,
				   ac - i - 1, ap + 1, 0);
	    if (pp->p_badupdate)
	    {
		empty = ap->a_w.w_symbol;
		break;
	    }
	}
    }
    ngrown += pp->p_natoms;
    if (ngrown > pp->p_size)
    {
	int nrequested = ngrown;
	pp->p_buffer = grow_withdata(&nrequested, &pp->p_natoms,
				     &pp->p_size, pp->p_buffer,
				     PROPS_INISIZE, pp->p_bufini,
				     sizeof(*pp->p_buffer));
	if (nrequested != ngrown)
	    goto done;
    }
    props_update(pp, mode, filter, s, ac, av, 1);
    if (pp->p_badupdate)
	empty = s;
    else for (i = 0, ap = av; i < ac; i++, ap++)
    {
	if (ap->a_type == A_SYMBOL &&
	    props_iskey(pp, PROPS_SINGLEMODE,
			ap->a_w.w_symbol->s_name) != PROPS_NONE)
	{
	    props_update(pp, mode, filter, ap->a_w.w_symbol,
			 ac - i - 1, ap + 1, 1);
	    if (pp->p_badupdate)
	    {
		empty = ap->a_w.w_symbol;
		break;
	    }
	}
    }
done:
    return (empty);
}

int props_remove(t_props *pp, t_symbol *keysym)
{
    int ac;
    t_atom *av = props_getone(pp, keysym, &ac);
    if (keysym && *keysym->s_name)
	props_removevalue(pp, keysym->s_name + 1);
    if (av)
    {
	int i;
	t_atom *ap = av + ac;
	t_atom *guard = pp->p_buffer + pp->p_natoms;
	while (ap < guard) *av++ = *ap++;
	pp->p_natoms -= ac;
	return (1);
    }
    else return (0);
}

static void props_clearone(t_props *pp)
{
    pp->p_natoms = 0;
    props_clearvalues(pp);
}

void props_clearall(t_props *pp)
{
    if (pp && (pp = pp->p_firstmixup))
    {
	while (pp)
	{
	    props_clearone(pp);
	    pp = pp->p_next;
	}
    }
    else loudbug_bug("props_clearall");
}

/* Compute pp0 = pp1 - pp2, using key-only equivalence. */
void props_diff(t_props *pp0, t_props *pp1, t_props *pp2)
{
    int ac1;
    t_atom *ap1 = props_getfirst(pp1, &ac1);
    props_clearone(pp0);
    while (ap1)
    {
	int ac2;
	if (!props_getone(pp2, ap1->a_w.w_symbol, &ac2))
	    props_add(pp0, 0, 0, 0, ac1, ap1);
	ap1 = props_getnext(pp1, &ac1);
    }
}

static void props_freeone(t_props *pp)
{
    if (pp->p_buffer != pp->p_bufini)
	freebytes(pp->p_buffer, pp->p_size * sizeof(*pp->p_buffer));
    props_clearvalues(pp);
    freebytes(pp, sizeof(*pp));
}

void props_freeall(t_props *pp)
{
    if (pp && (pp = pp->p_firstmixup))
    {
	while (pp)
	{
	    t_props *pp1 = pp->p_next;
	    props_freeone(pp);
	    pp = pp1;
	}
    }
    else loudbug_bug("props_freeall");
}

static void props_setupmixups(t_props *pp, t_props *mixup)
{
    t_props *pp1;
    pp->p_next = (mixup ? mixup->p_firstmixup : 0);
    for (pp1 = pp; pp1; pp1 = pp1->p_next)
    {
	t_props *pp2;
	char *bp = pp1->p_mixupescapes;
	int i;
	pp1->p_firstmixup = pp;
	for (pp2 = pp, i = 1; pp2 && i < PROPS_MAXMIXUPS;
	     pp2 = pp2->p_next, i++)
	    if (pp2 != pp1)
		*bp++ = pp2->p_thisescape;
	*bp = 0;
#ifdef PROPS_DEBUG
	loudbug_startpost("%c \"%s\" ", pp1->p_thisescape, pp1->p_mixupescapes);
#endif
    }
#ifdef PROPS_DEBUG
    loudbug_endpost();
#endif
}

/* nonzero resolver requires the owner to be nonzero */
t_props *props_new(t_pd *owner, char *name, char *thisdelim,
		   t_props *mixup, t_propsresolver resolver)
{
    t_props *pp = getbytes(sizeof(*pp));
    if (pp)
    {
	pp->p_name = name;
	if (thisdelim && *thisdelim)
	{
	    pp->p_thisescape = *thisdelim++;
	    pp->p_thisinitial = (*thisdelim ? thisdelim : 0);
	}
	else
	{
	    loudbug_bug("props_new (no escape)");
	    pp->p_thisescape = '-';
	    pp->p_thisinitial = 0;
	}
	props_setupmixups(pp, mixup);
	pp->p_size = PROPS_INISIZE;
	pp->p_natoms = 0;
	pp->p_buffer = pp->p_bufini;
	pp->p_nextindex = -1;
	if (pp->p_owner = owner)
	    pp->p_resolver = resolver;
	else
	{
	    if (resolver)
		loudbug_bug("props_new (no owner)");
	    pp->p_resolver = 0;
	}
	pp->p_dict = 0;
	pp->p_nextelem = 0;
    }
    return (pp);
}
