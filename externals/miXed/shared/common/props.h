/* Copyright (c) 2003-2005 krzYszcz and others.
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

#ifndef __PROPS_H__
#define __PROPS_H__

EXTERN_STRUCT _props;
#define t_props  struct _props

typedef char *(*t_propsresolver)(t_pd *, int, t_atom *);

char *props_getvalue(t_props *pp, char *key);
char *props_firstvalue(t_props *pp, char **keyp);
char *props_nextvalue(t_props *pp, char **keyp);
void props_clearvalues(t_props *pp);
void props_clonevalues(t_props *to, t_props *from);

t_atom *props_getone(t_props *pp, t_symbol *keysym, int *npp);
t_atom *props_getfirst(t_props *pp, int *npp);
t_atom *props_getnext(t_props *pp, int *npp);
t_atom *props_getall(t_props *pp, int *npp);
char *props_getname(t_props *pp);

t_symbol *props_add(t_props *pp, int single,  t_props *filter,
		    t_symbol *s, int ac, t_atom *av);
int props_remove(t_props *pp, t_symbol *keysym);
void props_diff(t_props *pp0, t_props *pp1, t_props *pp2);
void props_clearall(t_props *pp);
void props_freeall(t_props *pp);
t_props *props_new(t_pd *owner, char *name, char *thisdelim,
		   t_props *mixup, t_propsresolver resolver);

#endif
