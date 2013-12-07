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
// h_stack.cpp


#include "include/HStack.h"


static t_class *h_stack_class;

typedef struct _h_stack 
{
  t_object  x_obj;
  t_canvas *x_canvas;
  t_outlet *out0, *out1, *out2;
  HStack *hstack;
} t_h_stack;

static void h_stack_push(t_h_stack *x, t_symbol *s, int argc, t_atom *argv)
{
  if(argc)
    {
      Element key(argc,argv);
      x->hstack->push(key);
    }
  else
    post("h_stack, push: no arguments");
}

static void h_stack_pop(t_h_stack *x)
{
  if(x->hstack->getSize()<=0)
    {
      post("h_stack, pop: size is 0 !");
      return;
    }

  x->hstack->pop();
}

static void h_stack_top(t_h_stack *x)
{
  if(x->hstack->getSize() == 0)
    {
      outlet_bang(x->out2);
      return;
    }

  Element output = x->hstack->top();
 
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
    
  // no data
  outlet_bang(x->out2);
}

static void h_stack_getsize(t_h_stack *x)
{
  outlet_float(x->out1,x->hstack->getSize());
}

static void h_stack_help(t_h_stack *x)
{
  x->hstack->help();
}

static void h_stack_set_namespace(t_h_stack *x, t_symbol *s)
{
  x->hstack->setNamespace(s->s_name);
}

static void h_stack_get_namespace(t_h_stack *x)
{
  post("h_stack current namespace: %s",x->hstack->getNamespace().c_str());
}

static void h_stack_clear(t_h_stack *x)
{
  x->hstack->clearNamespace();
}

static void h_stack_clear_all(t_h_stack *x)
{
  x->hstack->clearAll();
}

static void *h_stack_new(t_symbol *s, int argc, t_atom *argv) 
{
  t_h_stack *x = (t_h_stack *)pd_new(h_stack_class);
  
  switch(argc)
    {
    default:
      post("h_stack warning: only one argument for namespace is possible!");
   case 1:
      x->hstack = new HStack(atom_getsymbol(argv)->s_name);
      break;
    case 0:
      x->hstack = new HStack();
      break;
    }

  x->out0 = outlet_new(&x->x_obj, 0);
  x->out1 = outlet_new(&x->x_obj, &s_float);
  x->out2 = outlet_new(&x->x_obj, &s_bang);
  x->x_canvas = canvas_getcurrent();
    
  return (void *)x;
}

static void *h_stack_free(t_h_stack *x)
{
  delete x->hstack;
  return (void *)x;
}

#if defined(PDCONTAINER_SINGLE_OBJECT)
// for PD-Extended
extern "C" {
#endif

void h_stack_setup(void) 
{
  // the object class
  h_stack_class = class_new(gensym("h_stack"), (t_newmethod)h_stack_new,
				(t_method)h_stack_free, sizeof(t_h_stack), 
				CLASS_DEFAULT, A_GIMME, 0);

  class_addmethod(h_stack_class, (t_method)h_stack_push, 
		  gensym("push"), A_GIMME, 0);
  class_addmethod(h_stack_class, (t_method)h_stack_pop, 
		  gensym("pop"), A_DEFFLOAT, 0);
  class_addmethod(h_stack_class, (t_method)h_stack_top, 
		  gensym("top"), A_DEFFLOAT, 0);
  class_addmethod(h_stack_class, (t_method)h_stack_getsize, 
		  gensym("getsize"), A_DEFFLOAT , 0);
  class_addmethod(h_stack_class, (t_method)h_stack_set_namespace, 
		  gensym("namespace"), A_DEFSYMBOL , 0);
  class_addmethod(h_stack_class, (t_method)h_stack_get_namespace, 
		  gensym("getnamespace"), A_DEFFLOAT, 0);
  class_addmethod(h_stack_class, (t_method)h_stack_clear,  
		  gensym("clear"), A_DEFFLOAT, 0);
  class_addmethod(h_stack_class, (t_method)h_stack_clear_all,  
		  gensym("clearall"), A_DEFFLOAT, 0);

  // without an argument the following two methods wont work ??? why?? because of c++?
  class_addmethod(h_stack_class, (t_method)h_stack_help, gensym("help"),A_DEFFLOAT, 0);
}

#if defined(PDCONTAINER_SINGLE_OBJECT)
// for PD-Extended
}
#endif
