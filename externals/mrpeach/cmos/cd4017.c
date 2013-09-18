/* cd4017.c MP 20070305 */
/* Emulate a cd4017b */
#include "m_pd.h"

typedef struct _cd4017
{
    t_object        x_obj;
    t_int           x_state;/* clock_inhibit value in bit 0, clock level in bit 1, reset level in bit 2 */
    t_int           x_count;
    t_outlet        *x_0;
    t_outlet        *x_1;
    t_outlet        *x_2;
    t_outlet        *x_3;
    t_outlet        *x_4;
    t_outlet        *x_5;
    t_outlet        *x_6;
    t_outlet        *x_7;
    t_outlet        *x_8;
    t_outlet        *x_9;
    t_outlet        *x_carry_out;/* Outputs 1 when any of outlets 0-3 is one */
    t_inlet         *x_clock_inhibit;/* clock_inhibit takes one or zero as acceptable inputs. 1 inhibits the clock. */
    t_inlet         *x_reset;/* reset */
    /* The main inlet (clcok) should accept a bang or a one as valid clocks. */
    /* If a one is received, it must be followed by a zero before the clock will work again. */
} t_cd4017;

static t_class *cd4017_class;

void cd4017_setup(void);
static void *cd4017_new(t_symbol *s, int argc, t_atom *argv);
static void cd4017_free(t_cd4017 *x);
static void cd4017_bang(t_cd4017 *x);
static void cd4017_float(t_cd4017 *x, t_float f);
static void cd4017_clock_inhibit(t_cd4017 *x, t_float f);
static void cd4017_reset(t_cd4017 *x, t_float f);
static void cd4017_update_outlets(t_cd4017 *x);

static void cd4017_float(t_cd4017 *x, t_float f)
{
    if (f == 1)
    { /* if clock is high and was low, clock it. */
        if ((x->x_state & 2) == 0) cd4017_bang(x);
        x->x_state |= 2;
    }
    else if (f == 0) x->x_state &= ~2;
    else post("cd4017 accepts bang, 1 or 0.");
}

static void cd4017_bang(t_cd4017 *x)
{
    if ((x->x_state & 5) == 0)
    { /* if clock inhibit is low and reset is low, clock forward */
        x->x_count = ((x->x_count + 1)%10);
        cd4017_update_outlets(x);
    }
}

static void cd4017_clock_inhibit(t_cd4017 *x, t_float f)
{
    if (f == 1) x->x_state |= 1; /* clock inhibited */
    else if (f == 0) x->x_state &= ~1; /* clock uninhibited */
    else post("cd4017 clock inhibit takes 1 or 0 only.");
    return;
}

static void cd4017_reset(t_cd4017 *x, t_float f)
{
    if (f == 1)
    {
        x->x_count = 0;
        x->x_state |= 4; /* reset */
        if ((x->x_state & 8) == 0) /* don't reenter any update_oulets in progress */
        {
            cd4017_update_outlets(x);
        }
        else x->x_state |= 16; /* reset during outlet_update */
    }
    else if (f == 0) x->x_state &= ~4; /* no reset */
}

static void cd4017_update_outlets(t_cd4017 *x)
{
    x->x_state |= 8;/* updating outlets */
reset:
    x->x_state &= ~16; /* clear reset during outlet_update */
    outlet_float(x->x_0, (x->x_count == 0)?1:0);
/* we might get reset as a result of feedback from one of these outlets. */
    if ((x->x_state & 16) != 0) goto reset;
    outlet_float(x->x_1, (x->x_count == 1)?1:0);
    if ((x->x_state & 16) != 0) goto reset;
    outlet_float(x->x_2, (x->x_count == 2)?1:0);
    if ((x->x_state & 16) != 0) goto reset;
    outlet_float(x->x_3, (x->x_count == 3)?1:0);
    if ((x->x_state & 16) != 0) goto reset;
    outlet_float(x->x_4, (x->x_count == 4)?1:0);
    if ((x->x_state & 16) != 0) goto reset;
    outlet_float(x->x_5, (x->x_count == 5)?1:0);
    if ((x->x_state & 16) != 0) goto reset;
    outlet_float(x->x_6, (x->x_count == 6)?1:0);
    if ((x->x_state & 16) != 0) goto reset;
    outlet_float(x->x_7, (x->x_count == 7)?1:0);
    if ((x->x_state & 16) != 0) goto reset;
    outlet_float(x->x_8, (x->x_count == 8)?1:0);
    if ((x->x_state & 16) != 0) goto reset;
    outlet_float(x->x_9, (x->x_count == 9)?1:0);
    if ((x->x_state & 16) != 0) goto reset;
    outlet_float(x->x_carry_out, (x->x_count < 5)?1:0);
    if ((x->x_state & 16) != 0) goto reset;
    x->x_state &= ~8;/* finished updating outlets */
}

static void cd4017_free(t_cd4017 *x)
{
    return;
}

static void *cd4017_new(t_symbol *s, int argc, t_atom *argv)
{
    t_cd4017           *x;

    x = (t_cd4017 *)pd_new(cd4017_class);
    if (x == NULL) return (x);
    x->x_0 = outlet_new((t_object *)x, &s_float);
    x->x_1 = outlet_new((t_object *)x, &s_float);
    x->x_2 = outlet_new((t_object *)x, &s_float);
    x->x_3 = outlet_new((t_object *)x, &s_float);
    x->x_4 = outlet_new((t_object *)x, &s_float);
    x->x_5 = outlet_new((t_object *)x, &s_float);
    x->x_6 = outlet_new((t_object *)x, &s_float);
    x->x_7 = outlet_new((t_object *)x, &s_float);
    x->x_8 = outlet_new((t_object *)x, &s_float);
    x->x_9 = outlet_new((t_object *)x, &s_float);
    x->x_carry_out = outlet_new((t_object *)x, &s_float);
    x->x_clock_inhibit = inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_float, gensym("clock_inhibit"));
    x->x_reset = inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_float, gensym("reset"));

    return (x);
}

void cd4017_setup(void)
{
    cd4017_class = class_new(gensym("cd4017"),
                    (t_newmethod)cd4017_new,
                    (t_method)cd4017_free,
                    sizeof(t_cd4017), 0, 0); /* no arguments */
    class_addbang(cd4017_class, cd4017_bang);
    class_addfloat(cd4017_class, cd4017_float);
    class_addmethod(cd4017_class, (t_method)cd4017_reset, gensym("reset"), A_FLOAT, 0);
    class_addmethod(cd4017_class, (t_method)cd4017_clock_inhibit, gensym("clock_inhibit"), A_FLOAT, 0);
}
/* end cd4017.c */

