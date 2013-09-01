/* Copyright (c) 1997-2000 Miller Puckette.
* For information on usage and redistribution, and for a DISCLAIMER OF ALL
* WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

/* dialogs.  LATER, deal with the situation where the object goes 
away before the panel does... */

#include "m_pd.h"
#include <stdio.h>
#include <string.h>

static t_class *openpanel_class;

typedef struct _openpanel
{
    t_object x_obj;
    t_symbol *x_s;
} t_openpanel;

static void *openpanel_new( void)
{
    char buf[50];
    t_openpanel *x = (t_openpanel *)pd_new(openpanel_class);
    sprintf(buf, "d%lx", (t_int)x);
    x->x_s = gensym(buf);
    pd_bind(&x->x_obj.ob_pd, x->x_s);
    outlet_new(&x->x_obj, &s_symbol);
    return (x);
}

static void openpanel_symbol(t_openpanel *x, t_symbol *s)
{
    char *path = (s && s->s_name) ? s->s_name : "\"\"";
    sys_vgui("pdtk_openpanel {%s} {%s}\n", x->x_s->s_name, path);
}

static void openpanel_bang(t_openpanel *x)
{
    openpanel_symbol(x, &s_);
}

static void openpanel_callback(t_openpanel *x, t_symbol *s)
{
    outlet_symbol(x->x_obj.ob_outlet, s);
}


static void openpanel_free(t_openpanel *x)
{
    pd_unbind(&x->x_obj.ob_pd, x->x_s);
}

void openpanel_setup(void)
{
    openpanel_class = class_new(gensym("openpanel"),
        (t_newmethod)openpanel_new, (t_method)openpanel_free,
        sizeof(t_openpanel), 0, 0);
    class_addbang(openpanel_class, openpanel_bang);
    class_addsymbol(openpanel_class, openpanel_symbol);
    class_addmethod(openpanel_class, (t_method)openpanel_callback,
        gensym("callback"), A_SYMBOL, 0);
}
