#include "m_pd.h"

static t_class *iterate_class;

typedef struct _iterate {
  t_object  x_obj;

  t_float start;
  t_float step;
  t_int iterating;
  t_outlet* outlet1;
  //t_outlet* outlet2;
} t_iterate;


static void iterate_bang(t_iterate *x)
{
  //x->i_count = x->i_down;
  x->iterating = 0;
  
}


static void iterate_start(t_iterate *x, t_float f) {
	
	x->start = f;
}


static void iterate_step(t_iterate *x, t_float f) {
	
	x->step = f;
}

static void iterate_float(t_iterate *x, t_float f)
{
  
  if ( f < 0 ) f = 0;
  
  int i;
  t_float value = x->start;
  x->iterating = 1;
  
  for ( i = 0; i < f; i++)   {
	  if ( !(x->iterating) ) break;
	  
	  outlet_float(x->outlet1, value);
	  value = value + x->step;
  }
  
}


static void iterate_free(t_iterate *x)
{
		
    
}

static void *iterate_new(t_symbol *s, int argc, t_atom *argv)
{
  t_iterate *x = (t_iterate *)pd_new(iterate_class);
  
  x->iterating = 0;
  if ( argc >= 2) {
	x->start =  atom_getfloat(argv);
	x->step =  atom_getfloat(argv+1);
  } else if ( argc == 1) {
	x->start =  atom_getfloat(argv);
	x->step = 1;
  } else {
	x->start = 0;
	x->step = 1;
  }
  
  floatinlet_new(&x->x_obj, &x->start);
  floatinlet_new(&x->x_obj, &x->step);
  x->outlet1 = outlet_new(&x->x_obj, &s_float);
  //x->outlet2 = outlet_new(&x->x_obj, &s_float);


  return (void *)x;
}

void iterate_setup(void) {
  iterate_class = class_new(gensym("iterate"),
        (t_newmethod)iterate_new,
        (t_method)iterate_free, sizeof(t_iterate),
        CLASS_DEFAULT, 
        A_GIMME, 0);

  class_addbang  (iterate_class, iterate_bang);
  class_addfloat (iterate_class, iterate_float);
  
  class_addmethod(iterate_class, 
        (t_method)iterate_start, gensym("start"),
        A_DEFFLOAT, 0);
		
  class_addmethod(iterate_class, 
        (t_method)iterate_step, gensym("step"),
        A_DEFFLOAT, 0);
  
  //class_addlist (iterate_class, iterate_list);
  
}
