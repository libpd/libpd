/* Copyright (c) 1997-2001 Miller Puckette and others.
* For information on usage and redistribution, and for a DISCLAIMER OF ALL
* WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

/* MIDI. */

#include "m_pd.h"

static t_class *makenote_class;

typedef struct _hang
{
    t_clock *h_clock;
    struct _hang *h_next;
    t_float h_pitch;
    struct _makenote *h_owner;
} t_hang;

typedef struct _makenote
{
    t_object x_obj;
    t_float x_velo;
    t_float x_dur;
    t_outlet *x_pitchout;
    t_outlet *x_velout;
    t_hang *x_hang;
} t_makenote;

static void *makenote_new(t_floatarg velo, t_floatarg dur)
{
    t_makenote *x = (t_makenote *)pd_new(makenote_class);
    x->x_velo = velo;
    x->x_dur = dur;
    floatinlet_new(&x->x_obj, &x->x_velo);
    floatinlet_new(&x->x_obj, &x->x_dur);
    x->x_pitchout = outlet_new(&x->x_obj, &s_float);
    x->x_velout = outlet_new(&x->x_obj, &s_float);
    x->x_hang = 0;
    return (x);
}

static void makenote_tick(t_hang *hang)
{
    t_makenote *x = hang->h_owner;
    t_hang *h2, *h3;
    outlet_float(x->x_velout, 0);
    outlet_float(x->x_pitchout, hang->h_pitch);
    if (x->x_hang == hang) x->x_hang = hang->h_next;
    else for (h2 = x->x_hang; h3 = h2->h_next; h2 = h3)
    {
        if (h3 == hang)
        {
            h2->h_next = h3->h_next;
            break;
        }
    }
    clock_free(hang->h_clock);
    freebytes(hang, sizeof(*hang));
}

static void makenote_float(t_makenote *x, t_float f)
{
    t_hang *hang;
    if (!x->x_velo) return;
    outlet_float(x->x_velout, x->x_velo);
    outlet_float(x->x_pitchout, f);
    hang = (t_hang *)getbytes(sizeof *hang);
    hang->h_next = x->x_hang;
    x->x_hang = hang;
    hang->h_pitch = f;
    hang->h_owner = x;
    hang->h_clock = clock_new(hang, (t_method)makenote_tick);
    clock_delay(hang->h_clock, (x->x_dur >= 0 ? x->x_dur : 0));
}

static void makenote_stop(t_makenote *x)
{
    t_hang *hang;
    while (hang = x->x_hang)
    {
        outlet_float(x->x_velout, 0);
        outlet_float(x->x_pitchout, hang->h_pitch);
        x->x_hang = hang->h_next;
        clock_free(hang->h_clock);
        freebytes(hang, sizeof(*hang));
    }
}

static void makenote_clear(t_makenote *x)
{
    t_hang *hang;
    while (hang = x->x_hang)
    {
        x->x_hang = hang->h_next;
        clock_free(hang->h_clock);
        freebytes(hang, sizeof(*hang));
    }
}

void makenote_setup(void)
{
    makenote_class = class_new(gensym("makenote"), 
        (t_newmethod)makenote_new, (t_method)makenote_clear,
        sizeof(t_makenote), 0, A_DEFFLOAT, A_DEFFLOAT, 0);
    class_addfloat(makenote_class, makenote_float);
    class_addmethod(makenote_class, (t_method)makenote_stop, gensym("stop"),
        0);
    class_addmethod(makenote_class, (t_method)makenote_clear, gensym("clear"),
        0);
}
