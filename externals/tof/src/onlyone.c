
#include "m_pd.h"

struct _onlyone;

typedef struct _oo {
    struct _onlyone*     x;
    //t_symbol*         name;
    struct _oo* 	    next; 
    struct _oo* 	    previous; 
} t_oo;

static t_oo* oos = NULL;

static t_class *onlyone_class;

typedef struct _onlyone {
  t_object  		x_obj;
  t_outlet*         outlet;
  t_outlet*         outlet2;
  t_symbol*         name;
  int               iamtheone;
  t_symbol*         s_empty;
  //t_sample 			f_common;
  //t_common*	pg; 
  //t_sample 			f;
} t_onlyone;


static void onlyone_bang( t_onlyone *x) {
    if (x->iamtheone) {
        outlet_bang(x->outlet);
    } else {
        outlet_bang(x->outlet2);
    }
}


static void onlyone_register(t_onlyone* x) {
  

  t_oo* oo = oos;
  
  int outputme = 1;
  
  
   t_oo* new_oo = getbytes(sizeof(*new_oo));
   new_oo->x = x;
    
    if ( oo != NULL) {
        //post("oo is not null");
        if (outputme && (oo->x->name == x->name)) outputme = 0;
        // Get the last oo
        while (oo->next) {
            oo = oo->next;
        }
    }
    
    // Append
   
    new_oo->previous = oo;
    new_oo->next = NULL;
    if (oo != NULL) {
        oo->next = new_oo;
    } else {
        //post("Adding the first");
        oos = new_oo;
    }
   
    x->iamtheone = outputme;
	
}


static void onlyone_unregister(t_onlyone* x) {
	
    t_oo* oo = oos;
    
    t_onlyone* theotherone = NULL;
    t_oo* deleteone = NULL;
    
    if ( oo != NULL) {
        // Get the mathching oo and the x to output to
        while ( oo ) {
            if ( oo->x == x ) {
                //post("Found oo to delete");
                deleteone = oo;
            } else if (x->iamtheone && theotherone==NULL && (oo->x->name == x->name) ) {
                theotherone = oo->x;
            }
            if ( deleteone && theotherone) break;
            oo = oo->next;
        }
        
       
        
        // Delete the oo matching the x that called this function
        if (deleteone) {
            
            if (deleteone->previous) {
				deleteone->previous->next = deleteone->next;
				if (deleteone->next) deleteone->next->previous = deleteone->previous;
				
			} else {
				oos = deleteone->next;
				if ( deleteone->next != NULL) deleteone->next->previous = NULL;
			}
            
            freebytes(deleteone,sizeof(*deleteone));
            x->iamtheone = 0;
            x->name = NULL;
        } else {
            post("Hum, did not find oo to delete!");
        }
        
        // Output the bang to the other one
         if (theotherone) {
             theotherone->iamtheone = 1;
             onlyone_bang(theotherone);
         }
        
    } else {
        post("Weird, the list is empty...");
    }
    
   // if (oos==NULL) post("this is the end");

}


static void onlyone_free( t_onlyone *x) {

	 if ( x->name) onlyone_unregister(x);
}

static void onlyone_set( t_onlyone *x, t_symbol *s) {
   if ( s == x->s_empty) {
       if ( x->name) {
           onlyone_unregister(x);
           //onlyone_bang(x);
       }
   } else  if ( x->name && s != x->name) {
	 onlyone_unregister(x);
     x->name = s;
     onlyone_register(x);
     //onlyone_bang(x);
    } else if ( x->name == NULL) {
     x->name = s;
     onlyone_register(x);
     //onlyone_bang(x);
    }
    onlyone_bang(x);
}




static void *onlyone_new(t_symbol* s)
{
  t_onlyone *x = (t_onlyone *)pd_new(onlyone_class);
  
  x->s_empty = gensym("");
   x->iamtheone = 0;
   x->name = NULL;
  
  if ( s != x->s_empty) x->name = s;
  
  x->outlet = outlet_new(&x->x_obj, &s_bang);
 
  if ( x->name) onlyone_register(x);
  
  
  x->outlet2 = outlet_new(&x->x_obj, &s_bang);
 
  return (void *)x;
}

void onlyone_setup(void) {
  onlyone_class = class_new(gensym("onlyone"),
        (t_newmethod)onlyone_new,
        (t_method)onlyone_free, sizeof(t_onlyone),
        0, A_DEFSYMBOL, 0);
  
  class_addbang(onlyone_class, onlyone_bang);
        
    class_addmethod(onlyone_class, (t_method)onlyone_set, gensym("set"),A_DEFSYMBOL,0);
    
}
