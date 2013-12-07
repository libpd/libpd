/* Copyright (c) 2002-2003 krzYszcz and others.
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

#include "m_pd.h"

typedef struct _midiformat
{
    t_object  x_ob;
    t_float   x_channel;
} t_midiformat;

static t_class *midiformat_class;

static int midiformat_channel(t_midiformat *x)
{
    int ch = (int)x->x_channel;
    return (ch > 0 ? (ch - 1) & 0x0F : 0);
}

static void midiformat_note(t_midiformat *x, t_symbol *s, int ac, t_atom *av)
{
    if (ac >= 2 && av[0].a_type == A_FLOAT && av[1].a_type == A_FLOAT)
    {
	int pitch = (int)av[0].a_w.w_float;  /* CHECKED: anything goes */
	int velocity = (int)av[1].a_w.w_float;
	outlet_float(((t_object *)x)->ob_outlet, 0x90 | midiformat_channel(x));
	outlet_float(((t_object *)x)->ob_outlet, pitch);
	outlet_float(((t_object *)x)->ob_outlet, velocity);
    }
}

static void midiformat_polytouch(t_midiformat *x,
				 t_symbol *s, int ac, t_atom *av)
{
    if (ac >= 2 && av[0].a_type == A_FLOAT && av[1].a_type == A_FLOAT)
    {
	int touch = (int)av[0].a_w.w_float;
	int key = (int)av[1].a_w.w_float;
	outlet_float(((t_object *)x)->ob_outlet, 0xA0 | midiformat_channel(x));
	outlet_float(((t_object *)x)->ob_outlet, key);
	outlet_float(((t_object *)x)->ob_outlet, touch);
    }
}

static void midiformat_controller(t_midiformat *x,
				  t_symbol *s, int ac, t_atom *av)
{
    if (ac >= 2 && av[0].a_type == A_FLOAT && av[1].a_type == A_FLOAT)
    {
	int val = (int)av[0].a_w.w_float;
	int ctl = (int)av[1].a_w.w_float;
	outlet_float(((t_object *)x)->ob_outlet, 0xB0 | midiformat_channel(x));
	outlet_float(((t_object *)x)->ob_outlet, ctl);
	outlet_float(((t_object *)x)->ob_outlet, val);
    }
}

static void midiformat_program(t_midiformat *x, t_floatarg f)
{
    int pgm = (int)f;
    outlet_float(((t_object *)x)->ob_outlet, 0xC0 | midiformat_channel(x));
    outlet_float(((t_object *)x)->ob_outlet, pgm);
}

static void midiformat_touch(t_midiformat *x, t_floatarg f)
{
    int touch = (int)f;
    outlet_float(((t_object *)x)->ob_outlet, 0xD0 | midiformat_channel(x));
    outlet_float(((t_object *)x)->ob_outlet, touch);
}

static void midiformat_bend(t_midiformat *x, t_floatarg f)
{
    int val = (int)f;
    outlet_float(((t_object *)x)->ob_outlet, 0xE0 | midiformat_channel(x));
    outlet_float(((t_object *)x)->ob_outlet, 0);
    outlet_float(((t_object *)x)->ob_outlet, val);
}

static void *midiformat_new(t_floatarg f)
{
    t_midiformat *x = (t_midiformat *)pd_new(midiformat_class);
    x->x_channel = f;
    inlet_new((t_object *)x, (t_pd *)x, &s_list, gensym("lst1"));
    inlet_new((t_object *)x, (t_pd *)x, &s_list, gensym("lst2"));
    inlet_new((t_object *)x, (t_pd *)x, &s_float, gensym("ft3"));
    inlet_new((t_object *)x, (t_pd *)x, &s_float, gensym("ft4"));
    inlet_new((t_object *)x, (t_pd *)x, &s_float, gensym("ft5"));
    floatinlet_new((t_object *)x, &x->x_channel);
    outlet_new((t_object *)x, &s_float);
    return (x);
}

void midiformat_setup(void)
{
    midiformat_class = class_new(gensym("midiformat"), 
				 (t_newmethod)midiformat_new, 0,
				 sizeof(t_midiformat), 0,
				 A_DEFFLOAT, 0);
    class_addlist(midiformat_class, midiformat_note);
    class_addmethod(midiformat_class, (t_method)midiformat_polytouch,
		    gensym("lst1"), A_GIMME, 0);
    class_addmethod(midiformat_class, (t_method)midiformat_controller,
		    gensym("lst2"), A_GIMME, 0);
    class_addmethod(midiformat_class, (t_method)midiformat_program,
		    gensym("ft3"), A_FLOAT, 0);
    class_addmethod(midiformat_class, (t_method)midiformat_touch,
		    gensym("ft4"), A_FLOAT, 0);
    class_addmethod(midiformat_class, (t_method)midiformat_bend,
		    gensym("ft5"), A_FLOAT, 0);
}
