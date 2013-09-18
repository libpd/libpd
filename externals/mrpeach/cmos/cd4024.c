/* cd4024.c MP 20070319 */
/* Emulate a cd4024b */
#include "m_pd.h"

typedef struct _cd4024
{
    t_object        x_obj;
    t_int           x_reset;
    t_int           x_clock;
    t_int           x_count;
    t_int           x_updating;
    t_outlet        *x_Q1;
    t_outlet        *x_Q2;
    t_outlet        *x_Q3;
    t_outlet        *x_Q4;
    t_outlet        *x_Q5;
    t_outlet        *x_Q6;
    t_outlet        *x_Q7;
    t_inlet         *x_resetIn;/* reset */
    /* The main inlet (clcok) should accept a bang or a one as valid clocks. */
    /* If a one is received, it must be followed by a zero before the clock will work again. */
} t_cd4024;

static t_class *cd4024_class;

void cd4024_setup(void);
static void *cd4024_new(t_symbol *s, int argc, t_atom *argv);
static void cd4024_free(t_cd4024 *x);
static void cd4024_bang(t_cd4024 *x);
static void cd4024_float(t_cd4024 *x, t_float f);
static void cd4024_reset(t_cd4024 *x, t_float f);
static void cd4024_update_outlets(t_cd4024 *x);

static void cd4024_float(t_cd4024 *x, t_float f)
{
    if (f == 0)
    { /* if clock is low and was high, clock it. */
        if (x->x_clock == 1)
        {
            x->x_clock = 0;
            cd4024_bang(x);
        }
    }
    else if (f == 1) x->x_clock = 1;
    else post("cd4024 clock accepts bang, 1 or 0.");
}

static void cd4024_bang(t_cd4024 *x)
{
    if (x->x_reset == 0)
    { /* if reset is low, clock forward */
        x->x_count = (x->x_count + 1)%128;
        cd4024_update_outlets(x);
    }
}

static void cd4024_reset(t_cd4024 *x, t_float f)
{
    if (f == 1)
    {
        x->x_count = 0;
        x->x_reset = 1;
        if (x->x_updating != 0) x->x_updating |= 2;
        else cd4024_update_outlets(x);
    }
    else if (f == 0) x->x_reset = 0;
}

static void cd4024_update_outlets(t_cd4024 *x)
{
reset:
    x->x_updating = 1;/* updating outlets */
    outlet_float(x->x_Q7, ((x->x_count & 64) != 0)?1:0);
/* we might get reset as a result of feedback from one of these outlets. */
    if ((x->x_updating & 2) != 0) goto reset;
    outlet_float(x->x_Q6, ((x->x_count & 32) != 0)?1:0);
    if ((x->x_updating & 2) != 0) goto reset;
    outlet_float(x->x_Q5, ((x->x_count & 16) != 0)?1:0);
    if ((x->x_updating & 2) != 0) goto reset;
    outlet_float(x->x_Q4, ((x->x_count & 8) != 0)?1:0);
    if ((x->x_updating & 2) != 0) goto reset;
    outlet_float(x->x_Q3, ((x->x_count & 4) != 0)?1:0);
    if ((x->x_updating & 2) != 0) goto reset;
    outlet_float(x->x_Q2, ((x->x_count & 2) != 0)?1:0);
    if ((x->x_updating & 2) != 0) goto reset;
    outlet_float(x->x_Q1, ((x->x_count & 1) != 0)?1:0);
    if ((x->x_updating & 2) != 0) goto reset;
    x->x_updating = 0; /* finished updating outlets */
}

static void cd4024_free(t_cd4024 *x)
{
    return;
}

static void *cd4024_new(t_symbol *s, int argc, t_atom *argv)
{
    t_cd4024           *x;

    x = (t_cd4024 *)pd_new(cd4024_class);
    if (x == NULL) return (x);
    x->x_Q1 = outlet_new((t_object *)x, &s_float);
    x->x_Q2 = outlet_new((t_object *)x, &s_float);
    x->x_Q3 = outlet_new((t_object *)x, &s_float);
    x->x_Q4 = outlet_new((t_object *)x, &s_float);
    x->x_Q5 = outlet_new((t_object *)x, &s_float);
    x->x_Q6 = outlet_new((t_object *)x, &s_float);
    x->x_Q7 = outlet_new((t_object *)x, &s_float);
    x->x_resetIn = inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_float, gensym("reset"));

    return (x);
}

void cd4024_setup(void)
{
    cd4024_class = class_new(gensym("cd4024"),
                    (t_newmethod)cd4024_new,
                    (t_method)cd4024_free,
                    sizeof(t_cd4024), 0, 0); /* no arguments */
    class_addbang(cd4024_class, cd4024_bang);
    class_addfloat(cd4024_class, cd4024_float);
    class_addmethod(cd4024_class, (t_method)cd4024_reset, gensym("reset"), A_FLOAT, 0);
}
/* end cd4024.c */

