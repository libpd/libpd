#include "m_pd.h"
#include <string.h>
#define IS_A_SYMBOL(atom,index) ((atom+index)->a_type == A_SYMBOL)
#define IS_A_FLOAT(atom,index) ((atom+index)->a_type == A_FLOAT)

/* Mode 0: unfold all the list till its end or when banged.
 * Mode 1: unfold each element of the list at each bang.
 * Mode 0 is by default. Mode 1 is activated when listUnfold is
 * created with "wait" or "manually" as only argument.
 */

static t_class *listUnfold_class;

typedef struct _listUnfold {
  t_object  		x_obj;
  t_int 			iterating;
  t_outlet* 		outlet1;
  t_outlet* 		outlet2;
  t_int 			mode;
  int 				memSize;
  int 				ac;
  t_atom*			av;
} t_listUnfold;





void listUnfold_bang(t_listUnfold *x)
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


void listUnfold_anything(t_listUnfold *x, t_symbol* s, int ac, t_atom* av)
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


static void listUnfold_free(t_listUnfold *x)
{
    freebytes(x->av, x->memSize * sizeof(*(x->av)));
}

void *listUnfold_new(t_symbol *s, int argc, t_atom *argv)
{
  t_listUnfold *x = (t_listUnfold *)pd_new(listUnfold_class);
  
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

void listUnfold_setup(void) {
	
	post("WARNING: listUnfold is deprecated, please use list_unfold instead.");
	
  listUnfold_class = class_new(gensym("listUnfold"),
        (t_newmethod)listUnfold_new,
        (t_method)listUnfold_free, sizeof(t_listUnfold),
        CLASS_DEFAULT, 
        A_GIMME, 0);

  class_addbang  (listUnfold_class, listUnfold_bang);
  class_addanything (listUnfold_class, listUnfold_anything);
  //class_addlist (listUnfold_class, listUnfold_list);
  
}
