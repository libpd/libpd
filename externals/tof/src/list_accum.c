#include "m_pd.h"
//#include <string.h>
#define IS_A_SYMBOL(atom,index) ((atom+index)->a_type == A_SYMBOL)
#define IS_A_FLOAT(atom,index) ((atom+index)->a_type == A_FLOAT)

// The maxsize should match the maximum value of an int (2147483647)
#define MAXSIZE 2147483647
//#define MAXSIZE 20




static t_class *list_accum_class;

typedef struct _list_accum {
  t_object  		x_obj;
  int 			   got_data;
  t_outlet* 		outlet1;
  t_outlet* 		outlet2;
  int 				mem_size;
  int 				ac;
  t_atom*			av;
} t_list_accum;




// Bang: output accumulated list
static void list_accum_bang(t_list_accum *x)
{
	
	if ( x->got_data) {
		x->got_data = 0;
		outlet_list(x->outlet1,&s_list,x->ac,x->av);
	} else {
		outlet_bang(x->outlet2);
	}
	
	
}

void list_accum_clear(t_list_accum *x) {
	x->got_data = 0;
	
}


void list_accum_anything(t_list_accum *x, t_symbol* s, int ac, t_atom* av)
{
	// COPY 
	
	  if ( !x->got_data ) x->ac =0;
	  x->got_data = 1;
	  
	  
	  int do_selector = ( s != &s_list && s != &s_float && s != &s_symbol );
	  int ac_prev = x->ac;
	  unsigned int ac_new = x->ac + ac + do_selector; //One more for the selector
	  
	  
	  if ( ac_new < MAXSIZE ) {
	  
	     
		 x->ac = ac_new;
		 
		// Resize memory if required and add 10 atoms just in case
		if(x->ac > x->mem_size) {	
				x->av = resizebytes(x->av, x->mem_size * sizeof(*(x->av)), 
					(10 + x->ac) * sizeof(*(x->av)));
				x->mem_size = 10 + x->ac;
		}
		
		t_atom* dst = x->av + ac_prev; //Offset destination by current size
		
		// Copy selector
		if ( do_selector ) {
			SETSYMBOL(dst, s);
			dst++;
		}
		// Copy atoms
		while(ac--) *dst++ = *av++;
		
	} else {
		pd_error(x, "[list_accum]: Input was ignored because maximum size(%d) would be exceeded.", MAXSIZE);
	}
	
}


static void list_accum_free(t_list_accum *x)
{
    freebytes(x->av, x->mem_size * sizeof(*(x->av)));
}

void *list_accum_new(t_symbol *s, int argc, t_atom *argv)
{
  t_list_accum *x = (t_list_accum *)pd_new(list_accum_class);
  
  //x->iterating = 0;
 /*
  x->mode = 0;
 
 
  if (argc && IS_A_SYMBOL(argv,0) ) {
	  t_symbol* type = atom_getsymbol(argv);
	  if (type->s_name == gensym("prepend") ) {
		  x->mode = 1;
	}	
  }
 */

  // Initialize memory
  x->mem_size = 10;
  x->ac = 0;
  x->av = getbytes(x->mem_size * sizeof(*(x->av)));

  x->outlet1 = outlet_new(&x->x_obj, &s_list);
  x->outlet2 = outlet_new(&x->x_obj, &s_bang);


  inlet_new(&x->x_obj, &x->x_obj.ob_pd, gensym("bang"), gensym("clear"));

  return (void *)x;
}

void list_accum_setup(void) {
  list_accum_class = class_new(gensym("list_accum"),
        (t_newmethod)list_accum_new,
        (t_method)list_accum_free, sizeof(t_list_accum),
        CLASS_DEFAULT, 
        A_GIMME, 0);

  class_addbang  (list_accum_class, list_accum_bang);
  class_addmethod(list_accum_class, (t_method)list_accum_clear, gensym("clear"),0);
  class_addanything (list_accum_class, list_accum_anything);
  
  //class_addlist (list_accum_class, list_accum_list);
  
}
