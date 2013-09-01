/* Copyright (c) 1997-2001 Miller Puckette and others.
* For information on usage and redistribution, and for a DISCLAIMER OF ALL
* WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

/* MIDI. */

#include "m_pd.h"
void outmidi_polyaftertouch(int portno, int channel, int pitch, int value);

static t_class *polytouchout_class;

typedef struct _polytouchout
{
    t_object x_obj;
    t_float x_channel;
    t_float x_pitch;
} t_polytouchout;

static void *polytouchout_new(t_floatarg channel)
{
    t_polytouchout *x = (t_polytouchout *)pd_new(polytouchout_class);
    if (channel <= 0) channel = 1;
    x->x_channel = channel;
    x->x_pitch = 0;
    floatinlet_new(&x->x_obj, &x->x_pitch);
    floatinlet_new(&x->x_obj, &x->x_channel);
    return (x);
}

static void polytouchout_float(t_polytouchout *x, t_float n)
{
    int binchan = x->x_channel - 1;
    if (binchan < 0)
        binchan = 0;
    outmidi_polyaftertouch((binchan >> 4), (binchan & 15), x->x_pitch, n);
}

void polytouchout_setup(void)
{
    polytouchout_class = class_new(gensym("polytouchout"), 
        (t_newmethod)polytouchout_new, 0,
        sizeof(t_polytouchout), 0, A_DEFFLOAT, 0);
    class_addfloat(polytouchout_class, polytouchout_float);

}
