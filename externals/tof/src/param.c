/*
 *      param.c
 *      
 *      Copyright 2009 Thomas O Fredericks <tom@hp>
 *      
 *      This program is free software; you can redistribute it and/or modify
 *      it under the terms of the GNU General Public License as published by
 *      the Free Software Foundation; either version 2 of the License, or
 *      (at your option) any later version.
 *      
 *      This program is distributed in the hope that it will be useful,
 *      but WITHOUT ANY WARRANTY; without even the implied warranty of
 *      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *      GNU General Public License for more details.
 *      
 *      You should have received a copy of the GNU General Public License
 *      along with this program; if not, write to the Free Software
 *      Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 *      MA 02110-1301, USA.
 */

// Post a lot of debug messages.
//#define PARAMDEBUG

// Adds the root's $0 to the start of the paths.
//#define LOCAL

// Uses the symbol's s_thing to target the proper param
// Instead of searching through paramroots for a match.
// This is faster but can potentially target two params.
#define USEBINDINGS

#include "param.h"

// Global symbols
static t_symbol* s_vis;
static t_symbol* s_empty;
static t_symbol* s_clear;
static t_symbol* s_set;
static t_symbol* s_obj;
static t_symbol* s_nbx;
static t_symbol* s_bng;
static t_symbol* s_slider;
static t_symbol* s_hsl;
static t_symbol* s_knob;
static t_symbol* s_tgl;
static t_symbol* s_symbolatom;
static t_symbol* s_sym;
static t_symbol* s_text;
static t_symbol* s_cnv;


#include "paramCustom.h"
#include "paramDump.h"
#include "paramFile.h"
#include "paramPath.h"
#include "paramRoute.h"
#include "paramGui.h"

// For loadbang
extern int sys_noloadbang;

static t_class *param_class;
static t_class *param_inlet2_class;
struct _paramClass_inlet2;

typedef struct _paramClass {
  t_object                  	x_obj;
  struct _paramClass_inlet2     *inlet2;
  t_param*						param;
  int 							noloadbang;
  t_symbol*						selector;
  int							alloc;
  int 							ac;
  t_atom*						av;
  int							gac;  // gui options count
  t_atom*						gav;  // gui options
  t_outlet*						outlet;
  t_symbol*						receive;
  t_symbol*						send;
  t_symbol*						set_s;
  int							nowaitforbang;
  int							nopresets;
} t_paramClass;

typedef struct _paramClass_inlet2
{
  t_object       		 x_obj;
  t_paramClass  		*p_owner;
} t_paramClass_inlet2;



static void paramClass_bang(t_paramClass *x)
{
	outlet_anything(x->outlet, x->selector, x->ac, x->av);
	
	if(x->selector != &s_bang ) tof_send_anything_prepend(x->send,x->selector,x->ac,x->av,x->set_s );
   
}

static void paramClass_loadbang(t_paramClass *x)
{
    if (!sys_noloadbang && !x->noloadbang)
        paramClass_bang(x);
}

static void paramClass_anything(t_paramClass *x, t_symbol *s, int ac, t_atom *av)
{
 #ifdef PARAMDEBUG
  post("RECEIVING SOMETHING");
 #endif
	if ( s == &s_bang || ac == 0 ) {
		x->ac = 0;
		x->selector = &s_bang;
	} else {
		if(ac > x->alloc) {	
			x->av = resizebytes(x->av, x->alloc*sizeof(*(x->av)), 
				(10 + ac)*sizeof(*(x->av)));
			x->alloc = 10 + ac;
		}
		x->ac = ac;
		x->selector = s;
		tof_copy_atoms(av, x->av, ac);
   }
  
  if (x->nowaitforbang) paramClass_bang(x);
    
}




// SECOND INLET METHOD

static void paramClass_inlet2_anything(t_paramClass_inlet2 *p, t_symbol *s, int ac, t_atom *av)
{
  paramClass_anything(p->p_owner, s,ac,av);
}  

// DECONSTRUCTOR

static void paramClass_free(t_paramClass *x)
{
	
	if(x->inlet2) pd_free((t_pd *)x->inlet2);

    if (x->receive) pd_unbind(&x->x_obj.ob_pd, x->receive);

	if (x->param) param_unregister(x->param);
	
	freebytes(x->gav, x->gac * sizeof(*(x->gav)));
	
	freebytes(x->av, x->alloc * sizeof(*(x->av)));
    
}

// SPECIAL PARAM GET FUNCTION
static void paramClass_get(t_paramClass *x, t_symbol** s, int* ac, t_atom** av) {
	*s = x->selector;
	*ac = x->ac;
	*av = x->av;
}

// SPECIAL PARAM SAVE FUNCTION
static void paramClass_save(t_paramClass *x, t_binbuf* bb,int f) {
	
	//post("save:%i",f);
	
	
	// f = -1 for the main save file
	// f => 0 if it is a preset
	if ( f >= 0 && x->nopresets) return;
	
	//Put my data in binbuf
	if ((x->selector != &s_bang)) {
		int ac = x->ac + 2;
		t_atom *av = getbytes(ac*sizeof(*av));	
		tof_copy_atoms(x->av,av+2,x->ac);
		SETSYMBOL(av, x->param->path);
		SETSYMBOL(av+1, x->selector);
		binbuf_add(bb, ac, av);
		binbuf_addsemi(bb);
		freebytes(av, ac*sizeof(*av));
	}
}

// SPECIAL PARAM GUI FUNCTION
static void paramClass_GUI(t_paramClass *x, int* ac, t_atom** av, t_symbol** send,t_symbol** receive) {
	*ac = x->gac;
	*av = x->gav;
    *send = x->receive;
    *receive = x->send;
}

/*
// SPECIAL PARAM GUI FUNCTION
static void paramClass_GUIUpdate(t_paramClass *x) {
	if(x->selector != &s_bang ) tof_send_anything_prepend(x->send,x->selector,x->ac,x->av,x->set_s );
}
*/

// CONSTRUCTOR
static void* paramClass_new(t_symbol *s, int ac, t_atom *av)
{
	t_paramClass *x = (t_paramClass *)pd_new(param_class);
  
       
	// GET THE CURRENT CANVAS
	t_canvas* canvas=tof_get_canvas();
  
	// GET THE NAME
	t_symbol* name = param_get_name(ac,av);
  
	if (!name) return NULL;
	  
	t_symbol* path = param_get_path(canvas,name);
	t_symbol* root = tof_get_dollarzero(tof_get_root_canvas(canvas));
	  
	  
	 //FIND THE GUI OPTIONS: /g
	 int ac_temp = 0;
	 t_atom* av_temp = NULL;
	 
	 tof_find_tagged_argument('/',gensym("/g"), ac-1, av+1,&ac_temp,&av_temp);
	 x->gac = ac_temp;
	 x->gav = getbytes(x->gac * sizeof(*(x->gav)));
	 tof_copy_atoms(av_temp,x->gav,x->gac);	  
	  
	  // FIND THE NO LOADBANG TAG: /nlb
	x->noloadbang = tof_find_symbol(gensym("/nlb"), ac-1, av+1);
	  //post("nlb: %i",x->noloadbang);
	  
	  
	  
	  // FIND THE WAIT FOR BANG TAG: /wfb
	  x->nowaitforbang = !(tof_find_symbol(gensym("/wfb"), ac-1, av+1));
	    // FIND THE NO SAVE TAG: /ns
	int nosave = tof_find_symbol(gensym("/ns"), ac-1, av+1);
	  //post("ns: %i",nosave);
	  
	   // FIND THE NO PRESET TAG: /nps
	  x->nopresets = tof_find_symbol(gensym("/nps"), ac-1, av+1);
	  
	  
	  // REGISTER PARAM
	 t_paramSaveMethod paramSaveMethod = NULL;
	 t_paramGUIMethod paramGUIMethod = NULL;
	 
	 
	 //post("no save:%i",nosave);
	 
	if ( x->gac > 0 ) paramGUIMethod = (t_paramGUIMethod) paramClass_GUI;
	if ( nosave == 0 ) paramSaveMethod = (t_paramSaveMethod) paramClass_save;
		
	x->param = param_register(x,root,path, \
	(t_paramGetMethod) paramClass_get, \
	paramSaveMethod, \
	paramGUIMethod);
  
	if (!x->param) return NULL;
	 
	// FIND PARAM VALUE
	// A. In canvas' arguments
	// B. In object's arguments
	// C. Defaults to a bang

	int ac_p = 0;
	t_atom* av_p = NULL;

		  
	// A. In canvas' arguments
	int ac_c = 0;
	t_atom* av_c = NULL;

	t_canvas * before = tof_get_canvas_before_root(canvas);
	tof_get_canvas_arguments(before,&ac_c , &av_c);
	tof_find_tagged_argument('/',name, ac_c, av_c,&ac_p,&av_p);

	// B. I object's arguments
	if ( ac_p == 0  && ac > 1) {
	int ac_a = 0;
	t_atom* av_a = NULL;
	tof_find_tagged_argument('/',name, ac, av,&ac_p,&av_p);
	//tof_get_tagged_argument('/',ac,av,&start,&count);
	//if (count > 1) {
	//	ac_p = ac_a;
	//	av_p = av_a + 1;
	//}
	}
		  

	  
	  int l = strlen(path->s_name) + strlen(root->s_name) + 2;
	  char* receiver = getbytes( l * sizeof(*receiver));
	  receiver[0]='\0';
	  #ifdef LOCAL
	  strcat(receiver,root->s_name);
	  #endif
	  strcat(receiver,path->s_name);
	  x->receive = gensym(receiver);
	  strcat(receiver,"_");
	  x->send = gensym(receiver);
	  freebytes(receiver,  l * sizeof(*receiver));
	  #ifdef PARAMDEBUG
			post("receive:%s",x->receive->s_name);
			post("send:%s",x->send->s_name);
	  #endif
	  
	  
	  // BIND RECEIVER
  	  pd_bind(&x->x_obj.ob_pd, x->receive );
	  
	  
	  // Create memory space
	  t_symbol* selector;
	  param_set_selector(&selector,&ac_p,&av_p);
	  x->selector = selector;
	  x->alloc = ac_p + 10;
	  x->ac = ac_p;
	  x->av = getbytes(x->alloc * sizeof(*(x->av)));
	  tof_copy_atoms(av_p, x->av, x->ac);	  
	  
     
    
	x->set_s = gensym("set");
  
  // Set up second inlet proxy
  t_paramClass_inlet2 *p = (t_paramClass_inlet2 *)pd_new(param_inlet2_class);
  x->inlet2 = p;
  p->p_owner = x;
  
   // CREATE INLETS AND OUTLETS
      inlet_new((t_object *)x, (t_pd *)p, 0, 0);
      x->outlet = outlet_new(&x->x_obj, &s_list);
  
	//post("Selector: %s",x->selector->s_name);
  
  
  return (x);
}

static void* param_new(t_symbol *s, int argc, t_atom *argv) {
    
    //post("RUNNING COMMON NEW");
    
    t_pd* x = NULL;
    
    if ( !argc || argv[0].a_type == A_FLOAT ) {
        x = NULL;
    } else {
    
		t_symbol *s2 = argv[0].a_w.w_symbol;
    
		//post("Loading: %s",s2->s_name);
        if ( s2->s_name[0] == '/' )
			x = paramClass_new(s, argc, argv);
        else if (s2 == gensym("custom"))
            x = paramCustom_new(s, argc-1, argv+1);
        else if (s2 == gensym("dump"))
            x = paramDump_new(s, argc-1, argv+1);
         else if (s2 == gensym("file"))
            x = paramFile_new(s, argc-1, argv+1);
         else if (s2 == gensym("id") || s2 == gensym("path"))
            x = paramPath_new(s, argc-1, argv+1);
         else if (s2 == gensym("route"))
            x = paramRoute_new(s, argc-1, argv+1);
        else if (s2 == gensym("gui"))
            x = paramGui_new(s, argc-1, argv+1);
        else 
			post("Param is missing an argument. Possible values: custom, dump, file, path, route, gui or a /name.");
    }
    
    return (x);
    
}



void param_setup(void)
{
  
  // SETUP THE PARAM CLASS
  param_class = class_new(gensym("param /"),
    (t_newmethod)paramClass_new, (t_method)paramClass_free,
    sizeof(t_paramClass), 0, A_GIMME, 0);


  class_addanything(param_class, paramClass_anything);
  class_addbang(param_class, paramClass_bang);
  
  class_addmethod(param_class, (t_method)paramClass_loadbang, gensym("loadbang"), 0);
  
  param_inlet2_class = class_new(gensym("_param_inlet2"),
    0, 0, sizeof(t_paramClass_inlet2), CLASS_PD | CLASS_NOINLET, 0);
	
  class_addanything(param_inlet2_class, paramClass_inlet2_anything);
  
   class_sethelpsymbol(param_class,gensym("param"));
  
  // SETUP OTHER CLASSES
   paramCustom_setup();
   paramDump_setup();
   paramFile_setup();
   paramPath_setup();
   paramRoute_setup();
   paramGui_setup();
  
  
  // ADD THE COMMON CREATOR
  
   class_addcreator((t_newmethod)param_new, gensym("param"), A_GIMME, 0);
   class_addcreator((t_newmethod)param_new, gensym("tof/param"), A_GIMME, 0);
   
   // GENERATE THE SYMBOLS
   
   s_vis = gensym("vis");
  s_empty = gensym("empty");
  s_clear = gensym("clear");
  s_set = gensym("set");
  s_obj = gensym("obj");
  s_nbx = gensym("nbx");
  s_bng = gensym("bng");
  s_slider = gensym("slider");
  s_hsl=gensym("hsl");
  s_knob = gensym("knob");
  s_tgl = gensym("tgl");
  s_symbolatom = gensym("symbolatom");
  s_sym = gensym("sym");
  s_text = gensym("text");
  s_cnv = gensym("cnv");
   
   
  
}
