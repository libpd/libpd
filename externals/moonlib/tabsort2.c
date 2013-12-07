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


/* ---------------- tabsort2 - sort a table to a table ----------------- */


typedef struct tabsort2
{
    t_object x_obj; 	    	    /* header */
    t_symbol *x_arrayname1;
    t_symbol *x_arrayname2;
    t_symbol *x_arrayname3;
    t_clock *x_clock;		    /* a "clock" object */
} t_tabsort2;

t_class *tabsort2_class;

static void tabsort2_tick(t_tabsort2 *x);

static void *tabsort2_new(t_symbol *tab1,t_symbol *tab2,t_symbol *tab3)
{
    t_tabsort2 *x;
    x = (t_tabsort2 *)pd_new(tabsort2_class);

    x->x_arrayname1 = tab1;
    x->x_arrayname2 = tab2;
    x->x_arrayname3 = tab3;
    x->x_clock = clock_new(x, (t_method)tabsort2_tick);
    outlet_new((t_object *)x, &s_float);

    return (x);
}

static void tabsort2_set1(t_tabsort2 *x, t_symbol *s)
{
    x->x_arrayname1 = s;
}
static void tabsort2_set2(t_tabsort2 *x, t_symbol *s)
{
    x->x_arrayname2 = s;
}

static void tabsort2_set3(t_tabsort2 *x, t_symbol *s)
{
    x->x_arrayname3 = s;
}

static void tabsort2_float(t_tabsort2 *x, t_floatarg n)
{
    t_garray *a;
    int n1,n2,n3,i,j,h,sqn;
    t_float *vec1,*vec2,*vec3,tmp;

    if (!(a = (t_garray *)pd_findbyclass(x->x_arrayname1, garray_class)))
    {
        if (*x->x_arrayname1->s_name) pd_error(x, "tabsort2: %s: no such array",
                                                   x->x_arrayname1->s_name);
        return;
    }
    else if (!garray_getfloatarray(a, &n1, &vec1))
    {
        error("%s: bad template for tabsort2", x->x_arrayname1->s_name);
        return;
    }

    if (!(a = (t_garray *)pd_findbyclass(x->x_arrayname2, garray_class)))
    {
        if (*x->x_arrayname2->s_name) pd_error(x, "tabsort2: %s: no such array",
                                                   x->x_arrayname2->s_name);
        return;
    }
    else if (!garray_getfloatarray(a, &n2, &vec2))
    {
        error("%s: bad template for tabsort2", x->x_arrayname2->s_name);
        return;
    }

    if (!(a = (t_garray *)pd_findbyclass(x->x_arrayname3, garray_class)))
    {
        if (*x->x_arrayname3->s_name) pd_error(x, "tabsort2: %s: no such array",
                                                   x->x_arrayname3->s_name);
        return;
    }
    else if (!garray_getfloatarray(a, &n3, &vec3))
    {
        error("%s: bad template for tabsort2", x->x_arrayname3->s_name);
        return;
    }

    if(n>n1) n=n1;
    if(n>n2) n=n2;
    if(n>n3) n=n3;

    for(i=0; i<n; vec3[i]=i++);

    for(i=0; i<n-1; i++)
        for(j=n-1; j>i; j--)
            if(vec1[(int)vec3[j-1]]<vec1[(int)vec3[j]])
            {
                tmp=vec3[j];
                vec3[j]=vec3[j-1];
                vec3[j-1]=tmp;
            }

    sqn=(int)sqrt(n);

    for(h=0; h<sqn; h++)
        for(i=0; i<sqn-1; i++)
            for(j=sqn-1; j>i; j--)
                if(vec2[(int)vec3[h*sqn+j-1]]<vec2[(int)vec3[h*sqn+j]])
                {
                    tmp=vec3[h*sqn+j];
                    vec3[h*sqn+j]=vec3[h*sqn+j-1];
                    vec3[h*sqn+j-1]=tmp;
                }

    garray_redraw(a);
    outlet_float(((t_object *)x)->ob_outlet,(t_float)sqn);
}

static void tabsort2_tick(t_tabsort2 *x)	/* callback function for the env clock */
{

    //clock_delay(x->x_clock, 0L);
}

static void tabsort2_ff(t_tabsort2 *x)		/* cleanup on free */
{
    clock_free(x->x_clock);
}


void tabsort2_setup(void )
{
    tabsort2_class = class_new(gensym("tabsort2"), (t_newmethod)tabsort2_new,
                               (t_method)tabsort2_ff, sizeof(t_tabsort2), 0, A_DEFSYM, A_DEFSYM, A_DEFSYM, 0);
    class_addmethod(tabsort2_class, (t_method)tabsort2_set1,
                    gensym("set1"), A_DEFSYM, 0);
    class_addmethod(tabsort2_class, (t_method)tabsort2_set2,
                    gensym("set2"), A_DEFSYM, 0);
    class_addfloat(tabsort2_class, tabsort2_float);

}

