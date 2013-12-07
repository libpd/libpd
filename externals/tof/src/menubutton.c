/* menubutton widget for PD                                              *
 * Based on:
 * menubutton by Ben Bogart         
 * and button from GGEE by Guenter Geiger                                *

 * This program is distributed under the terms of the GNU General Public *
 * License                                                               *

 * menubutton is free software; you can redistribute it and/or modify         *
 * it under the terms of the GNU General Public License as published by  *
 * the Free Software Foundation; either version 2 of the License, or     *
 * (at your option) any later version.                                   *

 * menubutton is distributed in the hope that it will be useful,              *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 * GNU General Public License for more details.                          */




/* Append " x " to the following line to show debugging messages */
#define DEBUG(x)


#include <m_pd.h>
#include <g_canvas.h>
#include <stdio.h>
#include <string.h>


#define HANDLEWIDTH 10

#ifndef IOWIDTH 
#define IOWIDTH 4
#endif

static t_symbol* COMMA;


typedef struct _menubutton
{
     t_object x_obj;

     t_glist * x_glist;
     //t_outlet* out2;
     //int x_rect_width;
     //int x_rect_height;
     t_symbol*  x_sym;
	
     int x_height;
     int x_width;
	 
     int current_selection;
     int x_num_options;	 
     t_symbol* bg_color;
     t_symbol* fg_color;
     t_symbol* hi_color;
     t_symbol* co_color;
     int saveitems;
     int halign;
     
     t_symbol* send;
     t_symbol* receive;
     
     int send_set;
     int receive_set;
     
     t_symbol* s_empty;
     t_symbol* s_;
     //t_symbol* x_name;
	
     t_symbol** x_options;
     int        x_maxoptions;
     
     //t_symbol** current_options;

     int initialized; /* 1 when we are allowed to draw, 0 otherwise */
     int x_disabled; /* when disabled, graphical chosing is prohibited */
} t_menubutton;


#include "menubutton_w.h"



static void menubutton_output(t_menubutton* x)
{
  
  
  t_atom atoms[2];
  SETFLOAT(atoms,x->current_selection);
  SETSYMBOL(atoms+1,x->x_options[x->current_selection]);
  outlet_list(x->x_obj.ob_outlet, &s_list, 2, atoms);
  if ( x->send != x->s_empty && x->send->s_thing) pd_forwardmess(x->send->s_thing, 2,atoms);

}


static void menubutton_callback(t_menubutton* x, t_floatarg f)
{
  //DEBUG(post("output start");)
  x->current_selection= f;
 menubutton_output(x);
  
}
static void menubutton_save(t_gobj *z, t_binbuf *b)
{
        DEBUG(post("save start");)
    
	
    t_menubutton *x = (t_menubutton *)z;
    
    t_atom* creation = binbuf_getvec(x->x_obj.te_binbuf);
       
    
    t_symbol* send = x->s_empty;
    t_symbol* receive = x->s_empty;
    char buf[80];
    if ( x->send_set ) {
        atom_string(creation + 3, buf, 80);
        send = gensym(buf);
	}
	
    if ( x->receive_set ) {
		atom_string(creation + 4, buf, 80);
		receive = gensym(buf);
	}
    
	DEBUG(post("send: %s receive: %s",send->s_name,receive->s_name);)
	
    binbuf_addv(b, "ssiisiississssi", gensym("#X"), gensym("obj"),
                x->x_obj.te_xpix, x->x_obj.te_ypix ,  
                atom_getsymbol(creation),
                x->x_width, x->x_height, send,receive,x->saveitems,
                x->bg_color, x->fg_color,x->hi_color,x->co_color,x->halign);
                
	// Loop for menu items
	int i;
	if ( x->saveitems) {
		//binbuf_addv(b, "s", gensym(","));
		for(i=0 ; i<x->x_num_options ; i++)
		{
			binbuf_addv(b, "s", gensym(","));
			DEBUG(post("saving option: %s",x->x_options[i]->s_name);)
			binbuf_addv(b, "s", x->x_options[i]);
			
		}
	}	
    binbuf_addv(b, ";");
    
    DEBUG(post("save end");)
}


static void menubutton_clear(t_menubutton* x) {
	
	x->x_num_options = 0;
	x->current_selection = -1;
	if ( menubutton_w_is_visible(x) ) {
		menubutton_w_clear(x);
	}
}

static void menubutton_size(t_menubutton* x,t_symbol *s, int argc, t_atom *argv) {
	
	if (argc>2) argc =2;
	switch (argc) {
		case 2: if ( (argv+1)->a_type == A_FLOAT) x->x_height = atom_getfloat(argv+1);
		case 1: if ( argv->a_type == A_FLOAT) x->x_width = atom_getfloat(argv);
		break;
	}
	
	if ( x->x_width < 10) x->x_width = 10;
	if ( x->x_height < 10) x->x_height = 10;
	
	if ( menubutton_w_is_visible(x) ) {
		menubutton_w_resize(x);
	}
}



static void menubutton_add(t_menubutton* x, t_symbol *s, int argc, t_atom *argv) {
	
	int visible = menubutton_w_is_visible(x);
	
	// resize the options-array if it is too small
	if((argc + x->x_num_options) > x->x_maxoptions){
          
          x->x_options = resizebytes( x->x_options, x->x_maxoptions*sizeof(*(x->x_options)), 
				(argc + x->x_num_options+10)*sizeof(*(x->x_options)));
          x->x_maxoptions=argc + x->x_num_options+10;
    }
	
	int i;
	t_symbol* label;
	for  ( i=0;i<argc;i++) {
		if ((argv+i)->a_type==A_SYMBOL) {
			
		    label = atom_getsymbol(argv+i);
		    DEBUG(post("adding option: %s",label->s_name);)
			 x->x_options[x->x_num_options] = label;
			if ( visible ) {
				menubutton_w_additem( x,label,x->x_num_options);
			}
			x->x_num_options = x->x_num_options + 1;
		 }
		
	}
	
    
}



// function to change the colors

static void menubutton_colors(t_menubutton* x, t_symbol* s, int argc, t_atom* argv)
{
    
	DEBUG(post("bgcolour start");)
    
    if ( argc && argv->a_type==A_SYMBOL && atom_getsymbol(argv)==gensym("default")) {
		x->bg_color = gensym("grey90");
		x->fg_color = gensym("black");
		x->hi_color = gensym("grey95");
		x->co_color = gensym("black");
	} else {
    
		if (argc > 4) argc = 4;
		switch (argc) {
			case 4: if ((argv+3)->a_type==A_SYMBOL) x->co_color = atom_getsymbol(argv+3);
			case 3: if ((argv+2)->a_type==A_SYMBOL) x->hi_color = atom_getsymbol(argv+2);
			case 2: if ((argv+1)->a_type==A_SYMBOL) x->fg_color = atom_getsymbol(argv+1);
			case 1: if ((argv)->a_type==A_SYMBOL) x->bg_color = atom_getsymbol(argv);
			break;
		}
    }
    
	if(menubutton_w_is_visible(x)) {
		//sys_vgui(".x%lx.c.s%x configure -background \"%s\" -foreground \"%s\"\n", x->x_glist, x, x->bg_color->s_name,x->fg_color->s_name);
	    menubutton_w_apply_colors(x);
	}
}


static int menubutton_set_float(t_menubutton* x, t_floatarg item) {
	
	    
	int i=(int)item;
	if( (i < x->x_num_options) && (i >= 0)) {
		x->current_selection = i;
		if(menubutton_w_is_visible(x)) menubutton_w_text(x,x->x_options[x->current_selection]);
         return 1;
	} else {
         return 0;
   }
	
}

/* Function to select a menu option by inlet */
static void menubutton_float(t_menubutton* x, t_floatarg item)
{
	DEBUG(post("iselect start");)
        
	if ( menubutton_set_float(x,item) ) {
		menubutton_output(x);
	} else {
		pd_error(x,"menubutton: expecting value between 0 and %i",x->x_num_options);
	}

	DEBUG(post("iselect end");)
}

static int menubutton_set_symbol(t_menubutton* x, t_symbol *s) {
	

	int i;
        
	/* Compare inlet symbol to each option */
	for(i=0; i < x->x_num_options; i++) {
	  if(x->x_options[i]->s_name == s->s_name) {
		    x->current_selection = i;
			if(menubutton_w_is_visible(x)) menubutton_w_text(x,s);
	        return 1;
       }
	}
	return 0;
}


/* Function to choose value via symbol name */
static void menubutton_symbol(t_menubutton* x, t_symbol *s)
{
	if(menubutton_set_symbol(x,s)) {
	   menubutton_output(x);
  } else {
	   post("menubutton: '%s' is not an available option.", s->s_name);
  }
	
}

/* Function to choose value via name/index but without outputting it*/
static void menubutton_set(t_menubutton* x, t_symbol *S, int argc, t_atom*argv)
{
  if(!argc)return;

  if(argv->a_type==A_FLOAT) {
      menubutton_set_float(x,atom_getfloat(argv));
    } else if(argv->a_type==A_SYMBOL) {
		menubutton_set_symbol(x,atom_getsymbol(argv));
     } else {
      pd_error(x, "menubutton: can only 'set' symbols or floats");
    }
}






static t_class *menubutton_class;


static void menubutton_receive(t_menubutton* x, t_symbol* s) {
	if ( x->receive != x->s_empty ) pd_unbind(&x->x_obj.ob_pd, x->receive);
	if ( s == x->s_empty || s ==  x->s_) {
		x->receive = x->s_empty;
	} else {
		x->receive = s;
		pd_bind(&x->x_obj.ob_pd, x->receive);
	}
	
}

static void menubutton_send(t_menubutton* x, t_symbol* s) {

	if ( s == x->s_empty || s ==  x->s_ ) {
		x->send = x->s_empty;
	} else {
		x->send = s;
	}
	
	
	
}

static void menubutton_free(t_menubutton*x)
{
  if(x->x_options)freebytes(x->x_options, sizeof(t_symbol*)*x->x_maxoptions);
   pd_unbind(&x->x_obj.ob_pd, x->x_sym);
   
   if ( x->receive != x->s_empty ) pd_unbind(&x->x_obj.ob_pd, x->receive);
}

static void menubutton_saveitems( t_menubutton* x, t_float f) {
	x->saveitems = (f != 0);
}

static void menubutton_align( t_menubutton* x, t_float f) {
	if ( f > 0 ) {
		x->halign = 1;
	} else if ( f == 0) {
		x->halign = 0;
	} else {
		x->halign = -1;
	}
	if(menubutton_w_is_visible(x)) {
		//sys_vgui(".x%lx.c.s%x configure -background \"%s\" -foreground \"%s\"\n", x->x_glist, x, x->bg_color->s_name,x->fg_color->s_name);
	    menubutton_w_set_align(x);
	}
}

static void *menubutton_new(t_symbol *s, int argc, t_atom *argv)
{
    DEBUG(post("menubutton new start");)

    t_menubutton *x = (t_menubutton *)pd_new(menubutton_class);
    int i;
	char buf[256];

    x->x_glist = (t_glist*)NULL;

    x->x_height = 25;
    x->current_selection = -1;

    x->x_maxoptions=10;
    x->x_options=(t_symbol**)getbytes(sizeof(t_symbol*)*x->x_maxoptions);
    //x->current_options=(t_symbol**)getbytes(sizeof(t_symbol*)*x->x_maxoptions);
    x->x_num_options = 0 ;
     //x->x_options[0] = gensym("");
     
    x->x_width = 124;
    x->x_height = 25;
    
    // These should match the default colors in menubutton_colors
    x->bg_color = gensym("grey90");
    x->fg_color = gensym("black");
    x->hi_color = gensym("grey95");
    x->co_color = gensym("black");
    x->saveitems = 0;
    x->halign = -1;
    
    x->s_empty = gensym("empty");
    x->s_ = gensym("");
    
    x->send = x->s_empty;
    x->receive = x->s_empty;
    x->send_set =0;
    x->receive_set =0;
    
    x->initialized=0;

    x->x_disabled=0;
    
    
    int conf_argc;
    t_atom* item_argv;
    int item_argc;
    
    // loop through arguments and search for a comma
    for ( conf_argc = 0; conf_argc < argc; conf_argc++) {
		if ((argv+conf_argc)->a_type == A_SYMBOL && atom_getsymbol(argv+conf_argc) == COMMA) {
			break;
		}
	}
	
	
	
	DEBUG(post("conf_argc: %i", conf_argc);)
	item_argc = argc - (conf_argc + 1); // The +1 is to skip the comma
	if (item_argc < 0) item_argc = 0;
	item_argv = argv + conf_argc + 1; // Point to the start of the items
	DEBUG(post("item_argc: %i", item_argc);)
	
    
    // The maximum number of configuration arguments is 10
    if (conf_argc > 10) conf_argc = 10;
    
    switch(conf_argc){ 
	case 10: if ((argv+9)->a_type == A_FLOAT) x->halign = atom_getfloat(argv+9);	
    case 9: if ((argv+8)->a_type == A_SYMBOL) x->co_color = atom_getsymbol(argv+8);
    case 8: if ((argv+7)->a_type==A_SYMBOL) x->hi_color = atom_getsymbol(argv+7);
    case 7: if ((argv+6)->a_type==A_SYMBOL) x->fg_color = atom_getsymbol(argv+6);
    case 6: if ((argv+5)->a_type==A_SYMBOL) x->bg_color = atom_getsymbol(argv+5);
    case 5: if ((argv+4)->a_type==A_FLOAT) x->saveitems = atom_getfloat(argv+4);
    case 4: if ((argv+3)->a_type==A_SYMBOL) {
		x->receive = atom_getsymbol(argv+3);
		x->receive_set =1;
		}
	case 3: if ((argv+2)->a_type==A_SYMBOL) {
		x->send = atom_getsymbol(argv+2);
		x->send_set =1;
		}
	case 2: if ((argv+1)->a_type==A_FLOAT) x->x_height = atom_getfloat(argv+1);  
	case 1: if ((argv)->a_type==A_FLOAT) x->x_width = atom_getfloat(argv);  
      break;
    }

   DEBUG(post("send: %s receive: %s",x->send->s_name,x->receive->s_name);)
   
   
   if ( x->x_width < 10) x->x_width = 10;
   if ( x->x_height < 10) x->x_height = 10;
   
   // Bind receiver
   if ( x->receive != x->s_empty ) pd_bind(&x->x_obj.ob_pd, x->receive);


      /* Bind the recieve "menubutton%p" to the widget outlet*/
      sprintf(buf,"menubutton%p",x);
      x->x_sym = gensym(buf);
      pd_bind(&x->x_obj.ob_pd, x->x_sym);

      /* define proc in tcl/tk where "menubutton%p" is the receive, "callback" is the method, and "$index" is an argument. */
    sys_vgui("proc select%x {index} {\n pdsend \"menubutton%p callback $index \"\n }\n",x,x); 


    outlet_new(&x->x_obj, &s_symbol);



    // PARSE ITEMS ALONG COMMAS
    
   unsigned int buffer_size = 0;
   char buffer[MAXPDSTRING];
   buffer[0] = '\0';
   char* bp = buffer;
   t_atom tempatom; 
   int word_length;
   	  
	i = 0;
	while (item_argc && i <= item_argc) {
	   if (((item_argv+i)->a_type == A_SYMBOL && atom_getsymbol(item_argv+i) == COMMA)|| i == item_argc ) {
			
			if ( buffer_size ) {
				SETSYMBOL(&tempatom,gensym(buffer));
				menubutton_add(x,&s_list,1,&tempatom);
				DEBUG(post("buffer: %s",buffer);)
			}
			buffer_size = 0;
			bp = buffer; 
			buffer[0] = '\0';
			
		} else {
			if ( buffer_size > 0) {
			  // Add a space separator
			  *bp = ' ';
			  bp++;
			  *bp = '\0';
			  buffer_size++;
			}
			atom_string(item_argv+i, bp , MAXPDSTRING-buffer_size);
			word_length = strlen(bp);
			DEBUG(post("word_length: %i",word_length);)
			DEBUG(post("bp: %s",bp);)
			bp = bp + word_length;
			buffer_size = buffer_size + word_length;
			}
	   i++;
	}
		
	//menubutton_add(x,&s_list,item_argc,item_argv);
	


DEBUG(post("menubutton new end");)

    return (x);
}

void menubutton_setup(void) {

    DEBUG(post("setup start");)

    COMMA = gensym(",");


      menubutton_class = class_new(gensym("menubutton"), (t_newmethod)menubutton_new, (t_method)menubutton_free,
				sizeof(t_menubutton),0,A_GIMME,0);
				
	class_addmethod(menubutton_class, (t_method)menubutton_callback,
								  gensym("callback"),A_DEFFLOAT,0);
								  
	class_addmethod(menubutton_class, (t_method)menubutton_send,
								  gensym("send"),A_DEFSYMBOL,0);
								  
	class_addmethod(menubutton_class, (t_method)menubutton_receive,
								  gensym("receive"),A_DEFSYMBOL,0);
								  
	class_addmethod(menubutton_class, (t_method)menubutton_saveitems,
								  gensym("saveitems"),A_FLOAT,0);
				  
	class_addmethod(menubutton_class, (t_method)menubutton_add,
								  gensym("add"), A_GIMME,0);
	
	class_addmethod(menubutton_class, (t_method)menubutton_clear,
								  gensym("clear"), 0);
								  
	class_addmethod(menubutton_class, (t_method)menubutton_colors,
								  gensym("colors"),A_GIMME,0);
								 
    class_addmethod(menubutton_class, (t_method)menubutton_size,
								  gensym("size"),A_GIMME,0);

        class_addmethod(menubutton_class, (t_method)menubutton_set,
                                    gensym("set"),A_GIMME,0);
                                    
class_addmethod(menubutton_class, (t_method)menubutton_align,
                                    gensym("align"),A_FLOAT,0);


	class_addsymbol(menubutton_class, (t_method)menubutton_symbol);

	class_addfloat(menubutton_class, (t_method)menubutton_float);



    class_setwidget(menubutton_class,&menubutton_widgetbehavior);

    class_setsavefn(menubutton_class,&menubutton_save);


	post("menubutton v0.12 tof, based on popup by Ben Bogart and button by ggee");
}


