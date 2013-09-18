/* cd4094.c MP 20070312 */
/* Emulate a cd4094b */
#include "m_pd.h"

typedef struct _cd4094
{
    t_object        x_obj;
    t_outlet        *x_Q1Out;
    t_outlet        *x_Q2Out;
    t_outlet        *x_Q3Out;
    t_outlet        *x_Q4Out;
    t_outlet        *x_Q5Out;
    t_outlet        *x_Q6Out;
    t_outlet        *x_Q7Out;
    t_outlet        *x_Q8Out;
    t_outlet        *x_QSOut;
    t_outlet        *x_QprimeSOut;
    t_int           x_clock;
    t_int           x_data;
    t_int           x_data_in;
    t_int           x_strobe;
    t_int           x_output_enable;
    t_int           x_qprime;
    t_inlet         *x_StrobeIn;/* Strobe takes one or zero as acceptable inputs. */
    t_inlet         *x_DataIn;/* Data takes one or zero as acceptable inputs. */
    t_inlet         *x_OutputEnable;/* Output Enable takes one or zero as acceptable inputs. */
    /* The main inlet (clock) should accept a bang or a one as valid clocks. */
    /* If a one is received, it must be followed by a zero before the clock will work again. */
} t_cd4094;

static t_class *cd4094_class;

void cd4094_setup(void);
static void *cd4094_new(t_symbol *s, int argc, t_atom *argv);
static void cd4094_free(t_cd4094 *x);
static void cd4094_bang(t_cd4094 *x);
static void cd4094_float(t_cd4094 *x, t_float f);
static void cd4094_strobe(t_cd4094 *x, t_float f);
static void cd4094_data(t_cd4094 *x, t_float f);
static void cd4094_output_enable(t_cd4094 *x, t_float f);
static void cd4094_update_outlets(t_cd4094 *x);

static void cd4094_float(t_cd4094 *x, t_float f)
{
    if (f == 1)
    { /* if clock is high and was low, clock it. */
        if ((x->x_clock == 0)&&(x->x_strobe == 1))
        {
            x->x_data <<= 1;
            if (x->x_data_in != 0) x->x_data |= 1;
            cd4094_update_outlets(x);
        }
        x->x_clock = 1;
    }
    else if (f == 0)
    {
        if (x->x_clock == 1) /* if clock was high and is low, clock Q prime. */
        {
            x->x_qprime = ((x->x_data & 256) != 0)?1:0;
            outlet_float(x->x_QprimeSOut, x->x_qprime);
        }
        x->x_clock = 0;
    }
    else post("cd4094 accepts bang, 1 or 0.");
}

static void cd4094_bang(t_cd4094 *x)
{
    if (x->x_strobe == 1)
    {
        /* rising edge clock */
        x->x_data <<= 1;
        if (x->x_data_in != 0) x->x_data |= 1;
        cd4094_update_outlets(x);
        /* Q'7 is clocked on a falling edge */
        x->x_qprime = ((x->x_data & 256) != 0)?1:0;
        outlet_float(x->x_QprimeSOut, x->x_qprime);
    }
}

static void cd4094_strobe(t_cd4094 *x, t_float f)
{
    if (f == 1)
    {
        x->x_strobe = 1;
    }
    else if (f == 0)
    {
        x->x_strobe = 0;
    }
    else
    {
        post("cd4094 strobe takes 1 or 0 only.");
        return;
    }
}

static void cd4094_output_enable(t_cd4094 *x, t_float f)
{
    if (f == 1)
    {
        x->x_output_enable = 1;
        cd4094_update_outlets(x);
    }
    else if (f == 0)
    {
        x->x_output_enable = 0;
    }
    else
    {
        post("cd4094 output enable takes 1 or 0 only.");
        return;
    }
}

static void cd4094_data(t_cd4094 *x, t_float f)
{
    if (f == 1) x->x_data_in = 1;
    else if (f == 0) x->x_data_in = 0;
    else
    {
        post("cd4094 data takes 1 or 0 only.");
        return;
    }
}

static void cd4094_update_outlets(t_cd4094 *x)
{
    outlet_float(x->x_QprimeSOut, ((x->x_qprime) != 0)?1:0);
    outlet_float(x->x_QSOut, ((x->x_data & 256) != 0)?1:0);
    if (x->x_output_enable != 0)
    {
        outlet_float(x->x_QSOut, ((x->x_data & 256) != 0)?1:0);
        outlet_float(x->x_Q8Out, ((x->x_data & 128) != 0)?1:0);
        outlet_float(x->x_Q7Out, ((x->x_data & 64) != 0)?1:0);
        outlet_float(x->x_Q6Out, ((x->x_data & 32) != 0)?1:0);
        outlet_float(x->x_Q5Out, ((x->x_data & 16) != 0)?1:0);
        outlet_float(x->x_Q4Out, ((x->x_data & 8) != 0)?1:0);
        outlet_float(x->x_Q3Out, ((x->x_data & 4) != 0)?1:0);
        outlet_float(x->x_Q2Out, ((x->x_data & 2) != 0)?1:0);
        outlet_float(x->x_Q1Out, ((x->x_data & 1) != 0)?1:0);
    }
}

static void cd4094_free(t_cd4094 *x)
{
    return;
}

static void *cd4094_new(t_symbol *s, int argc, t_atom *argv)
{
    t_cd4094           *x;

    x = (t_cd4094 *)pd_new(cd4094_class);
    if (x == NULL) return (x);
    x->x_Q1Out = outlet_new((t_object *)x, &s_float);
    x->x_Q2Out = outlet_new((t_object *)x, &s_float);
    x->x_Q3Out = outlet_new((t_object *)x, &s_float);
    x->x_Q4Out = outlet_new((t_object *)x, &s_float);
    x->x_Q5Out = outlet_new((t_object *)x, &s_float);
    x->x_Q6Out = outlet_new((t_object *)x, &s_float);
    x->x_Q7Out = outlet_new((t_object *)x, &s_float);
    x->x_Q8Out = outlet_new((t_object *)x, &s_float);
    x->x_QSOut = outlet_new((t_object *)x, &s_float);
    x->x_QprimeSOut = outlet_new((t_object *)x, &s_float);

    x->x_StrobeIn = inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_float, gensym("strobe"));
    x->x_DataIn = inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_float, gensym("data"));
    x->x_OutputEnable = inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_float, gensym("enableoutput"));
    return (x);
}

void cd4094_setup(void)
{
    cd4094_class = class_new(gensym("cd4094"),
                    (t_newmethod)cd4094_new,
                    (t_method)cd4094_free,
                    sizeof(t_cd4094), 0, 0); /* no arguments */
    class_addbang(cd4094_class, cd4094_bang);
    class_addfloat(cd4094_class, cd4094_float);
    class_addmethod(cd4094_class, (t_method)cd4094_strobe, gensym("strobe"), A_FLOAT, 0);
    class_addmethod(cd4094_class, (t_method)cd4094_data, gensym("data"), A_FLOAT, 0);
    class_addmethod(cd4094_class, (t_method)cd4094_output_enable, gensym("enableoutput"), A_FLOAT, 0);
}
/* end cd4094.c */

