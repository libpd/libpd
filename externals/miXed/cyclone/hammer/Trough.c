/* Copyright (c) 2002-2003 krzYszcz and others.
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

#include "m_pd.h"

#define TROUGH_INITIAL  128  /* CHECKME */

typedef struct _Trough
{
    t_object   x_ob;
    t_float    x_value;
    t_outlet  *x_out2;
    t_outlet  *x_out3;
} t_Trough;

static t_class *Trough_class;

static void Trough_bang(t_Trough *x)
{
    outlet_float(((t_object *)x)->ob_outlet, x->x_value);
}

static void Trough_ft1(t_Trough *x, t_floatarg f)
{
    /* CHECKME loud_checkint */
    outlet_float(x->x_out3, 0);  /* CHECKME */
    outlet_float(x->x_out2, 1);
    outlet_float(((t_object *)x)->ob_outlet, x->x_value = f);
}

static void Trough_float(t_Trough *x, t_float f)
{
    /* CHECKME loud_checkint */
    if (f < x->x_value) Trough_ft1(x, f);
    else
    {
	outlet_float(x->x_out3, 1);
	outlet_float(x->x_out2, 0);
    }
}

static void *Trough_new(t_floatarg f)
{
    t_Trough *x = (t_Trough *)pd_new(Trough_class);
    x->x_value = TROUGH_INITIAL;
    inlet_new((t_object *)x, (t_pd *)x, &s_float, gensym("ft1"));
    outlet_new((t_object *)x, &s_float);
    x->x_out2 = outlet_new((t_object *)x, &s_float);
    x->x_out3 = outlet_new((t_object *)x, &s_float);
    return (x);
}

void Trough_setup(void)
{
    Trough_class = class_new(gensym("Trough"),
			     (t_newmethod)Trough_new, 0,
			     sizeof(t_Trough), 0, 0);
    class_addcreator((t_newmethod)Trough_new, gensym("trough"), 0, 0);
    class_addcreator((t_newmethod)Trough_new, gensym("cyclone/trough"), 0, 0);
    class_addbang(Trough_class, Trough_bang);
    class_addfloat(Trough_class, Trough_float);
    class_addmethod(Trough_class, (t_method)Trough_ft1,
		    gensym("ft1"), A_FLOAT, 0);
}

void trough_setup(void)
{
    Trough_setup();
}
