

#include "tof.h" 


EXTERN void open_via_helppath(const char *name, const char *dir);

static t_class *openHelp_class;




typedef struct _openHelp {
  t_object  x_obj;
  char       buffer[MAXPDSTRING];
  //t_outlet* outlet1;
  //t_canvas* canvas;
  t_symbol* dir;
  
} t_openHelp;



/*
// BANG: output the current root path
static void openHelp_bang(t_openHelp *x) {
		
	outlet_symbol(x->outlet1, x->dir  );
	
}
*/

//int canvas_open(t_canvas *x, const char *name, const char *ext,
//    char *dirresult, char **nameresult, unsigned int size, int bin)


static void openHelp_symbol(t_openHelp *x, t_symbol *s) {
	
	
    //int length = tof_anything_to_string(s,ac,av,tof_buf_temp_a);
	//strcat(tof_buf_temp_a,".pd");
    int length = strlen(s->s_name) + 4;
	char* name = getbytes(sizeof(*name)*length);
	strcpy(name,s->s_name);
	strcat(name,".pd");
	open_via_helppath(name, x->dir->s_name);
	freebytes(name, length * sizeof *name );
	
	
	}




static void openHelp_free(t_openHelp *x)
{
		
  //binbuf_free(x->binbuf);
    
}

void *openHelp_new(t_symbol *s, int argc, t_atom *argv)
{
  t_openHelp *x = (t_openHelp *)pd_new(openHelp_class);
    
  x->dir = tof_get_dir(tof_get_canvas());
  //strcpy(tof_buf_temp_a,x->dir->s_name);
  //strcat(tof_buf_temp_a,"/");
  //x->dir = gensym(tof_buf_temp_a);
  
  //x->canvas = tof_get_canvas();
  
 
  

  return (void *)x;
}

void openHelp_setup(void) {
	
	post("WARNING: openHelp is deprecated, use open_help instead");
	
  openHelp_class = class_new(gensym("openHelp"),
        (t_newmethod)openHelp_new,
        (t_method)openHelp_free, sizeof(t_openHelp),
        CLASS_DEFAULT, 
        A_GIMME, 0);

 // class_addbang(openHelp_class, openHelp_bang);
  //class_addanything(openHelp_class,openHelp_anything);
  class_addsymbol(openHelp_class,openHelp_symbol);
  //class_addmethod(openHelp_class, 
   //     (t_method)openHelp_append, gensym("append"),
    //    A_GIMME, 0);
  
  //class_addfloat (openHelp_class, openHelp_float);
  /*class_addmethod(openHelp_class, 
        (t_method)openHelp_set, gensym("set"),
        A_DEFFLOAT, 0);
*/
  //class_addlist (openHelp_class, openHelp_list);
  
}
