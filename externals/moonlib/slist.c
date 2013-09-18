/*
Copyright (C) 2003 Antoine Rousseau

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

*/

/* a shared symbol list, ala "value" .*/

#include <m_pd.h>
#include <stdlib.h>
#include <string.h>

static t_class *slist_class, *scommon_class;

typedef struct _sitem t_sitem;

struct _sitem
{
    t_sitem *next;
    t_symbol *s;
};

typedef struct scommon
{
    t_pd c_pd;
    t_symbol *c_sym;
    int c_refcount;
    t_sitem *first;
} t_scommon;

typedef struct _slist
{
    t_object x_obj;
    t_symbol *x_sym;
    t_scommon *x_c;
    t_outlet *x_symout;
    t_outlet *x_lenout;
} t_slist;


static void sitem_delete(t_sitem **x)
{
    t_sitem *next=(*x)->next;

    //freebytes((*x)->name,strlen((*x)->name)+1);
    freebytes(*x,sizeof(t_sitem));
    (*x)=next;
}

static void sitem_add(t_sitem **x,t_symbol *s)
{
    t_int l;
    t_sitem *newone=getbytes(sizeof(t_sitem));

    //newone->name=getbytes(l=(strlen(s->s_name)+1));
    //strncpy(newone->name,s->s_name,l);
    newone->s=s;
    newone->next=0;

    while(*x) x=&((*x)->next);

    *x=newone;
}

static void *scommon_new(t_symbol *s)
{
    t_scommon *x = (t_scommon *)pd_new(scommon_class);

    x->c_refcount = 0;
    x->c_sym=s;
    pd_bind((t_pd *)x, s);

    x->first=0;

    return (x);
}

static int scommon_find(t_scommon *x, t_symbol *s)
{
    t_sitem *si=x->first;
    t_int i=1;

    while(si)
    {
        if(!strcmp(si->s->s_name,s->s_name)) return i;
        si=si->next;
        i++;
    }
    return 0;
}

static void scommon_add(t_scommon *x, t_symbol *s)
{
    sitem_add(&x->first,s);
}

static void scommon_reset(t_scommon *x)
{
    while(x->first) sitem_delete(&x->first);
}

static void scommon_ff(t_scommon *x)
{
    scommon_reset(x);
    pd_unbind((t_pd *)x, x->c_sym);
}


/* get a pointer to a named symbol list (a "scommon" object),
which is created if necessary. */
t_scommon *slist_get(t_symbol *s)
{
    t_scommon *c = (t_scommon *)pd_findbyclass(s, scommon_class);

    if (!c) c = (t_scommon *)scommon_new(s);
    c->c_refcount++;
    return (c);
}

/* release a variable.  This only frees the "scommon" resource when the
last interested party releases it. */
void slist_release(t_scommon *c)
{
    if (!--c->c_refcount) scommon_ff(c);
}


static void *slist_new(t_symbol *s)
{
    t_slist *x = (t_slist *)pd_new(slist_class);
    x->x_sym = s;
    x->x_c = slist_get(s);
    outlet_new(&x->x_obj, &s_float);
    x->x_symout=outlet_new(&x->x_obj, &s_symbol);
    x->x_lenout=outlet_new(&x->x_obj, &s_float);
    return (x);
}

static void slist_ff(t_slist *x)
{
    slist_release(x->x_c);
}

static void slist_print(t_slist *x)
{
    t_sitem *t=x->x_c->first;
    int i=0;

    while(t)
    {
        post("item %d: %s",++i,t->s->s_name);
        t=t->next;
    }
}

static void slist_reset(t_slist *x)
{
    scommon_reset(x->x_c);
}

static void slist_add(t_slist *x,t_symbol *s)
{
    scommon_add(x->x_c,s);
}

static void slist_find(t_slist *x,t_symbol *s)
{
    outlet_float(x->x_obj.ob_outlet,scommon_find(x->x_c,s));
}

static void slist_setlist(t_slist *x,t_symbol *s)
{
    slist_release(x->x_c);
    x->x_c = slist_get(s);
}

static void slist_float(t_slist *x, t_float f)
{
    t_sitem *t=x->x_c->first;
    int i=0;

    if(!f) return;

    while(t&&((++i)!=f))
    {
        t=t->next;
    }

    if(t) outlet_symbol(x->x_symout,t->s);
}

static void slist_len(t_slist *x)
{
    t_sitem *t=x->x_c->first;
    int i=0;

    while(t)
    {
        t=t->next;
        i++;
    }

    outlet_float(x->x_lenout,i);
}


void slist_setup(void)
{
    slist_class = class_new(gensym("slist"), (t_newmethod)slist_new,
                            (t_method)slist_ff,
                            sizeof(t_slist), 0, A_DEFSYM, 0);

    //class_addbang(slist_class, slist_bang);
    class_addfloat(slist_class, slist_float);
    class_addmethod(slist_class,(t_method)slist_add, gensym("add"),A_SYMBOL,0);
    class_addmethod(slist_class,(t_method)slist_find, gensym("find"),A_SYMBOL,0);
    class_addmethod(slist_class,(t_method)slist_setlist, gensym("setlist"),A_SYMBOL,0);
    class_addmethod(slist_class,(t_method)slist_reset, gensym("reset"),0);
    class_addmethod(slist_class,(t_method)slist_print, gensym("print"),0);
    class_addmethod(slist_class,(t_method)slist_len, gensym("len"),0);
    scommon_class = class_new(gensym("slist"), 0, 0,
                              sizeof(t_scommon), CLASS_PD, 0);
}

