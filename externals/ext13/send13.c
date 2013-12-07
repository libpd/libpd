#include "m_pd.h"

#include <string.h>
#include <stdio.h>

/* ------------------------- send13 ------------------------------ */
/*  this is pd's biultin send13 plus the hack for "set" messages   */
/*  it's a pitty, but i don't know where this patch originally     */
/*  came from, so i can'z give correct credits...                  */
/* --------------------------------------------------------------- */


static t_class *send13_class;

typedef struct _send13
{
    t_object x_obj;
    t_symbol *x_sym;
} t_send13;

static void send13_bang(t_send13 *x)
{
    if (x->x_sym->s_thing) pd_bang(x->x_sym->s_thing);
}

 static void send13_set(t_send13 *x, t_symbol *s)
{
  x->x_sym = s;
}

static void send13_float(t_send13 *x, t_float f)
{
    if (x->x_sym->s_thing) pd_float(x->x_sym->s_thing, f);
}

static void send13_symbol(t_send13 *x, t_symbol *s)
{
    if (x->x_sym->s_thing) pd_symbol(x->x_sym->s_thing, s);
}

static void send13_pointer(t_send13 *x, t_gpointer *gp)
{
    if (x->x_sym->s_thing) pd_pointer(x->x_sym->s_thing, gp);
}

static void send13_list(t_send13 *x, t_symbol *s, int argc, t_atom *argv)
{
    if (x->x_sym->s_thing) pd_list(x->x_sym->s_thing, s, argc, argv);
}

static void send13_anything(t_send13 *x, t_symbol *s, int argc, t_atom *argv)
{
    if (x->x_sym->s_thing) typedmess(x->x_sym->s_thing, s, argc, argv);
}

static void *send13_new(t_symbol *s)
{
    t_send13 *x = (t_send13 *)pd_new(send13_class);
    x->x_sym = s;
    return (x);
}

void send13_setup(void)
{
    send13_class = class_new(gensym("send13"), (t_newmethod)send13_new, 0,
    	sizeof(t_send13), 0, A_DEFSYM, 0);
    class_addcreator((t_newmethod)send13_new, gensym("s13"), A_DEFSYM, 0);
    
    class_addbang(send13_class, send13_bang);
    class_addfloat(send13_class, send13_float);
    class_addsymbol(send13_class, send13_symbol);
    class_addpointer(send13_class, send13_pointer);
    class_addlist(send13_class, send13_list);
    class_addanything(send13_class, send13_anything);
    class_addmethod(send13_class, (t_method)send13_set, gensym("set"), A_SYMBOL, 0);
}
