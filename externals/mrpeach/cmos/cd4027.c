/* cd4027.c MP 20070306 */
/* Emulate a cd4027b */
#include "m_pd.h"

typedef struct _cd4027
{
    t_object        x_obj;
    t_int           x_state;/* set = bit0, clock = bit1, reset = bit2 j = bit3 k = bit4 q = bit 5 */
    t_outlet        *x_Q;
    t_outlet        *x_notQ;
    t_inlet         *x_set;/* set takes one or zero as acceptable inputs. */
    t_inlet         *x_reset;/* reset takes one or zero as acceptable inputs. */
    t_inlet         *x_j;/* j takes one or zero as acceptable inputs. */
    t_inlet         *x_k;/* k takes one or zero as acceptable inputs. */
    /* The main inlet (clock) should accept a bang or a one as valid clocks. */
    /* If a one is received, it must be followed by a zero before the clock will work again. */
} t_cd4027;
#define SET_4027 1
#define CLOCK_4027 2
#define RESET_4027 4
#define J_4027 8
#define K_4027 16
#define Q_4027 32
#define UPDATING_4027 64
#define CHANGED_4027 128

static t_class *cd4027_class;

void cd4027_setup(void);
static void *cd4027_new(t_symbol *s, int argc, t_atom *argv);
static void cd4027_free(t_cd4027 *x);
static void cd4027_bang(t_cd4027 *x);
static void cd4027_float(t_cd4027 *x, t_float f);
static void cd4027_set(t_cd4027 *x, t_float f);
static void cd4027_reset(t_cd4027 *x, t_float f);
static void cd4027_j(t_cd4027 *x, t_float f);
static void cd4027_k(t_cd4027 *x, t_float f);
static void cd4027_update_outlets(t_cd4027 *x);
static void cd4027_printstate (t_cd4027 *x);

static void cd4027_float(t_cd4027 *x, t_float f)
{
    if (f == 1)
    { /* if clock is high and was low, clock it. */
        if ((x->x_state & CLOCK_4027) == 0) cd4027_bang(x);
        x->x_state |= CLOCK_4027;
    }
    else if (f == 0) x->x_state &= ~CLOCK_4027;
    else post("cd4027 accepts bang, 1 or 0.");
}

static void cd4027_printstate (t_cd4027 *x)
{
    post ("SET %d RESET %d J %d K %d Q %d", ((x->x_state & SET_4027) != 0)?1:0
        , ((x->x_state & RESET_4027) != 0)?1:0
        , ((x->x_state & J_4027) != 0)?1:0
        , ((x->x_state & K_4027) != 0)?1:0
        , ((x->x_state & Q_4027) != 0)?1:0);
}

static void cd4027_bang(t_cd4027 *x)
{
    if ((x->x_state & (SET_4027 | RESET_4027)) == 0)
    { /* if set is low and reset is low, clock forward */
        if ((x->x_state & Q_4027) != 0)
        { /* Q is high */
            if ((x->x_state & K_4027)== 0) x->x_state |= Q_4027;
            else  x->x_state &= ~Q_4027;
        }
        else
        { /* Q is low */
            if ((x->x_state & J_4027) != 0) x->x_state |= Q_4027;
            else  x->x_state &= ~Q_4027;
        }
        cd4027_update_outlets(x);
    }
}

static void cd4027_set(t_cd4027 *x, t_float f)
{
    if (f == 1)
    {
        x->x_state |= SET_4027; /* set = 1 */
    }
    else if (f == 0)
    {
        x->x_state &= ~SET_4027; /* set = 0 */
    }
    else
    {
        post("cd4027 set takes 1 or 0 only.");
        return;
    }
    /* update outlets if not already doing that */
    if ((x->x_state & UPDATING_4027) == 0)
    {
        cd4027_update_outlets(x);
    }
    else
    {
        x->x_state |= CHANGED_4027;
    }
    return;
}

static void cd4027_reset(t_cd4027 *x, t_float f)
{
    if (f == 1)
    {
        x->x_state |= RESET_4027; /* reset = 1 */
    }
    else if (f == 0)
    {
        x->x_state &= ~RESET_4027; /* reset = 0 */
    }
    else
    {
        return;
    }
    /* update outlets if not already doing that */
    if ((x->x_state & UPDATING_4027) == 0)
    {
        cd4027_update_outlets(x);
    }
    else
    {
        x->x_state |= CHANGED_4027;
    }
    return;
}

static void cd4027_j(t_cd4027 *x, t_float f)
{
    if (f == 1)
    {
        x->x_state |= J_4027; /* j = 1 */
    }
    else if (f == 0)
    {
        x->x_state &= ~J_4027; /* j = 0 */
    }
    else post("cd4027 j takes 1 or 0 only.");
    return;
}

static void cd4027_k(t_cd4027 *x, t_float f)
{
    if (f == 1)
    {
        x->x_state |= K_4027; /* k = 1 */
    }
    else if (f == 0)
    {
        x->x_state &= ~K_4027; /* k = 0 */
    }
    else post("cd4027 k takes 1 or 0 only.");
    return;
}

static void cd4027_update_outlets(t_cd4027 *x)
{
    /*  cd4027_printstate (x);  */
    x->x_state |= UPDATING_4027;/* updating outlets */
reset:
    if ((x->x_state & SET_4027) != 0) x->x_state |= Q_4027;
    else if ((x->x_state & RESET_4027) != 0) x->x_state &= ~Q_4027;
    x->x_state &= ~CHANGED_4027; /* prepare to flag any changes that occur during outlet_update */
    outlet_float(x->x_notQ, ((x->x_state & Q_4027) == 0)?1:0);
/* we might get set or reset as a result of feedback from one of these outlets. */
    if ((x->x_state & CHANGED_4027) != 0) goto reset;
    outlet_float(x->x_Q, ((x->x_state & Q_4027) != 0)?1:0);
    if ((x->x_state & CHANGED_4027) != 0) goto reset;
    x->x_state &= ~UPDATING_4027;/* finished updating outlets */
}

static void cd4027_free(t_cd4027 *x)
{
    return;
}

static void *cd4027_new(t_symbol *s, int argc, t_atom *argv)
{
    t_cd4027           *x;

    x = (t_cd4027 *)pd_new(cd4027_class);
    if (x == NULL) return (x);
    x->x_Q = outlet_new((t_object *)x, &s_float);
    x->x_notQ = outlet_new((t_object *)x, &s_float);
    x->x_set = inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_float, gensym("set"));
    x->x_reset = inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_float, gensym("reset"));
    x->x_j = inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_float, gensym("j"));
    x->x_k = inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_float, gensym("k"));
    return (x);
}

void cd4027_setup(void)
{
    cd4027_class = class_new(gensym("cd4027"),
                    (t_newmethod)cd4027_new,
                    (t_method)cd4027_free,
                    sizeof(t_cd4027), 0, 0); /* no arguments */
    class_addbang(cd4027_class, cd4027_bang);
    class_addfloat(cd4027_class, cd4027_float);
    class_addmethod(cd4027_class, (t_method)cd4027_reset, gensym("reset"), A_FLOAT, 0);
    class_addmethod(cd4027_class, (t_method)cd4027_set, gensym("set"), A_FLOAT, 0);
    class_addmethod(cd4027_class, (t_method)cd4027_j, gensym("j"), A_FLOAT, 0);
    class_addmethod(cd4027_class, (t_method)cd4027_k, gensym("k"), A_FLOAT, 0);
}
/* end cd4027.c */

