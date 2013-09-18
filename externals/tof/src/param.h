#include "tof.h"
#include <stdio.h>
#include <fcntl.h>



typedef void (*t_paramGetMethod)(void*,t_symbol**,int*,t_atom**);
typedef void (*t_paramSaveMethod)(void*,t_binbuf*,int);
typedef void (*t_paramGUIMethod)(void*,int*,t_atom**,t_symbol**,t_symbol**);
//typedef void (*t_paramGUIUpdateMethod)(void*);

static char param_buf_temp_a[MAXPDSTRING];
static  char param_buf_temp_b[MAXPDSTRING];
static  char* param_separator = "/";


//char PARAMECHO = 0;

//struct _paramroot;

typedef struct _param {
   t_symbol*			root;
   t_symbol* 			path; //Path(name) of the param
   void*				x;
   struct _param* 		next; //Next param
   struct _param* 		previous; //Previous param
   t_paramGetMethod		get; //Function to get current value
   t_paramSaveMethod	save; //Function to save
   t_paramGUIMethod		GUI;
  // t_paramGUIUpdateMethod GUIUpdate;
   //t_symbol*			send;
   //t_symbol*			receive;
} t_param;

typedef struct _paramroot {
	t_symbol*			root;
	t_param* 			params; //param list
	struct _paramroot* 	next; //Next paramroot
    struct _paramroot* 	previous; //Previous paramroot
} t_paramroot;

static t_paramroot* PARAMROOTS;


// set selector
// CHANGES
// USUALLY THE SELECTED SELECTOR WOULD ONLY BE BANG, ANYTHING OR LIST.
// I ADDED FLOAT
static void param_set_selector(t_symbol** selector_sym_p,int* ac_p, t_atom** av_p ) {
	if(!(*ac_p)) {
		*selector_sym_p = &s_bang;
	} else if( IS_A_SYMBOL(*av_p, 0 )) {
		
			*selector_sym_p = atom_getsymbol(*av_p);
			*ac_p = (*ac_p)-1;
			*av_p = (*av_p)+1;
		
	} else if ( IS_A_FLOAT(*av_p, 0 ) && *ac_p == 1 ) {
		*selector_sym_p = &s_float;
		*ac_p = 1;
		
	} else {
		*selector_sym_p = &s_list;
	}
}


static t_paramroot* param_get_root(t_symbol* root) {
	
	if (PARAMROOTS == NULL) {
		#ifdef PARAMDEBUG
		post("Could not get...not even one root created");
		#endif
		return NULL;
	 }
	
	
	// Pointer to the start of paramroots
	t_paramroot* branch = PARAMROOTS;
		
		while( branch ) {
			if ( branch->root == root) {
				#ifdef PARAMDEBUG
				  post("Found root:%s",root->s_name);
				#endif
				
				return branch;
			}
			branch = branch->next; 
		}
	#ifdef PARAMDEBUG
		post("Could not find root");
	#endif
	return branch;
	
}


static t_paramroot* param_root_attach(t_symbol* root){
		
		// Pointer to the start of paramroots
		t_paramroot* branch = PARAMROOTS;
		
		while( branch ) {
			if ( branch->root == root) {
				#ifdef PARAMDEBUG
				  post("Found root:%s",root->s_name);
				#endif
				
				return branch;
			}
			if ( branch->next == NULL ) break;
			branch = branch->next; 
		}
		
		// we did not find a paramroot linked to this root canvas
		// so we create it
		#ifdef PARAMDEBUG
			 post("Creating root:%s",root->s_name);
		#endif
		
		// Create and add paramroot to the end
		t_paramroot* newbranch = getbytes(sizeof(*newbranch));
		newbranch->root = root;
		newbranch->next = NULL;
		newbranch->params = NULL;
		
		if (branch) {
			#ifdef PARAMDEBUG
			  post("Appending it to previous roots");
			#endif
			newbranch->previous = branch;
			branch->next = newbranch;
		} else {
			#ifdef PARAMDEBUG
				post("Creating first root");
			#endif
			newbranch->previous = NULL;
			PARAMROOTS = newbranch;
		}
		
		
		return newbranch;
	
}



static t_param* get_param_list(t_symbol* root) {
	
	
	t_paramroot* branch = param_get_root(root);
	if (branch) {
		
	#ifdef PARAMDEBUG
		post("Getting params from %s",branch->root->s_name);
		if (!branch->params) post("Root contains no params");
	#endif
		return branch->params;
	} 
	
	return NULL;
    
}


static t_symbol* param_get_name ( int ac, t_atom* av  ) {
	
	if (ac  && IS_A_SYMBOL(av, 0)) {
		char* name =  atom_getsymbol(av)->s_name;
		if (*name == *param_separator ) {
			int length = strlen(name);
			if (name[length-1] != '_' || name[length-1] != '/') return atom_getsymbol(av);
		 }
	} 
	post("Param names must start with a \"/\" and can not end with either a \"_\" or a \"/\"!");
	return NULL;
}


static t_symbol* param_get_path( t_canvas* i_canvas,  t_symbol* name) {
	
	char* sbuf_name = param_buf_temp_a;
	char* sbuf_temp = param_buf_temp_b;
	sbuf_name[0] = '\0';
	sbuf_temp[0] = '\0';
	//char* separator = "/";
	
	
	t_symbol* id_s = gensym("/id"); // symbol that points to "/id" symbol
	   
    // arguments of the current canvas being analyzed
    int i_ac;
    t_atom * i_av;
	
	// temp pointer to the current id being added to the path
	t_symbol* id_temp;
	
	/* FIND ID AND BASEPATH  */
   while( i_canvas->gl_owner) {
	   // Ignore all supatches
	   if ( tof_canvas_is_not_subpatch(i_canvas) ) {
		tof_get_canvas_arguments(i_canvas,&i_ac, &i_av);
		id_temp=NULL;
		//id_temp= canvas_realizedollar(i_canvas, gensym("$0"));
		int ac_a = 0;
		t_atom* av_a = NULL;
		int iter = 0;
		//found_id_flag = 0;
		
		while( tof_next_tagged_argument(*param_separator,i_ac,i_av,&ac_a,&av_a,&iter) ) {
			
			if ( IS_A_SYMBOL(av_a,0)
			   && (id_s == av_a->a_w.w_symbol) 
			   && (ac_a > 1) ) {  
	           	id_temp = atom_getsymbol(av_a+1);
	           	//id_canvas = i_canvas;
				//found_id_flag = 1;
	           	break;
			}
		}
		
		if (id_temp == NULL) {
			
			id_temp = tof_remove_extension(tof_get_canvas_name(i_canvas));	    
		}    
        // if ever an /id is missing, this param is not saveable
        //if (found_id_flag == 0)  saveable = 0;
        
	   // Prepend newly found ID
		   strcpy(sbuf_temp,sbuf_name);
		   strcpy(sbuf_name, param_separator);
		   strcat(sbuf_name, id_temp->s_name);
		   strcat(sbuf_name,sbuf_temp);  
		}
        i_canvas = i_canvas->gl_owner;
    } 
  
  // If no name, the path will always end with a /
  
  if ( name != NULL) {
	  strcat(sbuf_name,name->s_name);
  } else {
      //if (strlen(sbuf_name)==0) 
      strcat(sbuf_name,param_separator); 
  }
  
  return gensym(sbuf_name);
	
}



// root, path, ac, av, ac_g, av_g
// From there, deduct id, path_, etc...

//static struct param* register_param( t_canvas* canvas, int o_ac, t_atom* o_av) {

static t_param* param_register(void* x,t_symbol* root, t_symbol* path,\
 t_paramGetMethod get, t_paramSaveMethod save, t_paramGUIMethod GUI) {
	
			
     //char *separator = "/";
		
		/* GET POINTER TO PARAMLIST FOR THAT ROOT  */
		t_paramroot* branch = param_root_attach(root);
        t_param* last = branch->params;
		
		// Search for param with same path
		while( last ) {
			if ( last->path == path) {
				
				  pd_error(x,"Found param with same name: %s", path->s_name);
				
				return NULL;
			}
			if ( last->next == NULL ) break;
			last = last->next; 
		}
		
		// Create and add param to the end
		
		t_param* p = getbytes(sizeof(*p));
		p->root = root;
		//p->alloc = 0;
		p->path = path;
		
		// Create receive and send symbols: $0/path
		//strcpy(param_buf_temp_a,p->root->s_name);  
		//strcat(param_buf_temp_a,separator);
		//strcat(param_buf_temp_a,p->path->s_name);
		//p->receive = gensym(param_buf_temp_a);
		//strcat(param_buf_temp_a,"_");  
		//p->send = gensym(param_buf_temp_a);
		
		p->next = NULL;
		p->x = x;
		p->get = get;
		p->save = save;
		p->GUI = GUI;
		//p->GUIUpdate = GUIUpdate;
		//p->id = id;
		//set_param( p, ac, av);
		//p->ac_g = ac_g;
		//p->av_g = getbytes(ac_g*sizeof(*(p->av_g)));
		//tof_copy_atoms(av_g,p->av_g,ac_g);
		if (last) {
			#ifdef PARAMDEBUG
			  post("Appending param");
			#endif
			p->previous = last;
			last->next = p;
		} else {
			#ifdef PARAMDEBUG
				post("Creating first param");
			#endif
			p->previous = NULL;
			branch->params = p;
		}
		
		
		return p;
   
}

static void param_unregister(t_param* p) {
	
	//post("unregistering %s", p->path->s_name);
	t_paramroot* branch = param_get_root(p->root);
	t_param* paramlist = branch->params;
	
	if ( paramlist) {

		
			if (p->previous) {
				p->previous->next = p->next;
				if (p->next) p->next->previous = p->previous;
				
			} else {
				paramlist = p->next;
				if ( p->next != NULL) p->next->previous = NULL;
			}
			
			freebytes(p, sizeof *p);
		
		// Update the params for that root
		if (paramlist == NULL) {
			if (branch->previous) {
				branch->previous->next = branch->next;
					if (branch->next) branch->next->previous = branch->previous;
			} else {
				PARAMROOTS = branch->next;
				if ( branch->next != NULL) branch->next->previous = NULL;
			}
			#ifdef PARAMDEBUG
			  post("Removing root:%s",branch->root->s_name);
			#endif
			freebytes(branch, sizeof *branch);
		} else {
			branch->params = paramlist;
		}
		
	} else {
		post("Euh... no params found!");
	}
	
	
}



