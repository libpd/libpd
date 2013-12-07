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
// h_set.cpp


#include "include/HSet.h"


static t_class *h_set_class;

typedef struct _h_set 
{
  t_object  x_obj;
  t_canvas *x_canvas;
  t_outlet *out0, *out1;
  HSet *hset;
} t_h_set;

static void h_set_add(t_h_set *x, t_symbol *s, int argc, t_atom *argv)
{
  if(!argc)
    {
      post("h_set, add: no arguments"); 
      return;
    }
  
  Element key(argc,argv);
  x->hset->add( key );
}

static void h_set_get(t_h_set *x, t_symbol *s, int argc, t_atom *argv)
{
  if(!argc)
    {
      post("h_set, get: no arguments"); 
      return;
    }  
  
  Element key(argc,argv);
  int output = x->hset->get( key );

  outlet_float(x->out0, output);
}

static void h_set_remove(t_h_set *x, t_symbol *s, int argc, t_atom *argv)
{
  if(!argc)
    {
      post("h_set, remove: no arguments"); 
      return;
    }  
  
  Element key(argc,argv);
  x->hset->remove( key );
}

static void h_set_getsize(t_h_set *x)
{
  outlet_float(x->out1,x->hset->getSize());
}

static void h_set_help(t_h_set *x)
{
  x->hset->help();
}

static void h_set_set_namespace(t_h_set *x, t_symbol *s)
{
  x->hset->setNamespace(s->s_name);
}

static void h_set_get_namespace(t_h_set *x)
{
  post("h_set current namespace: %s",x->hset->getNamespace().c_str());
}

static void h_set_getall(t_h_set *x)
{
  set<Element>::iterator iter  = x->hset->getAll().begin();
  
  while(iter != x->hset->getAll().end())
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

static void h_set_print(t_h_set *x)
{
  x->hset->printAll();
}

static void h_set_clear(t_h_set *x)
{
  x->hset->clearNamespace();
}

static void h_set_clear_all(t_h_set *x)
{
  x->hset->clearAll();
}

static void h_set_save(t_h_set *x, t_symbol *s)
{
  // make correct path
  char filnam[MAXPDSTRING];
  char filename[MAXPDSTRING];
  canvas_makefilename(x->x_canvas, s->s_name, filnam, MAXPDSTRING);
  sys_bashfilename(filnam, filename);

  if(x->hset->saveToFile(filename))
    post("h_set: data of namespace %s written to file %s",
	 x->hset->getNamespace().c_str(),s->s_name);
  else
    post("h_set: couldn't write to file %s",s->s_name);
}

static void h_set_read(t_h_set *x, t_symbol *s)
{
  // make correct path
  char filnam[MAXPDSTRING];
  char filename[MAXPDSTRING];
  canvas_makefilename(x->x_canvas, s->s_name, filnam, MAXPDSTRING);
  sys_bashfilename(filnam, filename);

  if(!x->hset->readFromFile(filename))
    post("h_set: couldn't read from file %s",s->s_name);
}

static void h_set_save_xml(t_h_set *x, t_symbol *s)
{
  // make correct path
  char filnam[MAXPDSTRING];
  char filename[MAXPDSTRING];
  canvas_makefilename(x->x_canvas, s->s_name, filnam, MAXPDSTRING);
  sys_bashfilename(filnam, filename);

  if(x->hset->saveToFileXML(filename))
    post("h_set: data of namespace %s written to file %s",
         x->hset->getNamespace().c_str(),s->s_name);
  else
    post("h_set: couldn't write to file %s",s->s_name);
}

static void h_set_read_xml(t_h_set *x, t_symbol *s)
{
  // make correct path
  char filnam[MAXPDSTRING];
  char filename[MAXPDSTRING];
  canvas_makefilename(x->x_canvas, s->s_name, filnam, MAXPDSTRING);
  sys_bashfilename(filnam, filename);

  if(!x->hset->readFromFileXML(filename))
    post("h_set: couldn't read from file %s",s->s_name);
}

static void *h_set_new(t_symbol *s, int argc, t_atom *argv) 
{

  t_h_set *x = (t_h_set *)pd_new(h_set_class);
  
  switch(argc)
    {
    default:
      post("h_set warning: only one argument for namespace is possible!");
    case 1:
      x->hset = new HSet(atom_getsymbol(argv)->s_name);
      break;
    case 0:
      x->hset = new HSet();
      break;
    }

  x->out0 = outlet_new(&x->x_obj, &s_float);
  x->out1 = outlet_new(&x->x_obj, &s_float);
  x->x_canvas = canvas_getcurrent();
    
  return (void *)x;
}

static void *h_set_free(t_h_set *x)
{
  delete x->hset;
  return (void *)x;
}

#if defined(PDCONTAINER_SINGLE_OBJECT)
// for PD-Extended
extern "C" {
#endif

void h_set_setup(void) 
{
  // the object class
  h_set_class = class_new(gensym("h_set"), (t_newmethod)h_set_new,
				(t_method)h_set_free, sizeof(t_h_set), 
				CLASS_DEFAULT, A_GIMME, 0);

  class_addmethod(h_set_class, (t_method)h_set_add, 
		  gensym("add"), A_GIMME , 0);
  class_addmethod(h_set_class, (t_method)h_set_get, 
		  gensym("get"), A_GIMME , 0);
  class_addmethod(h_set_class, (t_method)h_set_remove, 
		  gensym("remove"), A_GIMME , 0);
  class_addmethod(h_set_class, (t_method)h_set_getsize, 
		  gensym("getsize"), A_DEFFLOAT , 0);
  class_addmethod(h_set_class, (t_method)h_set_set_namespace, 
		  gensym("namespace"), A_DEFSYMBOL , 0);
  class_addmethod(h_set_class, (t_method)h_set_get_namespace, 
		  gensym("getnamespace"), A_DEFFLOAT, 0);
  class_addmethod(h_set_class, (t_method)h_set_getall,
		  gensym("getall"), A_DEFFLOAT, 0);
  class_addmethod(h_set_class, (t_method)h_set_print,
		  gensym("print"), A_DEFFLOAT, 0);
  class_addmethod(h_set_class, (t_method)h_set_clear,  
		  gensym("clear"), A_DEFFLOAT, 0);
  class_addmethod(h_set_class, (t_method)h_set_clear_all,  
		  gensym("clearall"), A_DEFFLOAT, 0);
  class_addmethod(h_set_class, (t_method)h_set_save, 
		  gensym("save"), A_DEFSYMBOL , 0);
  class_addmethod(h_set_class, (t_method)h_set_read, 
		  gensym("read"), A_DEFSYMBOL , 0);
  class_addmethod(h_set_class, (t_method)h_set_save_xml, 
		  gensym("saveXML"), A_DEFSYMBOL , 0);
  class_addmethod(h_set_class, (t_method)h_set_read_xml, 
		  gensym("readXML"), A_DEFSYMBOL , 0);

  // without an argument the following two methods wont work ??? why?? because of c++?
  class_addmethod(h_set_class, (t_method)h_set_help, gensym("help"),A_DEFFLOAT, 0);
}

#if defined(PDCONTAINER_SINGLE_OBJECT)
// for PD-Extended
}
#endif
