#include "m_pd.h"

#include <string.h>
#include <stdio.h>

/* -------------------- receive13 ------------------------------ */
/* this is pd's biultin recieve plus the hack for "set" messages */
/*  it's a pitty, but i don't know where this patch originally   */
/*  came from, so i can'z give correct credits...                */
/* ------------------------------------------------------------- */

static t_class *receive13_class;

typedef struct _receive13
{
    t_object x_obj;
    t_symbol *x_sym;
} t_receive13;

static void receive13_set(t_receive13 *x, t_symbol *s)
{
  pd_unbind(&x->x_obj.ob_pd, x->x_sym);
  x->x_sym = s;
  pd_bind(&x->x_obj.ob_pd, s);
}

static void receive13_bang(t_receive13 *x)
{
    outlet_bang(x->x_obj.ob_outlet);
}

static void receive13_float(t_receive13 *x, t_float f)
{
    outlet_float(x->x_obj.ob_outlet, f);
}

static void receive13_symbol(t_receive13 *x, t_symbol *s)
{
    outlet_symbol(x->x_obj.ob_outlet, s);
}

static void receive13_pointer(t_receive13 *x, t_gpointer *gp)
{
    outlet_pointer(x->x_obj.ob_outlet, gp);
}

static void receive13_list(t_receive13 *x, t_symbol *s, int argc, t_atom *argv)
{
    outlet_list(x->x_obj.ob_outlet, s, argc, argv);
}

static void receive13_anything(t_receive13 *x, t_symbol *s, int argc, t_atom *argv)
{
    outlet_anything(x->x_obj.ob_outlet, s, argc, argv);
}

static void *receive13_new(t_symbol *s)
{
    t_receive13 *x = (t_receive13 *)pd_new(receive13_class);
    x->x_sym = s;
    pd_bind(&x->x_obj.ob_pd, s);
    outlet_new(&x->x_obj, 0);
    return (x);
}

static void receive13_free(t_receive13 *x)
{
    pd_unbind(&x->x_obj.ob_pd, x->x_sym);
}

void receive13_setup(void)
{
    receive13_class = class_new(gensym("receive13"), (t_newmethod)receive13_new, 
        (t_method)receive13_free, sizeof(t_receive13), 0, A_DEFSYM, 0);
    class_addcreator((t_newmethod)receive13_new, gensym("r13"), A_DEFSYM, 0);
    
    class_addbang(receive13_class, receive13_bang);
    class_addfloat(receive13_class, (t_method)receive13_float);
    class_addsymbol(receive13_class, receive13_symbol);
    class_addpointer(receive13_class, receive13_pointer);
    class_addlist(receive13_class, receive13_list);
    class_addanything(receive13_class, receive13_anything);
    class_addmethod(receive13_class, (t_method)receive13_set, gensym("set"), A_SYMBOL, 0);

}

