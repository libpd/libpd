#include "m_pd.h"
#include <math.h>

static t_class *wrapper_class;

typedef struct _wrapper {
  t_object  x_obj;

  t_float min;
  t_float max;
  //t_int iterating;
  t_outlet* outlet1;
  //t_outlet* outlet2;
} t_wrapper;

/*
void wrapper_bang(t_wrapper *x)
{
  //x->i_count = x->i_down;
  x->iterating = 0;
  
}
*/

void wrapper_float(t_wrapper *x, t_float f)
{
  
   t_float value = fmod(f - x->min ,x->max - x->min);
   if ( value < 0 ) value = value + x->max;
   else value = value + x->min; 
	 
   outlet_float(x->outlet1, value);
	  
  
}



static void wrapper_free(t_wrapper *x)
{
		
    
}

void *wrapper_new(t_symbol *s, int argc, t_atom *argv)
{
  t_wrapper *x = (t_wrapper *)pd_new(wrapper_class);
  
  
  if ( argc >= 2) {
	x->min =  atom_getfloat(argv);
	x->max =  atom_getfloat(argv+1);
  } else if ( argc == 1) {
	x->min =  0;
	x->max = atom_getfloat(argv);
  } else {
	x->min = 0;
	x->max = 1;
  }
  
  floatinlet_new(&x->x_obj, &x->min);
  floatinlet_new(&x->x_obj, &x->max);
  x->outlet1 = outlet_new(&x->x_obj, &s_float);
  


  return (void *)x;
}

void wrapper_setup(void) {
  wrapper_class = class_new(gensym("wrapper"),
        (t_newmethod)wrapper_new,
        (t_method)wrapper_free, sizeof(t_wrapper),
        CLASS_DEFAULT, 
        A_GIMME, 0);

  //class_addbang  (wrapper_class, wrapper_bang);
  class_addfloat (wrapper_class, wrapper_float);
  //class_addlist (wrapper_class, wrapper_list);
  
}
