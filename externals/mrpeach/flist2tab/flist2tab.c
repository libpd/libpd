/* flist2tab started 20090121 by mrpeach */
/* loads table from a list of floats at offset specified by second inlet */
/* floats negative offsets will not be loaded. */
/* Table will be resized to fit all the floats at positive offsets */

#include "m_pd.h"

#if (PD_MINOR_VERSION > 40)
#define USE_GETFLOATWORDS *//* if garray_getfloatwords is implemented */
#endif
/* garray_getfloatwords uses t_word but doesn't exist in some versions of pd */
/* garray_getfloatarray uses t_float but is not 64-bit */

static t_class *flist2tab_class;

typedef struct _flist2tab
{
    t_object    x_obj;
    t_symbol    *x_arrayname;
    t_float     x_ft1;
    t_float     x_offset;
    t_outlet    *x_sizeout;
} t_flist2tab;

static void flist2tab_list(t_flist2tab *x, t_symbol *s, int argc, t_atom *argv);
static void flist2tab_float(t_flist2tab *x, t_float f);
static void flist2tab_set(t_flist2tab *x, t_symbol *s);
static void *flist2tab_new(t_symbol *s);
void flist2tab_setup(void);

static void flist2tab_list(t_flist2tab *x, t_symbol *s, int argc, t_atom *argv)
{
    t_garray    *a;
#ifdef USE_GETFLOATWORDS
    t_word      *vec;
#else
    t_float     *vec;
#endif
    int         npoints, newsize, i;
    int         offset = x->x_offset;

    /* first check the list for floatness... */
    for (i = 0; i < argc; ++i)
    {
        if (argv[i].a_type != A_FLOAT)
        {
            pd_error(x, "flist2tab_list: list must be all floats");
            return;
        }
    }

    /* then find the array again... */
    if (!(a = (t_garray *)pd_findbyclass(x->x_arrayname, garray_class)))
        pd_error(x, "flist2tab_list: %s: no such array", x->x_arrayname->s_name);
#ifdef USE_GETFLOATWORDS
    else if (!garray_getfloatwords(a, &npoints, &vec))
#else
    else if (!garray_getfloatarray(a, &npoints, &vec))
#endif
        pd_error(x, "fflist2tab_list: %s: bad template", x->x_arrayname->s_name);
    else
    /* put the list in a starting at offset */
    {
        newsize = offset + argc;
        if (newsize >= npoints)
        {
            garray_resize(a, newsize);
#ifdef USE_GETFLOATWORDS
            if (!garray_getfloatwords(a, &npoints, &vec))
#else
            if (!garray_getfloatarray(a, &npoints, &vec))
#endif
                pd_error(x, "fflist2tab_list: %s: bad template", x->x_arrayname->s_name);
        }
        for (i = 0; i < argc; ++i)
        {
            if (i+offset >= 0)
#ifdef USE_GETFLOATWORDS
                vec[i+offset].w_float = argv[i].a_w.w_float;
#else
                vec[i+offset] = argv[i].a_w.w_float;
#endif
        }
        /* output the size of the array */
        outlet_float(x->x_sizeout, npoints);
    }
}

static void flist2tab_float(t_flist2tab *x, t_float f)
{
    int         i, npoints, offset = x->x_offset;
    t_garray    *a;
#ifdef USE_GETFLOATWORDS
    t_word      *vec;
#else
    t_float     *vec;
#endif

    if (!(a = (t_garray *)pd_findbyclass(x->x_arrayname, garray_class)))
        pd_error(x, "flist2tab_float %s: no such array", x->x_arrayname->s_name);
#ifdef USE_GETFLOATWORDS
    else if (!garray_getfloatwords(a, &npoints, &vec))
#else
    else if (!garray_getfloatarray(a, &npoints, &vec))
#endif
        pd_error(x, "flist2tab_float %s: bad template", x->x_arrayname->s_name);
    else if (offset >= 0)
    {
        if (offset >= npoints)
        {
            garray_resize(a, offset+1);
#ifdef USE_GETFLOATWORDS
            if (!garray_getfloatwords(a, &npoints, &vec))
#else
            if (!garray_getfloatarray(a, &npoints, &vec))
#endif
                pd_error(x, "flist2tab_float %s: bad template", x->x_arrayname->s_name);
        }
#ifdef USE_GETFLOATWORDS
        vec[offset].w_float = f;
#else
        vec[offset] = f;
#endif
        garray_redraw(a);
        /* output the size of the array */
        outlet_float(x->x_sizeout, npoints);
    }
}

static void flist2tab_set(t_flist2tab *x, t_symbol *s)
{
    x->x_arrayname = s;
}

static void *flist2tab_new(t_symbol *s)
{
    t_flist2tab *x = (t_flist2tab *)pd_new(flist2tab_class);
    x->x_offset = 0;
    x->x_arrayname = s;
    x->x_sizeout = outlet_new(&x->x_obj, &s_float);
    floatinlet_new(&x->x_obj, &x->x_offset);
    return (x);
}

void flist2tab_setup(void)
{
    flist2tab_class = class_new(gensym("flist2tab"), (t_newmethod)flist2tab_new,
        0, sizeof(t_flist2tab), 0, A_DEFSYM, 0);
    class_addfloat(flist2tab_class, (t_method)flist2tab_float);
    class_addlist(flist2tab_class, (t_method)flist2tab_list);
    class_addmethod(flist2tab_class, (t_method)flist2tab_set, gensym("set"),
        A_SYMBOL, 0);
}

