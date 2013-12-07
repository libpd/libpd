#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <m_pd.h>
#include <m_imp.h>

static t_class *colorpanel_class;

typedef struct _colorpanel
{
    t_object x_obj;
    t_symbol *x_s;
    char current_color[MAXPDSTRING];
} t_colorpanel;

static void colorpanel_bang(t_colorpanel *x)
{
    sys_vgui("after idle [list after 100 ::hcs::colorpanel::open %s %s]\n",
             x->x_s->s_name, x->current_color);
}

static void colorpanel_symbol(t_colorpanel *x, t_symbol *s)
{
    strncpy(x->current_color, s->s_name, MAXPDSTRING);
    colorpanel_bang(x);
}

static void colorpanel_list(t_colorpanel *x, t_symbol *s, int argc, t_atom *argv)
{
    t_symbol *tmp_symbol = s; /* <-- this gets rid of the unused variable warning */
    int i;
    unsigned int tmp_int;
    char color_buffer[3];
    char color_string[MAXPDSTRING];

    strncpy(color_string,"#",MAXPDSTRING);
    if(argc > 3) 
        logpost(x, 2, "[colorpanel] warning more than three elements in list");
    for(i=0; i<3; i++)
    {
        tmp_symbol = atom_getsymbolarg(i, argc, argv);
        if(tmp_symbol == &s_)
        {
            tmp_int = (unsigned int)(atom_getfloatarg(i, argc , argv) * 255);
            snprintf(color_buffer, 3, "%02x", (tmp_int > 255 ? 255 : tmp_int));
            strncat(color_string, color_buffer, 3);
        }
        else 
        {
            pd_error(x,"[colorpanel] symbols are not allowed in the color list");
            return;
        }
    }
    memcpy(x->current_color, color_string, 7);
    colorpanel_bang(x);
}

static void colorpanel_callback(t_colorpanel *x, t_symbol *color)
{
    t_atom output_atoms[3];
    unsigned int red, green, blue;
    
    if(color != &s_)
    {
        strncpy(x->current_color, color->s_name, MAXPDSTRING);
        sscanf(x->current_color, "#%02x%02x%02x", &red, &green, &blue);
        SETFLOAT(output_atoms, (t_float) red / 255);
        SETFLOAT(output_atoms + 1, (t_float) green / 255);
        SETFLOAT(output_atoms + 2, (t_float) blue / 255);
        outlet_list(x->x_obj.ob_outlet, &s_list, 3, output_atoms);
    }
}

static void colorpanel_free(t_colorpanel *x)
{
    pd_unbind(&x->x_obj.ob_pd, x->x_s);
}

static void *colorpanel_new( void)
{
    char buf[MAXPDSTRING];
    t_colorpanel *x = (t_colorpanel *)pd_new(colorpanel_class);
    sprintf(buf, "#%lx", (t_int)x);
    x->x_s = gensym(buf);
    pd_bind(&x->x_obj.ob_pd, x->x_s);
    outlet_new(&x->x_obj, &s_list);
    strcpy(x->current_color,"#ffffff");
    return(x);
}

void colorpanel_setup(void)
{
    colorpanel_class = class_new(gensym("colorpanel"),
        (t_newmethod)colorpanel_new, (t_method)colorpanel_free,
        sizeof(t_colorpanel), 0, 0);
    class_addbang(colorpanel_class, (t_method)colorpanel_bang);
    class_addsymbol(colorpanel_class, (t_method)colorpanel_symbol);
    class_addlist(colorpanel_class, (t_method)colorpanel_list);
    class_addmethod(colorpanel_class, (t_method)colorpanel_callback, 
                    gensym("callback"), A_DEFSYMBOL, 0);

    sys_vgui("eval [read [open {%s/%s.tcl}]]\n",
             colorpanel_class->c_externdir->s_name,
             colorpanel_class->c_name->s_name);
}
