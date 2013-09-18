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

#include "m_pd.h"
#include "math.h"

/* ---------- tabreadl: control, linear interpolating ------------------------ */

static t_class *tabreadl_class;

typedef struct _tabreadl
{
    t_object x_obj;
    t_symbol *x_arrayname;
} t_tabreadl;

static void tabreadl_float(t_tabreadl *x, t_float f)
{
    t_garray *a;
    int npoints;
    t_float *vec;

    if (!(a = (t_garray *)pd_findbyclass(x->x_arrayname, garray_class)))
        error("%s: no such array", x->x_arrayname->s_name);
    else if (!garray_getfloatarray(a, &npoints, &vec))
        error("%s: bad template for tabreadl", x->x_arrayname->s_name);
    else
    {
        int n ;
        float r,v;

        if (f < 0) f = 0;
        else if (f >= npoints) f = npoints - 1;
        n=f;
        if(npoints>1)
        {
            r=f-n;
            v=vec[n]*(1-r)+vec[n+1]*r;
            outlet_float(x->x_obj.ob_outlet, v );
        }
        else
            outlet_float(x->x_obj.ob_outlet, (npoints ? vec[n] : 0));
    }
}

static void tabreadl_set(t_tabreadl *x, t_symbol *s)
{
    x->x_arrayname = s;
}

static void *tabreadl_new(t_symbol *s)
{
    t_tabreadl *x = (t_tabreadl *)pd_new(tabreadl_class);
    x->x_arrayname = s;
    outlet_new(&x->x_obj, &s_float);
    return (x);
}

void tabreadl_setup(void)
{
    tabreadl_class = class_new(gensym("tabreadl"), (t_newmethod)tabreadl_new,
                               0, sizeof(t_tabreadl), 0, A_DEFSYM, 0);
    class_addfloat(tabreadl_class, (t_method)tabreadl_float);
    class_addmethod(tabreadl_class, (t_method)tabreadl_set, gensym("set"),
                    A_SYMBOL, 0);

}


