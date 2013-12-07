

#include "tof.h" 


EXTERN void open_via_helppath(const char *name, const char *dir);

static t_class *open_help_class;




typedef struct _open_help {
  t_object  x_obj;
  char       buffer[MAXPDSTRING];
  //t_outlet* outlet1;
  //t_canvas* canvas;
  t_symbol* dir;
	t_symbol* help;
  
} t_open_help;



/*
// BANG: output the current root path
static void open_help_bang(t_open_help *x) {
		
	outlet_symbol(x->outlet1, x->dir  );
	
}
*/

//int canvas_open(t_canvas *x, const char *name, const char *ext,
//    char *dirresult, char **nameresult, unsigned int size, int bin)


static void open_help_symbol(t_open_help *x, t_symbol *s) {
	
	x->help = s;
	
    //int length = tof_anything_to_string(s,ac,av,tof_buf_temp_a);
	//strcat(tof_buf_temp_a,".pd");
    int length = strlen(s->s_name) + 4;
	char* name = getbytes(sizeof(*name)*length);
	strcpy(name,s->s_name);
	strcat(name,".pd");
	open_via_helppath(name, x->dir->s_name);
	freebytes(name, length * sizeof *name );
	
	
	}




static void open_help_free(t_open_help *x)
{
		
  //binbuf_free(x->binbuf);
    
}


static void open_help_bang(t_open_help *x) {
	if ( x->help) open_help_symbol(x,x->help);
}


static void open_help_click(t_open_help *x, t_floatarg xpos, t_floatarg ypos,
						  t_floatarg shift, t_floatarg ctrl, t_floatarg alt)
{
    if ( x->help) open_help_symbol(x,x->help);
}


void *open_help_new(t_symbol *s, int argc, t_atom *argv)
{
  t_open_help *x = (t_open_help *)pd_new(open_help_class);
    
  x->dir = tof_get_dir(tof_get_canvas());
	
	
	
	x->help=NULL;
	if ( argc && IS_A_SYMBOL(argv,0)  ) {
		x->help = atom_getsymbol(argv);
		
	}
	
	
	
  //strcpy(tof_buf_temp_a,x->dir->s_name);
  //strcat(tof_buf_temp_a,"/");
  //x->dir = gensym(tof_buf_temp_a);
  
  //x->canvas = tof_get_canvas();
  
 
  

  return (void *)x;
}



void open_help_setup(void) {
  open_help_class = class_new(gensym("open_help"),
        (t_newmethod)open_help_new,
        (t_method)open_help_free, sizeof(t_open_help),
        0, 
        A_GIMME, 0);

 class_addbang(open_help_class, open_help_bang);
  //class_addanything(open_help_class,open_help_anything);
  class_addsymbol(open_help_class,open_help_symbol);
  //class_addmethod(open_help_class, 
   //     (t_method)open_help_append, gensym("append"),
    //    A_GIMME, 0);
  
  //class_addfloat (open_help_class, open_help_float);
  /*class_addmethod(open_help_class, 
        (t_method)open_help_set, gensym("set"),
        A_DEFFLOAT, 0);
*/
  //class_addlist (open_help_class, open_help_list);
  
	class_addmethod(open_help_class, (t_method)open_help_click,
					gensym("click"),
					A_FLOAT, A_FLOAT, A_FLOAT, A_FLOAT, A_FLOAT, 0);
	
}
