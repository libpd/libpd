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

/* a shared symbol array, ala "value" .*/

#include "m_pd.h"
#include <stdlib.h>
#include <string.h>

static t_class *sarray_class, *scommon_class;
static t_symbol *s__;

typedef struct scommon
{
    t_pd c_pd;
    t_symbol *c_sym;
    int c_refcount;
    int c_len;
    t_symbol **c_array;
} t_scommon;

typedef struct _sarray
{
    t_object x_obj;
    t_symbol *x_sym;
    t_scommon *x_c;
    //t_outlet *x_symout;
    //t_outlet *x_lenout;
} t_sarray;

/*static int scommon_find(t_scommon *x, t_symbol *s)
{
	t_sitem *si=x->first;
	t_int i=1;

	while(si) {
		if(!strcmp(si->s->s_name,s->s_name)) return i;
		si=si->next;
		i++;
	}
	return 0;
}*/

static t_symbol *scommon_get(t_scommon *x, t_float f)
{
    int i=(int)f;

    if(i<0) i=0;
    if(i>=x->c_len) i=x->c_len-1;
    return x->c_array[i];
}

static void scommon_set(t_scommon *x, t_float f, t_symbol *s)
{
    int i=(int)f;

    if((i<0)||(i>=x->c_len)) return;
    x->c_array[i]=s;
    //x->c_array[i]=gensym(strdup(s->s_name));
    //x->c_array[i]=gensym("ok");
}

static void scommon_setlen(t_scommon *x, t_float flen)
{
    int i,oldlen=x->c_len,len=flen;

    if(len<1) len=1;
    x->c_len=len;

    x->c_array=realloc(x->c_array,sizeof(t_symbol *)*len);

    if(len>oldlen) for(i=oldlen; i<len; i++) x->c_array[i]=&s_;
}

static void scommon_reset(t_scommon *x)
{
    int i;

    for(i=0; i<x->c_len; i++) x->c_array[i]=&s_;
}

static t_atom *scommon_dump(t_scommon *x,t_symbol *s)
{
    int i;
    t_atom *atombuf;

    atombuf = (t_atom *)getbytes(sizeof(t_atom)*x->c_len);

    for(i=0; i<x->c_len; i++)
    {
        if(x->c_array[i]==&s_) SETSYMBOL(&atombuf[i],s);
        else SETSYMBOL(&atombuf[i],x->c_array[i]);
    }

    return atombuf;
}

static void scommon_ff(t_scommon *x)
{
    //scommon_reset(x);
    pd_unbind((t_pd *)x, x->c_sym);
}

static void *scommon_new(t_symbol *s)
{
    t_scommon *x = (t_scommon *)pd_new(scommon_class);

    x->c_refcount = 0;
    x->c_sym=s;
    pd_bind((t_pd *)x, s);

    x->c_len=1;
    x->c_array=malloc(sizeof(t_symbol *)*1);
    scommon_reset(x);

    return (x);
}


/* get a pointer to a named symbol list (a "scommon" object),
which is created if necessary. */
t_scommon *sarray_scget(t_symbol *s)
{
    t_scommon *c = (t_scommon *)pd_findbyclass(s, scommon_class);

    if (!c) c = (t_scommon *)scommon_new(s);
    c->c_refcount++;
    return (c);
}

/* release a variable.  This only frees the "scommon" resource when the
last interested party releases it. */
void sarray_release(t_scommon *c)
{
    if (!--c->c_refcount) scommon_ff(c);
}


static void *sarray_new(t_symbol *s,t_float len)
{
    t_sarray *x = (t_sarray *)pd_new(sarray_class);

    x->x_sym = s;
    x->x_c = sarray_scget(s);
    if(len) scommon_setlen(x->x_c,len);

    outlet_new(&x->x_obj, &s_anything);
    //x->x_symout=outlet_new(&x->x_obj, &s_anything);
    //x->x_lenout=outlet_new(&x->x_obj, &s_float);
    return (x);
}

static void sarray_ff(t_sarray *x)
{
    sarray_release(x->x_c);
}

static void sarray_print(t_sarray *x)
{
    int i;

    for(i=0; i<x->x_c->c_len; i++)
    {
        post("item %d: %s",i,x->x_c->c_array[i]->s_name);
    }
}

static void sarray_reset(t_sarray *x)
{
    scommon_reset(x->x_c);
}

static void sarray_set(t_sarray *x, t_symbol *sfoo,int argc, t_atom *argv)
{
    int i,j=0;
    t_symbol *snull=&s_,*s;

    /*if((argc<2)||(argv[0].a_type!=A_FLOAT))
    {
    	error("Bad arguments for message 'set' to object 'sarray'");
    	return ;
    }*/
    if(argv[0].a_type==A_SYMBOL)
    {
        snull=atom_getsymbol(&argv[0]);
        j=1;
    }

    if(argv[j].a_type!=A_FLOAT)
    {
        error("Bad arguments for message 'set' to object 'sarray'");
        return ;
    }

    i=atom_getfloat(&argv[j++]);

    while((j<argc)&&(argv[j].a_type==A_SYMBOL))
    {
        s=atom_getsymbol(&argv[j++]);
        if(s==snull) s=&s_;
        scommon_set(x->x_c,i++,s);
    }
}

static void sarray_get(t_sarray *x,t_float i)
{
    t_symbol *s=scommon_get(x->x_c,i);

    if(s==&s_) outlet_bang(x->x_obj.ob_outlet);
    else outlet_symbol(x->x_obj.ob_outlet,scommon_get(x->x_c,i));
}

static void sarray_dump(t_sarray *x,t_symbol *s)
{
    t_atom *l=scommon_dump(x->x_c,s);

    outlet_list(x->x_obj.ob_outlet, &s_list, x->x_c->c_len, l);
    free(l);
}

static void sarray_setarray(t_sarray *x,t_symbol *s)
{
    sarray_release(x->x_c);
    x->x_c = sarray_scget(s);
    x->x_sym = s;
}

static void sarray_setlen(t_sarray *x,t_float len)
{
    scommon_setlen(x->x_c,len);
}


void sarray_setup(void)
{
    s__=gensym("_");
    sarray_class = class_new(gensym("sarray"), (t_newmethod)sarray_new,
                             (t_method)sarray_ff,
                             sizeof(t_sarray), 0, A_DEFSYM, A_DEFFLOAT,0);

    //class_addbang(sarray_class, sarray_bang);
    //class_addfloat(sarray_class, sarray_float);
    class_addmethod(sarray_class,(t_method)sarray_set, gensym("set"),A_GIMME,0);
    class_addmethod(sarray_class,(t_method)sarray_get, gensym("get"),A_FLOAT,0);
    class_addmethod(sarray_class,(t_method)sarray_reset, gensym("reset"),0);
    class_addmethod(sarray_class,(t_method)sarray_print, gensym("print"),0);
    class_addmethod(sarray_class,(t_method)sarray_setlen, gensym("setlen"),A_FLOAT,0);
    class_addmethod(sarray_class,(t_method)sarray_dump, gensym("dump"),A_DEFSYM,0);
    class_addmethod(sarray_class,(t_method)sarray_setarray, gensym("setarray"),A_SYMBOL,0);
    scommon_class = class_new(gensym("sarray"), 0, 0,
                              sizeof(t_scommon), CLASS_PD, 0);
}

