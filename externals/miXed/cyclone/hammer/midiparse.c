/* Copyright (c) 2002-2003 krzYszcz and others.
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

#include "m_pd.h"

typedef struct _midiparse
{
    t_object       x_ob;
    unsigned char  x_ready;
    unsigned char  x_status;
    unsigned char  x_channel;
    unsigned char  x_data1;
    t_outlet      *x_polyout;
    t_outlet      *x_ctlout;
    t_outlet      *x_pgmout;
    t_outlet      *x_touchout;
    t_outlet      *x_bendout;
    t_outlet      *x_chanout;
} t_midiparse;

static t_class *midiparse_class;

static void midiparse_clear(t_midiparse *x)
{
    x->x_status = 0;
    x->x_ready = 0;
}

static void midiparse_float(t_midiparse *x, t_float f)
{
    int ival = (int)f;  /* CHECKED */
    if (ival < 0)
    {
	/* CHECKME */
	return;
    }
    if (ival < 256)  /* CHECKED clear if input over 255 */
    {
	unsigned char bval = ival;
	if (bval & 0x80)
	{
	    unsigned char status = bval & 0xF0;
	    if (status == 0xF0)
	    {
		/* CHECKED no such test in max -- this is incompatible,
		   but real-time messages are out-of-band, and they
		   should be ignored here.  LATER rethink the 0xFE case. */
		if (bval < 0xF8)
		    midiparse_clear(x);
	    }
	    else
	    {
		x->x_status = status;
		x->x_channel = bval & 0x0F;
		x->x_ready = (status == 0xC0 || status == 0xD0);
	    }
	}
	else if (x->x_ready)
	{
	    t_atom at[2];
	    x->x_ready = 0;
	    outlet_float(x->x_chanout, x->x_channel + 1);
	    switch (x->x_status)
	    {
	    case 0x80:
		SETFLOAT(&at[0], x->x_data1);
		SETFLOAT(&at[1], 0);
		outlet_list(((t_object *)x)->ob_outlet, 0, 2, at);
		break;
	    case 0x90:
		SETFLOAT(&at[0], x->x_data1);
		SETFLOAT(&at[1], bval);
		outlet_list(((t_object *)x)->ob_outlet, 0, 2, at);
		break;
	    case 0xA0:
		SETFLOAT(&at[0], bval);
		SETFLOAT(&at[1], x->x_data1);
		outlet_list(x->x_polyout, 0, 2, at);
		break;
	    case 0xB0:
		SETFLOAT(&at[0], bval);
		SETFLOAT(&at[1], x->x_data1);
		outlet_list(x->x_ctlout, 0, 2, at);
		break;
	    case 0xC0:
		outlet_float(x->x_pgmout, bval);
		x->x_ready = 1;
		break;
	    case 0xD0:
		outlet_float(x->x_touchout, bval);
		x->x_ready = 1;
		break;
	    case 0xE0:
		/* CHECKED: ignores data1 */
		outlet_float(x->x_bendout, bval);
		break;
	    default:;
	    }
	}
	else if (x->x_status)
	{
	    x->x_data1 = bval;  /* CHECKED key #0 accepted */
	    x->x_ready = 1;
	}
    }
    else midiparse_clear(x);
}

static void *midiparse_new(void)
{
    t_midiparse *x = (t_midiparse *)pd_new(midiparse_class);
    outlet_new((t_object *)x, &s_list);
    x->x_polyout = outlet_new((t_object *)x, &s_list);
    x->x_ctlout = outlet_new((t_object *)x, &s_list);
    x->x_pgmout = outlet_new((t_object *)x, &s_float);
    x->x_touchout = outlet_new((t_object *)x, &s_float);
    x->x_bendout = outlet_new((t_object *)x, &s_float);
    x->x_chanout = outlet_new((t_object *)x, &s_float);
    midiparse_clear(x);
    return (x);
}

void midiparse_setup(void)
{
    midiparse_class = class_new(gensym("midiparse"), 
				(t_newmethod)midiparse_new, 0,
				sizeof(t_midiparse), 0, 0);
    class_addbang(midiparse_class, midiparse_clear);
    class_addfloat(midiparse_class, midiparse_float);
    /* CHECKED autocasting lists to floats */
}
