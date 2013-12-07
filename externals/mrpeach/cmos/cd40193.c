/* cd40193.c MP 20070312 */
/* Emulate a cd40193b */
#include "m_pd.h"

typedef struct _cd40193
{
    t_object        x_obj;
    t_outlet        *x_QAOut;
    t_outlet        *x_QBOut;
    t_outlet        *x_QCOut;
    t_outlet        *x_QDOut;
    t_outlet        *x_CarryOut;
    t_outlet        *x_BorrowOut;
    t_int           x_countUp;
    t_int           x_countDown;
    t_int           x_dataA;
    t_int           x_dataB;
    t_int           x_dataC;
    t_int           x_dataD;
    t_int           x_load;
    t_int           x_clear;
    t_int           x_count;
    t_inlet         *x_CountDownIn;/* All inlets take one or zero as acceptable inputs. */
    t_inlet         *x_ClearIn;
    t_inlet         *x_LoadIn;
    t_inlet         *x_DataAIn;
    t_inlet         *x_DataBIn;
    t_inlet         *x_DataCIn;
    t_inlet         *x_DataDIn;
    /* The main inlet (clock) should accept a bang or a one as valid clocks. */
    /* If a one is received, it must be followed by a zero before the clock will work again. */
} t_cd40193;

static t_class *cd40193_class;

void cd40193_setup(void);
static void *cd40193_new(t_symbol *s, int argc, t_atom *argv);
static void cd40193_free(t_cd40193 *x);
static void cd40193_bang(t_cd40193 *x);
static void cd40193_float(t_cd40193 *x, t_float f);
static void cd40193_clear(t_cd40193 *x, t_float f);
static void cd40193_load(t_cd40193 *x, t_float f);
static void cd40193_count_down(t_cd40193 *x, t_float f);
static void cd40193_data_A(t_cd40193 *x, t_float f);
static void cd40193_data_B(t_cd40193 *x, t_float f);
static void cd40193_data_C(t_cd40193 *x, t_float f);
static void cd40193_data_D(t_cd40193 *x, t_float f);
static void cd40193_update_outlets(t_cd40193 *x);

static void cd40193_float(t_cd40193 *x, t_float f)
{
    if (f == 1)
    { /* if clock is high and was low, count up. */
        if ((x->x_countUp == 0)&&(x->x_countDown == 1)&&(x->x_clear == 0)&&(x->x_load == 1))
        {
            x->x_countUp = 1;
            x->x_count = (x->x_count + 1)%16;
            cd40193_update_outlets(x);
        }
    }
    else if (f == 0)
    {
        if ((x->x_countUp == 1) && (x->x_count == 15)) outlet_float(x->x_CarryOut, 0);
        x->x_countUp = 0;
    }
    else post("cd40193 accepts bang, 1 or 0.");
}

static void cd40193_bang(t_cd40193 *x)
{
    if ((x->x_countDown == 1)&&(x->x_clear == 0)&&(x->x_load == 1))
    {
        x->x_countUp = 1;
        x->x_count = (x->x_count + 1)%16;
        cd40193_update_outlets(x);
        if (x->x_count == 15) outlet_float(x->x_CarryOut, 0);
    }
}

static void cd40193_clear(t_cd40193 *x, t_float f)
{
    if (f == 1)
    {
        if (x->x_clear == 0)
        {
            x->x_count = 0;
            cd40193_update_outlets(x);
            x->x_clear = 1;
        }
    }
    else if (f == 0)
    {
        x->x_clear = 0;
        if (x->x_load == 0)
        { /* the strange case of a low-going clear enabling an already low load */
            x->x_count = x->x_dataA + 2*x->x_dataB + 4*x->x_dataC + 8*x->x_dataD;
            cd40193_update_outlets(x);
        }
    }
    else
    {
        post("cd40193 clear takes 1 or 0 only.");
        return;
    }
}

static void cd40193_load(t_cd40193 *x, t_float f)
{
    if (f == 1)
    {
        x->x_load = 1;
    }
    else if (f == 0)
    {
        if (x->x_load == 1)
        {
            x->x_load = 0;
            if (x->x_clear == 0)
            {
                x->x_count = x->x_dataA + 2*x->x_dataB + 4*x->x_dataC + 8*x->x_dataD;
                cd40193_update_outlets(x);
            }
        }
    }
    else
    {
        post("cd40193 load takes 1 or 0 only.");
        return;
    }
}

static void cd40193_count_down(t_cd40193 *x, t_float f)
{
    if (f == 1)
    { /* if clock is high and was low, count down. */
        if (x->x_countDown == 0)
        {
            x->x_countDown = 1;
            if((x->x_countUp == 1)&&(x->x_clear == 0)&&(x->x_load = 1))
            {
                x->x_count = (x->x_count - 1)%16;
                cd40193_update_outlets(x);
            }
        }
    }
    else if (f == 0)
    {
        if ((x->x_countDown == 1) && (x->x_count == 0)) outlet_float(x->x_BorrowOut, 0);
        x->x_countDown = 0;
    }
    else post("cd40193 count down accepts bang, 1 or 0.");
}

static void cd40193_data_A(t_cd40193 *x, t_float f)
{
    if (f == 1) x->x_dataA = 1;
    else if (f == 0) x->x_dataA = 0;
    else
    {
        post("cd40193 data A takes 1 or 0 only.");
        return;
    }
}
static void cd40193_data_B(t_cd40193 *x, t_float f)
{
    if (f == 1) x->x_dataB = 1;
    else if (f == 0) x->x_dataB = 0;
    else
    {
        post("cd40193 data B takes 1 or 0 only.");
        return;
    }
}
static void cd40193_data_C(t_cd40193 *x, t_float f)
{
    if (f == 1) x->x_dataC = 1;
    else if (f == 0) x->x_dataC = 0;
    else
    {
        post("cd40193 data C takes 1 or 0 only.");
        return;
    }
}
static void cd40193_data_D(t_cd40193 *x, t_float f)
{
    if (f == 1) x->x_dataD = 1;
    else if (f == 0) x->x_dataD = 0;
    else
    {
        post("cd40193 data D takes 1 or 0 only.");
        return;
    }
}

static void cd40193_update_outlets(t_cd40193 *x)
{
    outlet_float(x->x_BorrowOut, 1);
    outlet_float(x->x_CarryOut, 1);
    outlet_float(x->x_QDOut, ((x->x_count & 8) != 0)?1:0);
    outlet_float(x->x_QCOut, ((x->x_count & 4) != 0)?1:0);
    outlet_float(x->x_QBOut, ((x->x_count & 2) != 0)?1:0);
    outlet_float(x->x_QAOut, ((x->x_count & 1) != 0)?1:0);
}

static void cd40193_free(t_cd40193 *x)
{
    return;
}

static void *cd40193_new(t_symbol *s, int argc, t_atom *argv)
{
    t_cd40193           *x;

    x = (t_cd40193 *)pd_new(cd40193_class);
    if (x == NULL) return (x);
    x->x_QAOut = outlet_new((t_object *)x, &s_float);
    x->x_QBOut = outlet_new((t_object *)x, &s_float);
    x->x_QCOut = outlet_new((t_object *)x, &s_float);
    x->x_QDOut = outlet_new((t_object *)x, &s_float);
    x->x_CarryOut = outlet_new((t_object *)x, &s_float);
    x->x_BorrowOut = outlet_new((t_object *)x, &s_float);

    x->x_CountDownIn = inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_float, gensym("countdown"));
    x->x_ClearIn = inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_float, gensym("clear"));
    x->x_LoadIn = inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_float, gensym("load"));
    x->x_DataAIn = inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_float, gensym("dataA"));
    x->x_DataBIn = inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_float, gensym("dataB"));
    x->x_DataCIn = inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_float, gensym("dataC"));
    x->x_DataDIn = inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_float, gensym("dataD"));
    return (x);
}

void cd40193_setup(void)
{
    cd40193_class = class_new(gensym("cd40193"),
                    (t_newmethod)cd40193_new,
                    (t_method)cd40193_free,
                    sizeof(t_cd40193), 0, 0); /* no arguments */
    class_addbang(cd40193_class, cd40193_bang);
    class_addfloat(cd40193_class, cd40193_float);
    class_addmethod(cd40193_class, (t_method)cd40193_count_down, gensym("countdown"), A_FLOAT, 0);
    class_addmethod(cd40193_class, (t_method)cd40193_load, gensym("load"), A_FLOAT, 0);
    class_addmethod(cd40193_class, (t_method)cd40193_clear, gensym("clear"), A_FLOAT, 0);
    class_addmethod(cd40193_class, (t_method)cd40193_data_A, gensym("dataA"), A_FLOAT, 0);
    class_addmethod(cd40193_class, (t_method)cd40193_data_B, gensym("dataB"), A_FLOAT, 0);
    class_addmethod(cd40193_class, (t_method)cd40193_data_C, gensym("dataC"), A_FLOAT, 0);
    class_addmethod(cd40193_class, (t_method)cd40193_data_D, gensym("dataD"), A_FLOAT, 0);
}
/* end cd40193.c */

