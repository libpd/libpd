/* Copyright (c) 1997-2001 Miller Puckette and others.
* For information on usage and redistribution, and for a DISCLAIMER OF ALL
* WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

/* MIDI. */

#include "m_pd.h"

static t_class *bag_class;

typedef struct _bagelem
{
    struct _bagelem *e_next;
    t_float e_value;
} t_bagelem;

typedef struct _bag
{
    t_object x_obj;
    t_float x_velo;
    t_bagelem *x_first;
} t_bag;

static void *bag_new(void )
{
    t_bag *x = (t_bag *)pd_new(bag_class);
    x->x_velo = 0;
    floatinlet_new(&x->x_obj, &x->x_velo);
    outlet_new(&x->x_obj, &s_float);
    x->x_first = 0;
    return (x);
}

static void bag_float(t_bag *x, t_float f)
{
    t_bagelem *bagelem, *e2, *e3;
    if (x->x_velo != 0)
    {
        bagelem = (t_bagelem *)getbytes(sizeof *bagelem);
        bagelem->e_next = 0;
        bagelem->e_value = f;
        if (!x->x_first) x->x_first = bagelem;
        else    /* LATER replace with a faster algorithm */
        {
            for (e2 = x->x_first; e3 = e2->e_next; e2 = e3)
                ;
            e2->e_next = bagelem;
        }
    }
    else
    {
        if (!x->x_first) return;
        if (x->x_first->e_value == f)
        {
            bagelem = x->x_first;
            x->x_first = x->x_first->e_next;
            freebytes(bagelem, sizeof(*bagelem));
            return;
        }
        for (e2 = x->x_first; e3 = e2->e_next; e2 = e3)
            if (e3->e_value == f)
        {
            e2->e_next = e3->e_next;
            freebytes(e3, sizeof(*e3));
            return;
        }
    }
}

static void bag_flush(t_bag *x)
{
    t_bagelem *bagelem;
    while (bagelem = x->x_first)
    {
        outlet_float(x->x_obj.ob_outlet, bagelem->e_value);
        x->x_first = bagelem->e_next;
        freebytes(bagelem, sizeof(*bagelem));
    }
}

static void bag_clear(t_bag *x)
{
    t_bagelem *bagelem;
    while (bagelem = x->x_first)
    {
        x->x_first = bagelem->e_next;
        freebytes(bagelem, sizeof(*bagelem));
    }
}

void bag_setup(void)
{
    bag_class = class_new(gensym("bag"), 
        (t_newmethod)bag_new, (t_method)bag_clear,
        sizeof(t_bag), 0, 0);
    class_addfloat(bag_class, bag_float);
    class_addmethod(bag_class, (t_method)bag_flush, gensym("flush"), 0);
    class_addmethod(bag_class, (t_method)bag_clear, gensym("clear"), 0);
}
