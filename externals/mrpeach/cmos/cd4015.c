/* cd4015.c MP 20070315 */
/* Emulate a cd4015b */
#include "m_pd.h"

typedef struct _cd4015
{
    t_object        x_obj;
    t_int           x_D;
    t_int           x_R;
    t_int           x_Clk;
    t_int           x_Q;
    t_outlet        *x_outQ1;
    t_outlet        *x_outQ2;
    t_outlet        *x_outQ3;
    t_outlet        *x_outQ4;
    t_inlet         *x_inD;/* extra inlets are 'live' like the first */
    t_inlet         *x_inR;
} t_cd4015;

static t_class *cd4015_class;

void cd4015_setup(void);
static void *cd4015_new(t_symbol *s, int argc, t_atom *argv);
static void cd4015_free(t_cd4015 *x);
static void cd4015_float(t_cd4015 *x, t_float f);
static void cd4015_bang(t_cd4015 *x);
static void cd4015_inD(t_cd4015 *x, t_float f);
static void cd4015_inR(t_cd4015 *x, t_float f);
static void cd4015_update_outlets(t_cd4015 *x);

static void cd4015_float(t_cd4015 *x, t_float f)
{
    if (f == 1)
    {
        if (x->x_Clk == 0)
        {
            x->x_Clk = 1;
            cd4015_bang(x);
        }
    }
    else if (f == 0) x->x_Clk = 0;
    else
    {
        post("cd4015 Clock inlet accepts 1 or 0.");
        return;
    }
}

static void cd4015_bang(t_cd4015 *x)
{
    if (x->x_R == 0)
    { /* shift left by one */
        x->x_Q <<= 1;
        x->x_Q |= x->x_D;
        cd4015_update_outlets(x);
    }
}

static void cd4015_inD(t_cd4015 *x, t_float f)
{
    if (f == 1) x->x_D = 1;
    else if (f == 0) x->x_D = 0;
    else
    {
        post("cd4015 D inlet accepts 1 or 0.");
        return;
    }
}

static void cd4015_inR(t_cd4015 *x, t_float f)
{
    if (f == 1)
    {
        x->x_R = 1;
        x->x_Q = 0;
        cd4015_update_outlets(x);
    }
    else if (f == 0) x->x_R = 0;
    else
    {
        post("cd4015 R inlet accepts 1 or 0.");
        return;
    }
}

static void cd4015_update_outlets(t_cd4015 *x)
{
    outlet_float(x->x_outQ4, ((x->x_Q & 8) != 0)?1:0);
    outlet_float(x->x_outQ3, ((x->x_Q & 4) != 0)?1:0);
    outlet_float(x->x_outQ2, ((x->x_Q & 2) != 0)?1:0);
    outlet_float(x->x_outQ1, ((x->x_Q & 1) != 0)?1:0);
}

static void cd4015_free(t_cd4015 *x)
{
    return;
}

static void *cd4015_new(t_symbol *s, int argc, t_atom *argv)
{
    t_cd4015           *x;

    x = (t_cd4015 *)pd_new(cd4015_class);
    if (x == NULL) return (x);
    x->x_outQ1 = outlet_new((t_object *)x, &s_float);
    x->x_outQ2 = outlet_new((t_object *)x, &s_float);
    x->x_outQ3 = outlet_new((t_object *)x, &s_float);
    x->x_outQ4 = outlet_new((t_object *)x, &s_float);
    x->x_inD = inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_float, gensym("D"));
    x->x_inR = inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_float, gensym("R"));
    return (x);
}

void cd4015_setup(void)
{
    cd4015_class = class_new(gensym("cd4015"),
                    (t_newmethod)cd4015_new,
                    (t_method)cd4015_free,
                    sizeof(t_cd4015), 0, 0); /* no arguments */
    class_addfloat(cd4015_class, cd4015_float);
    class_addbang(cd4015_class, cd4015_bang);
    class_addmethod(cd4015_class, (t_method)cd4015_inD, gensym("D"), A_FLOAT, 0);
    class_addmethod(cd4015_class, (t_method)cd4015_inR, gensym("R"), A_FLOAT, 0);
}
/* end cd4015.c */

