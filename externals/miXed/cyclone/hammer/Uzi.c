/* Copyright (c) 2002-2003 krzYszcz and others.
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

/* CHECKME negative 'nbangs' value set during run-time */

#include "m_pd.h"

typedef struct _Uzi
{
    t_object   x_obj;
    t_float    x_nbangs;
    int        x_count;
    int        x_running;
    t_outlet  *x_out2;
    t_outlet  *x_out3;
} t_Uzi;

static t_class *Uzi_class;

#define UZI_RUNNING  1
#define UZI_PAUSED   2

static void Uzi_dobang(t_Uzi *x)
{
    /* CHECKME reentrancy */
    if (!x->x_running)
    {
	int count, nbangs = (int)x->x_nbangs;
	x->x_running = UZI_RUNNING;
	for (count = x->x_count + 1; count <= nbangs; count++)
	{
	    outlet_float(x->x_out3, count);
	    outlet_bang(((t_object *)x)->ob_outlet);
	    if (x->x_running == UZI_PAUSED)
	    {
		/* CHECKED: carry bang not sent, even if this is last bang */
		x->x_count = count;
		return;
	    }
	}
	/* CHECKED: carry bang sent also when there are no left-outlet bangs */
	/* CHECKED: sent after left outlet, not before */
	outlet_bang(x->x_out2);
	x->x_count = 0;
	x->x_running = 0;
    }
}

static void Uzi_bang(t_Uzi *x)
{
    /* CHECKED: always restarts (when paused too) */
    x->x_count = 0;
    x->x_running = 0;
    Uzi_dobang(x);
}

static void Uzi_float(t_Uzi *x, t_float f)
{
    /* CHECKED: always sets a new value and restarts (when paused too) */
    x->x_nbangs = f;
    Uzi_bang(x);
}

/* CHECKED: 'pause, resume' (but not just 'resume')
   sends a carry bang when not running (a bug?) */
static void Uzi_pause(t_Uzi *x)
{
    if (!x->x_running)
	x->x_count = (int)x->x_nbangs;  /* bug emulation? */
    x->x_running = UZI_PAUSED;
}

static void Uzi_resume(t_Uzi *x)
{
    if (x->x_running == UZI_PAUSED)
    {
	x->x_running = 0;
	Uzi_dobang(x);
    }
}

static void *Uzi_new(t_floatarg f)
{
    t_Uzi *x = (t_Uzi *)pd_new(Uzi_class);
    x->x_nbangs = (f > 1. ? f : 1.);
    x->x_count = 0;
    x->x_running = 0;
    /* CHECKED: set when paused, but then 'resume' is blocked (a bug?) */
    floatinlet_new((t_object *)x, &x->x_nbangs);
    outlet_new((t_object *)x, &s_bang);
    x->x_out2 = outlet_new((t_object *)x, &s_bang);
    x->x_out3 = outlet_new((t_object *)x, &s_float);
    return (x);
}

void Uzi_setup(void)
{
    Uzi_class = class_new(gensym("Uzi"),
			  (t_newmethod)Uzi_new, 0,
			  sizeof(t_Uzi), 0, A_DEFFLOAT, 0);
    class_addcreator((t_newmethod)Uzi_new, gensym("uzi"), A_DEFFLOAT, 0);
    class_addcreator((t_newmethod)Uzi_new, gensym("cyclone/uzi"), A_DEFFLOAT, 0);
    class_addbang(Uzi_class, Uzi_bang);
    class_addfloat(Uzi_class, Uzi_float);
    class_addmethod(Uzi_class, (t_method)Uzi_pause, gensym("pause"), 0);
    class_addmethod(Uzi_class, (t_method)Uzi_pause, gensym("break"), 0);
    class_addmethod(Uzi_class, (t_method)Uzi_resume, gensym("resume"), 0);
    class_addmethod(Uzi_class, (t_method)Uzi_resume, gensym("continue"), 0);
}

void uzi_setup(void)
{
  Uzi_setup();
}
