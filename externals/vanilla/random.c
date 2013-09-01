/* Copyright (c) 1997-1999 Miller Puckette.
* For information on usage and redistribution, and for a DISCLAIMER OF ALL
* WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

/* misc. */

#include "m_pd.h"
#include "s_stuff.h"
#include <math.h>
#include <stdio.h>
#include <string.h>
#ifdef _WIN32
#include <wtypes.h>
#include <time.h>
#else
#include <sys/types.h>
#include <sys/time.h>
#include <sys/times.h>
#include <sys/param.h>
#include <unistd.h>
#endif /* _WIN32 */

#if defined (__APPLE__) || defined (__FreeBSD__)
#define CLOCKHZ CLK_TCK
#endif
#if defined (__linux__) || defined (__CYGWIN__) || defined (ANDROID)
#define CLOCKHZ sysconf(_SC_CLK_TCK)
#endif
#if defined (__FreeBSD_kernel__) || defined(__GNU__)
#include <time.h>
#define CLOCKHZ CLOCKS_PER_SEC
#endif

static t_class *random_class;

typedef struct _random
{
    t_object x_obj;
    t_float x_f;
    unsigned int x_state;
} t_random;


static int makeseed(void)
{
    static unsigned int random_nextseed = 1489853723;
    random_nextseed = random_nextseed * 435898247 + 938284287;
    return (random_nextseed & 0x7fffffff);
}

static void *random_new(t_floatarg f)
{
    t_random *x = (t_random *)pd_new(random_class);
    x->x_f = f;
    x->x_state = makeseed();
    floatinlet_new(&x->x_obj, &x->x_f);
    outlet_new(&x->x_obj, &s_float);
    return (x);
}

static void random_bang(t_random *x)
{
    int n = x->x_f, nval;
    int range = (n < 1 ? 1 : n);
    unsigned int randval = x->x_state;
    x->x_state = randval = randval * 472940017 + 832416023;
    nval = ((double)range) * ((double)randval)
        * (1./4294967296.);
    if (nval >= range) nval = range-1;
    outlet_float(x->x_obj.ob_outlet, nval);
}

static void random_seed(t_random *x, t_float f, t_float glob)
{
    x->x_state = f;
}

void random_setup(void)
{
    random_class = class_new(gensym("random"), (t_newmethod)random_new, 0,
        sizeof(t_random), 0, A_DEFFLOAT, 0);
    class_addbang(random_class, random_bang);
    class_addmethod(random_class, (t_method)random_seed,
        gensym("seed"), A_FLOAT, 0);
}
