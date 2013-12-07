/* Copyright (c) 1997-2001 Miller Puckette and others.
* For information on usage and redistribution, and for a DISCLAIMER OF ALL
* WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

/* MIDI. */

#include "m_pd.h"

static t_class *stripnote_class;

typedef struct _hang
{
    t_clock *h_clock;
    struct _hang *h_next;
    t_float h_pitch;
    struct _makenote *h_owner;
} t_hang;

typedef struct _stripnote
{
    t_object x_obj;
    t_float x_velo;
    t_outlet *x_pitchout;
    t_outlet *x_velout;
} t_stripnote;

static void *stripnote_new(void )
{
    t_stripnote *x = (t_stripnote *)pd_new(stripnote_class);
    floatinlet_new(&x->x_obj, &x->x_velo);
    x->x_pitchout = outlet_new(&x->x_obj, &s_float);
    x->x_velout = outlet_new(&x->x_obj, &s_float);
    return (x);
}
    
static void stripnote_float(t_stripnote *x, t_float f)
{
    t_hang *hang;
    if (!x->x_velo) return;
    outlet_float(x->x_velout, x->x_velo);
    outlet_float(x->x_pitchout, f);
}

void stripnote_setup(void)
{
    stripnote_class = class_new(gensym("stripnote"),
        (t_newmethod)stripnote_new, 0, sizeof(t_stripnote), 0, 0);
    class_addfloat(stripnote_class, stripnote_float);
}
