#include "m_pd.h"

static t_class *increment_class;
static t_class *increment_inlet2_class;
struct _increment_inlet2;

typedef struct _increment {
  t_object  x_obj;
  int reset;
  t_float value;
  t_float inc;
  t_outlet* outlet1;
  //t_float start;
  struct _increment_inlet2*	  inlet2;
  //t_outlet* outlet2;
} t_increment;

typedef struct _increment_inlet2 {
	t_object                    x_obj;
	t_increment					*x;
} t_increment_inlet2;



static void increment_bang(t_increment *x)
{
	
	if ( x->reset) {
		 x->reset = 0;
	} else {
		x->value = x->value + x->inc;
	}
	 outlet_float(x->outlet1,x->value);

}
/*
static void increment_float(t_increment *x, t_float f)
{
  x->inc = f;
  increment_bang(x);
}
*/

static void increment_float(t_increment *x, t_float f)
{
  x->inc = f;
  increment_bang(x);

}

static void increment_inlet2_bang(t_increment_inlet2 *x)
{
	

  
  x->x->value = 0;
  x->x->reset = 1;
  
 
}


static void increment_set(t_increment *x, t_float f)
{
  x->value = f;
  x->reset = 1;
  
}

static void increment_inlet2_float(t_increment_inlet2 *x, t_float f)
{
	
	//post("before");
 // post("start:%d",(int)x->start);
 // post("value:%d",(int)x->value);
  
  increment_set(x->x, f);
  
 // post("after");
  //post("start:%d",(int)x->start);
  //post("value:%d",(int)x->value);
}



/*
void increment_list(t_increment *x,t_symbol *s, int argc, t_atom *argv)
{
  
 
  if ( argc >= 2) {
	x->value =  atom_getfloat(argv);
	x->inc =  atom_getfloat(argv+1);
  } 
  
  
}
*/

static void increment_free(t_increment *x)
{
		
    
}

void *increment_new(t_symbol *s, int argc, t_atom *argv)
{
  t_increment *x = (t_increment *)pd_new(increment_class);
  
  x->reset = 1;
  x->value = 0;
  
  if ( argc >= 2) {
	x->value =  atom_getfloat(argv);
	x->inc =  atom_getfloat(argv+1);
  } else if ( argc == 1) {
	x->value =  atom_getfloat(argv);
	x->inc = 1;
  } else {
	x->value = 0;
	x->inc = 1;
  }
  
  //x->start = x->value;
  
  //post("start:%d",(int)x->start);
  //post("value:%d",(int)x->value);
  
   //inlet_new(&x->x_obj, &x->x_obj.ob_pd,
       // gensym("float"), gensym("set"));

  //floatinlet_new(&x->x_obj, &x->inc);
  
  //inlet_new(&x->x_obj, &x->x_obj.ob_pd,
   //     gensym("bang"), gensym("reset"));
   
  t_increment_inlet2 *inlet2 = (t_increment_inlet2 *)pd_new(increment_inlet2_class);
  
  inlet2->x = x;
  x->inlet2 = inlet2;
   
  inlet_new((t_object *)x, (t_pd *)inlet2, 0, 0);

  x->outlet1 = outlet_new(&x->x_obj, &s_float);
  //x->outlet2 = outlet_new(&x->x_obj, &s_float);
  

  return (void *)x;
}

void increment_setup(void) {
  increment_class = class_new(gensym("increment"),
        (t_newmethod)increment_new,
        (t_method)increment_free, sizeof(t_increment),
        CLASS_DEFAULT, 
        A_GIMME, 0);

  class_addbang  (increment_class, increment_bang);
  class_addfloat (increment_class, increment_float);
  class_addmethod(increment_class, 
        (t_method)increment_set, gensym("set"),
        A_DEFFLOAT, 0);
        
increment_inlet2_class = class_new(gensym("increment_inlet2"),
    0, 0, sizeof(t_increment_inlet2), CLASS_PD | CLASS_NOINLET, 0);
 
 class_addbang(increment_inlet2_class, increment_inlet2_bang);
 class_addfloat(increment_inlet2_class, increment_inlet2_float);
        
        
        /*
	class_addmethod(increment_class, 
        (t_method)increment_step, gensym("step"),
        A_DEFFLOAT, 0);
	class_addmethod(increment_class,
		(t_method)increment_reset, gensym("reset"),
		0);
   */
  //class_addlist (increment_class, increment_list);
  
}
