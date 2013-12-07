/* this object just asks Tcl for the screen resolution and outputs it */
#include <stdio.h>
#include <m_pd.h>

static t_class *screensize_class;

typedef struct _screensize
{
    t_object x_obj;
    t_symbol *receive_symbol;
    t_outlet *width_outlet;
    t_outlet *height_outlet;
} t_screensize;

static void screensize_bang(t_screensize *x)
{
    sys_vgui("pdsend \"%s screensize [winfo screenwidth .] [winfo screenheight .]\"\n",
             x->receive_symbol->s_name);
}

static void screensize_callback(t_screensize *x, t_float width, t_float height)
{
    outlet_float(x->height_outlet, height);
    outlet_float(x->width_outlet, width);
}

static void *screensize_new(void)
{
    char buf[MAXPDSTRING];
    t_screensize *x = (t_screensize *)pd_new(screensize_class);

    sprintf(buf, "#%lx", (t_int)x);
    x->receive_symbol = gensym(buf);
    pd_bind(&x->x_obj.ob_pd, x->receive_symbol);

 	x->width_outlet = outlet_new(&x->x_obj, 0);
	x->height_outlet = outlet_new(&x->x_obj, 0);

    return(x);
}

static void screensize_free(t_screensize *x)
{
    pd_unbind(&x->x_obj.ob_pd, x->receive_symbol);
}

void screensize_setup(void)
{
    screensize_class = class_new(gensym("screensize"),
        (t_newmethod)screensize_new, (t_method)screensize_free,
        sizeof(t_screensize), 0, 0);

    class_addbang(screensize_class, (t_method)screensize_bang);

    class_addmethod(screensize_class, (t_method)screensize_callback, 
                    gensym("screensize"), A_DEFFLOAT, A_DEFFLOAT, 0);
}
