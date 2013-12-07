/* Copyright (c) 2002-2003 krzYszcz and others.
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

#include "m_pd.h"
#include "hammer/gui.h"

typedef struct _mousefilter
{
    t_object   x_ob;
    int        x_isup;
    int        x_ispending;
    t_float    x_value;
} t_mousefilter;

static t_class *mousefilter_class;

static void mousefilter_float(t_mousefilter *x, t_float f)
{
    if (x->x_isup)
	outlet_float(((t_object *)x)->ob_outlet, f);
    else
    {
	x->x_ispending = 1;
	x->x_value = f;
    }
}

static void mousefilter_anything(t_mousefilter *x,
				 t_symbol *s, int ac, t_atom *av)
{
    /* dummy method, filtering out those messages from gui,
       which are not handled explicitly */
}

static void mousefilter_doup(t_mousefilter *x, t_floatarg f)
{
    if ((x->x_isup = (int)f) && x->x_ispending)
    {
	x->x_ispending = 0;
	outlet_float(((t_object *)x)->ob_outlet, x->x_value);
    }
}

static void mousefilter_free(t_mousefilter *x)
{
    hammergui_unbindmouse((t_pd *)x);
}

static void *mousefilter_new(void)
{
    t_mousefilter *x = (t_mousefilter *)pd_new(mousefilter_class);
    x->x_isup = 0;  /* LATER rethink */
    x->x_ispending = 0;
    outlet_new((t_object *)x, &s_float);
    hammergui_bindmouse((t_pd *)x);
    return (x);
}

void mousefilter_setup(void)
{
    mousefilter_class = class_new(gensym("mousefilter"),
				  (t_newmethod)mousefilter_new,
				  (t_method)mousefilter_free,
				  sizeof(t_mousefilter), 0, 0);
    class_addfloat(mousefilter_class, mousefilter_float);
    class_addanything(mousefilter_class, mousefilter_anything);
    class_addmethod(mousefilter_class, (t_method)mousefilter_doup,
		    gensym("_up"), A_FLOAT, 0);
}
