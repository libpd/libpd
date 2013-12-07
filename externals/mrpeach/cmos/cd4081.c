/* cd4081.c MP 20070308 */
/* Emulate a cd4081b */
#include "m_pd.h"

typedef struct _cd4081
{
    t_object        x_obj;
    t_int           x_in1;
    t_int           x_in2;
    t_outlet        *x_out;
    t_inlet         *x_inlet2;/* Second inlet is 'live' like the first */
} t_cd4081;

static t_class *cd4081_class;

void cd4081_setup(void);
static void *cd4081_new(t_symbol *s, int argc, t_atom *argv);
static void cd4081_free(t_cd4081 *x);
static void cd4081_bang(t_cd4081 *x);
static void cd4081_float(t_cd4081 *x, t_float f);
static void cd4081_inlet2(t_cd4081 *x, t_float f);
static void cd4081_update_outlets(t_cd4081 *x);

static void cd4081_float(t_cd4081 *x, t_float f)
{
    if (f == 1) x->x_in1 = 1;
    else if (f == 0) x->x_in1 = 0;
    else
    {
        post("cd4081 inlet 2 accepts 1 or 0.");
        return;
    }
    cd4081_update_outlets(x);
}

static void cd4081_bang(t_cd4081 *x)
{
    cd4081_update_outlets(x);
}

static void cd4081_inlet2(t_cd4081 *x, t_float f)
{
    if (f == 1) x->x_in2 = 1;
    else if (f == 0) x->x_in2 = 0;
    else
    {
        post("cd4081 inlet 2 accepts 1 or 0.");
        return;
    }
    cd4081_update_outlets(x);
}

static void cd4081_update_outlets(t_cd4081 *x)
{ /* AND function */
    outlet_float(x->x_out, ((x->x_in1 + x->x_in2) == 2)?1:0);
}

static void cd4081_free(t_cd4081 *x)
{
    return;
}

static void *cd4081_new(t_symbol *s, int argc, t_atom *argv)
{
    t_cd4081           *x;

    x = (t_cd4081 *)pd_new(cd4081_class);
    if (x == NULL) return (x);
    x->x_out = outlet_new((t_object *)x, &s_float);
    x->x_inlet2 = inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_float, gensym("inlet2"));
    return (x);
}

void cd4081_setup(void)
{
    cd4081_class = class_new(gensym("cd4081"),
                    (t_newmethod)cd4081_new,
                    (t_method)cd4081_free,
                    sizeof(t_cd4081), 0, 0); /* no arguments */
    class_addbang(cd4081_class, cd4081_bang);
    class_addfloat(cd4081_class, cd4081_float);
    class_addmethod(cd4081_class, (t_method)cd4081_inlet2, gensym("inlet2"), A_FLOAT, 0);
}
/* end cd4081.c */

