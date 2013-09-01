/* Copyright (c) 1997-2001 Miller Puckette and others.
* For information on usage and redistribution, and for a DISCLAIMER OF ALL
* WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

/* MIDI. */

#include "m_pd.h"

static t_class *poly_class;

typedef struct voice
{
    t_float v_pitch;
    int v_used;
    unsigned long v_serial;
} t_voice;

typedef struct poly
{
    t_object x_obj;
    int x_n;
    t_voice *x_vec;
    t_float x_vel;
    t_outlet *x_pitchout;
    t_outlet *x_velout;
    unsigned long x_serial;
    int x_steal;
} t_poly;

static void *poly_new(t_float fnvoice, t_float fsteal)
{
    int i, n = fnvoice;
    t_poly *x = (t_poly *)pd_new(poly_class);
    t_voice *v;
    if (n < 1) n = 1;
    x->x_n = n;
    x->x_vec = (t_voice *)getbytes(n * sizeof(*x->x_vec));
    for (v = x->x_vec, i = n; i--; v++)
        v->v_pitch = v->v_used = v->v_serial = 0;
    x->x_vel = 0;
    x->x_steal = (fsteal != 0);
    floatinlet_new(&x->x_obj, &x->x_vel);
    outlet_new(&x->x_obj, &s_float);
    x->x_pitchout = outlet_new(&x->x_obj, &s_float);
    x->x_velout = outlet_new(&x->x_obj, &s_float);
    x->x_serial = 0;
    return (x);
}

static void poly_float(t_poly *x, t_float f)
{
    int i;
    t_voice *v;
    t_voice *firston, *firstoff;
    unsigned int serialon, serialoff, onindex = 0, offindex = 0;
    if (x->x_vel > 0)
    {
            /* note on.  Look for a vacant voice */
        for (v = x->x_vec, i = 0, firston = firstoff = 0,
            serialon = serialoff = 0xffffffff; i < x->x_n; v++, i++)
        {
            if (v->v_used && v->v_serial < serialon)
                    firston = v, serialon = v->v_serial, onindex = i;
            else if (!v->v_used && v->v_serial < serialoff)
                    firstoff = v, serialoff = v->v_serial, offindex = i;
        }
        if (firstoff)
        {
            outlet_float(x->x_velout, x->x_vel);
            outlet_float(x->x_pitchout, firstoff->v_pitch = f);
            outlet_float(x->x_obj.ob_outlet, offindex+1);
            firstoff->v_used = 1;
            firstoff->v_serial = x->x_serial++;
        }
            /* if none, steal one */
        else if (firston && x->x_steal)
        {
            outlet_float(x->x_velout, 0);
            outlet_float(x->x_pitchout, firston->v_pitch);
            outlet_float(x->x_obj.ob_outlet, onindex+1);
            outlet_float(x->x_velout, x->x_vel);
            outlet_float(x->x_pitchout, firston->v_pitch = f);
            outlet_float(x->x_obj.ob_outlet, onindex+1);
            firston->v_serial = x->x_serial++;
        }
    }
    else    /* note off. Turn off oldest match */
    {
        for (v = x->x_vec, i = 0, firston = 0, serialon = 0xffffffff;
            i < x->x_n; v++, i++)
                if (v->v_used && v->v_pitch == f && v->v_serial < serialon)
                    firston = v, serialon = v->v_serial, onindex = i;
        if (firston)
        {
            firston->v_used = 0;
            firston->v_serial = x->x_serial++;
            outlet_float(x->x_velout, 0);
            outlet_float(x->x_pitchout, firston->v_pitch);
            outlet_float(x->x_obj.ob_outlet, onindex+1);
        }
    }
}

static void poly_stop(t_poly *x)
{
    int i;
    t_voice *v;
    for (i = 0, v = x->x_vec; i < x->x_n; i++, v++)
        if (v->v_used)
    {
        outlet_float(x->x_velout, 0L);
        outlet_float(x->x_pitchout, v->v_pitch);
        outlet_float(x->x_obj.ob_outlet, i+1);
        v->v_used = 0;
        v->v_serial = x->x_serial++;
    }
}

static void poly_clear(t_poly *x)
{
    int i;
    t_voice *v;
    for (v = x->x_vec, i = x->x_n; i--; v++) v->v_used = v->v_serial = 0;
}

static void poly_free(t_poly *x)
{
    freebytes(x->x_vec, x->x_n * sizeof (*x->x_vec));
}

void poly_setup(void)
{
    poly_class = class_new(gensym("poly"), 
        (t_newmethod)poly_new, (t_method)poly_free,
        sizeof(t_poly), 0, A_DEFFLOAT, A_DEFFLOAT, 0);
    class_addfloat(poly_class, poly_float);
    class_addmethod(poly_class, (t_method)poly_stop, gensym("stop"), 0);
    class_addmethod(poly_class, (t_method)poly_clear, gensym("clear"), 0);
}
