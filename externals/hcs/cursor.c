/* TODO add reset method for cursor icons, this should probably be done in
pd.tk, or cursor reset method could be done in help patch */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <m_pd.h>
#include <m_imp.h>
#include "g_canvas.h"

#define POLLTIME 10

static t_symbol *button_symbol;
static t_symbol *mousewheel_symbol;
static t_symbol *motion_symbol;
static t_symbol *x_symbol;
static t_symbol *y_symbol;
static t_symbol *cursor_receive_symbol;

t_int cursor_instances_polling;

static t_class *cursor_class;

typedef struct _cursor
{
    t_object x_obj;
    t_int    am_polling;
    t_symbol *receive_symbol;
    t_canvas *parent_canvas;
    t_outlet *data_outlet;
//    t_outlet *status_outlet; // not used (yet?)
} t_cursor;

static void cursor_setmethod(t_cursor *x, t_symbol *s, int argc, t_atom *argv)
{
    sys_vgui("set ::cursor_%s \"%s\"\n", s->s_name, atom_getsymbol(argv)->s_name);
    canvas_setcursor(x->parent_canvas, 0); /* hack to refresh the cursor */
}
 
static void cursor_bang(t_cursor *x)
{
    sys_vgui("pdsend \"%s motion [winfo pointerxy .]\"",
             x->receive_symbol->s_name);
}

static void cursor_float(t_cursor *x, t_float f)
{
    /* "bind all <Motion> only gives data when over windows, so its commented
     * out. See the cursor_bang function to see the pointer x,y data getting */
    if(f == 0) {
        if (x->am_polling) {
            x->am_polling = 0;
            cursor_instances_polling--;
            /* if no more objects are listening, stop sending the events */
            if (cursor_instances_polling == 0)
                sys_gui("::hcs::cursor::stoppolling \n");
            pd_unbind(&x->x_obj.ob_pd, cursor_receive_symbol);
        }
    } else {
        if ( ! x->am_polling) {
            x->am_polling = 1;
            pd_bind(&x->x_obj.ob_pd, cursor_receive_symbol);
            cursor_instances_polling++;
            /* if this is the first instance to start, set up Tcl binding and polling */
            if (cursor_instances_polling == 1)
                sys_gui("::hcs::cursor::startpolling\n");
        }
    }
}

static void cursor_button_callback(t_cursor *x, t_float button, t_float state)
{
    t_atom output_atoms[2];
    
    SETFLOAT(output_atoms, button);
    SETFLOAT(output_atoms + 1, state);
    outlet_anything(x->data_outlet, button_symbol, 2, output_atoms);
}

static void cursor_motion_callback(t_cursor *x, t_float x_position, t_float y_position)
{
    t_atom output_atoms[2];
    
    SETSYMBOL(output_atoms, x_symbol);
    SETFLOAT(output_atoms + 1, x_position);
    outlet_anything(x->data_outlet, motion_symbol, 2, output_atoms);
    SETSYMBOL(output_atoms, y_symbol);
    SETFLOAT(output_atoms + 1, y_position);
    outlet_anything(x->data_outlet, motion_symbol, 2, output_atoms);
}
 
static void cursor_mousewheel_callback(t_cursor *x, t_float f)
{
    t_atom output_atom;
    
    SETFLOAT(&output_atom, f);
    outlet_anything(x->data_outlet, mousewheel_symbol, 1, &output_atom);
}

static void cursor_free(t_cursor *x)
{
    cursor_float(x, 0);
    pd_unbind(&x->x_obj.ob_pd, x->receive_symbol);
}

static void *cursor_new(void)
{
    char buf[MAXPDSTRING];
    t_cursor *x = (t_cursor *)pd_new(cursor_class);

    x->parent_canvas = canvas_getcurrent();

    sprintf(buf, "#%lx", (t_int)x);
    x->receive_symbol = gensym(buf);
    pd_bind(&x->x_obj.ob_pd, x->receive_symbol);
	x->data_outlet = outlet_new(&x->x_obj, 0);
	//x->status_outlet = outlet_new(&x->x_obj, 0);

    x->am_polling = 0;

    return(x);
}

void cursor_setup(void)
{
    cursor_class = class_new(gensym("cursor"),
        (t_newmethod)cursor_new, (t_method)cursor_free,
        sizeof(t_cursor), 0, 0);

    class_addbang(cursor_class, (t_method)cursor_bang);
    class_addfloat(cursor_class, (t_method)cursor_float);

    button_symbol = gensym("button");
    mousewheel_symbol = gensym("mousewheel");
    motion_symbol = gensym("motion");
    x_symbol = gensym("x");
    y_symbol = gensym("y");
    //status_symbol = gensym("status");
    cursor_receive_symbol = gensym("#hcs_cursor_class_receive");

    class_addmethod(cursor_class, (t_method)cursor_button_callback, 
                    button_symbol, A_DEFFLOAT, A_DEFFLOAT, 0);
    class_addmethod(cursor_class, (t_method)cursor_motion_callback, 
                    motion_symbol, A_DEFFLOAT, A_DEFFLOAT, 0);
    class_addmethod(cursor_class, (t_method)cursor_mousewheel_callback, 
                    mousewheel_symbol, A_DEFFLOAT, 0);

    /* methods for setting the cursor icon */
    class_addmethod(cursor_class, (t_method)cursor_setmethod, 
                    gensym("runmode_nothing"), A_GIMME, 0);
    class_addmethod(cursor_class, (t_method)cursor_setmethod, 
                    gensym("runmode_clickme"), A_GIMME, 0);
    class_addmethod(cursor_class, (t_method)cursor_setmethod, 
                    gensym("runmode_thicken"), A_GIMME, 0);
    class_addmethod(cursor_class, (t_method)cursor_setmethod, 
                    gensym("runmode_addpoint"), A_GIMME, 0);
    class_addmethod(cursor_class, (t_method)cursor_setmethod, 
                    gensym("editmode_nothing"), A_GIMME, 0);
    class_addmethod(cursor_class, (t_method)cursor_setmethod, 
                    gensym("editmode_connect"), A_GIMME, 0);
    class_addmethod(cursor_class, (t_method)cursor_setmethod, 
                    gensym("editmode_disconnect"), A_GIMME, 0);

    sys_vgui("eval [read [open {%s/%s.tcl}]]\n",
             cursor_class->c_externdir->s_name,
             cursor_class->c_name->s_name);
    sys_vgui("::hcs::cursor::setup %s\n", cursor_receive_symbol->s_name);
}
