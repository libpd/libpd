/* Copyright (c) 1997-2001 Miller Puckette and others.
* For information on usage and redistribution, and for a DISCLAIMER OF ALL
* WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

/* MIDI. */

#include "m_pd.h"

void outmidi_controlchange(int portno, int channel, int ctlno, int value);

static t_class *ctlout_class;

typedef struct _ctlout
{
    t_object x_obj;
    t_float x_ctl;
    t_float x_channel;
} t_ctlout;

static void *ctlout_new(t_floatarg ctl, t_floatarg channel)
{
    t_ctlout *x = (t_ctlout *)pd_new(ctlout_class);
    x->x_ctl = ctl;
    if (channel <= 0) channel = 1;
    x->x_channel = channel;
    floatinlet_new(&x->x_obj, &x->x_ctl);
    floatinlet_new(&x->x_obj, &x->x_channel);
    return (x);
}

static void ctlout_float(t_ctlout *x, t_float f)
{
    int binchan = x->x_channel - 1;
    if (binchan < 0)
        binchan = 0;
    outmidi_controlchange((binchan >> 4),
        (binchan & 15), (int)(x->x_ctl), (int)f);
}

void ctlout_setup(void)
{
    ctlout_class = class_new(gensym("ctlout"), (t_newmethod)ctlout_new, 0,
        sizeof(t_ctlout), 0, A_DEFFLOAT, A_DEFFLOAT, 0);
    class_addfloat(ctlout_class, ctlout_float);

}
