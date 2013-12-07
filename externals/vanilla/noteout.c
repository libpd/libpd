/* Copyright (c) 1997-2001 Miller Puckette and others.
* For information on usage and redistribution, and for a DISCLAIMER OF ALL
* WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

/* MIDI. */

#include "m_pd.h"

void outmidi_noteon(int portno, int channel, int pitch, int velo);

static t_class *noteout_class;

typedef struct _noteout
{
    t_object x_obj;
    t_float x_velo;
    t_float x_channel;
} t_noteout;

static void *noteout_new(t_floatarg channel)
{
    t_noteout *x = (t_noteout *)pd_new(noteout_class);
    x->x_velo = 0;
    if (channel < 1) channel = 1;
    x->x_channel = channel;
    floatinlet_new(&x->x_obj, &x->x_velo);
    floatinlet_new(&x->x_obj, &x->x_channel);
    return (x);
}

static void noteout_float(t_noteout *x, t_float f)
{
    int binchan = x->x_channel - 1;
    if (binchan < 0)
        binchan = 0;
    outmidi_noteon((binchan >> 4),
        (binchan & 15), (int)f, (int)x->x_velo);
}

void noteout_setup(void)
{
    noteout_class = class_new(gensym("noteout"), (t_newmethod)noteout_new, 0,
        sizeof(t_noteout), 0, A_DEFFLOAT, 0);
    class_addfloat(noteout_class, noteout_float);

}
