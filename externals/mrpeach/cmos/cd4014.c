/* cd4014.c MP 20070315 */
/* Emulate a cd4014b */
#include "m_pd.h"

typedef struct _cd4014
{
    t_object        x_obj;
    t_int           x_P1;
    t_int           x_P2;
    t_int           x_P3;
    t_int           x_P4;
    t_int           x_P5;
    t_int           x_P6;
    t_int           x_P7;
    t_int           x_P8;
    t_int           x_SerialIn;
    t_int           x_Clk;
    t_int           x_Q;
    t_int           x_ParallelSerial;
    t_outlet        *x_outQ6;
    t_outlet        *x_outQ7;
    t_outlet        *x_outQ8;
    t_inlet         *x_inP1;/* extra inlets are 'live' like the first */
    t_inlet         *x_inP2;
    t_inlet         *x_inP3;
    t_inlet         *x_inP4;
    t_inlet         *x_inP5;
    t_inlet         *x_inP6;
    t_inlet         *x_inP7;
    t_inlet         *x_inP8;
    t_inlet         *x_inParallelSerial;
    t_inlet         *x_inSerial;
} t_cd4014;

static t_class *cd4014_class;

void cd4014_setup(void);
static void *cd4014_new(t_symbol *s, int argc, t_atom *argv);
static void cd4014_free(t_cd4014 *x);
static void cd4014_float(t_cd4014 *x, t_float f);
static void cd4014_bang(t_cd4014 *x);
static void cd4014_inP1(t_cd4014 *x, t_float f);
static void cd4014_inP2(t_cd4014 *x, t_float f);
static void cd4014_inP3(t_cd4014 *x, t_float f);
static void cd4014_inP4(t_cd4014 *x, t_float f);
static void cd4014_inP5(t_cd4014 *x, t_float f);
static void cd4014_inP6(t_cd4014 *x, t_float f);
static void cd4014_inP7(t_cd4014 *x, t_float f);
static void cd4014_inP8(t_cd4014 *x, t_float f);
static void cd4014_inParallelSerial(t_cd4014 *x, t_float f);
static void cd4014_inSerial(t_cd4014 *x, t_float f);
static void cd4014_update_outlets(t_cd4014 *x);

static void cd4014_float(t_cd4014 *x, t_float f)
{
    if (f == 1)
    {
        if (x->x_Clk == 0)
        {
            x->x_Clk = 1;
            cd4014_bang(x);
        }
    }
    else if (f == 0) x->x_Clk = 0;
    else
    {
        post("cd4014 Clock inlet accepts 1 or 0.");
        return;
    }
}

static void cd4014_bang(t_cd4014 *x)
{
    if (x->x_ParallelSerial == 0)
    { /* shift left by one */
        x->x_Q <<= 1;
        x->x_Q |= x->x_SerialIn;
    }
    else
    { /* parallel load */
        x->x_Q = 128*x->x_P8 + 64*x->x_P7 + 32*x->x_P6 + 16*x->x_P5 + 8*x->x_P4 + 4*x->x_P3 + 2*x->x_P2 + x->x_P1;
    }
    cd4014_update_outlets(x);
}

static void cd4014_inP1(t_cd4014 *x, t_float f)
{
    if (f == 1) x->x_P1 = 1;
    else if (f == 0) x->x_P1 = 0;
    else
    {
        post("cd4014 inlet P1 accepts 1 or 0.");
        return;
    }
}

static void cd4014_inP2(t_cd4014 *x, t_float f)
{
    if (f == 1) x->x_P2 = 2;
    else if (f == 0) x->x_P2 = 0;
    else
    {
        post("cd4014 inlet P2 accepts 1 or 0.");
        return;
    }
}

static void cd4014_inP3(t_cd4014 *x, t_float f)
{
    if (f == 1) x->x_P3 = 1;
    else if (f == 0) x->x_P3 = 0;
    else
    {
        post("cd4014 inlet P3 accepts 1 or 0.");
        return;
    }
}

static void cd4014_inP4(t_cd4014 *x, t_float f)
{
    if (f == 1) x->x_P4 = 1;
    else if (f == 0) x->x_P4 = 0;
    else
    {
        post("cd4014 inlet P4 accepts 1 or 0.");
        return;
    }
}

static void cd4014_inP5(t_cd4014 *x, t_float f)
{
    if (f == 1) x->x_P5 = 1;
    else if (f == 0) x->x_P5 = 0;
    else
    {
        post("cd4014 inlet P5 accepts 1 or 0.");
        return;
    }
}

static void cd4014_inP6(t_cd4014 *x, t_float f)
{
    if (f == 1) x->x_P6 = 1;
    else if (f == 0) x->x_P6 = 0;
    else
    {
        post("cd4014 inlet P6 accepts 1 or 0.");
        return;
    }
}

static void cd4014_inP7(t_cd4014 *x, t_float f)
{
    if (f == 1) x->x_P7 = 1;
    else if (f == 0) x->x_P7 = 0;
    else
    {
        post("cd4014 inlet P7 accepts 1 or 0.");
        return;
    }
}

static void cd4014_inP8(t_cd4014 *x, t_float f)
{
    if (f == 1) x->x_P8 = 1;
    else if (f == 0) x->x_P8 = 0;
    else
    {
        post("cd4014 inlet P8 accepts 1 or 0.");
        return;
    }
}

static void cd4014_inParallelSerial(t_cd4014 *x, t_float f)
{
    if (f == 1) x->x_ParallelSerial = 1;
    else if (f == 0) x->x_ParallelSerial = 0;
    else
    {
        post("cd4014 inlet ParallelSerial accepts 1 or 0.");
        return;
    }
}

static void cd4014_inSerial(t_cd4014 *x, t_float f)
{
    if (f == 1) x->x_SerialIn = 1;
    else if (f == 0) x->x_SerialIn = 0;
    else
    {
        post("cd4014 inlet Serial accepts 1 or 0.");
        return;
    }
}

static void cd4014_update_outlets(t_cd4014 *x)
{ /* Output Q8, Q7, Q6 */

    outlet_float(x->x_outQ8, ((x->x_Q & 128) != 0)?1:0);
    outlet_float(x->x_outQ7, ((x->x_Q & 64) != 0)?1:0);
    outlet_float(x->x_outQ6, ((x->x_Q & 32) != 0)?1:0);
}

static void cd4014_free(t_cd4014 *x)
{
    return;
}

static void *cd4014_new(t_symbol *s, int argc, t_atom *argv)
{
    t_cd4014           *x;

    x = (t_cd4014 *)pd_new(cd4014_class);
    if (x == NULL) return (x);
    x->x_outQ6 = outlet_new((t_object *)x, &s_float);
    x->x_outQ7 = outlet_new((t_object *)x, &s_float);
    x->x_outQ8 = outlet_new((t_object *)x, &s_float);
    x->x_inP1 = inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_float, gensym("P1"));
    x->x_inP2 = inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_float, gensym("P2"));
    x->x_inP3 = inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_float, gensym("P3"));
    x->x_inP4 = inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_float, gensym("P4"));
    x->x_inP5 = inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_float, gensym("P5"));
    x->x_inP6 = inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_float, gensym("P6"));
    x->x_inP7 = inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_float, gensym("P7"));
    x->x_inP8 = inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_float, gensym("P8"));
    x->x_inParallelSerial = inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_float, gensym("ParallelSerial"));
    x->x_inSerial = inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_float, gensym("Serial"));
    return (x);
}

void cd4014_setup(void)
{
    cd4014_class = class_new(gensym("cd4014"),
                    (t_newmethod)cd4014_new,
                    (t_method)cd4014_free,
                    sizeof(t_cd4014), 0, 0); /* no arguments */
    class_addfloat(cd4014_class, cd4014_float);
    class_addbang(cd4014_class, cd4014_bang);
    class_addmethod(cd4014_class, (t_method)cd4014_inP1, gensym("P1"), A_FLOAT, 0);
    class_addmethod(cd4014_class, (t_method)cd4014_inP2, gensym("P2"), A_FLOAT, 0);
    class_addmethod(cd4014_class, (t_method)cd4014_inP3, gensym("P3"), A_FLOAT, 0);
    class_addmethod(cd4014_class, (t_method)cd4014_inP4, gensym("P4"), A_FLOAT, 0);
    class_addmethod(cd4014_class, (t_method)cd4014_inP5, gensym("P5"), A_FLOAT, 0);
    class_addmethod(cd4014_class, (t_method)cd4014_inP6, gensym("P6"), A_FLOAT, 0);
    class_addmethod(cd4014_class, (t_method)cd4014_inP7, gensym("P7"), A_FLOAT, 0);
    class_addmethod(cd4014_class, (t_method)cd4014_inP8, gensym("P8"), A_FLOAT, 0);
    class_addmethod(cd4014_class, (t_method)cd4014_inParallelSerial, gensym("ParallelSerial"), A_FLOAT, 0);
    class_addmethod(cd4014_class, (t_method)cd4014_inSerial, gensym("Serial"), A_FLOAT, 0);
}
/* end cd4014.c */

