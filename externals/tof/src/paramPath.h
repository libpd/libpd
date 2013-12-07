

static t_class *paramPath_class;
 

typedef struct _paramPath
{
  t_object                    x_obj;
  t_outlet*			          outlet;
  t_symbol*						path;
  //t_symbol*					root;
} t_paramPath;



// Dump out everything (OR THE ID'S OR JUST THE NAMES?)
static void paramPath_bang(t_paramPath *x) {
	
	outlet_symbol(x->outlet,x->path);
	
    
}



static void paramPath_free(t_paramPath *x)
{
	
	
}


static void *paramPath_new(t_symbol *s, int ac, t_atom *av) {
  t_paramPath *x = (t_paramPath *)pd_new(paramPath_class);
  
  //x->root = tof_get_dollarzero(tof_get_root_canvas(tof_get_canvas()));
   t_canvas* canvas = tof_get_canvas();
	x->path = param_get_path(canvas,NULL);
	
    x->outlet = outlet_new(&x->x_obj, &s_list);
    
  return (x);
}

void paramPath_setup(void) {
  paramPath_class = class_new(gensym("param path"),
    (t_newmethod)paramPath_new, (t_method)paramPath_free,
    sizeof(t_paramPath), 0, A_GIMME, 0);

 class_addbang(paramPath_class, paramPath_bang);
 
 class_sethelpsymbol(paramPath_class, gensym("param"));
 
}
