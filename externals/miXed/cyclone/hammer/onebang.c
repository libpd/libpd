/* Copyright (c) 2002-2003 krzYszcz and others.
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

#include "m_pd.h"

typedef struct _onebang
{
    t_object  x_ob;
    int       x_isopen;
} t_onebang;

static t_class *onebang_class;

static void onebang_bang(t_onebang *x)
{
    if (x->x_isopen)
    {
	outlet_bang(((t_object *)x)->ob_outlet);
	x->x_isopen = 0;
    }
}

static void onebang_bang1(t_onebang *x)
{
    x->x_isopen = 1;
}

static void *onebang_new(t_floatarg f)
{
    t_onebang *x = (t_onebang *)pd_new(onebang_class);
    x->x_isopen = ((int)f != 0);  /* CHECKED */
    inlet_new((t_object *)x, (t_pd *)x, &s_bang, gensym("bang1"));
    outlet_new((t_object *)x, &s_bang);
    return (x);
}

void onebang_setup(void)
{
    onebang_class = class_new(gensym("onebang"),
			      (t_newmethod)onebang_new, 0,
			      sizeof(t_onebang), 0, A_DEFFLOAT, 0);
    class_addbang(onebang_class, onebang_bang);
    class_addmethod(onebang_class, (t_method)onebang_bang1,
		    gensym("bang1"), 0);
}
