#include <stdio.h>
#include <string.h>
#include <m_pd.h>
#include "g_canvas.h"

#define DEBUG(x)

static t_class *window_name_class;

typedef struct _window_name
{
    t_object  x_obj;
    t_canvas  *x_canvas;
    t_symbol  *receive_name;
    t_int     depth;
} t_window_name;

static t_symbol* make_canvas_symbol(t_canvas* canvas)
{
        char buf[MAXPDSTRING];
        snprintf(buf, MAXPDSTRING, ".x%lx", (long unsigned int)canvas);
        return gensym(buf);
}

static t_symbol* get_canvas_by_depth(t_window_name *x)
{
    t_canvas *canvas = x->x_canvas;
    int depth = x->depth;
    if(depth<0)depth=0;
    while(depth && canvas)
    {
        canvas = canvas->gl_owner;
        depth--;
    }
    return make_canvas_symbol(canvas);
}

static t_symbol* get_canvas_by_name(t_window_name *x)
{
    return make_canvas_symbol((t_canvas *)pd_findbyclass(x->receive_name, canvas_class));
}

static t_symbol* get_window_name(t_window_name *x)
{
    if(x->receive_name == &s_)
        return get_canvas_by_depth(x);
    else
        return get_canvas_by_name(x);
}

static void window_name_bang(t_window_name *x)
{
    /* actually get the canvas name each time to make sure we get
     * accurate info in case of changes.  If we cache the result, a
     * canvas could have been deleted or renamed. */
    outlet_symbol(x->x_obj.ob_outlet, get_window_name(x));
}

static void window_name_anything(t_window_name *x, t_symbol *s, int argc, t_atom *argv)
{ 
    t_symbol *first_symbol = atom_getsymbolarg(0,argc,argv);
    x->depth = 0;
    x->receive_name = &s_;
    if(s == &s_float)
        x->depth = (t_int) atom_getfloatarg(0,argc,argv);
    else if (first_symbol == &s_)
        x->receive_name = s;
    else
        x->receive_name = first_symbol;
    window_name_bang(x);
}

static void *window_name_new(t_symbol *s, int argc, t_atom *argv)
{
    t_window_name *x = (t_window_name *)pd_new(window_name_class);

    t_glist* glist = (t_glist *)canvas_getcurrent(); 
    x->x_canvas = (t_canvas *)glist_getcanvas(glist);
 
    t_symbol *tmp = atom_getsymbolarg(0,argc,argv);
    x->depth = 0;
    x->receive_name = &s_;
    if(tmp == &s_)
        x->depth = (t_int) atom_getfloatarg(0,argc,argv);
    else
        x->receive_name = tmp;
    
	outlet_new(&x->x_obj, &s_symbol);

    return(x);
}

void window_name_setup(void)
{
    window_name_class = class_new(gensym("window_name"),
        (t_newmethod)window_name_new, NULL,
        sizeof(t_window_name), 0, A_GIMME, 0);

    class_addbang(window_name_class, (t_method)window_name_bang);
    class_addanything(window_name_class, (t_method)window_name_anything);
}
