#include "m_pd.h"

static t_class *streamMinMax_class;

typedef struct _streamMinMax {
  t_object  x_obj;

  t_float min;
  t_float max;
  t_int reset;
  //t_outlet *f_out, *b_out;
  t_outlet* outlet1;
  t_outlet* outlet2;
} t_streamMinMax;

void streamMinMax_float(t_streamMinMax *x, t_float f)
{
  
  if ( x->reset) {
	  x->min = f;
	  x->max = f;
	  x->reset = 0;
  } else {
	  if (f < x->min) x->min = f;
	  if (f > x->max) x->max = f;
  }
  
  outlet_float(x->outlet1, x->min);
  outlet_float(x->outlet2, x->max);
}

void streamMinMax_bang(t_streamMinMax *x)
{
  //x->i_count = x->i_down;
  x->reset = 1;
  
}

void streamMinMax_list(t_streamMinMax *x,t_symbol *s, int argc, t_atom *argv)
{
  
  if (argc >= 2) {
	  float a = atom_getfloat(argv+1);
	  float b = atom_getfloat(argv);
	  if (a > b ) {
		x->max=a;
		x->min=b;
	  } else {
		x->max=b;
		x->min=a;
	  }
	 x->reset = 0;
  } 
  
}

static void streamMinMax_free(t_streamMinMax *x)
{
		
    
}

void *streamMinMax_new(t_symbol *s, int argc, t_atom *argv)
{
  t_streamMinMax *x = (t_streamMinMax *)pd_new(streamMinMax_class);
  
  x->reset = 1;
  
  streamMinMax_list(x,&s_list,argc,argv);
  


  //inlet_new(&x->x_obj, &x->x_obj.ob_pd,
  //      gensym("list"), gensym("bound"));
  
  //floatinlet_new(&x->x_obj, &x->step);

  x->outlet1 = outlet_new(&x->x_obj, &s_float);
  x->outlet2 = outlet_new(&x->x_obj, &s_float);
  //x->b_out = outlet_new(&x->x_obj, &s_bang);

  return (void *)x;
}

void streamMinMax_setup(void) {
  streamMinMax_class = class_new(gensym("streamMinMax"),
        (t_newmethod)streamMinMax_new,
        (t_method)streamMinMax_free, sizeof(t_streamMinMax),
        CLASS_DEFAULT, 
        A_GIMME, 0);

  class_addbang  (streamMinMax_class, streamMinMax_bang);
  class_addfloat (streamMinMax_class, streamMinMax_float);
  class_addlist (streamMinMax_class, streamMinMax_list);
  /*
  class_addmethod(streamMinMax_class,
        (t_method)streamMinMax_reset, gensym("reset"), 0);
  class_addmethod(streamMinMax_class, 
        (t_method)streamMinMax_set, gensym("set"),
        A_DEFFLOAT, 0);
  class_addmethod(streamMinMax_class,
        (t_method)streamMinMax_bound, gensym("bound"),
        A_DEFFLOAT, A_DEFFLOAT, 0);

  class_sethelpsymbol(streamMinMax_class, gensym("help-streamMinMax"));
  */
}
