/* cd4076.c MP 20070312 */
/* Emulate a cd4076b */
#include "m_pd.h"

typedef struct _cd4076
{
    t_object        x_obj;
    t_outlet        *x_output_A;
    t_outlet        *x_output_B;
    t_outlet        *x_output_C;
    t_outlet        *x_output_D;
    t_int           x_clock;
    t_int           x_QA;
    t_int           x_QB;
    t_int           x_QC;
    t_int           x_QD;
    t_int           x_inA;
    t_int           x_inB;
    t_int           x_inC;
    t_int           x_inD;
    t_int           x_InDis1;
    t_int           x_InDis2;
    t_int           x_OutDis1;
    t_int           x_OutDis2;
    t_int           x_Clear;
    t_inlet         *x_clear;/* clear takes one or zero as acceptable inputs. */
    t_inlet         *x_input_disable_1;/* input_disable_1 takes one or zero as acceptable inputs. */
    t_inlet         *x_input_disable_2;/* input_disable_2 takes one or zero as acceptable inputs. */
    t_inlet         *x_output_disable_1;/* output_disable_1 takes one or zero as acceptable inputs. */
    t_inlet         *x_output_disable_2;/* output_disable_2 takes one or zero as acceptable inputs. */
    t_inlet         *x_input_A;/* input_A takes one or zero as acceptable inputs. */
    t_inlet         *x_input_B;/* input_B takes one or zero as acceptable inputs. */
    t_inlet         *x_input_C;/* input_C takes one or zero as acceptable inputs. */
    t_inlet         *x_input_D;/* input_D takes one or zero as acceptable inputs. */
    /* The main inlet (clock) should accept a bang or a one as valid clocks. */
    /* If a one is received, it must be followed by a zero before the clock will work again. */
} t_cd4076;

static t_class *cd4076_class;

void cd4076_setup(void);
static void *cd4076_new(t_symbol *s, int argc, t_atom *argv);
static void cd4076_free(t_cd4076 *x);
static void cd4076_bang(t_cd4076 *x);
static void cd4076_float(t_cd4076 *x, t_float f);
static void cd4076_clear(t_cd4076 *x, t_float f);
static void cd4076_input_disable_1(t_cd4076 *x, t_float f);
static void cd4076_input_disable_2(t_cd4076 *x, t_float f);
static void cd4076_output_disable_1(t_cd4076 *x, t_float f);
static void cd4076_output_disable_2(t_cd4076 *x, t_float f);
static void cd4076_input_a(t_cd4076 *x, t_float f);
static void cd4076_input_b(t_cd4076 *x, t_float f);
static void cd4076_input_c(t_cd4076 *x, t_float f);
static void cd4076_input_d(t_cd4076 *x, t_float f);
static void cd4076_update_outlets(t_cd4076 *x);

static void cd4076_float(t_cd4076 *x, t_float f)
{
    if (f == 1)
    { /* if clock is high and was low, clock it. */
        if ((x->x_clock) == 0) cd4076_bang(x);
        x->x_clock = 1;
    }
    else if (f == 0) x->x_clock = 0;
    else post("cd4076 accepts bang, 1 or 0.");
}

static void cd4076_bang(t_cd4076 *x)
{
    if ((x->x_InDis1 + x->x_InDis2 + x->x_Clear) == 0)
    {
        x->x_QA = x->x_inA;
        x->x_QB = x->x_inB;
        x->x_QC = x->x_inC;
        x->x_QD = x->x_inD;
    }
    cd4076_update_outlets(x);
}

static void cd4076_clear(t_cd4076 *x, t_float f)
{
    if (f == 1)
    {
        x->x_Clear = 1;
        x->x_QA = 0;
        x->x_QB = 0;
        x->x_QC = 0;
        x->x_QD = 0;
        cd4076_update_outlets(x);
    }
    else if (f == 0)
    {
        x->x_Clear = 0;
    }
    else
    {
        post("cd4076 clear takes 1 or 0 only.");
        return;
    }
}

static void cd4076_input_disable_1(t_cd4076 *x, t_float f)
{
    if (f == 1) x->x_InDis1 = 1;
    else if (f == 0) x->x_InDis1 = 0;
    else
    {
        post("cd4076 input disable 1 takes 1 or 0 only.");
        return;
    }
}

static void cd4076_input_disable_2(t_cd4076 *x, t_float f)
{
    if (f == 1) x->x_InDis2 = 1;
    else if (f == 0) x->x_InDis2 = 0;
    else
    {
        post("cd4076 input disable 2 takes 1 or 0 only.");
        return;
    }
}

static void cd4076_output_disable_1(t_cd4076 *x, t_float f)
{
    if (f == 1) x->x_OutDis1 = 1;
    else if (f == 0)
    {
        x->x_OutDis1 = 0;
        cd4076_update_outlets(x);
    }
    else
    {
        post("cd4076 output disable 1 takes 1 or 0 only.");
        return;
    }
}

static void cd4076_output_disable_2(t_cd4076 *x, t_float f)
{
    if (f == 1) x->x_OutDis2 = 1;
    else if (f == 0)
    {
        x->x_OutDis2 = 0;
        cd4076_update_outlets(x);
    }
    else
    {
        post("cd4076 output disable 2 takes 1 or 0 only.");
        return;
    }
}

static void cd4076_input_a(t_cd4076 *x, t_float f)
{
    if (f == 1) x->x_inA = 1;
    else if (f == 0) x->x_inA = 0;
    else
    {
        post("cd4076 input A takes 1 or 0 only.");
        return;
    }
}

static void cd4076_input_b(t_cd4076 *x, t_float f)
{
    if (f == 1) x->x_inB = 1;
    else if (f == 0) x->x_inB = 0;
    else
    {
        post("cd4076 input B takes 1 or 0 only.");
        return;
    }
}

static void cd4076_input_c(t_cd4076 *x, t_float f)
{
    if (f == 1) x->x_inC = 1;
    else if (f == 0) x->x_inC = 0;
    else
    {
        post("cd4076 input C takes 1 or 0 only.");
        return;
    }
}

static void cd4076_input_d(t_cd4076 *x, t_float f)
{
    if (f == 1) x->x_inD = 1;
    else if (f == 0) x->x_inD = 0;
    else
    {
        post("cd4076 input D takes 1 or 0 only.");
        return;
    }
}

static void cd4076_update_outlets(t_cd4076 *x)
{
    if (x->x_OutDis1 + x->x_OutDis2 == 0)
    {
        outlet_float(x->x_output_D, ((x->x_QD) == 1)?1:0);
        outlet_float(x->x_output_C, ((x->x_QC) == 1)?1:0);
        outlet_float(x->x_output_B, ((x->x_QB) == 1)?1:0);
        outlet_float(x->x_output_A, ((x->x_QA) == 1)?1:0);
    }
}

static void cd4076_free(t_cd4076 *x)
{
    return;
}

static void *cd4076_new(t_symbol *s, int argc, t_atom *argv)
{
    t_cd4076           *x;

    x = (t_cd4076 *)pd_new(cd4076_class);
    if (x == NULL) return (x);
    x->x_output_A = outlet_new((t_object *)x, &s_float);
    x->x_output_B = outlet_new((t_object *)x, &s_float);
    x->x_output_C = outlet_new((t_object *)x, &s_float);
    x->x_output_D = outlet_new((t_object *)x, &s_float);

    x->x_clear = inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_float, gensym("clear"));
    x->x_input_disable_1 = inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_float, gensym("inputdisable1"));
    x->x_input_disable_2 = inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_float, gensym("inputdisable2"));
    x->x_output_disable_1 = inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_float, gensym("outputdisable1"));
    x->x_output_disable_2 = inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_float, gensym("outputdisable2"));
    x->x_input_A = inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_float, gensym("inputA"));
    x->x_input_B = inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_float, gensym("inputB"));
    x->x_input_C = inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_float, gensym("inputC"));
    x->x_input_D = inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_float, gensym("inputD"));
    return (x);
}

void cd4076_setup(void)
{
    cd4076_class = class_new(gensym("cd4076"),
                    (t_newmethod)cd4076_new,
                    (t_method)cd4076_free,
                    sizeof(t_cd4076), 0, 0); /* no arguments */
    class_addbang(cd4076_class, cd4076_bang);
    class_addfloat(cd4076_class, cd4076_float);
    class_addmethod(cd4076_class, (t_method)cd4076_clear, gensym("clear"), A_FLOAT, 0);
    class_addmethod(cd4076_class, (t_method)cd4076_input_disable_1, gensym("inputdisable1"), A_FLOAT, 0);
    class_addmethod(cd4076_class, (t_method)cd4076_input_disable_2, gensym("inputdisable2"), A_FLOAT, 0);
    class_addmethod(cd4076_class, (t_method)cd4076_output_disable_1, gensym("outputdisable1"), A_FLOAT, 0);
    class_addmethod(cd4076_class, (t_method)cd4076_output_disable_2, gensym("outputdisable2"), A_FLOAT, 0);
    class_addmethod(cd4076_class, (t_method)cd4076_input_a, gensym("inputA"), A_FLOAT, 0);
    class_addmethod(cd4076_class, (t_method)cd4076_input_b, gensym("inputB"), A_FLOAT, 0);
    class_addmethod(cd4076_class, (t_method)cd4076_input_c, gensym("inputC"), A_FLOAT, 0);
    class_addmethod(cd4076_class, (t_method)cd4076_input_d, gensym("inputD"), A_FLOAT, 0);
}
/* end cd4076.c */

