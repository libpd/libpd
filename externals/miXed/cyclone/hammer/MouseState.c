/* Copyright (c) 2002-2003 krzYszcz and others.
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

#include "m_pd.h"
#include "hammer/gui.h"

typedef struct _MouseState
{
    t_object   x_ob;
    int        x_ispolling;
    int        x_wasbanged;
    int        x_waszeroed;
    int        x_hlast;
    int        x_vlast;
    int        x_hzero;
    int        x_vzero;
    t_outlet  *x_hposout;
    t_outlet  *x_vposout;
    t_outlet  *x_hdiffout;
    t_outlet  *x_vdiffout;
} t_MouseState;

static t_class *MouseState_class;

static void MouseState_anything(t_MouseState *x,
				t_symbol *s, int ac, t_atom *av)
{
    /* dummy method, filtering out those messages from gui,
       which are not handled explicitly */
}

static void MouseState_doup(t_MouseState *x, t_floatarg f)
{
    outlet_float(((t_object *)x)->ob_outlet, ((int)f ? 0 : 1));
}

static void MouseState_dobang(t_MouseState *x, t_floatarg f1, t_floatarg f2)
{
    if (x->x_wasbanged)
    {
	int h = (int)f1, v = (int)f2;
	outlet_float(x->x_vdiffout, v - x->x_vlast);
	outlet_float(x->x_hdiffout, h - x->x_hlast);
	outlet_float(x->x_vposout, v - x->x_vzero);
	outlet_float(x->x_hposout, h - x->x_hzero);
	x->x_hlast = h;
	x->x_vlast = v;
	x->x_wasbanged = 0;
    }
}

static void MouseState_dozero(t_MouseState *x, t_floatarg f1, t_floatarg f2)
{
    if (x->x_waszeroed)
    {
	int h = (int)f1, v = (int)f2;
	x->x_hzero = h;
	x->x_vzero = v;
	x->x_waszeroed = 0;
    }
}

static void MouseState_dopoll(t_MouseState *x, t_floatarg f1, t_floatarg f2)
{
    if (x->x_ispolling)
    {
	x->x_wasbanged = 1;
	MouseState_dobang(x, f1, f2);
    }
}

static void MouseState_bang(t_MouseState *x)
{
    hammergui_mousexy(gensym("_bang"));
    x->x_wasbanged = 1;
}

static void MouseState_poll(t_MouseState *x)
{
    if (!x->x_ispolling)
    {
	x->x_ispolling = 1;
	hammergui_startpolling((t_pd *)x);
    }
}

static void MouseState_nopoll(t_MouseState *x)
{
    if (x->x_ispolling)
    {
	x->x_ispolling = 0;
	hammergui_stoppolling((t_pd *)x);
    }
}

static void MouseState_zero(t_MouseState *x)
{
    hammergui_mousexy(gensym("_zero"));
    x->x_waszeroed = 1;
}

static void MouseState_reset(t_MouseState *x)
{
    x->x_hzero = x->x_vzero = 0;
}

static void MouseState_free(t_MouseState *x)
{
    MouseState_nopoll(x);
    hammergui_unbindmouse((t_pd *)x);
}

static void *MouseState_new(void)
{
    t_MouseState *x = (t_MouseState *)pd_new(MouseState_class);
    x->x_ispolling = x->x_wasbanged = x->x_waszeroed = 0;
    outlet_new((t_object *)x, &s_float);
    x->x_hposout = outlet_new((t_object *)x, &s_float);
    x->x_vposout = outlet_new((t_object *)x, &s_float);
    x->x_hdiffout = outlet_new((t_object *)x, &s_float);
    x->x_vdiffout = outlet_new((t_object *)x, &s_float);
    hammergui_bindmouse((t_pd *)x);
    hammergui_willpoll();
    MouseState_reset(x);
    return (x);
}

void MouseState_setup(void)
{
    MouseState_class = class_new(gensym("MouseState"),
				 (t_newmethod)MouseState_new,
				 (t_method)MouseState_free,
				 sizeof(t_MouseState), 0, 0);
    class_addcreator((t_newmethod)MouseState_new, gensym("mousestate"), 0, 0);
    class_addcreator((t_newmethod)MouseState_new, gensym("cyclone/mousestate"), 0, 0);
    class_addanything(MouseState_class, MouseState_anything);
    class_addmethod(MouseState_class, (t_method)MouseState_doup,
		    gensym("_up"), A_FLOAT, 0);
    class_addmethod(MouseState_class, (t_method)MouseState_dobang,
		    gensym("_bang"), A_FLOAT, A_FLOAT, 0);
    class_addmethod(MouseState_class, (t_method)MouseState_dozero,
		    gensym("_zero"), A_FLOAT, A_FLOAT, 0);
    class_addmethod(MouseState_class, (t_method)MouseState_dopoll,
		    gensym("_poll"), A_FLOAT, A_FLOAT, 0);
    class_addbang(MouseState_class, MouseState_bang);
    class_addmethod(MouseState_class, (t_method)MouseState_poll,
		    gensym("poll"), 0);
    class_addmethod(MouseState_class, (t_method)MouseState_nopoll,
		    gensym("nopoll"), 0);
    class_addmethod(MouseState_class, (t_method)MouseState_zero,
		    gensym("zero"), 0);
    class_addmethod(MouseState_class, (t_method)MouseState_reset,
		    gensym("reset"), 0);
}

void mousestate_setup(void)
{
    MouseState_setup();
}
