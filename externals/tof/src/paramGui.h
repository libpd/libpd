#include "objectlist.h"




#define GUI_X_STEP 200
#define GUI_Y_STEP 18

/*
 Version 0.4
 */



/*
 CHANGES
 
 0.4 Added property bang code from iemguts (and objectlist.h)
 0.3 Removed absolute paths. The path must start with a 'p'
 0.2 Added a symbol message handler and a subpath variable
 0.2 Banging inlet 2 resets the gui but does not recreate it (as before)
 
 
 */


static t_class *paramGui_class;
static t_propertiesfn originalproperties=NULL;

typedef struct _paramGui
{
	t_object		x_obj;
	t_canvas*		childcanvas;
	t_symbol*		path;
	int				path_l;
	t_symbol*		subpath;
	int				subpath_l;
	int				build;
	t_symbol*		root;
	t_symbol*		receive;
	int				waiting;
	
	t_symbol*  current;
	
} t_paramGui;


static void paramGui_buildCanvas(t_paramGui* x,int x_position,int y_position) {
	
	// Clear the canvas
	pd_typedmess((t_pd*)x->childcanvas,s_clear,0,NULL);
	
	
	
	int pos_x = 0;
	int pos_y = 0;
	
	t_atom atoms[22]; // This should be the maximum number of atoms
	
	// PINK HEADER
	SETSYMBOL(&atoms[0],s_obj);
	SETFLOAT(&atoms[1],pos_x);
	SETFLOAT(&atoms[2],pos_y);
	SETSYMBOL(&atoms[3],s_cnv);
	SETFLOAT(&atoms[4],15);
	SETFLOAT(&atoms[5],200);
	SETFLOAT(&atoms[6],20);
	SETSYMBOL(&atoms[7],s_empty);
	SETSYMBOL(&atoms[8],s_empty);
	if ( x->subpath != NULL) {
		SETSYMBOL(&atoms[9],x->subpath);	
	} else {
		SETSYMBOL(&atoms[9],x->path);	
	}
	SETFLOAT(&atoms[10],2);
	SETFLOAT(&atoms[11],12);
	SETFLOAT(&atoms[12],0);
	SETFLOAT(&atoms[13],8);
	SETFLOAT(&atoms[14],-258401);
	SETFLOAT(&atoms[15],-262144);
	SETFLOAT(&atoms[16],0);
	pd_forwardmess((t_pd*)x->childcanvas, 17, atoms);
	pos_y = pos_y + 23;
	
	
	int ac;
	t_atom* av;
	t_symbol* type;
	t_symbol* send;
	t_symbol* receive;
	
	int i;
	t_symbol* shortpath;
	
	int gui_update = 0;
	
	// ac & av for updating the values of the gui (p->get())
	// after it is created
	int ac_got = 0;
	t_atom* av_got;
	t_symbol* s_got;
	
	
	t_param* p = get_param_list(x->root);
	
	int match;
	
	
	while (p) {
		
		gui_update = 0;
		if (p->GUI ) {
			
			match = 0;
			match = (strncmp(p->path->s_name,x->path->s_name,x->path_l)==0);
			if ( x->subpath != NULL ) match = match && x->path_l > 0 && ( strncmp((p->path->s_name)+(x->path_l-1),x->subpath->s_name,x->subpath_l)==0);
			
			
			
			if ( match ) {
				
				
				p->GUI(p->x,&ac,&av,&send,&receive);
				if ( send == NULL ) send = s_empty;
				if ( receive == NULL ) receive = s_empty;
				
				/* 
				 // This code alows the positioning of the guis, but creates
				 // too many problems
				 if ( IS_A_FLOAT(av,0) ) {
				 pos_x = GUI_X_STEP * atom_getfloat(av);
				 av++;
				 ac--;
				 }
				 
				 if ( IS_A_FLOAT(av,0) ) {
				 pos_y = GUI_Y_STEP * atom_getfloat(av);
				 av++;
				 ac--;
				 }
				 */
				if ( IS_A_SYMBOL(av,0)) {
					
					// Make shortpath (removes what is common between paths)
					// Do not make shortpath if we are at the root (x->path_l==1)
					if ( x->path_l < 2) {
						shortpath = p->path;
					} else {
						shortpath = gensym(p->path->s_name + x->path_l);
					}
					
					
					
					
					type = atom_getsymbol(av);
					if ( type == s_nbx ) {
						SETSYMBOL(&atoms[0],s_obj);
						SETFLOAT(&atoms[1],pos_x);
						SETFLOAT(&atoms[2],pos_y);
						SETSYMBOL(&atoms[3],s_nbx);
						SETFLOAT(&atoms[4],5);
						SETFLOAT(&atoms[5],14);
						SETFLOAT(&atoms[6],-1.0e+37);
						SETFLOAT(&atoms[7],1.0e+37);
						SETFLOAT(&atoms[8],0);
						SETFLOAT(&atoms[9],0);
						SETSYMBOL(&atoms[10],send);
						SETSYMBOL(&atoms[11],receive);
						SETSYMBOL(&atoms[12],shortpath);
						SETFLOAT(&atoms[13],50);
						SETFLOAT(&atoms[14],8);
						SETFLOAT(&atoms[15],0);
						SETFLOAT(&atoms[16],8);
						SETFLOAT(&atoms[17],-262144);
						SETFLOAT(&atoms[18],-1);
						SETFLOAT(&atoms[19],-1);
						SETFLOAT(&atoms[20],0);
						SETFLOAT(&atoms[21],256);
						pd_forwardmess((t_pd*)x->childcanvas, 22, atoms);
						pos_y = pos_y + GUI_Y_STEP;
						gui_update = 1;
						
					} else if (type == s_bng) {
						
						SETSYMBOL(&atoms[0],s_obj);
						SETFLOAT(&atoms[1],pos_x);
						SETFLOAT(&atoms[2],pos_y);
						SETSYMBOL(&atoms[3],s_bng);
						SETFLOAT(&atoms[4],15);
						SETFLOAT(&atoms[5],250);
						SETFLOAT(&atoms[6],50);
						SETFLOAT(&atoms[7],0);
						SETSYMBOL(&atoms[8],send);
						SETSYMBOL(&atoms[9],receive);
						SETSYMBOL(&atoms[10],shortpath);
						SETFLOAT(&atoms[11],17);
						SETFLOAT(&atoms[12],7);
						SETFLOAT(&atoms[13],0);
						SETFLOAT(&atoms[14],8);
						SETFLOAT(&atoms[15],-262144);
						SETFLOAT(&atoms[16],-1);
						SETFLOAT(&atoms[17],-1);
						pd_forwardmess((t_pd*)x->childcanvas, 18, atoms);
						pos_y = pos_y + GUI_Y_STEP;
						gui_update = 0;
					} else if ( (type == s_slider) || (type == s_knob) || (type == s_hsl) ) {
						SETSYMBOL(&atoms[0],s_obj);
						SETFLOAT(&atoms[1],pos_x);
						SETFLOAT(&atoms[2],pos_y);
						SETSYMBOL(&atoms[3],s_hsl);
						SETFLOAT(&atoms[4],100);
						SETFLOAT(&atoms[5],15);
						if (ac > 1 && IS_A_FLOAT(av,1) ) {
							SETFLOAT(&atoms[6],atom_getfloat(av+1));
						} else {
							SETFLOAT(&atoms[6],0);
						}
						if (ac > 2 && IS_A_FLOAT(av,2) ) {
							SETFLOAT(&atoms[7],atom_getfloat(av+2));
						} else {
							SETFLOAT(&atoms[7],1);
						}
						SETFLOAT(&atoms[8],0);
						SETFLOAT(&atoms[9],0);
						SETSYMBOL(&atoms[10],send);
						SETSYMBOL(&atoms[11],receive);
						SETSYMBOL(&atoms[12],shortpath);
						SETFLOAT(&atoms[13],105);
						SETFLOAT(&atoms[14],7);
						SETFLOAT(&atoms[15],0);
						SETFLOAT(&atoms[16],8);
						SETFLOAT(&atoms[17],-262144);
						SETFLOAT(&atoms[18],-1);
						SETFLOAT(&atoms[19],-1);
						SETFLOAT(&atoms[20],0);
						SETFLOAT(&atoms[21],1);
						pd_forwardmess((t_pd*)x->childcanvas, 22, atoms);
						pos_y = pos_y + GUI_Y_STEP;
						gui_update = 1;
					} else if (type == s_tgl) {
						SETSYMBOL(&atoms[0],s_obj);
						SETFLOAT(&atoms[1],pos_x);
						SETFLOAT(&atoms[2],pos_y);
						SETSYMBOL(&atoms[3],s_tgl);
						SETFLOAT(&atoms[4],15);
						SETFLOAT(&atoms[5],0);
						SETSYMBOL(&atoms[6],send);
						SETSYMBOL(&atoms[7],receive);
						SETSYMBOL(&atoms[8],shortpath);
						SETFLOAT(&atoms[9],17);
						SETFLOAT(&atoms[10],7);
						SETFLOAT(&atoms[11],0);
						SETFLOAT(&atoms[12],8);
						SETFLOAT(&atoms[13],-262144);
						SETFLOAT(&atoms[14],1);
						SETFLOAT(&atoms[15],-1);
						SETFLOAT(&atoms[16],0);
						SETFLOAT(&atoms[17],1);
						pd_forwardmess((t_pd*)x->childcanvas, 18, atoms);
						pos_y = pos_y + GUI_Y_STEP;
						gui_update = 1;
						
					} else if ( type == s_symbolatom || type == s_sym) {
						SETSYMBOL(&atoms[0],s_symbolatom);
						SETFLOAT(&atoms[1],pos_x);
						SETFLOAT(&atoms[2],pos_y);
						SETFLOAT(&atoms[3],17);
						SETFLOAT(&atoms[4],0);
						SETFLOAT(&atoms[5],0);
						SETFLOAT(&atoms[6],1);
						SETSYMBOL(&atoms[7],shortpath);
						SETSYMBOL(&atoms[8],receive);
						SETSYMBOL(&atoms[9],send);
						pd_forwardmess((t_pd*)x->childcanvas, 10,atoms);
						pos_y = pos_y + GUI_Y_STEP;
						gui_update = 1;
					} else {
						SETSYMBOL(&atoms[0],s_text);
						SETFLOAT(&atoms[1],pos_x);
						SETFLOAT(&atoms[2],pos_y);
						SETSYMBOL(&atoms[3],shortpath);
						pd_forwardmess((t_pd*)x->childcanvas, 4,atoms);
						pos_y = pos_y + GUI_Y_STEP;
						gui_update = 0;									
					}
					//p->get(p->x,&s_got,&ac_got,&av_got);
					//post("path: %s selector: %s ac: %i update: %i",p->path->s_name,s_got->s_name,ac_got,gui_update);
					if ((gui_update) && (receive != s_empty) && (p->get) ) {
						p->get(p->x,&s_got,&ac_got,&av_got);
						
						
						if ( s_got != &s_bang && ac_got) tof_send_anything_prepend(receive,s_got,ac_got,av_got,s_set);
					}
					
				}
			}
		}
		p = p->next;
		
	}
	
	// Try to resize the canvas
	x->childcanvas->gl_screenx1 = x_position;
	x->childcanvas->gl_screeny1 = y_position;
	x->childcanvas->gl_screenx2 = pos_x + 300 + x->childcanvas->gl_screenx1;
	x->childcanvas->gl_screeny2 = pos_y + 30 + x->childcanvas->gl_screeny1;
	
	
	
	// Change the build flag
	x->build = 0;
	if ( x->subpath != NULL) {
		x->current = x->subpath;
	} else {
		x->current = NULL;
		
	}
	x->subpath=NULL;
	
	
	
	
	// Show canvas
	t_atom a;
	SETFLOAT(&a,1);
	pd_typedmess((t_pd*)x->childcanvas,s_vis,1,&a);
	
}


static void paramGui_motion_callback(t_paramGui *x, t_float x_position, t_float y_position)
{
	if ( x->waiting ) {
		x->waiting = 0;
		paramGui_buildCanvas(x,(int) x_position,(int) y_position );
	}
}





static void paramGui_bang(t_paramGui *x) {
    
	
	
    if (x->childcanvas && !x->waiting) {
        
        if (x->build) {
            
            // query for the mouse pointers position 
            // one it is received, build the canvas 
            x->waiting = 1;
			sys_vgui("pdsend \"%s motion [winfo pointerxy .] \"\n",x->receive->s_name);
        } else {
			// Show canvas
			t_atom a;
			SETFLOAT(&a,1);
			pd_typedmess((t_pd*)x->childcanvas,s_vis,1,&a);
		}
        
    }  else {
        
		pd_error(x,"No canvas to write to or mouse position not received!");
		
	}
}

static void paramGui_symbol(t_paramGui *x, t_symbol* s) {
	if ( s->s_name[0] == '/' ) {
		if ( x->current != s || x->build==1) {
			x->build = 1;
			x->subpath = s;
			x->subpath_l = strlen(x->subpath->s_name);
			//if (x->subpath_l && x->subpath->s_name[0] == '/') x->absolute = 1;
		}
		paramGui_bang(x);
	} else {
		pd_error(x,"[param gui]: symbol must start with a /.");
	}
	
}

static void paramGui_reset(t_paramGui *x) {
    x->build = 1;
    //paramGui_bang(x);
    // Hide canvas
	t_atom a;
	SETFLOAT(&a,0);
	x->current = NULL;
	pd_typedmess((t_pd*)x->childcanvas,s_vis,1,&a);
	
}


static void paramGui_free(t_paramGui *x)
{
	if (x->childcanvas) {
		//post("Deleting it");
		pd_free((t_pd *)x->childcanvas);
	}
	x->childcanvas = NULL;
	
	if (x->receive) {
		pd_unbind(&x->x_obj.ob_pd,x->receive);
	}
	
	removeElementFromLists((t_pd*)x);
	
}

static void paramGui_properties(t_gobj*z, t_glist*owner) {
	t_objectlist_element* objs= getElements((t_pd*)z);
	if(NULL==objs) {
		originalproperties(z, owner);
	}
	while(objs) {
		t_paramGui*x = (t_paramGui*)objs->obj;
		paramGui_bang(x);
		objs=objs->next;
	} 
}

static void *paramGui_new(t_symbol *s, int ac, t_atom *av) {
	t_paramGui *x = (t_paramGui *)pd_new(paramGui_class);
	
	
	x->build = 1;
	x->current = NULL;
	
	char buf[MAXPDSTRING];
	sprintf(buf, "#%lx", (long)x);
	x->receive = gensym(buf);
	pd_bind(&x->x_obj.ob_pd, x->receive );
	
	x->waiting = 0;
	
	t_canvas* currentcanvas = tof_get_canvas();
	
	x->root = tof_get_dollarzero(tof_get_root_canvas(currentcanvas));
	
	
	x->path = param_get_path(currentcanvas, NULL);
	x->path_l = strlen(x->path->s_name);
	// Prepend $0 to path 
	t_symbol* dollarzero = tof_get_dollarzero(currentcanvas);
	int zeropath_len = strlen(dollarzero->s_name)+strlen(x->path->s_name)+1;
	char* zeropath = getbytes(zeropath_len * sizeof(* zeropath));
	strcpy(zeropath,dollarzero->s_name);
	strcat(zeropath,x->path->s_name);
	t_symbol* fullpath = gensym(zeropath);
	freebytes(zeropath,zeropath_len * sizeof(* zeropath));
	
	//post("path: %s",x->path->s_name);
	
	// create a new canvas
    
    
	t_atom a;
	SETSYMBOL(&a, fullpath);
	pd_typedmess(&pd_objectmaker,gensym("pd"),1,&a);
	
    // From this point on, we are hoping the "pd" object has been created
    x->childcanvas = (t_canvas*) pd_newest();
	
	
    // Hide the window (stupid way of doing this)
    if (x->childcanvas) {
		SETFLOAT(&a,0);
		pd_typedmess((t_pd*)x->childcanvas,gensym("vis"),1,&a);
    }
	
	
	inlet_new(&x->x_obj, &x->x_obj.ob_pd,&s_bang, gensym("reset"));
	
	
	
	// SET THE PROPERTIES FUNCTION
	
	t_class *class = ((t_gobj*)currentcanvas)->g_pd;
	t_propertiesfn properties_tmp=NULL;
	properties_tmp=class_getpropertiesfn(class);
	if(properties_tmp!=paramGui_properties)
		originalproperties=properties_tmp;
	
	class_setpropertiesfn(class, paramGui_properties);
	
	addElement((t_pd*)currentcanvas, (t_pd*)x);
    
	return (x);
}

/*
 static void create_motion_proc(void)
 {
 //sys_gui("if { ![::tof_param_class::proc_test motion]} {\n");
 sys_gui ("  proc ::tof_param_class::motion {x y} {\n");
 //sys_gui ("    if { $x != $::hcs_cursor_class::last_x \\\n");
 //sys_gui ("      || $y != $::hcs_cursor_class::last_y} {\n");
 sys_vgui("        pd [concat %s motion $x $y \\;]\n",
 cursor_receive_symbol->s_name);
 //sys_gui ("        set ::hcs_cursor_class::last_x $x\n");
 //sys_gui ("        set ::hcs_cursor_class::last_y $y\n");
 sys_gui ("    }\n");
 //sys_gui ("  }\n");
 //sys_gui ("}\n");
 }
 */

void paramGui_setup(void) {
	paramGui_class = class_new(gensym("param gui"),
							   (t_newmethod)paramGui_new, (t_method)paramGui_free,
							   sizeof(t_paramGui), 0, A_GIMME, 0);
	
	class_addbang(paramGui_class, paramGui_bang);
	class_addsymbol(paramGui_class, paramGui_symbol);
	
	// The mouse position callback
	class_addmethod(paramGui_class, (t_method)paramGui_motion_callback,\
					gensym("motion"), A_DEFFLOAT, A_DEFFLOAT, 0);
	
	class_addmethod(paramGui_class, (t_method) paramGui_reset, gensym("reset"), 0);
	
	
	class_sethelpsymbol(paramGui_class,gensym("param"));
	
	//create_motion_proc();
	
}
