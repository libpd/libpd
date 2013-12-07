#include "m_pd.h"
#include <string.h>
#define IS_A_SYMBOL(atom,index) ((atom+index)->a_type == A_SYMBOL)
#define IS_A_FLOAT(atom,index) ((atom+index)->a_type == A_FLOAT)

/* Mode 0: unfold all the list till its end or when banged.
 * Mode 1: unfold each element of the list at each bang.
 * Mode 0 is by default. Mode 1 is activated when list_unfold is
 * created with "wait" or "manually" as only argument.
 */

static t_class *list_unfold_class;

typedef struct _list_unfold {
  t_object  		x_obj;
  t_int 			iterating;
  t_outlet* 		outlet1;
  t_outlet* 		outlet2;
  t_int 			mode;
  int 				memSize;
  int 				ac;
  t_atom*			av;
} t_list_unfold;





void list_unfold_bang(t_list_unfold *x)
{
	
	
  //x->i_count = x->i_down;
  if ( x->mode == 0) {
	x->iterating = 0;
  } else {
	  int i;
	  
	 if ( x->iterating < x->ac ) {
		 i = x->iterating;
		 x->iterating++;
		 outlet_float(x->outlet2,i);
		if ( IS_A_FLOAT(x->av,i) ) {
			outlet_float(x->outlet1,atom_getfloat(x->av + i));
		} else {
			outlet_symbol(x->outlet1,atom_getsymbol(x->av + i));
		}
		
	 }
  } 
  
}


void list_unfold_anything(t_list_unfold *x, t_symbol* s, int ac, t_atom* av)
{
	
	
	
	if ( x->mode == 0) {
		
		// Output all
		
		int i =0;
		int offset =0;
		x->iterating = 1;
		 
		if ( s != &s_list && s != &s_float && s != &s_symbol ) {
			outlet_float(x->outlet2,0);
			outlet_symbol(x->outlet1,s);
			offset=1;
		}
				
		for ( ; i < ac && x->iterating; i++ ) {
			outlet_float(x->outlet2,i+offset);
			if ( IS_A_FLOAT(av,0) ) {
				outlet_float(x->outlet1,atom_getfloat(av));
			} else {
				outlet_symbol(x->outlet1,atom_getsymbol(av));
			}
			av++;
		}
  } else {
	  
	  x->iterating = 0;
	  
	    // Copy and wait for bangs to output
	  
	  
	     int do_selector = ( s != &s_list && s != &s_float && s != &s_symbol );
		 x->ac = ac + do_selector; //One more for the selector
		 
		// Resize memory if required and add 3 atoms just in case
		if(x->ac > x->memSize) {	
				x->av = resizebytes(x->av, x->memSize * sizeof(*(x->av)), 
					(3 + x->ac) * sizeof(*(x->av)));
				x->memSize = 3 + x->ac;
		}
		t_atom* dst = x->av; 
		
		// Copy selector
		if ( do_selector ) {
			SETSYMBOL(dst, s);
			dst++;
		}
		// Copy atoms
		while(ac--) *dst++ = *av++;
		
	  
  }
	
}


static void list_unfold_free(t_list_unfold *x)
{
    freebytes(x->av, x->memSize * sizeof(*(x->av)));
}

void *list_unfold_new(t_symbol *s, int argc, t_atom *argv)
{
  t_list_unfold *x = (t_list_unfold *)pd_new(list_unfold_class);
  
  x->iterating = 0;
 
  x->mode = 0;
 
  if (argc && IS_A_SYMBOL(argv,0) ) {
	  t_symbol* type = atom_getsymbol(argv);
	  if (strcmp(type->s_name,"wait")==0 || strcmp(type->s_name,"manually")==0) {
		  x->mode = 1;
	}	
  }
 
  // Initialize memory
  x->memSize = 10;
  x->ac = 0;
  x->av = getbytes(x->memSize * sizeof(*(x->av)));

  x->outlet1 = outlet_new(&x->x_obj, &s_list);
  x->outlet2 = outlet_new(&x->x_obj, &s_float);


  return (void *)x;
}

void list_unfold_setup(void) {
  list_unfold_class = class_new(gensym("list_unfold"),
        (t_newmethod)list_unfold_new,
        (t_method)list_unfold_free, sizeof(t_list_unfold),
        CLASS_DEFAULT, 
        A_GIMME, 0);
   

  class_addbang  (list_unfold_class, list_unfold_bang);
  class_addanything (list_unfold_class, list_unfold_anything);
  //class_addlist (list_unfold_class, list_unfold_list);
  
}
