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

static t_class *namecanvas_class;

typedef struct _namecanvas
{
    t_object x_obj;
    t_symbol *x_sym;
    t_pd *x_owner;
} t_namecanvas;

static void *namecanvas_new(t_symbol *s)
{
    t_namecanvas *x = (t_namecanvas *)pd_new(namecanvas_class);
    x->x_owner = (t_pd *)canvas_getcurrent();
    x->x_sym = s;
    if (*s->s_name) pd_bind(x->x_owner, s);
    return (x);
}

static void namecanvas_free(t_namecanvas *x)
{
    if (*x->x_sym->s_name) pd_unbind(x->x_owner, x->x_sym);
}

void namecanvas_setup(void)
{
    namecanvas_class = class_new(gensym("namecanvas"),
        (t_newmethod)namecanvas_new, (t_method)namecanvas_free,
            sizeof(t_namecanvas), CLASS_NOINLET, A_DEFSYM, 0);
}
