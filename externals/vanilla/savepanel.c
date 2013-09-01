/* Copyright (c) 1997-2000 Miller Puckette.
* For information on usage and redistribution, and for a DISCLAIMER OF ALL
* WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

/* dialogs.  LATER, deal with the situation where the object goes 
away before the panel does... */

#include "m_pd.h"
#include <stdio.h>
#include <string.h>

static t_class *savepanel_class;

typedef struct _savepanel
{
    t_object x_obj;
    t_canvas *x_canvas;
    t_symbol *x_s;
} t_savepanel;

static void *savepanel_new( void)
{
    char buf[50];
    t_savepanel *x = (t_savepanel *)pd_new(savepanel_class);
    sprintf(buf, "d%lx", (t_int)x);
    x->x_s = gensym(buf);
    x->x_canvas = canvas_getcurrent();
    pd_bind(&x->x_obj.ob_pd, x->x_s);
    outlet_new(&x->x_obj, &s_symbol);
    return (x);
}

static void savepanel_symbol(t_savepanel *x, t_symbol *s)
{
    char *path = (s && s->s_name) ? s->s_name : "\"\"";
    sys_vgui("pdtk_savepanel {%s} {%s}\n", x->x_s->s_name, path);
}

static void savepanel_bang(t_savepanel *x)
{
    savepanel_symbol(x, &s_);
}

static void savepanel_callback(t_savepanel *x, t_symbol *s)
{
    outlet_symbol(x->x_obj.ob_outlet, s);
}

static void savepanel_free(t_savepanel *x)
{
    pd_unbind(&x->x_obj.ob_pd, x->x_s);
}

void savepanel_setup(void)
{
    savepanel_class = class_new(gensym("savepanel"),
        (t_newmethod)savepanel_new, (t_method)savepanel_free,
        sizeof(t_savepanel), 0, 0);
    class_addbang(savepanel_class, savepanel_bang);
    class_addsymbol(savepanel_class, savepanel_symbol);
    class_addmethod(savepanel_class, (t_method)savepanel_callback,
        gensym("callback"), A_SYMBOL, 0);
}
