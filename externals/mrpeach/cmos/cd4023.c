/* cd4023.c MP 20070312 */
/* Emulate a cd4023b */
#include "m_pd.h"

typedef struct _cd4023
{
    t_object        x_obj;
    t_int           x_in1;
    t_int           x_in2;
    t_int           x_in3;
    t_outlet        *x_out;
    t_inlet         *x_inlet2;/* Second inlet is 'live' like the first */
    t_inlet         *x_inlet3;/* Third inlet is 'live' like the first */
} t_cd4023;

static t_class *cd4023_class;

void cd4023_setup(void);
static void *cd4023_new(t_symbol *s, int argc, t_atom *argv);
static void cd4023_free(t_cd4023 *x);
static void cd4023_bang(t_cd4023 *x);
static void cd4023_float(t_cd4023 *x, t_float f);
static void cd4023_inlet2(t_cd4023 *x, t_float f);
static void cd4023_inlet3(t_cd4023 *x, t_float f);
static void cd4023_update_outlets(t_cd4023 *x);

static void cd4023_float(t_cd4023 *x, t_float f)
{
    if (f == 1) x->x_in1 = 1;
    else if (f == 0) x->x_in1 = 0;
    else
    {
        post("cd4023 inlet 2 accepts 1 or 0.");
        return;
    }
    cd4023_update_outlets(x);
}

static void cd4023_bang(t_cd4023 *x)
{
    cd4023_update_outlets(x);
}

static void cd4023_inlet2(t_cd4023 *x, t_float f)
{
    if (f == 1) x->x_in2 = 1;
    else if (f == 0) x->x_in2 = 0;
    else
    {
        post("cd4023 inlet 2 accepts 1 or 0.");
        return;
    }
    cd4023_update_outlets(x);
}

static void cd4023_inlet3(t_cd4023 *x, t_float f)
{
    if (f == 1) x->x_in3 = 1;
    else if (f == 0) x->x_in3 = 0;
    else
    {
        post("cd4023 inlet 3 accepts 1 or 0.");
        return;
    }
    cd4023_update_outlets(x);
}

static void cd4023_update_outlets(t_cd4023 *x)
{ /* Triple NAND function */
    outlet_float(x->x_out, ((x->x_in1 + x->x_in2 + x->x_in3) == 3)?0:1);
}

static void cd4023_free(t_cd4023 *x)
{
    return;
}

static void *cd4023_new(t_symbol *s, int argc, t_atom *argv)
{
    t_cd4023           *x;

    x = (t_cd4023 *)pd_new(cd4023_class);
    if (x == NULL) return (x);
    x->x_out = outlet_new((t_object *)x, &s_float);
    x->x_inlet2 = inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_float, gensym("inlet2"));
    x->x_inlet3 = inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_float, gensym("inlet3"));
    return (x);
}

void cd4023_setup(void)
{
    cd4023_class = class_new(gensym("cd4023"),
                    (t_newmethod)cd4023_new,
                    (t_method)cd4023_free,
                    sizeof(t_cd4023), 0, 0); /* no arguments */
    class_addbang(cd4023_class, cd4023_bang);
    class_addfloat(cd4023_class, cd4023_float);
    class_addmethod(cd4023_class, (t_method)cd4023_inlet2, gensym("inlet2"), A_FLOAT, 0);
    class_addmethod(cd4023_class, (t_method)cd4023_inlet3, gensym("inlet3"), A_FLOAT, 0);
}
/* end cd4023.c */

