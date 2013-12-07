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
// h_deque.cpp


#include "include/HDeque.h"


static t_class *h_deque_class;
static t_class *proxy_class;

typedef struct _h_deque 
{
  t_object  x_obj;
  t_canvas *x_canvas;
  t_outlet *out0, *out1, *out2;
  HDeque *hdeque;
  Element value;
  bool event_set;
} t_h_deque;

typedef struct proxy
{
  t_object obj;
  t_int index;  // number of proxy inlet(s)
  t_h_deque *x;	// we'll put the other struct in here
} t_proxy;

static void h_deque_set(t_h_deque *x, t_symbol *s, int argc, t_atom *argv)
{
  int index;
  if(argc && (argv[0].a_type == A_FLOAT))
    index = static_cast<int>(argv[0].a_w.w_float);
  else
    {
      post("h_deque, set: invalid index!");
      return;
    }

  if ( index >= x->hdeque->getSize() || index < 0 )
    {
      post("h_deque, set: invalid index!");
      return;
    }

  if(!x->event_set)
    {
      post("h_deque, set: you must first set a value at right inlet!");
      return;
    }
  
  x->hdeque->set( index , x->value); 
  x->event_set = false;
}

static void h_deque_insert(t_h_deque *x, t_symbol *s, int argc, t_atom *argv)
{
  int index;
  if(argc && (argv[0].a_type == A_FLOAT))
    index = static_cast<int>(argv[0].a_w.w_float);
  else
    {
      post("h_deque, insert: invalid index!");
      return;
    }

  if ( index >= x->hdeque->getSize() || index < 0 )
    {
      post("h_deque, insert: invalid index!");
      return;
    }

  if(!x->event_set)
    {
      post("h_deque, insert: you must first set a value at right inlet!");
      return;
    }
  
  x->hdeque->insert( index , x->value); 
  x->event_set = false;
}

static void h_deque_value(t_proxy *p, t_symbol *s, int argc, t_atom *argv)
{
  t_h_deque *x = (t_h_deque *)(p->x);
  
  // symbol without selector "symbol":
  if(argc == 0)
  {
    t_atom tmp;
    SETSYMBOL(&tmp, s);
    x->value.setAtoms(1, &tmp);
    x->event_set = true;
    return;
  }

  // input is a list without selector "list":
  if ( argc && (strcmp(s->s_name,"list")!=0) 
       && (strcmp(s->s_name,"float")!=0) 
       && (strcmp(s->s_name,"symbol")!=0)
       && (strcmp(s->s_name,"pointer")!=0) )
  {
    t_atom *atoms = (t_atom*)getbytes( (argc+1)*sizeof(t_atom) );

      // add the selector symbol to the list:
    SETSYMBOL(atoms, s);

    for(int i=0; i<argc; i++)
    {
      if(argv[i].a_type == A_FLOAT)
	SETFLOAT(&atoms[i+1],argv[i].a_w.w_float);
      if(argv[i].a_type == A_SYMBOL)
	SETSYMBOL(&atoms[i+1],argv[i].a_w.w_symbol);
      if(argv[i].a_type == A_POINTER)
	SETPOINTER(&atoms[i+1],argv[i].a_w.w_gpointer);
    }

    x->value.setAtoms(argc+1, atoms);

    x->event_set = true;
    freebytes(atoms, (argc+1)*sizeof(t_atom));
    return;
  }

  // "normal" input (list, float or symbol):
  if (argc)
  {
    x->value.setAtoms(argc, argv);
    x->event_set = true;
    return;
  }
}

static void h_deque_get(t_h_deque *x, t_symbol *s, int argc, t_atom *argv)
{
  int index;
  if(argc && (argv[0].a_type == A_FLOAT))
    index = static_cast<int>(argv[0].a_w.w_float);
  else
    {
      post("h_deque, get: invalid index!");
      return;
    }

  if ( index >= x->hdeque->getSize() || index < 0 )
    {
      post("h_deque, get: invalid index!");
      return;
    }

  Element output = x->hdeque->get( index );
 
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

  // if there was no Element found, put out a bang at the right outlet
  outlet_bang(x->out2);
}

static void h_deque_push_back(t_h_deque *x, t_symbol *s, int argc, t_atom *argv)
{
  if(argc)
    {
      Element key(argc,argv);
      x->hdeque->pushBack(key);
    }
  else
    post("h_deque, pushback: no arguments");
}

static void h_deque_pop_back(t_h_deque *x)
{
  if(x->hdeque->getSize()<=0)
    {
      post("h_deque, popback: size is already 0 !");
      return;
    }

  x->hdeque->popBack();
}

static void h_deque_back(t_h_deque *x)
{
  if(x->hdeque->getSize() == 0)
    {
      post("h_deque, back: size is 0 !");
      return;
    }

  Element output = x->hdeque->back();
 
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

  // if there was no Element found, put out a bang at the right outlet
  outlet_bang(x->out2);
}

static void h_deque_push_front(t_h_deque *x, t_symbol *s, int argc, t_atom *argv)
{
  if(argc)
    {
      Element key(argc,argv);
      x->hdeque->pushFront(key);
    }
  else
    post("h_deque, pushfront: no arguments");
}

static void h_deque_pop_front(t_h_deque *x)
{
  if(x->hdeque->getSize()<=0)
    {
      post("h_deque, popfront: size is already 0 !");
      return;
    }

  x->hdeque->popFront();
}

static void h_deque_front(t_h_deque *x)
{
  if(x->hdeque->getSize() == 0)
    {
      post("h_deque, front: size is 0 !");
      return;
    }

  Element output = x->hdeque->front();
 
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

  // if there was no Element found, put out a bang at the right outlet
  outlet_bang(x->out2);
}

static void h_deque_remove(t_h_deque *x, t_symbol *s, int argc, t_atom *argv)
{
  int index;
  if(argc && (argv[0].a_type == A_FLOAT))
    index = static_cast<int>(argv[0].a_w.w_float);
  else
    {
      post("h_deque, remove: invalid index!");
      return;
    }

  if ( index >= x->hdeque->getSize() || index < 0 )
    {
      post("h_deque, remove: invalid index!");
      return;
    }
  
  x->hdeque->remove(index); 
  x->event_set = false;
}

static void h_deque_resize(t_h_deque *x, t_symbol *s, int argc, t_atom *argv)
{
  int size;
  if(argc && (argv[0].a_type == A_FLOAT))
    size = static_cast<int>(argv[0].a_w.w_float);
  else
    {
      post("h_deque, resize: invalid index!");
      return;
    }

  if ( size < 0 )
    {
      post("h_deque, size: invalid size!");
      return;
    }

  x->hdeque->resize( size );
}

static void h_deque_getsize(t_h_deque *x)
{
  outlet_float(x->out1,x->hdeque->getSize());
}

static void h_deque_help(t_h_deque *x)
{
  x->hdeque->help();
}

static void h_deque_set_namespace(t_h_deque *x, t_symbol *s)
{
  x->hdeque->setNamespace(s->s_name);
}

static void h_deque_get_namespace(t_h_deque *x)
{
  post("h_deque current namespace: %s",x->hdeque->getNamespace().c_str());
}

static void h_deque_getall(t_h_deque *x)
{
  deque<Element>::iterator iter  = x->hdeque->getAll().begin();
  
  while(iter != x->hdeque->getAll().end())
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

static void h_deque_print(t_h_deque *x)
{
  x->hdeque->printAllIndex();
}

static void h_deque_clear(t_h_deque *x)
{
  x->hdeque->clearNamespace();
}

static void h_deque_clear_all(t_h_deque *x)
{
  x->hdeque->clearAll();
}

static void h_deque_save(t_h_deque *x, t_symbol *s)
{
  // make correct path
  char filnam[MAXPDSTRING];
  char filename[MAXPDSTRING];
  canvas_makefilename(x->x_canvas, s->s_name, filnam, MAXPDSTRING);
  sys_bashfilename(filnam, filename);

  if(x->hdeque->saveToFile(filename))
    post("h_deque: data of namespace %s written to file %s",
	 x->hdeque->getNamespace().c_str(),s->s_name);
  else
    post("h_deque: couldn't write to file %s",s->s_name);
}

static void h_deque_read(t_h_deque *x, t_symbol *s, int argc, t_atom *argv)
{
  // make correct path
  char filnam[MAXPDSTRING];
  char filename[MAXPDSTRING];
  canvas_makefilename(x->x_canvas, s->s_name, filnam, MAXPDSTRING);
  sys_bashfilename(filnam, filename);

  if(!x->hdeque->readFromFile(filename))
    post("h_deque: couldn't read from file %s",s->s_name);
}

static void h_deque_read_at(t_h_deque *x, t_symbol *s, int argc, t_atom *argv)
{
  string symbol;
  int index=0;

  switch(argc)
    {
    default:
      post("h_deque read: only two argument are possible!");
      break;
    case 2:
      symbol = argv[0].a_w.w_symbol->s_name;
      index = (int)argv[1].a_w.w_float;
      break; 
    case 1:
      symbol = argv[0].a_w.w_symbol->s_name;
      index = 0;
      break;
    case 0:
      post("h_deque read: no filename!");
    }

  // make correct path
  char filnam[MAXPDSTRING];
  char filename[MAXPDSTRING];
  canvas_makefilename(x->x_canvas, (char*)symbol.c_str(), filnam, MAXPDSTRING);
  sys_bashfilename(filnam, filename);

  if(!x->hdeque->readFromFile2(filename,index))
    post("h_deque: couldn't read from file %s",s->s_name);
}

static void h_deque_save_xml(t_h_deque *x, t_symbol *s)
{
  // make correct path
  char filnam[MAXPDSTRING];
  char filename[MAXPDSTRING];
  canvas_makefilename(x->x_canvas, s->s_name, filnam, MAXPDSTRING);
  sys_bashfilename(filnam, filename);

  if(x->hdeque->saveToFileXML(filename))
    post("h_deque: data of namespace %s written to file %s",
         x->hdeque->getNamespace().c_str(),s->s_name);
  else
    post("h_deque: couldn't write to file %s",s->s_name);
}

static void h_deque_read_xml(t_h_deque *x, t_symbol *s)
{
  // make correct path
  char filnam[MAXPDSTRING];
  char filename[MAXPDSTRING];
  canvas_makefilename(x->x_canvas, s->s_name, filnam, MAXPDSTRING);
  sys_bashfilename(filnam, filename);

  if(!x->hdeque->readFromFileXML(filename))
    post("h_deque: couldn't read from file %s",s->s_name);
}

static void h_deque_read_at_xml(t_h_deque *x, t_symbol *s, int argc, t_atom *argv)
{
  string symbol;
  int index=0;

  switch(argc)
    {
    default:
      post("h_deque read: only two argument are possible!");
      break;
    case 2:
      symbol = argv[0].a_w.w_symbol->s_name;
      index = (int)argv[1].a_w.w_float;
      break; 
    case 1:
      symbol = argv[0].a_w.w_symbol->s_name;
      index = 0;
      break;
    case 0:
      post("h_deque read: no filename!");
    }

  // make correct path
  char filnam[MAXPDSTRING];
  char filename[MAXPDSTRING];
  canvas_makefilename(x->x_canvas, (char*)symbol.c_str(), filnam, MAXPDSTRING);
  sys_bashfilename(filnam, filename);

  if(!x->hdeque->readFromFile2XML(filename,index))
    post("h_deque: couldn't read from file %s",s->s_name);
}

static void *h_deque_new(t_symbol *s, int argc, t_atom *argv) 
{
  t_h_deque *x = (t_h_deque *)pd_new(h_deque_class);
  t_proxy *inlet = (t_proxy *)pd_new(proxy_class); // for the proxy inlet

  inlet->x = x;	// make x visible to the proxy inlets
  
  switch(argc)
    {
    default:
      post("h_deque warning: only one argument for namespace is possible!");
   case 1:
      x->hdeque = new HDeque(atom_getsymbol(argv)->s_name);
      break;
    case 0:
      x->hdeque = new HDeque();
      break;
    }

  // we are going to create a proxy inlet no. 0
  // it belongs to the object x but the destination is t_proxy
  inlet->index = 0;
  inlet_new(&x->x_obj, &inlet->obj.ob_pd, 0,0);

  x->out0 = outlet_new(&x->x_obj, 0);
  x->out1 = outlet_new(&x->x_obj, &s_float);
  x->out2 = outlet_new(&x->x_obj, &s_bang);
  x->x_canvas = canvas_getcurrent();
    
  return (void *)x;
}

static void *h_deque_free(t_h_deque *x)
{
  delete x->hdeque;
  return (void *)x;
}


#if defined(PDCONTAINER_SINGLE_OBJECT)
// for PD-Extended
extern "C" {
#endif

void h_deque_setup(void) 
{
  // the object class
  h_deque_class = class_new(gensym("h_deque"), (t_newmethod)h_deque_new,
				(t_method)h_deque_free, sizeof(t_h_deque), 
				CLASS_DEFAULT, A_GIMME, 0);

  // a class for the proxy-inlet
  proxy_class = class_new(gensym("h_deque_proxy"), NULL, NULL, sizeof(t_proxy),
			  CLASS_PD|CLASS_NOINLET, A_NULL);

  class_addmethod(h_deque_class, (t_method)h_deque_set, 
		  gensym("set"), A_GIMME, 0);
  class_addmethod(h_deque_class, (t_method)h_deque_insert, 
		  gensym("insert"), A_GIMME, 0);
  class_addanything(proxy_class, (t_method)h_deque_value); // the right inlet
  class_addmethod(h_deque_class, (t_method)h_deque_get, 
		  gensym("get"), A_GIMME, 0);
  class_addmethod(h_deque_class, (t_method)h_deque_push_back, 
		  gensym("pushback"), A_GIMME, 0);
  class_addmethod(h_deque_class, (t_method)h_deque_pop_back, 
		  gensym("popback"), A_DEFFLOAT, 0);
  class_addmethod(h_deque_class, (t_method)h_deque_back, 
		  gensym("back"), A_DEFFLOAT, 0);
  class_addmethod(h_deque_class, (t_method)h_deque_push_front, 
		  gensym("pushfront"), A_GIMME, 0);
  class_addmethod(h_deque_class, (t_method)h_deque_pop_front, 
		  gensym("popfront"), A_DEFFLOAT, 0);
  class_addmethod(h_deque_class, (t_method)h_deque_front, 
		  gensym("front"), A_DEFFLOAT, 0);
  class_addmethod(h_deque_class, (t_method)h_deque_remove, 
		  gensym("remove"), A_GIMME, 0);
  class_addmethod(h_deque_class, (t_method)h_deque_resize, 
		  gensym("resize"), A_GIMME , 0);
  class_addmethod(h_deque_class, (t_method)h_deque_getsize, 
		  gensym("getsize"), A_DEFFLOAT , 0);
  class_addmethod(h_deque_class, (t_method)h_deque_set_namespace, 
		  gensym("namespace"), A_DEFSYMBOL , 0);
  class_addmethod(h_deque_class, (t_method)h_deque_get_namespace, 
		  gensym("getnamespace"), A_DEFFLOAT, 0);
  class_addmethod(h_deque_class, (t_method)h_deque_getall,
		  gensym("getall"), A_DEFFLOAT, 0);
  class_addmethod(h_deque_class, (t_method)h_deque_print,
		  gensym("print"), A_DEFFLOAT, 0);
  class_addmethod(h_deque_class, (t_method)h_deque_clear,  
		  gensym("clear"), A_DEFFLOAT, 0);
  class_addmethod(h_deque_class, (t_method)h_deque_clear_all,  
		  gensym("clearall"), A_DEFFLOAT, 0);
  class_addmethod(h_deque_class, (t_method)h_deque_save, 
		  gensym("save"), A_DEFSYMBOL, 0);
  class_addmethod(h_deque_class, (t_method)h_deque_read, 
		  gensym("read"), A_DEFSYMBOL, 0);
  class_addmethod(h_deque_class, (t_method)h_deque_read_at, 
		  gensym("readat"), A_GIMME, 0);
  class_addmethod(h_deque_class, (t_method)h_deque_save_xml, 
		  gensym("saveXML"), A_DEFSYMBOL , 0);
  class_addmethod(h_deque_class, (t_method)h_deque_read_xml, 
		  gensym("readXML"), A_DEFSYMBOL , 0);
  class_addmethod(h_deque_class, (t_method)h_deque_read_at_xml, 
		  gensym("readatXML"), A_GIMME, 0);

  // without an argument the following two methods wont work ??? why?? because of c++?
  class_addmethod(h_deque_class, (t_method)h_deque_help, gensym("help"),A_DEFFLOAT, 0);
}

#if defined(PDCONTAINER_SINGLE_OBJECT)
// for PD-Extended
}
#endif
