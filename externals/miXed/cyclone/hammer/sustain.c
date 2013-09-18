/* Copyright (c) 2002-2003 krzYszcz and others.
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

#include <string.h>
#include "m_pd.h"

#define SUSTAIN_NPITCHES  128

typedef struct _sustain
{
    t_object       x_ob;
    t_float        x_velocity;
    int            x_switch;
    unsigned char  x_pitches[SUSTAIN_NPITCHES];
    t_outlet      *x_voutlet;
} t_sustain;

static t_class *sustain_class;

static void sustain_float(t_sustain *x, t_float f)
{
    int pitch = (int)f;
    if (pitch >= 0 && pitch < SUSTAIN_NPITCHES)
    {
	/* CHECKED a plain note-off accumulator */
	if (x->x_velocity || !x->x_switch)
	{
	    outlet_float(x->x_voutlet, x->x_velocity);
	    outlet_float(((t_object *)x)->ob_outlet, pitch);
	}
	else x->x_pitches[pitch]++;
    }
}

static void sustain_bang(t_sustain *x)
{
    int i;
    unsigned char *pp;
    for (i = 0, pp = x->x_pitches; i < SUSTAIN_NPITCHES; i++, pp++)
    {
	while (*pp)
	{
	    outlet_float(x->x_voutlet, 0);
	    outlet_float(((t_object *)x)->ob_outlet, i);
	    (*pp)--;
	}
    }
}

static void sustain_clear(t_sustain *x)
{
    memset(x->x_pitches, 0, sizeof(x->x_pitches));
}

static void sustain_ft2(t_sustain *x, t_floatarg f)
{
    int newstate = ((int)f != 0);
    if (x->x_switch && !newstate) sustain_bang(x);
    x->x_switch = newstate;
}

static void *sustain_new(void)
{
    t_sustain *x = (t_sustain *)pd_new(sustain_class);
    x->x_velocity = 0;
    x->x_switch = 0;
    sustain_clear(x);
    floatinlet_new((t_object *)x, &x->x_velocity);
    inlet_new((t_object *)x, (t_pd *)x, &s_float, gensym("ft2"));
    outlet_new((t_object *)x, &s_float);
    x->x_voutlet = outlet_new((t_object *)x, &s_float);
    return (x);
}

void sustain_setup(void)
{
    sustain_class = class_new(gensym("sustain"), 
			      (t_newmethod)sustain_new,
			      0,  /* CHECKED: no flushout */
			      sizeof(t_sustain), 0, 0);
    class_addfloat(sustain_class, sustain_float);
    class_addbang(sustain_class, sustain_bang);
    class_addmethod(sustain_class, (t_method)sustain_ft2,
		    gensym("ft2"), A_FLOAT, 0);
}
