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


/* ---------------- tabsort - sort a table to a table ----------------- */


typedef struct tabsort
{
    /*env part*/
    t_object x_obj; 	    	    /* header */
    t_symbol *x_arrayname1;
    t_symbol *x_arrayname2;
} t_tabsort;

t_class *tabsort_class;

static void *tabsort_new(t_symbol *tab1,t_symbol *tab2)
{
    t_tabsort *x;
    x = (t_tabsort *)pd_new(tabsort_class);

    x->x_arrayname1 = tab1;
    x->x_arrayname2 = tab2;
    outlet_new((t_object *)x, &s_bang);

    return (x);
}

static void tabsort_set1(t_tabsort *x, t_symbol *s)
{
    x->x_arrayname1 = s;
}
static void tabsort_set2(t_tabsort *x, t_symbol *s)
{
    x->x_arrayname2 = s;
}

static void tabsort_float(t_tabsort *x, t_floatarg n)
{
    t_garray *a;
    int n1,n2,i,j;
    t_float *vec1,*vec2,tmp;

    if (!(a = (t_garray *)pd_findbyclass(x->x_arrayname1, garray_class)))
    {
        if (*x->x_arrayname1->s_name) pd_error(x, "tabsort: %s: no such array",
                                                   x->x_arrayname1->s_name);
        return;
    }
    else if (!garray_getfloatarray(a, &n1, &vec1))
    {
        error("%s: bad template for tabsort", x->x_arrayname1->s_name);
        return;
    }

    if (!(a = (t_garray *)pd_findbyclass(x->x_arrayname2, garray_class)))
    {
        if (*x->x_arrayname2->s_name) pd_error(x, "tabsort: %s: no such array",
                                                   x->x_arrayname2->s_name);
        return;
    }
    else if (!garray_getfloatarray(a, &n2, &vec2))
    {
        error("%s: bad template for tabsort", x->x_arrayname2->s_name);
        return;
    }

    if(n>n1) n=n1;
    if(n>n2) n=n2;


    for(i=0; i<n; vec2[i]=i++);

    for(i=0; i<n-1; i++)
        for(j=n-1; j>i; j--)
            if(vec1[(int)vec2[j-1]]<vec1[(int)vec2[j]])
            {
                tmp=vec2[j];
                vec2[j]=vec2[j-1];
                vec2[j-1]=tmp;
            }

    garray_redraw(a);
    outlet_bang(((t_object *)x)->ob_outlet);
}


static void tabsort_ff(t_tabsort *x)		/* cleanup on free */
{
}


void tabsort_setup(void )
{
    tabsort_class = class_new(gensym("tabsort"), (t_newmethod)tabsort_new,
                              (t_method)tabsort_ff, sizeof(t_tabsort), 0, A_DEFSYM, A_DEFSYM, 0);
    class_addmethod(tabsort_class, (t_method)tabsort_set1,
                    gensym("set1"), A_DEFSYM, 0);
    class_addmethod(tabsort_class, (t_method)tabsort_set2,
                    gensym("set2"), A_DEFSYM, 0);
    class_addfloat(tabsort_class, tabsort_float);

}

