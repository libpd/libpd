/* Copyright (c) 1997-2001 Miller Puckette and others.
* For information on usage and redistribution, and for a DISCLAIMER OF ALL
* WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

/* MIDI. */

#include "m_pd.h"
void outmidi_aftertouch(int portno, int channel, int value);

static t_class *touchout_class;

typedef struct _touchout
{
    t_object x_obj;
    t_float x_channel;
} t_touchout;

static void *touchout_new(t_floatarg channel)
{
    t_touchout *x = (t_touchout *)pd_new(touchout_class);
    if (channel <= 0) channel = 1;
    x->x_channel = channel;
    floatinlet_new(&x->x_obj, &x->x_channel);
    return (x);
}

static void touchout_float(t_touchout *x, t_float f)
{
    int binchan = x->x_channel - 1;
    if (binchan < 0)
        binchan = 0;
    outmidi_aftertouch((binchan >> 4), (binchan & 15), (int)f);
}

void touchout_setup(void)
{
    touchout_class = class_new(gensym("touchout"), (t_newmethod)touchout_new, 0,
        sizeof(t_touchout), 0, A_DEFFLOAT, 0);
    class_addfloat(touchout_class, touchout_float);

}
