/* Copyright (c) 1997-2004 Miller Puckette, krzYszcz, and others.
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

/* this is merely an abstraction layer over gensym() from m_class.c
   with various additions adapted from m_pd.c */

#include <stdio.h>
#include <string.h>
#include "m_pd.h"
#include "dict.h"

#ifdef KRZYSZCZ
//#define DICT_DEBUG
#endif
#define DICT_VERBOSE

#define DICT_HASHSIZE_DEFAULT  1024
#define DICT_HASHSIZE_MIN         8
#define DICT_HASHSIZE_MAX     16384

/* two structures local to m_pd.c */
typedef struct _dict_bindelem
{
    t_pd *e_who;
    struct _dict_bindelem *e_next;
} t_dict_bindelem;

typedef struct _dict_bindlist
{
    t_pd b_pd;
    t_dict_bindelem *b_list;
} t_dict_bindlist;

/* adapted bindlist_anything() from m_pd.c */
static void dict_bindlist_anything(t_dict_bindlist *x, t_symbol *s,
				   int argc, t_atom *argv)
{
    t_dict_bindelem *e;
    for (e = x->b_list; e; e = e->e_next)
    	pd_typedmess(e->e_who, s, argc, argv);
}

/* adapted m_pd_setup() from m_pd.c */
static void dict_bindlist_setup(t_dict *x)
{
    x->d_bindlist_class = class_new(dict_key(x, "bindlist"), 0, 0,
				    sizeof(t_dict_bindlist), CLASS_PD, 0);
    class_addanything(x->d_bindlist_class, dict_bindlist_anything);
}

t_dict *dict_new(size_t hashsize)
{
    t_dict *x = getbytes(sizeof(*x));
    size_t sz;
    if (x)
    {
	if (!hashsize)
	    sz = DICT_HASHSIZE_DEFAULT;
	else if (hashsize >= DICT_HASHSIZE_MAX)
	    sz = DICT_HASHSIZE_MAX;
	else if (hashsize <= DICT_HASHSIZE_MIN)
	    sz = DICT_HASHSIZE_MIN;
	else for (sz = DICT_HASHSIZE_MAX; sz > DICT_HASHSIZE_MIN; sz >>= 1)
	    if (sz <= hashsize) break;
#ifdef DICT_DEBUG
	fprintf(stderr,
		"allocating dictionary with %d-element hashtable\n", sz);
#endif
	if (x->d_hashtable =
	    getbytes((x->d_hashsize = sz) * sizeof(*x->d_hashtable)))
	{
	    dict_bindlist_setup(x);
	    return (x);
	}
	freebytes(x, sizeof(*x));
    }
    return (0);
}

void dict_free(t_dict *x)
{
    if (x->d_hashtable)
	freebytes(x->d_hashtable, x->d_hashsize * sizeof(*x->d_hashtable));
    freebytes(x, sizeof(*x));
}

/* adapted dogensym() from m_class.c */
t_symbol *dict_dokey(t_dict *x, char *s, t_symbol *oldsym)
{
    t_symbol **sym1, *sym2;
    unsigned int hash1 = 0,  hash2 = 0;
    int length = 0;
    char *s2 = s;
    int mask = x->d_hashsize - 1;
#ifdef DICT_DEBUG
    fprintf(stderr, "make symbol-key from \"%s\"", s);
#endif
    while (*s2)
    {
	hash1 += *s2;
	hash2 += hash1;
	length++;
	s2++;
    }
    sym1 = x->d_hashtable + (hash2 & mask);
#ifdef DICT_DEBUG
    fprintf(stderr, " in slot %d\n", (hash2 & mask));
#endif
    while (sym2 = *sym1)
    {
#ifdef DICT_DEBUG
	fprintf(stderr, "try \"%s\"\n", sym2->s_name);
#endif
	if (!strcmp(sym2->s_name, s))
	{
#ifdef DICT_DEBUG
	    fprintf(stderr, "found at address %x\n", (int)sym2);
#endif
	    return(sym2);
	}
	sym1 = &sym2->s_next;
    }
    if (oldsym) sym2 = oldsym;
    else
    {
    	sym2 = (t_symbol *)t_getbytes(sizeof(*sym2));
    	sym2->s_name = t_getbytes(length+1);
    	sym2->s_next = 0;
    	sym2->s_thing = 0;
    	strcpy(sym2->s_name, s);
    }
    *sym1 = sym2;
#ifdef DICT_DEBUG
    fprintf(stderr, "appended at address %x\n", (int)sym2);
#endif
    return (sym2);
}

/* adapted gensym() from m_class.c */
t_symbol *dict_key(t_dict *x, char *s)
{
    return (dict_dokey(x, s, 0));
}

/* adapted pd_bind() from m_pd.c */
void dict_bind(t_dict *x, t_pd *obj, t_symbol *s)
{
#ifdef DICT_DEBUG
    fprintf(stderr, "bind %x to \"%s\" at %x\n", (int)obj, s->s_name, (int)s);
#endif
    if (s->s_thing)
    {
#ifdef DICT_DEBUG
	fprintf(stderr, "(next one)\n");
#endif
    	if (*s->s_thing == x->d_bindlist_class)
    	{
    	    t_dict_bindlist *b = (t_dict_bindlist *)s->s_thing;
    	    t_dict_bindelem *e =
		(t_dict_bindelem *)getbytes(sizeof(t_dict_bindelem));
    	    e->e_next = b->b_list;
    	    e->e_who = obj;
    	    b->b_list = e;
    	}
    	else
    	{
    	    t_dict_bindlist *b =
		(t_dict_bindlist *)pd_new(x->d_bindlist_class);
    	    t_dict_bindelem *e1 =
		(t_dict_bindelem *)getbytes(sizeof(t_dict_bindelem));
    	    t_dict_bindelem *e2 =
		(t_dict_bindelem *)getbytes(sizeof(t_dict_bindelem));
    	    b->b_list = e1;
    	    e1->e_who = obj;
    	    e1->e_next = e2;
    	    e2->e_who = s->s_thing;
    	    e2->e_next = 0;
    	    s->s_thing = &b->b_pd;
    	}
    }
    else s->s_thing = obj;
}

/* adapted pd_unbind() from m_pd.c */
void dict_unbind(t_dict *x, t_pd *obj, t_symbol *s)
{
#ifdef DICT_DEBUG
    fprintf(stderr, "unbind %x from \"%s\" at %x\n",
	    (int)obj, s->s_name, (int)s);
#endif
    if (s->s_thing == obj) s->s_thing = 0;
    else if (s->s_thing && *s->s_thing == x->d_bindlist_class)
    {
    	    /* bindlists always have at least two elements... if the number
    	    goes down to one, get rid of the bindlist and bind the symbol
    	    straight to the remaining element. */

    	t_dict_bindlist *b = (t_dict_bindlist *)s->s_thing;
    	t_dict_bindelem *e, *e2;
    	if ((e = b->b_list)->e_who == obj)
    	{
    	    b->b_list = e->e_next;
    	    freebytes(e, sizeof(t_dict_bindelem));
    	}
    	else for (e = b->b_list; e2 = e->e_next; e = e2)
    	    if (e2->e_who == obj)
    	{
    	    e->e_next = e2->e_next;
    	    freebytes(e2, sizeof(t_dict_bindelem));
    	    break;
    	}
    	if (!b->b_list->e_next)
    	{
    	    s->s_thing = b->b_list->e_who;
    	    freebytes(b->b_list, sizeof(t_dict_bindelem));
    	    pd_free(&b->b_pd);
    	}
    }
    else pd_error(obj, "%s: couldn't unbind", s->s_name);
}

t_pd *dict_firstvalue(t_dict *dict, t_symbol *s, void **nextp)
{
    if (s->s_thing)
    {
	if (*s->s_thing == dict->d_bindlist_class)
	{
	    t_dict_bindelem *e = ((t_dict_bindlist *)s->s_thing)->b_list;
	    if (e)
	    {
		if (nextp)
		    *nextp = e->e_next;
		return (e->e_who);
	    }
	    else return (0);
	}
	else
	{
	    if (nextp)
		*nextp = 0;
	    return (s->s_thing);
	}
    }
    else return (0);
}

t_pd *dict_nextvalue(t_dict *dict, t_symbol *s, void **nextp)
{
    if (s->s_thing)
    {
	if (*s->s_thing == dict->d_bindlist_class && *nextp)
	{
	    t_dict_bindelem *e = (t_dict_bindelem *)*nextp;
	    *nextp = e->e_next;
	    return (e->e_who);
	}
    }
    else bug("dict_nextvalue");
    return (0);
}

#if 0
t_pd *dict_xvalue(t_dict *x, t_symbol *s)
{
    return (s && s != &s_ ? dict_value(x, dict_key(x, s->s_name)) : 0);
}
#endif

int dict_forall(t_dict *x, t_symbol *s, t_dict_hook hook, void *hookarg)
{
    if (!s->s_thing)
	return (0);
    if (*s->s_thing == x->d_bindlist_class)
    {
    	t_dict_bindelem *e = ((t_dict_bindlist *)s->s_thing)->b_list;
	if (!e)
	    return (0);
	if (!hook)
	    return (1);
	do
	    if (!hook(e->e_who, hookarg))
		return (0);
	while (e = e->e_next);
	return (1);
    }
    return (hook ? hook(s->s_thing, hookarg) : 1);
}
