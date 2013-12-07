/* (C) 2005 Guenter Geiger */

#include "m_pd.h"

// HACK !!!
struct _glist
{
    t_object gl_obj;            /* header in case we're a glist */
    t_gobj *gl_list;            /* the actual data */
    struct _gstub *gl_stub;     /* safe pointer handler */
    int gl_valid;               /* incremented when pointers might be stale */
    struct _glist *gl_owner;    /* parent glist, supercanvas, or 0 if none */
};
// END HACK !!!


typedef struct getdir
{
    t_object x_ob;
    t_canvas * x_canvas;
    t_outlet* x_outlet;
    int x_level;
} t_getdir;


static void getdir_bang(t_getdir *x)
{
    int i = x->x_level;
    t_canvas* last = x->x_canvas;

    while (i>0) {
        i--;
        if (last->gl_owner) last = last->gl_owner;
    }

    outlet_symbol(x->x_outlet,canvas_getdir(last));
}

t_class *getdir_class;

static void *getdir_new(t_floatarg level)
{
    t_getdir *x = (t_getdir *)pd_new(getdir_class);
    x->x_canvas =  canvas_getcurrent();
    x->x_outlet =  outlet_new(&x->x_ob, &s_);
    x->x_level  =  level;
    return (void *)x;
}

void getdir_setup(void)
{
    getdir_class = class_new(gensym("getdir"), (t_newmethod)getdir_new, 0,
    	sizeof(t_getdir), 0, A_DEFFLOAT,0);
    class_addbang(getdir_class, getdir_bang);
}

