/* cd4070.c MP 20070305 */
/* Emulate a cd4070b */
#include "m_pd.h"

typedef struct _cd4070
{
    t_object        x_obj;
    t_int           x_in1;
    t_int           x_in2;
    t_outlet        *x_out;
    t_inlet         *x_inlet2;/* Second inlet is 'live' like the first */
} t_cd4070;

static t_class *cd4070_class;

void cd4070_setup(void);
static void *cd4070_new(t_symbol *s, int argc, t_atom *argv);
static void cd4070_free(t_cd4070 *x);
static void cd4070_bang(t_cd4070 *x);
static void cd4070_float(t_cd4070 *x, t_float f);
static void cd4070_inlet2(t_cd4070 *x, t_float f);
static void cd4070_update_outlets(t_cd4070 *x);

static void cd4070_float(t_cd4070 *x, t_float f)
{
    if (f == 1) x->x_in1 = 1;
    else if (f == 0) x->x_in1 = 0;
    else
    {
        post("cd4070 inlet 2 accepts 1 or 0.");
        return;
    }
    cd4070_update_outlets(x);
}

static void cd4070_bang(t_cd4070 *x)
{
    cd4070_update_outlets(x);
}

static void cd4070_inlet2(t_cd4070 *x, t_float f)
{
    if (f == 1) x->x_in2 = 1;
    else if (f == 0) x->x_in2 = 0;
    else
    {
        post("cd4070 inlet 2 accepts 1 or 0.");
        return;
    }
    cd4070_update_outlets(x);
}

static void cd4070_update_outlets(t_cd4070 *x)
{
    outlet_float(x->x_out, (x->x_in1 != x->x_in2)?1:0);
}

static void cd4070_free(t_cd4070 *x)
{
    return;
}

static void *cd4070_new(t_symbol *s, int argc, t_atom *argv)
{
    t_cd4070           *x;

    x = (t_cd4070 *)pd_new(cd4070_class);
    if (x == NULL) return (x);
    x->x_out = outlet_new((t_object *)x, &s_float);
    x->x_inlet2 = inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_float, gensym("inlet2"));
    return (x);
}

void cd4070_setup(void)
{
    cd4070_class = class_new(gensym("cd4070"),
                    (t_newmethod)cd4070_new,
                    (t_method)cd4070_free,
                    sizeof(t_cd4070), 0, 0); /* no arguments */
    class_addbang(cd4070_class, cd4070_bang);
    class_addfloat(cd4070_class, cd4070_float);
    class_addmethod(cd4070_class, (t_method)cd4070_inlet2, gensym("inlet2"), A_FLOAT, 0);
}
/* end cd4070.c */

