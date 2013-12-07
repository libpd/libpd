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
// h_list.cpp


#include "include/HList.h"


static t_class *h_list_class;

typedef struct _h_list 
{
  t_object  x_obj;
  t_canvas *x_canvas;
  t_outlet *out0, *out1, *out2, *out3;
  HList *hlist;
} t_h_list;

static void h_list_push_back(t_h_list *x, t_symbol *s, int argc, t_atom *argv)
{
  if(argc)
    {
      Element key(argc,argv);
      x->hlist->pushBack(key);
    }
  else
    post("h_list, pushback: no arguments");
}

static void h_list_pop_back(t_h_list *x)
{
  if(x->hlist->getSize()<=0)
    {
      post("h_list, popback: size is 0 !");
      return;
    }

  x->hlist->popBack();
}

static void h_list_push_front(t_h_list *x, t_symbol *s, int argc, t_atom *argv)
{
  if(argc)
    {
      Element key(argc,argv);
      x->hlist->pushFront(key);
    }
  else
    post("h_list, pushfront: no arguments");
}

static void h_list_pop_front(t_h_list *x)
{
  if(x->hlist->getSize()<=0)
    {
      post("h_list, popfront: size is 0 !");
      return;
    }

  x->hlist->popFront();
}

static void h_list_back(t_h_list *x)
{
  if(x->hlist->getSize() == 0)
    {
      outlet_bang(x->out3);
      return;
    }

  Element output = x->hlist->back();
 
  if(output.getLength() == 1) // symbol, float or pointer
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

  outlet_bang(x->out3);
}

static void h_list_front(t_h_list *x)
{
  if(x->hlist->getSize() == 0)
    {
      outlet_bang(x->out3);
      return;
    }

  Element output = x->hlist->front();
 
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
    
  outlet_bang(x->out3);
}

static void h_list_get(t_h_list *x)
{
  if(x->hlist->getSize() == 0)
    {
      outlet_bang(x->out3);
      return;
    }

  Element output;

  try
  { output = x->hlist->get(); }
  
  catch(const char* s)
  {
    // if there was no Element found, put out a bang at the right outlet
    post("%s", s);
    outlet_bang(x->out3);
    return;
  }

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

  outlet_bang(x->out3);
}

static void h_list_insert(t_h_list *x, t_symbol *s, int argc, t_atom *argv)
{
  if(argc)
    {
      Element key(argc,argv);
      x->hlist->insert(key);
    }
  else
    post("h_list, insert: no arguments");
}

static void h_list_modify(t_h_list *x, t_symbol *s, int argc, t_atom *argv)
{
  if(argc)
  {
    Element key(argc,argv);
    x->hlist->modify(key);
  }
  else
    post("h_list, modify: no arguments");
}

static void h_list_remove(t_h_list *x, t_symbol *s, int argc, t_atom *argv)
{
  if(argc)
    {
      Element key(argc,argv);
      x->hlist->remove(key);
    }
  else
    post("h_list, remove: no arguments");
}

static void h_list_delete(t_h_list *x)
{
  x->hlist->del();
}

static void h_list_begin(t_h_list *x)
{
  x->hlist->begin();
}

static void h_list_end(t_h_list *x)
{
  x->hlist->end();
}

static void h_list_next(t_h_list *x)
{
  x->hlist->next();
}

static void h_list_last(t_h_list *x)
{
  x->hlist->last();
}

static void h_list_unique(t_h_list *x)
{
  x->hlist->unique();
}

static void h_list_reverse(t_h_list *x)
{
  x->hlist->reverse();
}

static void h_list_sort(t_h_list *x)
{
  x->hlist->sort();
}

static void h_list_getsize(t_h_list *x)
{
  outlet_float(x->out2,x->hlist->getSize());
}

static void h_list_get_iter_pos(t_h_list *x)
{
  outlet_float(x->out1,x->hlist->getIterPos());
}

static void h_list_set_iter_pos(t_h_list *x, t_floatarg f)
{
  x->hlist->setIterPos(static_cast<int>(f));
}

static void h_list_help(t_h_list *x)
{
  x->hlist->help();
}

static void h_list_set_namespace(t_h_list *x, t_symbol *s)
{
  x->hlist->setNamespace(s->s_name);
}

static void h_list_get_namespace(t_h_list *x)
{
  post("h_list current namespace: %s",x->hlist->getNamespace().c_str());
}

static void h_list_clear(t_h_list *x)
{
  x->hlist->clearNamespace();
}

static void h_list_clear_all(t_h_list *x)
{
  x->hlist->clearAll();
}

static void h_list_getall(t_h_list *x)
{
  list<Element>::iterator iter  = x->hlist->getAll().begin();
  
  while(iter != x->hlist->getAll().end())
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

static void h_list_print(t_h_list *x)
{
  x->hlist->printAllIndex();
}

static void h_list_save(t_h_list *x, t_symbol *s)
{
  // make correct path
  char filnam[MAXPDSTRING];
  char filename[MAXPDSTRING];
  canvas_makefilename(x->x_canvas, s->s_name, filnam, MAXPDSTRING);
  sys_bashfilename(filnam, filename);

  if(x->hlist->saveToFile(filename))
    post("h_list: data of namespace %s written to file %s",
	 x->hlist->getNamespace().c_str(),s->s_name);
  else
    post("h_list: couldn't write to file %s",s->s_name);
}

static void h_list_read(t_h_list *x, t_symbol *s)
{
  // make correct path
  char filnam[MAXPDSTRING];
  char filename[MAXPDSTRING];
  canvas_makefilename(x->x_canvas, s->s_name, filnam, MAXPDSTRING);
  sys_bashfilename(filnam, filename);

  if(!x->hlist->readFromFile(filename))
    post("h_list: couldn't read from file %s",s->s_name);
}

static void h_list_save_xml(t_h_list *x, t_symbol *s)
{
  // make correct path
  char filnam[MAXPDSTRING];
  char filename[MAXPDSTRING];
  canvas_makefilename(x->x_canvas, s->s_name, filnam, MAXPDSTRING);
  sys_bashfilename(filnam, filename);

  if(x->hlist->saveToFileXML(filename))
    post("h_list: data of namespace %s written to file %s",
         x->hlist->getNamespace().c_str(),s->s_name);
  else
    post("h_list: couldn't write to file %s",s->s_name);
}

static void h_list_read_xml(t_h_list *x, t_symbol *s)
{
  // make correct path
  char filnam[MAXPDSTRING];
  char filename[MAXPDSTRING];
  canvas_makefilename(x->x_canvas, s->s_name, filnam, MAXPDSTRING);
  sys_bashfilename(filnam, filename);

  if(!x->hlist->readFromFileXML(filename))
    post("h_list: couldn't read from file %s",s->s_name);
}

static void *h_list_new(t_symbol *s, int argc, t_atom *argv) 
{
  t_h_list *x = (t_h_list *)pd_new(h_list_class);
  
  switch(argc)
    {
    default:
      post("h_list warning: only one argument for namespace is possible!");
   case 1:
      x->hlist = new HList(atom_getsymbol(argv)->s_name);
      break;
    case 0:
      x->hlist = new HList();
      break;
    }

  x->out0 = outlet_new(&x->x_obj, 0);
  x->out1 = outlet_new(&x->x_obj, &s_float);
  x->out2 = outlet_new(&x->x_obj, &s_float);
  x->out3 = outlet_new(&x->x_obj, &s_bang);
  x->x_canvas = canvas_getcurrent();
    
  return (void *)x;
}

static void *h_list_free(t_h_list *x)
{
  delete x->hlist;
  return (void *)x;
}

#if defined(PDCONTAINER_SINGLE_OBJECT)
// for PD-Extended
extern "C" {
#endif

void h_list_setup(void) 
{
  // the object class
  h_list_class = class_new(gensym("h_list"), (t_newmethod)h_list_new,
				(t_method)h_list_free, sizeof(t_h_list), 
				CLASS_DEFAULT, A_GIMME, 0);

  class_addmethod(h_list_class, (t_method)h_list_push_back, 
		  gensym("pushback"), A_GIMME, 0);
  class_addmethod(h_list_class, (t_method)h_list_pop_back, 
		  gensym("popback"), A_DEFFLOAT, 0);
  class_addmethod(h_list_class, (t_method)h_list_push_front, 
		  gensym("pushfront"), A_GIMME, 0);
  class_addmethod(h_list_class, (t_method)h_list_pop_front, 
		  gensym("popfront"), A_DEFFLOAT, 0);
  class_addmethod(h_list_class, (t_method)h_list_back, 
		  gensym("back"), A_DEFFLOAT, 0);
  class_addmethod(h_list_class, (t_method)h_list_front, 
		  gensym("front"), A_DEFFLOAT, 0);
  class_addmethod(h_list_class, (t_method)h_list_get, 
		  gensym("get"), A_DEFFLOAT, 0);
  class_addmethod(h_list_class, (t_method)h_list_insert, 
		  gensym("insert"), A_GIMME, 0);
  class_addmethod(h_list_class, (t_method)h_list_modify, 
      gensym("modify"), A_GIMME, 0);
  class_addmethod(h_list_class, (t_method)h_list_remove, 
		  gensym("remove"), A_GIMME, 0);
  class_addmethod(h_list_class, (t_method)h_list_delete, 
		  gensym("delete"), A_DEFFLOAT, 0);
  class_addmethod(h_list_class, (t_method)h_list_getsize, 
		  gensym("getsize"), A_DEFFLOAT , 0);
  class_addmethod(h_list_class, (t_method)h_list_get_iter_pos, 
		  gensym("getiter"), A_DEFFLOAT , 0);
  class_addmethod(h_list_class, (t_method)h_list_set_iter_pos, 
		  gensym("setiter"), A_DEFFLOAT , 0);
  class_addmethod(h_list_class, (t_method)h_list_begin, 
		  gensym("begin"), A_DEFFLOAT, 0);
  class_addmethod(h_list_class, (t_method)h_list_end, 
		  gensym("end"), A_DEFFLOAT, 0);
  class_addmethod(h_list_class, (t_method)h_list_next, 
		  gensym("next"), A_DEFFLOAT, 0);
  class_addmethod(h_list_class, (t_method)h_list_last, 
		  gensym("last"), A_DEFFLOAT, 0);
  class_addmethod(h_list_class, (t_method)h_list_unique, 
		  gensym("unique"), A_DEFFLOAT, 0);
  class_addmethod(h_list_class, (t_method)h_list_reverse, 
		  gensym("reverse"), A_DEFFLOAT, 0);
  class_addmethod(h_list_class, (t_method)h_list_sort, 
		  gensym("sort"), A_DEFFLOAT, 0);
  class_addmethod(h_list_class, (t_method)h_list_set_namespace, 
		  gensym("namespace"), A_DEFSYMBOL , 0);
  class_addmethod(h_list_class, (t_method)h_list_get_namespace, 
		  gensym("getnamespace"), A_DEFFLOAT, 0);
  class_addmethod(h_list_class, (t_method)h_list_clear,  
		  gensym("clear"), A_DEFFLOAT, 0);
  class_addmethod(h_list_class, (t_method)h_list_clear_all,  
		  gensym("clearall"), A_DEFFLOAT, 0);
  class_addmethod(h_list_class, (t_method)h_list_getall,
		  gensym("getall"), A_DEFFLOAT, 0);
  class_addmethod(h_list_class, (t_method)h_list_print,
		  gensym("print"), A_DEFFLOAT, 0);
  class_addmethod(h_list_class, (t_method)h_list_save, 
		  gensym("save"), A_DEFSYMBOL , 0);
  class_addmethod(h_list_class, (t_method)h_list_read, 
		  gensym("read"), A_DEFSYMBOL , 0);
  class_addmethod(h_list_class, (t_method)h_list_save_xml, 
		  gensym("saveXML"), A_DEFSYMBOL , 0);
  class_addmethod(h_list_class, (t_method)h_list_read_xml, 
		  gensym("readXML"), A_DEFSYMBOL , 0);

  // without an argument the following two methods wont work ??? why?? because of c++?
  class_addmethod(h_list_class, (t_method)h_list_help, gensym("help"),A_DEFFLOAT, 0);
}

#if defined(PDCONTAINER_SINGLE_OBJECT)
// for PD-Extended
}
#endif
