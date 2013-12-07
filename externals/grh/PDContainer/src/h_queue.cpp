// *********************(c)*2004*********************>
// -holzilib--holzilib--holzilib--holzilib--holzilib->
// ++++PD-External++by+Georg+Holzmann++grh@gmx.at++++>
//
// PDContainer: 
// this is a port of the containers from the C++ STL
// (Standard Template Library)
// for usage see the documentation and PD help files
// for license see readme.txt
//
// h_queue.cpp


#include "include/HQueue.h"


static t_class *h_queue_class;

typedef struct _h_queue 
{
  t_object  x_obj;
  t_canvas *x_canvas;
  t_outlet *out0, *out1, *out2;
  HQueue *hqueue;
} t_h_queue;

static void h_queue_push(t_h_queue *x, t_symbol *s, int argc, t_atom *argv)
{
  if(argc)
    {
      Element key(argc,argv);
      x->hqueue->push(key);
    }
  else
    post("h_queue, push: no arguments");
}

static void h_queue_pop(t_h_queue *x)
{
  if(x->hqueue->getSize()<=0)
    {
      post("h_queue, pop: size is 0 !");
      return;
    }

  x->hqueue->pop();
}

static void h_queue_front(t_h_queue *x)
{
  if(x->hqueue->getSize() == 0)
    {
      outlet_bang(x->out2);
      return;
    }

  Element output = x->hqueue->front();
 
  if(output.getLength() == 1) // symbol or float
    {
      if (output.getAtom()[0].a_type == A_FLOAT)
	outlet_float(x->out0, output.getAtom()[0].a_w.w_float);
      if (output.getAtom()[0].a_type == A_SYMBOL)
	outlet_symbol(x->out0, output.getAtom()[0].a_w.w_symbol);
      if (output.getAtom()[0].a_type == A_POINTER)
	outlet_pointer(x->out0, output.getAtom()[0].a_w.w_gpointer);
      return;
    }
  if(output.getLength() > 1) // list
    {
      outlet_list(x->out0,&s_list,output.getLength(),output.getAtom());
      return;
    }
    
  // outlet bang if no data here
  outlet_bang(x->out2);
}

static void h_queue_getsize(t_h_queue *x)
{
  outlet_float(x->out1,x->hqueue->getSize());
}

static void h_queue_help(t_h_queue *x)
{
  x->hqueue->help();
}

static void h_queue_set_namespace(t_h_queue *x, t_symbol *s)
{
  x->hqueue->setNamespace(s->s_name);
}

static void h_queue_get_namespace(t_h_queue *x)
{
  post("h_queue current namespace: %s",x->hqueue->getNamespace().c_str());
}

static void h_queue_clear(t_h_queue *x)
{
  x->hqueue->clearNamespace();
}

static void h_queue_clear_all(t_h_queue *x)
{
  x->hqueue->clearAll();
}

static void *h_queue_new(t_symbol *s, int argc, t_atom *argv) 
{
  t_h_queue *x = (t_h_queue *)pd_new(h_queue_class);
  
  switch(argc)
    {
    default:
      post("h_queue warning: only one argument for namespace is possible!");
   case 1:
      x->hqueue = new HQueue(atom_getsymbol(argv)->s_name);
      break;
    case 0:
      x->hqueue = new HQueue();
      break;
    }

  x->out0 = outlet_new(&x->x_obj, 0);
  x->out1 = outlet_new(&x->x_obj, &s_float);
  x->out2 = outlet_new(&x->x_obj, &s_bang);
  x->x_canvas = canvas_getcurrent();
    
  return (void *)x;
}

static void *h_queue_free(t_h_queue *x)
{
  delete x->hqueue;
  return (void *)x;
}

#if defined(PDCONTAINER_SINGLE_OBJECT)
// for PD-Extended
extern "C" {
#endif

void h_queue_setup(void) 
{
  // the object class
  h_queue_class = class_new(gensym("h_queue"), (t_newmethod)h_queue_new,
				(t_method)h_queue_free, sizeof(t_h_queue), 
				CLASS_DEFAULT, A_GIMME, 0);

  class_addmethod(h_queue_class, (t_method)h_queue_push, 
		  gensym("push"), A_GIMME, 0);
  class_addmethod(h_queue_class, (t_method)h_queue_pop, 
		  gensym("pop"), A_DEFFLOAT, 0);
  class_addmethod(h_queue_class, (t_method)h_queue_front, 
		  gensym("front"), A_DEFFLOAT, 0);
  class_addmethod(h_queue_class, (t_method)h_queue_getsize, 
		  gensym("getsize"), A_DEFFLOAT , 0);
  class_addmethod(h_queue_class, (t_method)h_queue_set_namespace, 
		  gensym("namespace"), A_DEFSYMBOL , 0);
  class_addmethod(h_queue_class, (t_method)h_queue_get_namespace, 
		  gensym("getnamespace"), A_DEFFLOAT, 0);
  class_addmethod(h_queue_class, (t_method)h_queue_clear,  
		  gensym("clear"), A_DEFFLOAT, 0);
  class_addmethod(h_queue_class, (t_method)h_queue_clear_all,  
		  gensym("clearall"), A_DEFFLOAT, 0);

  // without an argument the following two methods wont work ??? why?? because of c++?
  class_addmethod(h_queue_class, (t_method)h_queue_help, gensym("help"),A_DEFFLOAT, 0);
}

#if defined(PDCONTAINER_SINGLE_OBJECT)
// for PD-Extended
}
#endif
