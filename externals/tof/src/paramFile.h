

static t_class *paramFile_class;
static t_class *paramFile_inlet2_class;
struct _paramFile_inlet2;

typedef struct _paramFile
{
  t_object                    	x_obj;
  t_canvas			          	*canvas;
  t_symbol*						basename;
  t_symbol*						root;
  struct _paramFile_inlet2	   *inlet2;
  int 							working;
} t_paramFile;


typedef struct _paramFile_inlet2 {
	t_object                    x_obj;
	t_paramFile					*x;
} t_paramFile_inlet2;

static t_symbol* paramFile_makefilename(t_paramFile* x, t_float f) {
	t_symbol* filename;
	
	int i = (int) f;
	
	if (i<0) {
		int length = strlen(x->basename->s_name)+7;
		char* buf = getbytes( length * sizeof (*buf));	
		sprintf(buf,"%s.param",x->basename->s_name);
		filename = gensym(buf);
		freebytes(buf, length * sizeof (*buf));
	} else {
		int length = strlen(x->basename->s_name)+11;
		char* buf = getbytes( length * sizeof (*buf));	
		sprintf(buf,"%s-%03d.param",x->basename->s_name,i);
		filename = gensym(buf);
		freebytes(buf, length * sizeof (*buf));
	}
	return filename;
}

static void paramFile_do_save(t_paramFile* x, t_float f) {
	
	
	
	if ( x->working ) {
		pd_error(x,"paramFile can only save or load to one file at a time");
		return;
	}
	t_symbol* filename = paramFile_makefilename(x,f);
	
	x->working = 1;
	
	
	post("Writing: %s",filename->s_name);
	
	int w_error;
			
	t_binbuf *bbuf = binbuf_new();
	
	t_param *p = get_param_list(x->root);
	
	while(p) {
		
		if ( p->save ) p->save(p->x,bbuf,f);
		
		p = p->next;
	}
	
	
    char buf[MAXPDSTRING];
    canvas_makefilename(x->canvas, filename->s_name,buf, MAXPDSTRING);
		
	
    w_error = (binbuf_write(bbuf, buf, "", 0));
 
			
	binbuf_free(bbuf);
	
	if (w_error) pd_error(x,"%s: write failed", filename->s_name);
	
	x->working = 0;
	
}





static void paramFile_do_load(t_paramFile* x, t_float f) {
	
		if ( x->working ) {
		pd_error(x,"paramFile can only save or load to one file at a time");
		return;
	}
	x->working = 1;
	
	t_symbol* filename = paramFile_makefilename(x,f);
	post("Reading: %s",filename->s_name);
	
	
	
	t_symbol* root = x->root;
	#ifndef USEBINDINGS
	t_param* pp = get_param_list(root);
	t_param* p;
	
	if (pp) {
	#endif
	int r_error;
	t_binbuf *bbuf = binbuf_new();
	
    r_error= (binbuf_read_via_canvas(bbuf, filename->s_name, x->canvas, 0));
           
	int bb_ac = binbuf_getnatom(bbuf);
	int ac = 0;
    t_atom *bb_av = binbuf_getvec(bbuf);
    t_atom *av = bb_av;
	t_symbol* s;
	
	
	  while (bb_ac--) {
		if (bb_av->a_type == A_SEMI) {
			if ( IS_A_SYMBOL(av,0) && ac > 1) {
				#ifdef USEBINDINGS
					#ifdef LOCAL
					t_symbol* path = atom_getsymbol(av);
					strcpy(param_buf_temp_a,root->s_name);
					strcat(param_buf_temp_a,path->s_name);
					s = gensym(param_buf_temp_a);
					#else
					s = atom_getsymbol(av);
					#endif
				#else
					 s = atom_getsymbol(av);
					p = pp;
					while(p && p->path != s) p=p->next;
			    #endif
			  
			   
			  #ifdef USEBINDINGS
				if (s->s_thing) {
				   #else
				   if (p) {
				#endif
				   
				   if ( ac > 3 && IS_A_SYMBOL(av,1) &&  atom_getsymbol(av+1) == &s_symbol) {
					   // STUPID MANAGEMENT OF SYMBOLS WITH SPACES 
					   // This whole block is simply to convert symbols saved with spaces to complete symbols
					   
					   t_binbuf *bbuf_stupid = binbuf_new();
					   binbuf_add(bbuf_stupid, ac-2, av+2);
					   
					   char *char_buf;
					   int char_length;
					   binbuf_gettext(bbuf_stupid, &char_buf, &char_length);
					   char_buf = resizebytes(char_buf, char_length, char_length+1);
					   char_buf[char_length] = 0;
					   t_symbol* stupid_symbol = gensym(char_buf);
					   //post("STUPID: %s",stupid_symbol->s_name);
					   freebytes(char_buf, char_length+1);
					   binbuf_free(bbuf_stupid);
					   t_atom* stupid_atom = getbytes(sizeof(*stupid_atom));
					   SETSYMBOL(stupid_atom, stupid_symbol);
					   
					   #ifdef USEBINDINGS
							pd_typedmess(s->s_thing, &s_symbol, 1, stupid_atom);
						#else
							pd_typedmess(p->x, &s_symbol, 1, stupid_atom);
						#endif
					   freebytes(stupid_atom, sizeof(*stupid_atom));
					   
				   } else {
					   #ifdef USEBINDINGS
							pd_forwardmess(s->s_thing, ac-1, av+1);
						#else
							pd_forwardmess(p->x, ac-1, av+1);
						#endif
					
					}
				}
		    }
		  
		  ac = 0;
		  av = bb_av + 1;
		} else {
		  
		  ac = ac + 1;
		}
		bb_av++;
	  }
	
	binbuf_free(bbuf);
	
	if ( r_error) pd_error(x, "%s: read failed", filename->s_name);
	#ifndef USEBINDINGS
	}
	#endif
	
	
	x->working = 0;
	
}





static void paramFile_bang(t_paramFile *x) {
	
	paramFile_do_save(x,-1);	
}


static void paramFile_float(t_paramFile *x, t_float f) {
	
	if (f < 0 || f > 999 ) {
		pd_error(x,"paramFile preset number must be between 0 and 999");
		return;
	}
	
	paramFile_do_save(x,f);
		
}


static void paramFile_inlet2_bang(t_paramFile_inlet2 *inlet2) {
	
	paramFile_do_load(inlet2->x,-1);
	
}

static void paramFile_inlet2_float(t_paramFile_inlet2 *inlet2,t_float f) {
	
	if (f < 0 || f > 999 ) {
		pd_error(inlet2->x,"paramFile preset number must be between 0 and 999");
		return;
	}
	
	paramFile_do_load(inlet2->x,f);
	
}

static void paramFile_free(t_paramFile *x)
{
	
	if(x->inlet2) pd_free((t_pd *)x->inlet2);
	
 
}


static void* paramFile_new(t_symbol *s, int ac, t_atom *av) {
  t_paramFile *x = (t_paramFile *)pd_new(paramFile_class);
  t_paramFile_inlet2 *inlet2 = (t_paramFile_inlet2 *)pd_new(paramFile_inlet2_class);
  
  inlet2->x = x;
  x->inlet2 = inlet2;
  
  t_canvas* canvas = tof_get_canvas();
  x->canvas = tof_get_root_canvas(canvas);
  t_symbol* canvasname = tof_get_canvas_name(x->canvas);
  
  // remove the .pd (actually removes everything after the .)
  x->basename = tof_remove_extension(canvasname);
  
  x->working = 0;
  
  x->root = tof_get_dollarzero(x->canvas);
  
   //x->outlet = outlet_new(&x->x_obj, &s_list);
   
   inlet_new((t_object *)x, (t_pd *)inlet2, 0, 0);
   
  return (x);
  
}


void paramFile_setup(void) {
  paramFile_class = class_new(gensym("param file"),
    (t_newmethod)paramFile_new, (t_method)paramFile_free,
    sizeof(t_paramFile), 0, A_GIMME, 0);
  
 class_addbang(paramFile_class, paramFile_bang);
 class_addfloat(paramFile_class, paramFile_float);
 
 paramFile_inlet2_class = class_new(gensym("paramFile_inlet2"),
    0, 0, sizeof(t_paramFile_inlet2), CLASS_PD | CLASS_NOINLET, 0);
 
 class_addbang(paramFile_inlet2_class, paramFile_inlet2_bang);
 class_addfloat(paramFile_inlet2_class, paramFile_inlet2_float);
 
 class_sethelpsymbol(paramFile_class,gensym("param"));
 
 //class_addmethod(paramFile_class, (t_method) paramFile_load, gensym("load"), A_DEFFLOAT,0);
 //class_addmethod(paramFile_class, (t_method) paramFile_float, gensym("save"), A_DEFFLOAT,0);
 
}


