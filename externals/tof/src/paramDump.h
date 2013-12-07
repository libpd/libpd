

static t_class *paramDump_class;
 

typedef struct _paramDump
{
  t_object                  x_obj;
  t_outlet*			        outlet;
  t_symbol*					empty_s;
  t_symbol*					root;

} t_paramDump;

/*
static void paramDump_updateguis(t_paramDump *x, t_symbol* s) {
	
	t_param* p = get_param_list(x->root);
	int ac;
	t_atom* av;
	
	if ( s == x->empty_s ) {
		while (p) {
			if (p->GUIUpdate) {
				p->GUIUpdate(p->x);
			}
		p = p->next;
		}
	} else {
		int length = strlen(s->s_name);
		while (p) {
			if (p->GUIUpdate && (strncmp(p->path->s_name,s->s_name,length)==0) ) {
				p->GUIUpdate(p->x);
				
			}
		p = p->next;
		}
	}
		
}
*/
static void paramDump_guis(t_paramDump *x, t_symbol* s) {
	
	t_param* p = get_param_list(x->root);
	int ac;
	t_atom* av;
    t_symbol* send;
     t_symbol* receive;
	
	if ( s == x->empty_s ) {
		while (p) {
			if (p->GUI ) {
				p->GUI(p->x,&ac,&av,&send,&receive);
				tof_outlet_list_prepend(x->outlet,&s_list,ac,av,p->path);
				//outlet_anything(x->outlet,p->path,ac,av);
				
			}
		p = p->next;
		}
	} else {
		int length = strlen(s->s_name);
		while (p) {
			if (p->GUI && (strncmp(p->path->s_name,s->s_name,length)==0) ) {
				p->GUI(p->x,&ac,&av,&send,&receive);
				tof_outlet_list_prepend(x->outlet,&s_list,ac,av,p->path);
				//outlet_anything(x->outlet,p->path,ac,av);
				
			}
		p = p->next;
		}
	}
		
}


static void paramDump_symbol(t_paramDump *x, t_symbol* s) {
	
	t_param* p = get_param_list(x->root);
	#ifdef PARAMDEBUG
	if (p == NULL) {
		post("No params found");
	} else {
		post("Found params");
	}
	#endif
	
	t_symbol* selector;
	int ac;
	t_atom* av;
	
	
	
	int length = strlen(s->s_name);
	
	while (p) {
		if ( p->get && (strncmp(p->path->s_name,s->s_name,length)==0) ) {
			p->get(p->x, &selector, &ac, &av);
			tof_outlet_list_prepend(x->outlet,selector,ac,av,p->path);
		}
		p = p->next;
	}
    
}

// Dump out
static void paramDump_bang(t_paramDump *x) {
	
	t_param* p = get_param_list(x->root);
	#ifdef PARAMDEBUG
	if (p == NULL) {
		post("No params found");
	} else {
		post("Found params");
	}
	#endif
	
	t_symbol* selector;
	int ac;
	t_atom* av;
	
	
	while (p) {
		if ( p->get ) {
			p->get(p->x, &selector, &ac, &av);
			tof_outlet_list_prepend(x->outlet,selector,ac,av,p->path);
		}
		p = p->next;
	}
	
    
}



static void paramDump_free(t_paramDump *x)
{
	
	
}


static void *paramDump_new(t_symbol *s, int ac, t_atom *av) {
  t_paramDump *x = (t_paramDump *)pd_new(paramDump_class);
  
  x->root = tof_get_dollarzero(tof_get_root_canvas(tof_get_canvas()));
  x->empty_s = gensym("");
  
    //x->s_set = gensym("set");
	
    x->outlet = outlet_new(&x->x_obj, &s_list);
    
  return (x);
}

void paramDump_setup(void) {
  paramDump_class = class_new(gensym("param dump"),
    (t_newmethod)paramDump_new, (t_method)paramDump_free,
    sizeof(t_paramDump), 0, A_GIMME, 0);

 class_addbang(paramDump_class, paramDump_bang);
 class_addsymbol(paramDump_class, paramDump_symbol);

 //class_addmethod(paramDump_class, (t_method) paramDump_values, gensym("values"), A_DEFSYMBOL,0);
 
 class_addmethod(paramDump_class, (t_method) paramDump_guis, gensym("guis"), A_DEFSYMBOL,0);
 //class_addmethod(paramDump_class, (t_method) paramDump_updateguis, gensym("updateguis"), A_DEFSYMBOL,0);

 class_sethelpsymbol(paramDump_class,gensym("param"));
 //class_addmethod(paramDump_class, (t_method) paramDump_update_guis, gensym("update"), A_DEFSYMBOL,0);

}
