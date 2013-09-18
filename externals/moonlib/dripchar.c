/*
Copyright (C) 2002 Antoine Rousseau

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

/*#include <m_imp.h>*/
#include "m_pd.h"

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

typedef struct _dripchar
{
    t_object x_obj;
} t_dripchar;

t_class *dripchar_class;

void dripchar_setup(void);

static void dripchar_symbol(t_dripchar *x,t_symbol *sym)
{
    static t_binbuf *binbuf=0;
    t_atom at;
    char *c,s[2]= {0};
    int l;

    if(!binbuf) binbuf=binbuf_new();
    /*post("dripchar_symbol");*/
    if(!sym->s_name) return;

    c=sym->s_name;
    while(*c)
    {
        s[0]=*c++;
        SETSYMBOL(&at,gensym(s));
        binbuf_add(binbuf,1,&at);
    }


    outlet_list(x->x_obj.ob_outlet,0,
                binbuf_getnatom(binbuf),binbuf_getvec(binbuf));
    binbuf_clear(binbuf);
}

static void dripchar_float(t_dripchar *x,t_floatarg f)
{
    post("dripchar_float");
    /*outlet_symbol(x->x_obj.ob_outlet,*/
}

static void *dripchar_new(void)
{
    t_dripchar *x = (t_dripchar *)pd_new(dripchar_class);
    outlet_new(&x->x_obj, 0);
    return (void *)x;
}

void dripchar_setup(void)
{
    dripchar_class = class_new(gensym("dripchar"),(t_newmethod)dripchar_new,
                               0, sizeof(t_dripchar), 0, 0);

    class_addsymbol(dripchar_class, dripchar_symbol);
    class_addfloat(dripchar_class, dripchar_float);
}

