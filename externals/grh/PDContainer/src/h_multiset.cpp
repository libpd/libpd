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
// h_multiset.cpp


#include "include/HMultiSet.h"


static t_class *h_multiset_class;

typedef struct _h_multiset 
{
  t_object  x_obj;
  t_canvas *x_canvas;
  t_outlet *out0, *out1;
  HMultiSet *hmultiset;
} t_h_multiset;

static void h_multiset_add(t_h_multiset *x, t_symbol *s, int argc, t_atom *argv)
{
  if(!argc)
    {
      post("h_multiset, add: no arguments"); 
      return;
    }
  
  Element key(argc,argv);
  x->hmultiset->add( key );
}

static void h_multiset_get(t_h_multiset *x, t_symbol *s, int argc, t_atom *argv)
{
  if(!argc)
    {
      post("h_multiset, get: no arguments"); 
      return;
    }  
  
  Element key(argc,argv);
  int output = x->hmultiset->get( key );

  outlet_float(x->out0, output);
}

static void h_multiset_remove(t_h_multiset *x, t_symbol *s, int argc, t_atom *argv)
{
  if(!argc)
    {
      post("h_multiset, remove: no arguments"); 
      return;
    }  
  
  Element key(argc,argv);
  x->hmultiset->remove( key );
}

static void h_multiset_getsize(t_h_multiset *x)
{
  outlet_float(x->out1,x->hmultiset->getSize());
}

static void h_multiset_help(t_h_multiset *x)
{
  x->hmultiset->help();
}

static void h_multiset_set_namespace(t_h_multiset *x, t_symbol *s)
{
  x->hmultiset->setNamespace(s->s_name);
}

static void h_multiset_get_namespace(t_h_multiset *x)
{
  post("h_multiset current namespace: %s",x->hmultiset->getNamespace().c_str());
}

static void h_multiset_getall(t_h_multiset *x)
{
  multiset<Element>::iterator iter  = x->hmultiset->getAll().begin();
  
  while(iter != x->hmultiset->getAll().end())
  {
    Element output = *iter;
 
    if(output.getLength() == 1) // symbol or float
    {
      if (output.getAtom()[0].a_type == A_FLOAT)
	outlet_float(x->out0, output.getAtom()[0].a_w.w_float);
      if (output.getAtom()[0].a_type == A_SYMBOL)
	outlet_symbol(x->out0, output.getAtom()[0].a_w.w_symbol);
      if (output.getAtom()[0].a_type == A_POINTER)
	outlet_pointer(x->out0, output.getAtom()[0].a_w.w_gpointer);
    }
    if(output.getLength() > 1) // list
      outlet_list(x->out0,&s_list,output.getLength(),output.getAtom());

    iter++;
  }
}

static void h_multiset_print(t_h_multiset *x)
{
  x->hmultiset->printAll();
}

static void h_multiset_clear(t_h_multiset *x)
{
  x->hmultiset->clearNamespace();
}

static void h_multiset_clear_all(t_h_multiset *x)
{
  x->hmultiset->clearAll();
}

static void h_multiset_save(t_h_multiset *x, t_symbol *s)
{
  // make correct path
  char filnam[MAXPDSTRING];
  char filename[MAXPDSTRING];
  canvas_makefilename(x->x_canvas, s->s_name, filnam, MAXPDSTRING);
  sys_bashfilename(filnam, filename);

  if(x->hmultiset->saveToFile(filename))
    post("h_multiset: data of namespace %s written to file %s",
	 x->hmultiset->getNamespace().c_str(),s->s_name);
  else
    post("h_multiset: couldn't write to file %s",s->s_name);
}

static void h_multiset_read(t_h_multiset *x, t_symbol *s)
{
  // make correct path
  char filnam[MAXPDSTRING];
  char filename[MAXPDSTRING];
  canvas_makefilename(x->x_canvas, s->s_name, filnam, MAXPDSTRING);
  sys_bashfilename(filnam, filename);

  if(!x->hmultiset->readFromFile(filename))
    post("h_multiset: couldn't read from file %s",s->s_name);
}

static void h_multiset_save_xml(t_h_multiset *x, t_symbol *s)
{
  // make correct path
  char filnam[MAXPDSTRING];
  char filename[MAXPDSTRING];
  canvas_makefilename(x->x_canvas, s->s_name, filnam, MAXPDSTRING);
  sys_bashfilename(filnam, filename);

  if(x->hmultiset->saveToFileXML(filename))
    post("h_multiset: data of namespace %s written to file %s",
         x->hmultiset->getNamespace().c_str(),s->s_name);
  else
    post("h_multiset: couldn't write to file %s",s->s_name);
}

static void h_multiset_read_xml(t_h_multiset *x, t_symbol *s)
{
  // make correct path
  char filnam[MAXPDSTRING];
  char filename[MAXPDSTRING];
  canvas_makefilename(x->x_canvas, s->s_name, filnam, MAXPDSTRING);
  sys_bashfilename(filnam, filename);

  if(!x->hmultiset->readFromFileXML(filename))
    post("h_multiset: couldn't read from file %s",s->s_name);
}

static void *h_multiset_new(t_symbol *s, int argc, t_atom *argv) 
{

  t_h_multiset *x = (t_h_multiset *)pd_new(h_multiset_class);
  
  switch(argc)
    {
    default:
      post("h_multiset warning: only one argument for namespace is possible!");
    case 1:
      x->hmultiset = new HMultiSet(atom_getsymbol(argv)->s_name);
      break;
    case 0:
      x->hmultiset = new HMultiSet();
      break;
    }

  x->out0 = outlet_new(&x->x_obj, &s_float);
  x->out1 = outlet_new(&x->x_obj, &s_float);
  x->x_canvas = canvas_getcurrent();
    
  return (void *)x;
}

static void *h_multiset_free(t_h_multiset *x)
{
  delete x->hmultiset;
  return (void *)x;
}

#if defined(PDCONTAINER_SINGLE_OBJECT)
// for PD-Extended
extern "C" {
#endif

void h_multiset_setup(void) 
{
  // the object class
  h_multiset_class = class_new(gensym("h_multiset"), (t_newmethod)h_multiset_new,
				(t_method)h_multiset_free, sizeof(t_h_multiset), 
				CLASS_DEFAULT, A_GIMME, 0);

  class_addmethod(h_multiset_class, (t_method)h_multiset_add, 
		  gensym("add"), A_GIMME , 0);
  class_addmethod(h_multiset_class, (t_method)h_multiset_get, 
		  gensym("get"), A_GIMME , 0);
  class_addmethod(h_multiset_class, (t_method)h_multiset_remove, 
		  gensym("remove"), A_GIMME , 0);
  class_addmethod(h_multiset_class, (t_method)h_multiset_getsize, 
		  gensym("getsize"), A_DEFFLOAT , 0);
  class_addmethod(h_multiset_class, (t_method)h_multiset_set_namespace, 
		  gensym("namespace"), A_DEFSYMBOL , 0);
  class_addmethod(h_multiset_class, (t_method)h_multiset_get_namespace, 
		  gensym("getnamespace"), A_DEFFLOAT, 0);
  class_addmethod(h_multiset_class, (t_method)h_multiset_getall,
		  gensym("getall"), A_DEFFLOAT, 0);
  class_addmethod(h_multiset_class, (t_method)h_multiset_print,
		  gensym("print"), A_DEFFLOAT, 0);
  class_addmethod(h_multiset_class, (t_method)h_multiset_clear,  
		  gensym("clear"), A_DEFFLOAT, 0);
  class_addmethod(h_multiset_class, (t_method)h_multiset_clear_all,  
		  gensym("clearall"), A_DEFFLOAT, 0);
  class_addmethod(h_multiset_class, (t_method)h_multiset_save, 
		  gensym("save"), A_DEFSYMBOL , 0);
  class_addmethod(h_multiset_class, (t_method)h_multiset_read, 
		  gensym("read"), A_DEFSYMBOL , 0);
  class_addmethod(h_multiset_class, (t_method)h_multiset_save_xml, 
		  gensym("saveXML"), A_DEFSYMBOL , 0);
  class_addmethod(h_multiset_class, (t_method)h_multiset_read_xml, 
		  gensym("readXML"), A_DEFSYMBOL , 0);

  // without an argument the following two methods wont work ??? why?? because of c++?
  class_addmethod(h_multiset_class, (t_method)h_multiset_help, gensym("help"),A_DEFFLOAT, 0);
}

#if defined(PDCONTAINER_SINGLE_OBJECT)
// for PD-Extended
}
#endif
