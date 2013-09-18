/* Copyright (c) 2005 krzYszcz and others.
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

#ifndef __PATCHVALUE_H__
#define __PATCHVALUE_H__

typedef struct _patchvalue
{
    t_pd                 pv_pd;
    t_symbol            *pv_name;
    int                  pv_refcount;
    struct _patchvalue  *pv_next;
} t_patchvalue;

t_class *patchvalue_classnew(t_symbol *cname, size_t size);
t_patchvalue *patchvalue_use(t_symbol *category, t_glist *glist,
			     t_class *cls, t_symbol *name);
t_patchvalue *patchvalue_get(t_symbol *category, t_glist *glist,
			     t_class *cls, t_symbol *name);
t_patchvalue *patchvalue_resolve(t_symbol *category, t_glist *glist,
				 t_class *cls, t_symbol *name);

#endif
