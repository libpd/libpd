/* Copyright (c) 2002-2003 krzYszcz and others.
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

#include "m_pd.h"
#include "common/loud.h"

typedef struct _TogEdge
{
    t_object   x_ob;
    int        x_wason;
    t_outlet  *x_out1;
} t_TogEdge;

static t_class *TogEdge_class;

static void TogEdge_bang(t_TogEdge *x)
{
    if (x->x_wason)
    {
	x->x_wason = 0;
	outlet_bang(x->x_out1);
    }
    else
    {
	x->x_wason = 1;
	outlet_bang(((t_object *)x)->ob_outlet);
    }
}

static void TogEdge_float(t_TogEdge *x, t_float f)
{
    int i;
    if (loud_checkint((t_pd *)x, f, &i, &s_float))  /* CHECKED */
    {
	if (x->x_wason)
	{
	    if (!i)
	    {
		x->x_wason = 0;
		outlet_bang(x->x_out1);
	    }
	}
	else
	{
	    if (i)
	    {
		x->x_wason = 1;
		outlet_bang(((t_object *)x)->ob_outlet);
	    }
	}
    }
}

static void *TogEdge_new(void)
{
    t_TogEdge *x = (t_TogEdge *)pd_new(TogEdge_class);
    x->x_wason = 0;  /* CHECKED */
    outlet_new((t_object *)x, &s_bang);
    x->x_out1 = outlet_new((t_object *)x, &s_bang);
    return (x);
}

void TogEdge_setup(void)
{
    TogEdge_class = class_new(gensym("TogEdge"),
			      (t_newmethod)TogEdge_new, 0,
			      sizeof(t_TogEdge), 0, 0);
    class_addcreator((t_newmethod)TogEdge_new, gensym("togedge"), 0, 0);
    class_addcreator((t_newmethod)TogEdge_new, gensym("cyclone/togedge"), 0, 0);
    class_addbang(TogEdge_class, TogEdge_bang);
    class_addfloat(TogEdge_class, TogEdge_float);
}

void togedge_setup(void)
{
    TogEdge_setup();
}
