/* cd4001.c MP 20070312 */
/* Emulate a cd4001b */
#include "m_pd.h"

typedef struct _cd4001
{
    t_object        x_obj;
    t_int           x_in1;
    t_int           x_in2;
    t_outlet        *x_out;
    t_inlet         *x_inlet2;/* Second inlet is 'live' like the first */
} t_cd4001;

static t_class *cd4001_class;

void cd4001_setup(void);
static void *cd4001_new(t_symbol *s, int argc, t_atom *argv);
static void cd4001_free(t_cd4001 *x);
static void cd4001_bang(t_cd4001 *x);
static void cd4001_float(t_cd4001 *x, t_float f);
static void cd4001_inlet2(t_cd4001 *x, t_float f);
static void cd4001_update_outlets(t_cd4001 *x);

static void cd4001_float(t_cd4001 *x, t_float f)
{
    if (f == 1) x->x_in1 = 1;
    else if (f == 0) x->x_in1 = 0;
    else
    {
        post("cd4001 inlet 2 accepts 1 or 0.");
        return;
    }
    cd4001_update_outlets(x);
}

static void cd4001_bang(t_cd4001 *x)
{
    cd4001_update_outlets(x);
}

static void cd4001_inlet2(t_cd4001 *x, t_float f)
{
    if (f == 1) x->x_in2 = 1;
    else if (f == 0) x->x_in2 = 0;
    else
    {
        post("cd4001 inlet 2 accepts 1 or 0.");
        return;
    }
    cd4001_update_outlets(x);
}

static void cd4001_update_outlets(t_cd4001 *x)
{ /* Double NOR function */
    outlet_float(x->x_out, ((x->x_in1 + x->x_in2) != 0)?0:1);
}

static void cd4001_free(t_cd4001 *x)
{
    return;
}

static void *cd4001_new(t_symbol *s, int argc, t_atom *argv)
{
    t_cd4001           *x;

    x = (t_cd4001 *)pd_new(cd4001_class);
    if (x == NULL) return (x);
    x->x_out = outlet_new((t_object *)x, &s_float);
    x->x_inlet2 = inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_float, gensym("inlet2"));
    return (x);
}

void cd4001_setup(void)
{
    cd4001_class = class_new(gensym("cd4001"),
                    (t_newmethod)cd4001_new,
                    (t_method)cd4001_free,
                    sizeof(t_cd4001), 0, 0); /* no arguments */
    class_addbang(cd4001_class, cd4001_bang);
    class_addfloat(cd4001_class, cd4001_float);
    class_addmethod(cd4001_class, (t_method)cd4001_inlet2, gensym("inlet2"), A_FLOAT, 0);
}
/* end cd4001.c */

