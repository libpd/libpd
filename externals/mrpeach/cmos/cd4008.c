/* cd4008.c MP 20070315 */
/* Emulate a cd4008b */
#include "m_pd.h"

typedef struct _cd4008
{
    t_object        x_obj;
    t_int           x_A1;
    t_int           x_A2;
    t_int           x_A3;
    t_int           x_A4;
    t_int           x_B1;
    t_int           x_B2;
    t_int           x_B3;
    t_int           x_B4;
    t_int           x_CarryIn;
    t_outlet        *x_outS1;
    t_outlet        *x_outS2;
    t_outlet        *x_outS3;
    t_outlet        *x_outS4;
    t_outlet        *x_outC;
    t_inlet         *x_inA1;/* extra inlets are 'live' like the first */
    t_inlet         *x_inA2;
    t_inlet         *x_inA3;
    t_inlet         *x_inA4;
    t_inlet         *x_inB1;
    t_inlet         *x_inB2;
    t_inlet         *x_inB3;
    t_inlet         *x_inB4;
} t_cd4008;

static t_class *cd4008_class;

void cd4008_setup(void);
static void *cd4008_new(t_symbol *s, int argc, t_atom *argv);
static void cd4008_free(t_cd4008 *x);
static void cd4008_float(t_cd4008 *x, t_float f);
static void cd4008_inA1(t_cd4008 *x, t_float f);
static void cd4008_inA2(t_cd4008 *x, t_float f);
static void cd4008_inA3(t_cd4008 *x, t_float f);
static void cd4008_inA4(t_cd4008 *x, t_float f);
static void cd4008_inB1(t_cd4008 *x, t_float f);
static void cd4008_inB2(t_cd4008 *x, t_float f);
static void cd4008_inB3(t_cd4008 *x, t_float f);
static void cd4008_inB4(t_cd4008 *x, t_float f);
static void cd4008_update_outlets(t_cd4008 *x);

static void cd4008_float(t_cd4008 *x, t_float f)
{
    if (f == 1) x->x_CarryIn = 1;
    else if (f == 0) x->x_CarryIn = 0;
    else
    {
        post("cd4008 Carry inlet accepts 1 or 0.");
        return;
    }
    cd4008_update_outlets(x);
}

static void cd4008_inA1(t_cd4008 *x, t_float f)
{
    if (f == 1) x->x_A1 = 1;
    else if (f == 0) x->x_A1 = 0;
    else
    {
        post("cd4008 inlet A1 accepts 1 or 0.");
        return;
    }
    cd4008_update_outlets(x);
}

static void cd4008_inA2(t_cd4008 *x, t_float f)
{
    if (f == 1) x->x_A2 = 1;
    else if (f == 0) x->x_A2 = 0;
    else
    {
        post("cd4008 inlet A2 accepts 1 or 0.");
        return;
    }
    cd4008_update_outlets(x);
}

static void cd4008_inA3(t_cd4008 *x, t_float f)
{
    if (f == 1) x->x_A3 = 1;
    else if (f == 0) x->x_A3 = 0;
    else
    {
        post("cd4008 inlet A3 accepts 1 or 0.");
        return;
    }
    cd4008_update_outlets(x);
}

static void cd4008_inA4(t_cd4008 *x, t_float f)
{
    if (f == 1) x->x_A4 = 1;
    else if (f == 0) x->x_A4 = 0;
    else
    {
        post("cd4008 inlet A4 accepts 1 or 0.");
        return;
    }
    cd4008_update_outlets(x);
}

static void cd4008_inB1(t_cd4008 *x, t_float f)
{
    if (f == 1) x->x_B1 = 1;
    else if (f == 0) x->x_B1 = 0;
    else
    {
        post("cd4008 inlet B1 accepts 1 or 0.");
        return;
    }
    cd4008_update_outlets(x);
}

static void cd4008_inB2(t_cd4008 *x, t_float f)
{
    if (f == 1) x->x_B2 = 1;
    else if (f == 0) x->x_B2 = 0;
    else
    {
        post("cd4008 inlet B2 accepts 1 or 0.");
        return;
    }
    cd4008_update_outlets(x);
}

static void cd4008_inB3(t_cd4008 *x, t_float f)
{
    if (f == 1) x->x_B3 = 1;
    else if (f == 0) x->x_B3 = 0;
    else
    {
        post("cd4008 inlet B3 accepts 1 or 0.");
        return;
    }
    cd4008_update_outlets(x);
}

static void cd4008_inB4(t_cd4008 *x, t_float f)
{
    if (f == 1) x->x_B4 = 1;
    else if (f == 0) x->x_B4 = 0;
    else
    {
        post("cd4008 inlet B4 accepts 1 or 0.");
        return;
    }
    cd4008_update_outlets(x);
}

static void cd4008_update_outlets(t_cd4008 *x)
{ /* Quadruple add function */
    t_int sum;

    sum = x->x_CarryIn + x->x_A1 + x->x_B1 + 2*(x->x_A2 + x->x_B2) + 4*(x->x_A3 + x->x_B3) + 8*(x->x_A4 + x->x_B4);
    outlet_float(x->x_outC, ((sum & 16) != 0)?1:0);
    outlet_float(x->x_outS4, ((sum & 8) != 0)?1:0);
    outlet_float(x->x_outS3, ((sum & 4) != 0)?1:0);
    outlet_float(x->x_outS2, ((sum & 2) != 0)?1:0);
    outlet_float(x->x_outS1, ((sum & 1) != 0)?1:0);
}

static void cd4008_free(t_cd4008 *x)
{
    return;
}

static void *cd4008_new(t_symbol *s, int argc, t_atom *argv)
{
    t_cd4008           *x;

    x = (t_cd4008 *)pd_new(cd4008_class);
    if (x == NULL) return (x);
    x->x_outS1 = outlet_new((t_object *)x, &s_float);
    x->x_outS2 = outlet_new((t_object *)x, &s_float);
    x->x_outS3 = outlet_new((t_object *)x, &s_float);
    x->x_outS4 = outlet_new((t_object *)x, &s_float);
    x->x_outC = outlet_new((t_object *)x, &s_float);
    x->x_inA1 = inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_float, gensym("A1"));
    x->x_inA2 = inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_float, gensym("A2"));
    x->x_inA3 = inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_float, gensym("A3"));
    x->x_inA4 = inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_float, gensym("A4"));
    x->x_inB1 = inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_float, gensym("B1"));
    x->x_inB2 = inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_float, gensym("B2"));
    x->x_inB3 = inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_float, gensym("B3"));
    x->x_inB4 = inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_float, gensym("B4"));
    return (x);
}

void cd4008_setup(void)
{
    cd4008_class = class_new(gensym("cd4008"),
                    (t_newmethod)cd4008_new,
                    (t_method)cd4008_free,
                    sizeof(t_cd4008), 0, 0); /* no arguments */
    class_addfloat(cd4008_class, cd4008_float);
    class_addmethod(cd4008_class, (t_method)cd4008_inA1, gensym("A1"), A_FLOAT, 0);
    class_addmethod(cd4008_class, (t_method)cd4008_inA2, gensym("A2"), A_FLOAT, 0);
    class_addmethod(cd4008_class, (t_method)cd4008_inA3, gensym("A3"), A_FLOAT, 0);
    class_addmethod(cd4008_class, (t_method)cd4008_inA4, gensym("A4"), A_FLOAT, 0);
    class_addmethod(cd4008_class, (t_method)cd4008_inB1, gensym("B1"), A_FLOAT, 0);
    class_addmethod(cd4008_class, (t_method)cd4008_inB2, gensym("B2"), A_FLOAT, 0);
    class_addmethod(cd4008_class, (t_method)cd4008_inB3, gensym("B3"), A_FLOAT, 0);
    class_addmethod(cd4008_class, (t_method)cd4008_inB4, gensym("B4"), A_FLOAT, 0);
}
/* end cd4008.c */

