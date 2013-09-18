/* tab2flist started 20090121 by mrpeach */
/* Message with two floats dumps a list of floats from a table at offset,length */
/* Bang dumps the entire table as a list of floats */
/* Floats at negative offsets will not be dumped. */

#include "m_pd.h"

#if (PD_MINOR_VERSION > 40)
#define USE_GETFLOATWORDS *//* if garray_getfloatwords is implemented */
#endif
/* garray_getfloatwords uses t_word but doesn't exist in some versions of pd */
/* garray_getfloatarray uses t_float but is not 64-bit */

static t_class *tab2flist_class;

typedef struct _tab2flist
{
    t_object    x_obj;
    t_outlet    *x_listout;
    t_symbol    *x_arrayname;
    t_float     x_offset;
    t_float     x_length;
    t_float     *x_delimiters;
    int         x_n_delimiters;
} t_tab2flist;

static void tab2flist_delimit(t_tab2flist *x, int *argc, t_atom *argv);
//static void tab2flist_list(t_tab2flist *x, t_symbol *s, int argc, t_atom *argv);
static void tab2flist_float(t_tab2flist *x, t_float f);
static void tab2flist_bang(t_tab2flist *x);
static void tab2flist_set(t_tab2flist *x, t_symbol *s);
static void tab2flist_delimiter(t_tab2flist *x, t_symbol *s, int argc, t_atom *argv);
static void tab2flist_free(t_tab2flist *x);
static void *tab2flist_new(t_symbol *s);
void tab2flist_setup(void);

static void tab2flist_bang(t_tab2flist *x)
{
    /* output a list of x_length floats from the table starting from x_offset */
    /* output zero for elements outside the table (e.g. negative x_offset) */
    /* if delimiters are specified, stop when encountering a delimiter */

    t_garray    *a;
    int         n, i, tabpoints, listpoints;
    int         listsize;
    t_atom      *atomlist = NULL;
#ifdef USE_GETFLOATWORDS
    t_word      *vec;
#else
    t_float     *vec;
#endif

    /* Read in the array */
    if (!(a = (t_garray *)pd_findbyclass(x->x_arrayname, garray_class)))
        pd_error(x, "tab2flist_list: %s: no such array", x->x_arrayname->s_name);
#ifdef USE_GETFLOATWORDS
    else if (!garray_getfloatwords(a, &tabpoints, &vec))
#else
    else if (!garray_getfloatarray(a, &tabpoints, &vec))
#endif
        pd_error(x, "tab2flist_list: %s: bad template", x->x_arrayname->s_name);
    else if (x->x_length != 0)
    {
        if (x->x_length > 0) listpoints = x->x_length;
        else listsize = tabpoints; /* when x_length < 0, output the whole table */
        listsize = listpoints * sizeof(t_atom);
        atomlist = getbytes(listsize);
        if (atomlist == NULL)
        {
            pd_error(x, "tab2flist_list: unable to allocate %lu bytes for list", listsize);
            return;
        }
        else 
        {
            for (n = x->x_offset, i = 0; i < listpoints; ++n)
            {
                if ((n >= 0) && (n < tabpoints))
                {
#ifdef USE_GETFLOATWORDS
                    SETFLOAT(&atomlist[i], vec[n].w_float);
#else
                    SETFLOAT(&atomlist[i], vec[n]);
#endif
                }
                else
                {
#ifdef USE_GETFLOATWORDS
                    SETFLOAT(&atomlist[i], 0);
#else
                    SETFLOAT(&atomlist[i], 0);
#endif
                }
                i++;
            }
            if (x->x_n_delimiters > 0) tab2flist_delimit(x, &listpoints, atomlist);
            outlet_list(x->x_listout, &s_list, listpoints, atomlist);
            freebytes(atomlist, listsize);
        }
    }
}

static void tab2flist_delimit(t_tab2flist *x, int *argc, t_atom *argv)
{
    /* for each element in argv, check if it matches a delimiter */
    /* if so, truncate the list and set the new length */
    int i,j, n = *argc;
    t_float f;

    for (i = 0; i < n; ++i)
    {
        f = atom_getfloat(&argv[i]);
        for (j = 0; j < x->x_n_delimiters; ++j)
        {
            if (f == x->x_delimiters[j])
            {
                *argc = i;
                return;
            }
        }
    }
    
}
#ifdef NOWAY

static void tab2flist_list(t_tab2flist *x, t_symbol *s, int argc, t_atom *argv)
{
    int i;
    /* expect a list of 2 floats for offset and packet length */

    /* Check the incoming list for length = 2 */
    if (argc != 2)
    {
        pd_error(x, "tab2flist_list: list must contain exactly two floats (offset, length)");
        return;
    }

    /* Check the incoming list for floatness... */
    for (i = 0; i < 2; ++i)
    {
        if (argv[i].a_type != A_FLOAT)
        {
            pd_error(x, "tab2flist_list: list must contain exactly two floats (offset, length)");
            return;
        }
    }
    /* Read offset and packet size from incoming list */
    x->x_offset = argv[0].a_w.w_float;
    x->x_length = argv[1].a_w.w_float;

}
#endif 

static void tab2flist_float(t_tab2flist *x, t_float f)
{
    x->x_offset = f;
}

static void tab2flist_set(t_tab2flist *x, t_symbol *s)
{
    x->x_arrayname = s;
}

static void tab2flist_delimiter(t_tab2flist *x, t_symbol *s, int argc, t_atom *argv)
{
    int i;
    /* expect a list of at least one float for delimiter values */

    /* Check the incoming list for floatness... */
    for (i = 0; i < argc; ++i)
    {
        if (argv[i].a_type != A_FLOAT)
        {
            pd_error(x, "tab2flist_delimiter: list must contain only floats");
            return;
        }
    }
    /* allocate storage for the delimiters */
    if (x->x_delimiters == NULL) x->x_delimiters = getbytes(argc*sizeof(t_float));
    else
        x->x_delimiters = resizebytes(x->x_delimiters, x->x_n_delimiters*sizeof(t_float), argc*sizeof(t_float));
    if (x->x_delimiters == NULL)
    {
        pd_error(x, "tab2flist_delimiter can't get %lu bytes for delimiters", argc*sizeof(t_float));
        x->x_n_delimiters = 0;
        return;
    }
    x->x_n_delimiters = argc;
    for (i = 0; i < argc; ++i)
       x->x_delimiters[i] = atom_getfloat(&argv[i]);
}

static void tab2flist_free(t_tab2flist *x)
{
    if (x->x_n_delimiters > 0) freebytes(x->x_delimiters, x->x_n_delimiters*sizeof(t_float));
}

static void *tab2flist_new(t_symbol *s)
{
    t_tab2flist *x = (t_tab2flist *)pd_new(tab2flist_class);
    x->x_listout = outlet_new(&x->x_obj, &s_list);
    floatinlet_new(&x->x_obj, &x->x_length);
    x->x_offset = x->x_n_delimiters = 0;
    x->x_delimiters = NULL;
    x->x_length = -1; /* default to output the whole table */
    x->x_arrayname = s;
    return (x);
}

void tab2flist_setup(void)
{
    tab2flist_class = class_new(gensym("tab2flist"), (t_newmethod)tab2flist_new,
        (t_method) tab2flist_free, sizeof(t_tab2flist), 0, A_DEFSYM, 0);
    class_addbang(tab2flist_class, (t_method)tab2flist_bang);
//    class_addlist(tab2flist_class, (t_method)tab2flist_list);
    class_addfloat(tab2flist_class, (t_method)tab2flist_float);
    class_addmethod(tab2flist_class, (t_method)tab2flist_set, gensym("set"),
        A_SYMBOL, 0);
    class_addmethod(tab2flist_class, (t_method)tab2flist_delimiter, gensym("delimiter"),
        A_GIMME, 0);
}

