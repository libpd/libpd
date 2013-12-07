/* Copyright (c) 1997-2001 Miller Puckette and others.
* For information on usage and redistribution, and for a DISCLAIMER OF ALL
* WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

/* MIDI. */

#include "m_pd.h"

static t_class *midiout_class;

void outmidi_byte(int portno, int value);

typedef struct _midiout
{
    t_object x_obj;
    t_float x_portno;
} t_midiout;

static void *midiout_new(t_floatarg portno)
{
    t_midiout *x = (t_midiout *)pd_new(midiout_class);
    if (portno <= 0) portno = 1;
    x->x_portno = portno;
    floatinlet_new(&x->x_obj, &x->x_portno);
#ifdef __irix__
    post("midiout: unimplemented in IRIX");
#endif
    return (x);
}

static void midiout_float(t_midiout *x, t_floatarg f)
{
    outmidi_byte(x->x_portno - 1, f);
}

void midiout_setup(void)
{
    midiout_class = class_new(gensym("midiout"), (t_newmethod)midiout_new, 0,
        sizeof(t_midiout), 0, A_DEFFLOAT, A_DEFFLOAT, 0);
    class_addfloat(midiout_class, midiout_float);

}
