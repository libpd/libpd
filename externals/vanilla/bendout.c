/* Copyright (c) 1997-2001 Miller Puckette and others.
* For information on usage and redistribution, and for a DISCLAIMER OF ALL
* WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

/* MIDI. */

#include "m_pd.h"

void outmidi_pitchbend(int portno, int channel, int value);

static t_class *bendout_class;

typedef struct _bendout
{
    t_object x_obj;
    t_float x_channel;
} t_bendout;

static void *bendout_new(t_floatarg channel)
{
    t_bendout *x = (t_bendout *)pd_new(bendout_class);
    if (channel <= 0) channel = 1;
    x->x_channel = channel;
    floatinlet_new(&x->x_obj, &x->x_channel);
    return (x);
}

static void bendout_float(t_bendout *x, t_float f)
{
    int binchan = x->x_channel - 1;
    int n = (int)f +  8192;
    if (binchan < 0)
        binchan = 0;
    outmidi_pitchbend((binchan >> 4), (binchan & 15), n);
}

void bendout_setup(void)
{
    bendout_class = class_new(gensym("bendout"), (t_newmethod)bendout_new, 0,
        sizeof(t_bendout), 0, A_DEFFLOAT, 0);
    class_addfloat(bendout_class, bendout_float);

}
