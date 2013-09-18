#include <stdio.h>
#include <string.h>
#include "m_pd.h"
//#include "g_canvas.h"

// tk_chooseDirectory
/*

proc pdtk_folderpanel {target localdir} {
    if {$localdir == ""} {
    	set filename [tk_getSaveFile]
    } else {
    	set filename [tk_getSaveFile -initialdir $localdir]
    }
    if {$filename != ""} {
        pd [concat $target callback [enquote_path $filename] \;]
    }
}


*/




t_class *folderpanel_class;

typedef struct _folderpanel
{
    t_object x_obj;
    t_symbol *x_s;
} t_folderpanel;


static void folderpanel_symbol(t_folderpanel *x, t_symbol *s)
{
    char *path = (s && s->s_name) ? s->s_name : "\"\"";
    sys_vgui("after idle [list after 100 tof_folderpanel %s]\n", x->x_s->s_name);
}

static void folderpanel_bang(t_folderpanel *x)
{
    folderpanel_symbol(x, &s_);
}

static void folderpanel_callback(t_folderpanel *x, t_symbol *s)
{
    outlet_symbol(x->x_obj.ob_outlet, s);
}

static void folderpanel_free(t_folderpanel *x)
{
    pd_unbind(&x->x_obj.ob_pd, x->x_s);
}

static void *folderpanel_new( void)
{
    char buf[50];
    t_folderpanel *x = (t_folderpanel *)pd_new(folderpanel_class);
    sprintf(buf, "d%lx", (t_int)x);
    x->x_s = gensym(buf);
    pd_bind(&x->x_obj.ob_pd, x->x_s);
    outlet_new(&x->x_obj, &s_symbol);
    return (x);
}

void folderpanel_setup(void)
{
    folderpanel_class = class_new(gensym("folderpanel"),
        (t_newmethod)folderpanel_new, (t_method)folderpanel_free,
        sizeof(t_folderpanel), 0, 0);
    class_addbang(folderpanel_class, folderpanel_bang);
    class_addsymbol(folderpanel_class, folderpanel_symbol);
    class_addmethod(folderpanel_class, (t_method)folderpanel_callback,
        gensym("callback"), A_SYMBOL, 0);

   
	sys_gui("proc tof_folderpanel {target} {\n");
	sys_gui("  set path [tk_chooseDirectory] \n");
	sys_gui(" if {$path != \"\"} {\n");
	sys_gui("  pdsend \"$target callback [enquote_path $path]\"\n");
	sys_gui(" }\n");
	sys_gui("}\n");



}


