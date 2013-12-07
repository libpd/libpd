/* ---------- tabfind: return the index of the input (float or list of floats)in the given table */
/* based on tabread in d_array.c */
/* Started 20081120 by Martin Peach (mrpeach) */

#include "m_pd.h"

#if (PD_MINOR_VERSION > 40)
#define USE_GETFLOATWORDS *//* if garray_getfloatwords is implemented */
#endif
/* garray_getfloatwords uses t_word but doesn't exist in some versions of pd */
/* garray_getfloatarray uses t_float but is not 64-bit */

static t_class *tabfind_class;

typedef struct _tabfind
{
    t_object    x_obj;
    t_symbol    *x_arrayname;
    t_int       x_nth; /* which match to find */
} t_tabfind;

static void tabfind_bang(t_tabfind *x);
static void tabfind_float(t_tabfind *x, t_float f);
static void tabfind_list(t_tabfind *x, t_symbol *s, int argc, t_atom *argv);
static void tabfind_nth(t_tabfind *x, t_float nth);
static void tabfind_set(t_tabfind *x, t_symbol *s);
static void *tabfind_new(t_symbol *s);
void tabfind_setup(void);

static void tabfind_nth(t_tabfind *x, t_float fnth)
{
/* set to find the nth instance of the key */
    int inth;
    if (fnth >= 1) inth = (int)fnth;
    else
    {
        inth = 1;
        pd_error(x, "tabfind: nth must be at least 1");
    }
    x->x_nth = inth;
}

static void tabfind_list(t_tabfind *x, t_symbol *s, int argc, t_atom *argv)
{
    /* find the nth occurrence of the list of floats argv in the array */
    t_garray    *a;
    int         npoints;
#ifdef USE_GETFLOATWORDS
    t_word      *vec;
#else
    t_float     *vec;
#endif
    int         n, count = 0;
    int         i, j;

    /* first check the list for floatness... */
    for (i = 0; i < argc; ++i)
    {
        if (argv[i].a_type != A_FLOAT)
        {
            pd_error(x, "tabfind: list must be all floats");
            return;
        }
    }

    /* then find the array again... */
    if (!(a = (t_garray *)pd_findbyclass(x->x_arrayname, garray_class)))
        pd_error(x, "tabfind: %s: no such array", x->x_arrayname->s_name);
#ifdef USE_GETFLOATWORDS
    else if (!garray_getfloatwords(a, &npoints, &vec))
#else
    else if (!garray_getfloatarray(a, &npoints, &vec))
#endif
        pd_error(x, "tabfind: %s: bad template for tabread", x->x_arrayname->s_name);
    else
    /* try to find the nth instance of the list in a and output its index */
    {
        for (n = 0; n < npoints; ++n)
        {
            for (i = 0; ((i < argc) && ((n+i) < npoints)); ++i)
#ifdef USE_GETFLOATWORDS
                if (vec[n+i].w_float != argv[i].a_w.w_float) break;
#else
                if (vec[n+i] != argv[i].a_w.w_float) break;
#endif
            if ((i == argc) && (x->x_nth == ++count)) break;
        }
        outlet_float(x->x_obj.ob_outlet, n);
    }
}

static void tabfind_bang(t_tabfind *x)
{
}

static void tabfind_float(t_tabfind *x, t_float f)
{
    /* find the nth occurrence of the float f in the array */
    t_garray    *a;
    int         npoints;
#ifdef USE_GETFLOATWORDS
    t_word      *vec;
#else
    t_float     *vec;
#endif
    int         n, count = 0;

    if (!(a = (t_garray *)pd_findbyclass(x->x_arrayname, garray_class)))
        pd_error(x, "tabfind: %s: no such array", x->x_arrayname->s_name);
#ifdef USE_GETFLOATWORDS
    else if (!garray_getfloatwords(a, &npoints, &vec))
#else
    else if (!garray_getfloatarray(a, &npoints, &vec))
#endif
        pd_error(x, "tabfind: %s: bad template for tabread", x->x_arrayname->s_name);
    else
    /* find the nth instance of f in a and output its index */
    {
        for (n = 0; n < npoints; ++n)
        {
#ifdef USE_GETFLOATWORDS
            if (vec[n].w_float == f)
#else
            if (vec[n] == f)
#endif
                if (x->x_nth == ++count) break;
        }
        outlet_float(x->x_obj.ob_outlet, n);
    }
}

static void tabfind_set(t_tabfind *x, t_symbol *s)
{
    /* set the name of the array we're working on */
    x->x_arrayname = s;
}

static void *tabfind_new(t_symbol *s)
{
    t_tabfind *x = (t_tabfind *)pd_new(tabfind_class);
    x->x_arrayname = s;
    outlet_new(&x->x_obj, &s_float);
    x->x_nth = 1;
    return (x);
}

void tabfind_setup(void)
{
    tabfind_class = class_new(gensym("tabfind"), (t_newmethod)tabfind_new,
        0, sizeof(t_tabfind), 0, A_DEFSYM, 0);
    class_addbang(tabfind_class, (t_method)tabfind_bang);
    class_addfloat(tabfind_class, (t_method)tabfind_float);
    class_addlist(tabfind_class, (t_method)tabfind_list); 
    class_addmethod(tabfind_class, (t_method)tabfind_nth, gensym("nth"), A_FLOAT, 0);
    class_addmethod(tabfind_class, (t_method)tabfind_set, gensym("set"), A_SYMBOL, 0);
}
