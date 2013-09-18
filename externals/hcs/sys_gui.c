#include <stdio.h>
#include <string.h>
#include <m_pd.h>
#include "g_canvas.h"

static t_class *sys_gui_class;

typedef struct _sys_gui
{
    t_object x_obj;
    t_symbol* x_receive_symbol;
	t_outlet* x_outlet;
    char *send_buffer;
} t_sys_gui;

static void execute_buffer(t_sys_gui *x, int argc, t_atom *argv)
{
    int i = 0;
    char buf[MAXPDSTRING];

    for(i=0;i<argc;++i)
    {
        atom_string(argv + i, buf, MAXPDSTRING);
        strncat(x->send_buffer, buf, MAXPDSTRING - strlen(x->send_buffer));
        strncat(x->send_buffer, " ", MAXPDSTRING - strlen(x->send_buffer));
    }
    strncat(x->send_buffer, " ;\n", MAXPDSTRING - strlen(x->send_buffer));
    snprintf(buf, MAXPDSTRING - strlen(x->send_buffer),
             "pdsend \"%s  finished\";\n",  x->x_receive_symbol->s_name );
    strncat(x->send_buffer, buf, MAXPDSTRING - strlen(x->send_buffer));
    sys_gui(x->send_buffer);
}

static void sys_gui_bang(t_sys_gui *x)
{
    sys_gui(x->send_buffer);
}

static void sys_gui_finished(t_sys_gui *x)
{
    outlet_bang(x->x_outlet);
}

static void sys_gui_anything(t_sys_gui *x, t_symbol *s, int argc, t_atom *argv)
{
    snprintf(x->send_buffer, MAXPDSTRING, "%s ", s->s_name);
    execute_buffer(x, argc, argv);
}

static void sys_gui_list(t_sys_gui *x, t_symbol *s, int argc, t_atom *argv)
{
    x->send_buffer = '\0';
    execute_buffer(x, argc, argv);
}

static void sys_gui_free(t_sys_gui *x)
{
    pd_unbind(&x->x_obj.ob_pd, x->x_receive_symbol);
    freebytes(x->send_buffer,MAXPDSTRING);
}

static void *sys_gui_new(t_symbol *s)
{
    t_sys_gui *x = (t_sys_gui *)pd_new(sys_gui_class);
	x->x_outlet = outlet_new(&x->x_obj, &s_anything);

    char buf[MAXPDSTRING];
    sprintf(buf, "#%lx", (t_int)x);
    x->x_receive_symbol = gensym(buf);
    pd_bind(&x->x_obj.ob_pd, x->x_receive_symbol);

    x->send_buffer = (char *)getbytes(MAXPDSTRING);

    return(x);
}

void sys_gui_setup(void)
{
    sys_gui_class = class_new(
		gensym("sys_gui"),
        (t_newmethod)sys_gui_new,
		(t_method)sys_gui_free,
        sizeof(t_sys_gui),
		0,
		0);

    class_addanything(sys_gui_class, (t_method)sys_gui_anything);
    class_addbang(sys_gui_class, (t_method)sys_gui_bang);
    class_addlist(sys_gui_class, (t_method)sys_gui_list);
    class_addmethod(sys_gui_class, (t_method)sys_gui_finished, 
                    gensym("finished"), 0);
}
