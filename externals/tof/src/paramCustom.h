

static t_class *paramCustom_class;
static t_class *paramCustom_receive_class;
struct _paramCustom_receive;

typedef struct _paramCustom {
  t_object                  	x_obj;
  t_param*						param;
  t_outlet*						outlet;
  t_outlet*						outlet2;
  t_binbuf*						bb;
  #ifdef USEBINDINGS
  t_symbol*						receive;
  #endif
  struct _paramCustom_receive*  r;
  int 							nopresets;
} t_paramCustom;

typedef struct _paramCustom_receive
{
  t_object       		 x_obj;
  t_paramCustom 		*owner;
} t_paramCustom_receive;




static void paramCustom_anything(t_paramCustom *x, t_symbol *selector, int argc, t_atom *argv)
{
	if (x->bb) {
		if ((selector != &s_bang)) {
			int ac = argc + 2;
			t_atom *av = getbytes(ac*sizeof(*av));	
			tof_copy_atoms(argv,av+2,argc);
			SETSYMBOL(av, x->param->path);
			SETSYMBOL(av+1, selector);
			binbuf_add(x->bb, ac, av);
			binbuf_addsemi(x->bb);
			freebytes(av, ac*sizeof(*av));
		}
	} else {
		pd_error(x,"No save triggered");
	}
}




// DECONSTRUCTOR

static void paramCustom_free(t_paramCustom *x)
{
	#ifdef USEBINDINGS
	if (x->receive)  pd_unbind(&x->r->x_obj.ob_pd, x->receive);
	#endif
	
	if (x->param) param_unregister(x->param);
	
    
}

// SPECIAL PARAM GET FUNCTION
/*
static void paramClass_get(t_paramClass *x, t_symbol** s, int* ac, t_atom** av) {
	*s = x->selector;
	*ac = x->ac;
	*av = x->av;
}
*/

// SPECIAL PARAM SAVE FUNCTION
static void paramCustom_save(t_paramCustom_receive *r, t_binbuf* bb, int f) {
	
	t_paramCustom* x = r->owner;
	// f = -1 for the main save file
	// f => 0 if it is a preset
	if ( f >= 0 && x->nopresets) return;
	
	
	if ( !x->bb ) {
	
	x->bb = bb;
	// TRIGGER OUTPUT
	outlet_bang(x->outlet);
	x->bb = NULL;
	
	} else {
		pd_error(x,"paramCustom is already saving");
	}
	
}



static void paramCustom_receive_anything(t_paramCustom_receive *r, t_symbol *s, int ac, t_atom *av){
	
	outlet_anything(r->owner->outlet2,s,ac,av);
	
	
}



// CONSTRUCTOR
static void* paramCustom_new(t_symbol *s, int ac, t_atom *av)
{
	t_paramCustom *x = (t_paramCustom *)pd_new(paramCustom_class);
  
       
	// GET THE CURRENT CANVAS
	t_canvas* canvas=tof_get_canvas();
  
	// GET THE NAME
	t_symbol* name = param_get_name(ac,av);
  
	if (!name) return NULL;
	  
	t_symbol* path = param_get_path(canvas,name);
	t_symbol* root = tof_get_dollarzero(tof_get_root_canvas(canvas));
	  
	  // FIND THE NO PRESET TAG: /nps
	 x->nopresets = tof_find_symbol(gensym("/nps"), ac-1, av+1);
	  
	   // Set up param proxy
		t_paramCustom_receive *r = (t_paramCustom_receive *)pd_new(paramCustom_receive_class);
		x->r = r;
		r->owner = x;
	  

	x->param = param_register(r,root,path, NULL,\
	(t_paramSaveMethod) paramCustom_save,NULL);
	  
	if (!x->param) return NULL;
	 
	  
	  #ifdef USEBINDINGS
		
		#ifdef LOCAL
		   int l = strlen(path->s_name) + strlen(root->s_name) + 2;
		   char* receiver = getbytes( l * sizeof(*receiver));
		   receiver[0] = '\0';
		   strcat(receiver,root->s_name);
		   strcat(receiver,path->s_name);
		   x->receive = gensym(receiver);
		   freebytes(receiver,  l * sizeof(*receiver));
		#else
			x->receive = path;
		#endif
		
		pd_bind(&r->x_obj.ob_pd, x->receive );
      #endif
      
	 
	  
	  #ifdef PARAMDEBUG
			post("receive:%s",x->receive->s_name);
			//post("send:%s",x->send->s_name);
	  #endif
	  
	  
	  x->bb = NULL;
	  
	  
	  
	  
	  

  
	// CREATE INLETS AND OUTLETS
      //inlet_new((t_object *)x, (t_pd *)p, 0, 0);
      x->outlet = outlet_new(&x->x_obj, &s_list);
	  x->outlet2 = outlet_new(&x->x_obj, &s_list);
  
  return (x);
}

void paramCustom_setup(void)
{
  paramCustom_class = class_new(gensym("param custom"),
    (t_newmethod)paramCustom_new, (t_method)paramCustom_free,
    sizeof(t_paramCustom), 0, A_GIMME, 0);


  class_addanything(paramCustom_class, paramCustom_anything);
  //class_addbang(paramCustom_class, paramCustom_bang);
  
  //class_addmethod(param_class, (t_method)paramClass_loadbang, gensym("loadbang"), 0);
  
  paramCustom_receive_class = class_new(gensym("paramCustom_receive"),
    0, 0, sizeof(t_paramCustom_receive), CLASS_PD | CLASS_NOINLET, 0);
	
  class_addanything(paramCustom_receive_class, paramCustom_receive_anything);
  
  class_sethelpsymbol(paramCustom_class,gensym("param"));
  
}
