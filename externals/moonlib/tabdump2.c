/*
Copyright (C) 2004 Antoine Rousseau

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

/*
	tabdump2 -- from tabdump (from zexy) but with min/max dumping limits.
*/

#include <m_pd.h>

static t_class *tabdump_class;

typedef struct _tabdump
{
    t_object x_obj;
    t_symbol *x_arrayname;
} t_tabdump;

static void tabdump_bang(t_tabdump *x, t_float findex)
{
    t_garray *A;
    int npoints;
    t_float *vec;

    if (!(A = (t_garray *)pd_findbyclass(x->x_arrayname, garray_class)))
        error("%s: no such array", x->x_arrayname->s_name);
    else if (!garray_getfloatarray(A, &npoints, &vec))
        error("%s: bad template for tabdump", x->x_arrayname->s_name);
    else
    {
        int n;
        t_atom *atombuf = (t_atom *)getbytes(sizeof(t_atom)*npoints);

        for (n = 0; n < npoints; n++) SETFLOAT(&atombuf[n], vec[n]);
        outlet_list(x->x_obj.ob_outlet, &s_list, npoints, atombuf);
    }
}


static void tabdump_dump(t_tabdump *x, t_float min, t_float max)
{
    t_garray *A;
    int npoints,nmin=(int)min,nmax=(int)max;
    t_float *vec;

    if (!(A = (t_garray *)pd_findbyclass(x->x_arrayname, garray_class)))
        error("%s: no such array", x->x_arrayname->s_name);
    else if (!garray_getfloatarray(A, &npoints, &vec))
        error("%s: bad template for tabdump", x->x_arrayname->s_name);
    else if ((min<0)||(max<=min)||(max>npoints))
        error("tabdump: bad arguments min=%d max=%d for %s (%d elements)",
              nmin,nmax,x->x_arrayname->s_name,npoints);
    else
    {
        int n;
        t_atom *atombuf;

        npoints=nmax-nmin;
        atombuf = (t_atom *)getbytes(sizeof(t_atom)*npoints);

        for (n = 0; n < npoints; n++) SETFLOAT(&atombuf[n], vec[n+nmin]);
        outlet_list(x->x_obj.ob_outlet, &s_list, npoints, atombuf);
    }
}


static void tabdump_set(t_tabdump *x, t_symbol *s)
{
    x->x_arrayname = s;
}

static void *tabdump_new(t_symbol *s)
{
    t_tabdump *x = (t_tabdump *)pd_new(tabdump_class);
    x->x_arrayname = s;
    outlet_new(&x->x_obj, &s_list);

    return (x);
}

static void tabdump_helper(void)
{
    post("\n tabdump2 - object : dumps a table as a package of floats");
    post("'set <table>'\t: read out another table\n"
         "'bang'\t\t: dump the table\n"
         "'dump <min> <max>'\t\t: dump the table from <min> to <max> (without <max>)\n"
         "outlet\t\t: table-data as package of floats");
    post("creation\t: \"tabdump2 <table>\"");

}

void tabdump2_setup(void)
{
    tabdump_class = class_new(gensym("tabdump2"), (t_newmethod)tabdump_new,
                              0, sizeof(t_tabdump), 0, A_DEFSYM, 0);
    class_addbang(tabdump_class, (t_method)tabdump_bang);
    class_addmethod(tabdump_class, (t_method)tabdump_dump,gensym("dump"),
                    A_FLOAT,A_FLOAT,0);
    class_addmethod(tabdump_class, (t_method)tabdump_set, gensym("set"),
                    A_SYMBOL, 0);

    class_addmethod(tabdump_class, (t_method)tabdump_helper, gensym("help"), 0);

}

