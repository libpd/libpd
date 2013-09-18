#include <stdio.h>
#include <string.h>
#include <m_pd.h>
#include "g_canvas.h"

#define DEBUG(x)

static t_class *canvas_name_class;

typedef struct _canvas_name
{
    t_object  x_obj;
    t_canvas  *x_canvas;
    t_symbol  *receive_name;
    t_int     depth;
} t_canvas_name;

static t_symbol* make_canvas_symbol(t_canvas* canvas)
{
        char buf[MAXPDSTRING];
        snprintf(buf, MAXPDSTRING, ".x%lx.c", (long unsigned int)canvas);
        return gensym(buf);
}

static t_symbol* get_canvas_by_depth(t_canvas_name *x)
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

static t_symbol* get_canvas_by_name(t_canvas_name *x)
{
    return make_canvas_symbol((t_canvas *)pd_findbyclass(x->receive_name, canvas_class));
}

static t_symbol* get_canvas_name(t_canvas_name *x)
{
    if(x->receive_name == &s_)
        return get_canvas_by_depth(x);
    else
        return get_canvas_by_name(x);
}

static void canvas_name_bang(t_canvas_name *x)
{
    /* actually get the canvas name each time to make sure we get
     * accurate info in case of changes.  If we cache the result, a
     * canvas could have been deleted or renamed. */
    outlet_symbol(x->x_obj.ob_outlet, get_canvas_name(x));
}

static void canvas_name_anything(t_canvas_name *x, t_symbol *s, int argc, t_atom *argv)
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
    canvas_name_bang(x);
}

static void *canvas_name_new(t_symbol *s, int argc, t_atom *argv)
{
    t_canvas_name *x = (t_canvas_name *)pd_new(canvas_name_class);

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

void canvas_name_setup(void)
{
    canvas_name_class = class_new(gensym("canvas_name"),
        (t_newmethod)canvas_name_new, NULL,
        sizeof(t_canvas_name), 0, A_GIMME, 0);

    class_addbang(canvas_name_class, (t_method)canvas_name_bang);
    class_addanything(canvas_name_class, (t_method)canvas_name_anything);
}
