/* Copyright (c) 1997-2001 Miller Puckette and others.
* For information on usage and redistribution, and for a DISCLAIMER OF ALL
* WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

/* MIDI. */

#include "m_pd.h"

void outmidi_programchange(int portno, int channel, int value);

static t_class *pgmout_class;

typedef struct _pgmout
{
    t_object x_obj;
    t_float x_channel;
} t_pgmout;

static void *pgmout_new(t_floatarg channel)
{
    t_pgmout *x = (t_pgmout *)pd_new(pgmout_class);
    if (channel <= 0) channel = 1;
    x->x_channel = channel;
    floatinlet_new(&x->x_obj, &x->x_channel);
    return (x);
}

static void pgmout_float(t_pgmout *x, t_floatarg f)
{
    int binchan = x->x_channel - 1;
    int n = f - 1;
    if (binchan < 0)
        binchan = 0;
    if (n < 0) n = 0;
    else if (n > 127) n = 127;
    outmidi_programchange((binchan >> 4),
        (binchan & 15), n);
}

void pgmout_setup(void)
{
    pgmout_class = class_new(gensym("pgmout"), (t_newmethod)pgmout_new, 0,
        sizeof(t_pgmout), 0, A_DEFFLOAT, 0);
    class_addfloat(pgmout_class, pgmout_float);

}
