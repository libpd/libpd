/* cd4516.c MP 20070312 */
/* Emulate a cd4516b */
#include "m_pd.h"

typedef struct _cd4516
{
    t_object        x_obj;
    t_outlet        *x_Q1Out;
    t_outlet        *x_Q2Out;
    t_outlet        *x_Q3Out;
    t_outlet        *x_Q4Out;
    t_outlet        *x_CarryOut;
    t_int           x_clock;
    t_int           x_upDown;
    t_int           x_preset_enable;
    t_int           x_carry;
    t_int           x_P1;
    t_int           x_P2;
    t_int           x_P3;
    t_int           x_P4;
    t_int           x_reset;
    t_int           x_count;
    t_inlet         *x_UpDownIn;/* All inlets take one or zero as acceptable inputs. */
    t_inlet         *x_ResetIn;
    t_inlet         *x_PresetEnableIn;
    t_inlet         *x_CarryIn;
    t_inlet         *x_P1In;
    t_inlet         *x_P2In;
    t_inlet         *x_P3In;
    t_inlet         *x_P4In;
    /* The main inlet (clock) should accept a bang or a one as valid clocks. */
    /* If a one is received, it must be followed by a zero before the clock will work again. */
} t_cd4516;

static t_class *cd4516_class;

void cd4516_setup(void);
static void *cd4516_new(t_symbol *s, int argc, t_atom *argv);
static void cd4516_free(t_cd4516 *x);
static void cd4516_bang(t_cd4516 *x);
static void cd4516_float(t_cd4516 *x, t_float f);
static void cd4516_reset(t_cd4516 *x, t_float f);
static void cd4516_preset_enable(t_cd4516 *x, t_float f);
static void cd4516_up_down(t_cd4516 *x, t_float f);
static void cd4516_carry(t_cd4516 *x, t_float f);
static void cd4516_P1(t_cd4516 *x, t_float f);
static void cd4516_P2(t_cd4516 *x, t_float f);
static void cd4516_P3(t_cd4516 *x, t_float f);
static void cd4516_P4(t_cd4516 *x, t_float f);
static void cd4516_update_outlets(t_cd4516 *x);

static void cd4516_float(t_cd4516 *x, t_float f)
{
    if (f == 1)
    { /* if clock is high and was low, count up. */
        if ((x->x_clock == 0)&&(x->x_reset == 0)&&(x->x_preset_enable == 0)&&(x->x_carry == 0))
        {
            x->x_clock = 1;
            if (x->x_upDown == 1) x->x_count = (x->x_count + 1)%16;
            else x->x_count = (x->x_count - 1)%16;
            cd4516_update_outlets(x);
        }
    }
    else if (f == 0)
    {
        x->x_clock = 0;
    }
    else post("cd4516 accepts bang, 1 or 0.");
}

static void cd4516_bang(t_cd4516 *x)
{
    if ((x->x_reset == 0)&&(x->x_preset_enable == 0)&&(x->x_carry == 0))
    {
        if (x->x_upDown == 1)x->x_count = (x->x_count + 1)%16;
        else x->x_count = (x->x_count - 1)%16;
        cd4516_update_outlets(x);
    }
}

static void cd4516_reset(t_cd4516 *x, t_float f)
{
    if (f == 1)
    {
        if (x->x_reset == 0)
        {
            x->x_count = 0;
            cd4516_update_outlets(x);
            x->x_reset = 1;
        }
    }
    else if (f == 0)
    {
        x->x_reset = 0;
        if (x->x_preset_enable == 1)
        { /* the strange case of a low-going reset enabling an already high preset enable */
            x->x_count = x->x_P1 + 2*x->x_P2 + 4*x->x_P3 + 8*x->x_P4;
            cd4516_update_outlets(x);
        }
    }
    else
    {
        post("cd4516 reset takes 1 or 0 only.");
        return;
    }
}

static void cd4516_preset_enable(t_cd4516 *x, t_float f)
{
    if (f == 0)
    {
        x->x_preset_enable = 0;
    }
    else if (f == 1)
    {
        if (x->x_preset_enable == 0)
        {
            x->x_preset_enable = 1;
            if (x->x_reset == 0)
            {
                x->x_count = x->x_P1 + 2*x->x_P2 + 4*x->x_P3 + 8*x->x_P4;
                cd4516_update_outlets(x);
            }
        }
    }
    else
    {
        post("cd4516 x_preset_enable takes 1 or 0 only.");
        return;
    }
}

static void cd4516_up_down(t_cd4516 *x, t_float f)
{
    if (f == 1)
    {
        x->x_upDown = 1;
    }
    else if (f == 0)
    {
        x->x_upDown = 0;
    }
    else post("cd4516 updown accepts bang, 1 or 0.");
}

static void cd4516_carry(t_cd4516 *x, t_float f)
{
    if (f == 1)
    {
        x->x_carry = 1;
    }
    else if (f == 0)
    {
        x->x_carry = 0;
    }
    else post("cd4516 carry accepts bang, 1 or 0.");
}

static void cd4516_P1(t_cd4516 *x, t_float f)
{
    if (f == 1) x->x_P1 = 1;
    else if (f == 0) x->x_P1 = 0;
    else
    {
        post("cd4516 P1 takes 1 or 0 only.");
        return;
    }
}
static void cd4516_P2(t_cd4516 *x, t_float f)
{
    if (f == 1) x->x_P2 = 1;
    else if (f == 0) x->x_P2 = 0;
    else
    {
        post("cd4516 P2 takes 1 or 0 only.");
        return;
    }
}
static void cd4516_P3(t_cd4516 *x, t_float f)
{
    if (f == 1) x->x_P3 = 1;
    else if (f == 0) x->x_P3 = 0;
    else
    {
        post("cd4516 P3 takes 1 or 0 only.");
        return;
    }
}
static void cd4516_P4(t_cd4516 *x, t_float f)
{
    if (f == 1) x->x_P4 = 1;
    else if (f == 0) x->x_P4 = 0;
    else
    {
        post("cd4516 P4 takes 1 or 0 only.");
        return;
    }
}

static void cd4516_update_outlets(t_cd4516 *x)
{
    if (x->x_upDown == 1) outlet_float(x->x_CarryOut, (x->x_count == 15)?0:1);
    else outlet_float(x->x_CarryOut, (x->x_count == 0)?0:1);
    outlet_float(x->x_Q4Out, ((x->x_count & 8) != 0)?1:0);
    outlet_float(x->x_Q3Out, ((x->x_count & 4) != 0)?1:0);
    outlet_float(x->x_Q2Out, ((x->x_count & 2) != 0)?1:0);
    outlet_float(x->x_Q1Out, ((x->x_count & 1) != 0)?1:0);
}

static void cd4516_free(t_cd4516 *x)
{
    return;
}

static void *cd4516_new(t_symbol *s, int argc, t_atom *argv)
{
    t_cd4516           *x;

    x = (t_cd4516 *)pd_new(cd4516_class);
    if (x == NULL) return (x);
    x->x_Q1Out = outlet_new((t_object *)x, &s_float);
    x->x_Q2Out = outlet_new((t_object *)x, &s_float);
    x->x_Q3Out = outlet_new((t_object *)x, &s_float);
    x->x_Q4Out = outlet_new((t_object *)x, &s_float);
    x->x_CarryOut = outlet_new((t_object *)x, &s_float);

    x->x_UpDownIn = inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_float, gensym("updown"));
    x->x_ResetIn = inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_float, gensym("reset"));
    x->x_PresetEnableIn = inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_float, gensym("presetenable"));
    x->x_CarryIn = inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_float, gensym("carry"));
    x->x_P1In = inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_float, gensym("P1"));
    x->x_P2In = inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_float, gensym("P2"));
    x->x_P3In = inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_float, gensym("P3"));
    x->x_P4In = inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_float, gensym("P4"));
    return (x);
}

void cd4516_setup(void)
{
    cd4516_class = class_new(gensym("cd4516"),
                    (t_newmethod)cd4516_new,
                    (t_method)cd4516_free,
                    sizeof(t_cd4516), 0, 0); /* no arguments */
    class_addbang(cd4516_class, cd4516_bang);
    class_addfloat(cd4516_class, cd4516_float);
    class_addmethod(cd4516_class, (t_method)cd4516_up_down, gensym("updown"), A_FLOAT, 0);
    class_addmethod(cd4516_class, (t_method)cd4516_preset_enable, gensym("presetenable"), A_FLOAT, 0);
    class_addmethod(cd4516_class, (t_method)cd4516_reset, gensym("reset"), A_FLOAT, 0);
    class_addmethod(cd4516_class, (t_method)cd4516_carry, gensym("carry"), A_FLOAT, 0);
    class_addmethod(cd4516_class, (t_method)cd4516_P1, gensym("P1"), A_FLOAT, 0);
    class_addmethod(cd4516_class, (t_method)cd4516_P2, gensym("P2"), A_FLOAT, 0);
    class_addmethod(cd4516_class, (t_method)cd4516_P3, gensym("P3"), A_FLOAT, 0);
    class_addmethod(cd4516_class, (t_method)cd4516_P4, gensym("P4"), A_FLOAT, 0);
}
/* end cd4516.c */

