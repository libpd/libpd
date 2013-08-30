/* Copyright (c) 1997-1999 Miller Puckette.
* For information on usage and redistribution, and for a DISCLAIMER OF ALL
* WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

/* misc. */

#include "m_pd.h"
#include "s_stuff.h"
#include <math.h>
#include <stdio.h>
#include <string.h>
#ifdef UNISTD
#include <sys/types.h>
#include <sys/time.h>
#include <sys/times.h>
#include <sys/param.h>
#include <unistd.h>
#endif
#ifdef MSW
#include <wtypes.h>
#include <time.h>
#endif

static t_class *loadbang_class;

typedef struct _loadbang
{
    t_object x_obj;
} t_loadbang;

static void *loadbang_new(void)
{
    t_loadbang *x = (t_loadbang *)pd_new(loadbang_class);
    outlet_new(&x->x_obj, &s_bang);
    return (x);
}

static void loadbang_loadbang(t_loadbang *x)
{
    if (!sys_noloadbang)
        outlet_bang(x->x_obj.ob_outlet);
}

static void loadbang_anything(t_loadbang *x, t_symbol *s, int argc, t_atom *argv)
{
    outlet_bang(x->x_obj.ob_outlet);
}

void loadbang_setup(void)
{
    loadbang_class = class_new(gensym("loadbang"), (t_newmethod)loadbang_new, 0,
        sizeof(t_loadbang), CLASS_DEFAULT, 0);
    class_addanything(loadbang_class, (t_method)loadbang_anything);
    class_addmethod(loadbang_class, (t_method)loadbang_loadbang,
        gensym("loadbang"), 0);
}
