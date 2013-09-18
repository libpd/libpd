/* cd4073.c MP 20070312 */
/* Emulate a cd4073b */
#include "m_pd.h"

typedef struct _cd4073
{
    t_object        x_obj;
    t_int           x_in1;
    t_int           x_in2;
    t_int           x_in3;
    t_outlet        *x_out;
    t_inlet         *x_inlet2;/* Second inlet is 'live' like the first */
    t_inlet         *x_inlet3;/* Third inlet is 'live' like the first */
} t_cd4073;

static t_class *cd4073_class;

void cd4073_setup(void);
static void *cd4073_new(t_symbol *s, int argc, t_atom *argv);
static void cd4073_free(t_cd4073 *x);
static void cd4073_bang(t_cd4073 *x);
static void cd4073_float(t_cd4073 *x, t_float f);
static void cd4073_inlet2(t_cd4073 *x, t_float f);
static void cd4073_inlet3(t_cd4073 *x, t_float f);
static void cd4073_update_outlets(t_cd4073 *x);

static void cd4073_float(t_cd4073 *x, t_float f)
{
    if (f == 1) x->x_in1 = 1;
    else if (f == 0) x->x_in1 = 0;
    else
    {
        post("cd4073 inlet 2 accepts 1 or 0.");
        return;
    }
    cd4073_update_outlets(x);
}

static void cd4073_bang(t_cd4073 *x)
{
    cd4073_update_outlets(x);
}

static void cd4073_inlet2(t_cd4073 *x, t_float f)
{
    if (f == 1) x->x_in2 = 1;
    else if (f == 0) x->x_in2 = 0;
    else
    {
        post("cd4073 inlet 2 accepts 1 or 0.");
        return;
    }
    cd4073_update_outlets(x);
}

static void cd4073_inlet3(t_cd4073 *x, t_float f)
{
    if (f == 1) x->x_in3 = 1;
    else if (f == 0) x->x_in3 = 0;
    else
    {
        post("cd4073 inlet 3 accepts 1 or 0.");
        return;
    }
    cd4073_update_outlets(x);
}

static void cd4073_update_outlets(t_cd4073 *x)
{ /* Triple AND function */
    outlet_float(x->x_out, ((x->x_in1 + x->x_in2 + x->x_in3) == 3)?1:0);
}

static void cd4073_free(t_cd4073 *x)
{
    return;
}

static void *cd4073_new(t_symbol *s, int argc, t_atom *argv)
{
    t_cd4073           *x;

    x = (t_cd4073 *)pd_new(cd4073_class);
    if (x == NULL) return (x);
    x->x_out = outlet_new((t_object *)x, &s_float);
    x->x_inlet2 = inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_float, gensym("inlet2"));
    x->x_inlet3 = inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_float, gensym("inlet3"));
    return (x);
}

void cd4073_setup(void)
{
    cd4073_class = class_new(gensym("cd4073"),
                    (t_newmethod)cd4073_new,
                    (t_method)cd4073_free,
                    sizeof(t_cd4073), 0, 0); /* no arguments */
    class_addbang(cd4073_class, cd4073_bang);
    class_addfloat(cd4073_class, cd4073_float);
    class_addmethod(cd4073_class, (t_method)cd4073_inlet2, gensym("inlet2"), A_FLOAT, 0);
    class_addmethod(cd4073_class, (t_method)cd4073_inlet3, gensym("inlet3"), A_FLOAT, 0);
}
/* end cd4073.c */

