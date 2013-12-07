/* cd4013.c MP 20070306 */
/* Emulate a cd4013b */
#include "m_pd.h"

typedef struct _cd4013
{
    t_object        x_obj;
    t_int           x_state;/* set = bit0, clock = bit1, reset = bit2 j = bit3 k = bit4 q = bit 5 */
    t_outlet        *x_Q;
    t_outlet        *x_notQ;
    t_inlet         *x_set;/* set takes one or zero as acceptable inputs. */
    t_inlet         *x_reset;/* reset takes one or zero as acceptable inputs. */
    t_inlet         *x_d;/* d takes one or zero as acceptable inputs. */
    /* The main inlet (clock) should accept a bang or a one as valid clocks. */
    /* If a one is received, it must be followed by a zero before the clock will work again. */
} t_cd4013;

#define SET_4013 1
#define CLOCK_4013 2
#define RESET_4013 4
#define D_4013 8
#define Q_4013 16
#define UPDATING_4013 32
#define CHANGED_4013 64

static t_class *cd4013_class;

void cd4013_setup(void);
static void *cd4013_new(t_symbol *s, int argc, t_atom *argv);
static void cd4013_free(t_cd4013 *x);
static void cd4013_bang(t_cd4013 *x);
static void cd4013_float(t_cd4013 *x, t_float f);
static void cd4013_set(t_cd4013 *x, t_float f);
static void cd4013_reset(t_cd4013 *x, t_float f);
static void cd4013_d(t_cd4013 *x, t_float f);
static void cd4013_update_outlets(t_cd4013 *x);
static void cd4013_printstate (t_cd4013 *x);

static void cd4013_float(t_cd4013 *x, t_float f)
{
    if (f == 1)
    { /* if clock is high and was low, clock it. */
        if ((x->x_state & CLOCK_4013) == 0) cd4013_bang(x);
        x->x_state |= CLOCK_4013;
    }
    else if (f == 0) x->x_state &= ~CLOCK_4013;
    else post("cd4013 accepts bang, 1 or 0.");
}

static void cd4013_printstate (t_cd4013 *x)
{
    post ("SET %d RESET %d D %d Q %d", ((x->x_state & SET_4013) != 0)?1:0
        , ((x->x_state & RESET_4013) != 0)?1:0
        , ((x->x_state & D_4013) != 0)?1:0
        , ((x->x_state & Q_4013) != 0)?1:0);
}

static void cd4013_bang(t_cd4013 *x)
{
    if ((x->x_state & (SET_4013 | RESET_4013)) == 0)
    { /* if set is low and reset is low, clock forward */
        if ((x->x_state & D_4013) != 0) x->x_state |= Q_4013;
        else  x->x_state &= ~Q_4013;
        cd4013_update_outlets(x);
    }
}

static void cd4013_set(t_cd4013 *x, t_float f)
{
    if (f == 1)
    {
        x->x_state |= SET_4013; /* set = 1 */
    }
    else if (f == 0)
    {
        x->x_state &= ~SET_4013; /* set = 0 */
    }
    else
    {
        post("cd4013 set takes 1 or 0 only.");
        return;
    }
    /* update outlets if not already doing that */
    if ((x->x_state & UPDATING_4013) == 0)
    {
        cd4013_update_outlets(x);
    }
    else
    {
        x->x_state |= CHANGED_4013;
    }
    return;
}

static void cd4013_reset(t_cd4013 *x, t_float f)
{
    if (f == 1)
    {
        x->x_state |= RESET_4013; /* reset = 1 */
    }
    else if (f == 0)
    {
        x->x_state &= ~RESET_4013; /* reset = 0 */
    }
    else
    {
        return;
    }
    /* update outlets if not already doing that */
    if ((x->x_state & UPDATING_4013) == 0)
    {
        cd4013_update_outlets(x);
    }
    else
    {
        x->x_state |= CHANGED_4013;
    }
    return;
}

static void cd4013_d(t_cd4013 *x, t_float f)
{
    if (f == 1)
    {
        x->x_state |= D_4013; /* d = 1 */
    }
    else if (f == 0)
    {
        x->x_state &= ~D_4013; /* d = 0 */
    }
    else post("cd4013 d takes 1 or 0 only.");
    return;
}

static void cd4013_update_outlets(t_cd4013 *x)
{
    /*  cd4013_printstate (x);  */
    x->x_state |= UPDATING_4013;/* updating outlets */
reset:
    if ((x->x_state & SET_4013) != 0) x->x_state |= Q_4013;
    else if ((x->x_state & RESET_4013) != 0) x->x_state &= ~Q_4013;
    x->x_state &= ~CHANGED_4013; /* prepare to flag any changes that occur during outlet_update */
    outlet_float(x->x_notQ, ((x->x_state & Q_4013) == 0)?1:0);
/* we might get set or reset as a result of feedback from one of these outlets. */
    if ((x->x_state & CHANGED_4013) != 0) goto reset;
    outlet_float(x->x_Q, ((x->x_state & Q_4013) != 0)?1:0);
    if ((x->x_state & CHANGED_4013) != 0) goto reset;
    x->x_state &= ~UPDATING_4013;/* finished updating outlets */
}

static void cd4013_free(t_cd4013 *x)
{
    return;
}

static void *cd4013_new(t_symbol *s, int argc, t_atom *argv)
{
    t_cd4013           *x;

    x = (t_cd4013 *)pd_new(cd4013_class);
    if (x == NULL) return (x);
    x->x_Q = outlet_new((t_object *)x, &s_float);
    x->x_notQ = outlet_new((t_object *)x, &s_float);
    x->x_set = inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_float, gensym("set"));
    x->x_reset = inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_float, gensym("reset"));
    x->x_d = inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_float, gensym("d"));
    return (x);
}

void cd4013_setup(void)
{
    cd4013_class = class_new(gensym("cd4013"),
                    (t_newmethod)cd4013_new,
                    (t_method)cd4013_free,
                    sizeof(t_cd4013), 0, 0); /* no arguments */
    class_addbang(cd4013_class, cd4013_bang);
    class_addfloat(cd4013_class, cd4013_float);
    class_addmethod(cd4013_class, (t_method)cd4013_reset, gensym("reset"), A_FLOAT, 0);
    class_addmethod(cd4013_class, (t_method)cd4013_set, gensym("set"), A_FLOAT, 0);
    class_addmethod(cd4013_class, (t_method)cd4013_d, gensym("d"), A_FLOAT, 0);
}
/* end cd4013.c */

