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
// h_vector.cpp


#include "include/HVector.h"


static t_class *h_vector_class;
static t_class *proxy_class;

typedef struct _h_vector 
{
  t_object  x_obj;
  t_canvas *x_canvas;
  t_outlet *out0, *out1, *out2;
  HVector *hvector;
  Element value;
  bool event_set;
} t_h_vector;

typedef struct proxy
{
  t_object obj;
  t_int index;  // number of proxy inlet(s)
  t_h_vector *x;	// we'll put the other struct in here
} t_proxy;

static void h_vector_set(t_h_vector *x, t_symbol *s, int argc, t_atom *argv)
{
  int index;
  if(argc && (argv[0].a_type == A_FLOAT))
    index = static_cast<int>(argv[0].a_w.w_float);
  else
    {
      post("h_vector, set: invalid index!");
      return;
    }

  if ( index >= x->hvector->getSize() || index < 0 )
    {
      post("h_vector, set: invalid index!");
      return;
    }

  if(!x->event_set)
    {
      post("h_vector, set: you must first set a value at right inlet!");
      return;
    }
  
  x->hvector->set( index , x->value); 
  x->event_set = false;
}

static void h_vector_push_back(t_h_vector *x, t_symbol *s, int argc, t_atom *argv)
{
  if(argc)
    {
      Element key(argc,argv);
      x->hvector->pushBack(key);
    }
  else
    post("h_vector, pushback: no arguments");
}

static void h_vector_insert(t_h_vector *x, t_symbol *s, int argc, t_atom *argv)
{
  int index;
  if(argc && (argv[0].a_type == A_FLOAT))
    index = static_cast<int>(argv[0].a_w.w_float);
  else
    {
      post("h_vector, insert: invalid index!");
      return;
    }

  if ( index >= x->hvector->getSize() || index < 0 )
    {
      post("h_vector, insert: invalid index!");
      return;
    }

  if(!x->event_set)
    {
      post("h_vector, insert: you must first set a value at right inlet!");
      return;
    }
  
  x->hvector->insert( index , x->value); 
  x->event_set = false;
}

static void h_vector_value(t_proxy *p, t_symbol *s, int argc, t_atom *argv)
{
  t_h_vector *x = (t_h_vector *)(p->x);

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

static void h_vector_get(t_h_vector *x, t_symbol *s, int argc, t_atom *argv)
{
  int index;
  if(argc && (argv[0].a_type == A_FLOAT))
    index = static_cast<int>(argv[0].a_w.w_float);
  else
    {
      post("h_vector, get: invalid index!");
      return;
    }

  if ( index >= x->hvector->getSize() || index < 0 )
    {
      post("h_vector, get: invalid index!");
      return;
    }

  Element output = x->hvector->get( index );
 
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

static void h_vector_pop_back(t_h_vector *x)
{
  if(x->hvector->getSize()<=0)
    {
      post("h_vector, popback: size is already 0 !");
      return;
    }

  x->hvector->popBack();
}

static void h_vector_remove(t_h_vector *x, t_symbol *s, int argc, t_atom *argv)
{
  int index;
  if(argc && (argv[0].a_type == A_FLOAT))
    index = static_cast<int>(argv[0].a_w.w_float);
  else
    {
      post("h_vector, remove: invalid index!");
      return;
    }

  if ( index >= x->hvector->getSize() || index < 0 )
    {
      post("h_vector, remove: invalid index!");
      return;
    }
  
  x->hvector->remove(index); 
  x->event_set = false;
}

static void h_vector_resize(t_h_vector *x, t_symbol *s, int argc, t_atom *argv)
{
  int size;
  if(argc && (argv[0].a_type == A_FLOAT))
    size = static_cast<int>(argv[0].a_w.w_float);
  else
    {
      post("h_vector, resize: invalid index!");
      return;
    }

  if ( size < 0 )
    {
      post("h_vector, size: invalid size!");
      return;
    }

  x->hvector->resize( size );
}

static void h_vector_getsize(t_h_vector *x)
{
  outlet_float(x->out1,x->hvector->getSize());
}

static void h_vector_help(t_h_vector *x)
{
  x->hvector->help();
}

static void h_vector_set_namespace(t_h_vector *x, t_symbol *s)
{
  x->hvector->setNamespace(s->s_name);
}

static void h_vector_get_namespace(t_h_vector *x)
{
  post("h_vector current namespace: %s",x->hvector->getNamespace().c_str());
}

static void h_vector_getall(t_h_vector *x)
{
  vector<Element>::iterator iter  = x->hvector->getAll().begin();
  
  while(iter != x->hvector->getAll().end())
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

static void h_vector_print(t_h_vector *x)
{
  x->hvector->printAllIndex();
}

static void h_vector_clear(t_h_vector *x)
{
  x->hvector->clearNamespace();
}

static void h_vector_clear_all(t_h_vector *x)
{
  x->hvector->clearAll();
}

static void h_vector_save(t_h_vector *x, t_symbol *s)
{
  // make correct path
  char filnam[MAXPDSTRING];
  char filename[MAXPDSTRING];
  canvas_makefilename(x->x_canvas, s->s_name, filnam, MAXPDSTRING);
  sys_bashfilename(filnam, filename);

  if(x->hvector->saveToFile(filename))
    post("h_vector: data of namespace %s written to file %s",
	 x->hvector->getNamespace().c_str(),s->s_name);
  else
    post("h_vector: couldn't write to file %s",s->s_name);
}

static void h_vector_read(t_h_vector *x, t_symbol *s, int argc, t_atom *argv)
{
  // make correct path
  char filnam[MAXPDSTRING];
  char filename[MAXPDSTRING];
  canvas_makefilename(x->x_canvas, s->s_name, filnam, MAXPDSTRING);
  sys_bashfilename(filnam, filename);

  if(!x->hvector->readFromFile(filename))
    post("h_vector: couldn't read from file %s",s->s_name);
}

static void h_vector_read_at(t_h_vector *x, t_symbol *s, int argc, t_atom *argv)
{
  string symbol;
  int index=0;

  switch(argc)
    {
    default:
      post("h_vector read: only two argument are possible!");
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
      post("h_vector read: no filename!");
    }

  // make correct path
  char filnam[MAXPDSTRING];
  char filename[MAXPDSTRING];
  canvas_makefilename(x->x_canvas, (char*)symbol.c_str(), filnam, MAXPDSTRING);
  sys_bashfilename(filnam, filename);

  if(!x->hvector->readFromFile2(filename,index))
    post("h_vector: couldn't read from file %s",s->s_name);
}

static void h_vector_save_xml(t_h_vector *x, t_symbol *s)
{
  // make correct path
  char filnam[MAXPDSTRING];
  char filename[MAXPDSTRING];
  canvas_makefilename(x->x_canvas, s->s_name, filnam, MAXPDSTRING);
  sys_bashfilename(filnam, filename);

  if(x->hvector->saveToFileXML(filename))
    post("h_vector: data of namespace %s written to file %s",
         x->hvector->getNamespace().c_str(),s->s_name);
  else
    post("h_vector: couldn't write to file %s",s->s_name);
}

static void h_vector_read_xml(t_h_vector *x, t_symbol *s)
{
  // make correct path
  char filnam[MAXPDSTRING];
  char filename[MAXPDSTRING];
  canvas_makefilename(x->x_canvas, s->s_name, filnam, MAXPDSTRING);
  sys_bashfilename(filnam, filename);

  if(!x->hvector->readFromFileXML(filename))
    post("h_vector: couldn't read from file %s",s->s_name);
}

static void h_vector_read_at_xml(t_h_vector *x, t_symbol *s, int argc, t_atom *argv)
{
  string symbol;
  int index=0;

  switch(argc)
    {
    default:
      post("h_vector read: only two argument are possible!");
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
      post("h_vector read: no filename!");
    }

  // make correct path
  char filnam[MAXPDSTRING];
  char filename[MAXPDSTRING];
  canvas_makefilename(x->x_canvas, (char*)symbol.c_str(), filnam, MAXPDSTRING);
  sys_bashfilename(filnam, filename);

  if(!x->hvector->readFromFile2XML(filename,index))
    post("h_vector: couldn't read from file %s",s->s_name);
}

static void *h_vector_new(t_symbol *s, int argc, t_atom *argv) 
{
  t_h_vector *x = (t_h_vector *)pd_new(h_vector_class);
  t_proxy *inlet = (t_proxy *)pd_new(proxy_class); // for the proxy inlet

  inlet->x = x;	// make x visible to the proxy inlets
  
  switch(argc)
    {
    default:
      post("h_vector warning: only one argument for namespace is possible!");
   case 1:
      x->hvector = new HVector(atom_getsymbol(argv)->s_name);
      break;
    case 0:
      x->hvector = new HVector();
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

static void *h_vector_free(t_h_vector *x)
{
  delete x->hvector;
  return (void *)x;
}

#if defined(PDCONTAINER_SINGLE_OBJECT)
// for PD-Extended
extern "C" {
#endif

void h_vector_setup(void) 
{
  // the object class
  h_vector_class = class_new(gensym("h_vector"), (t_newmethod)h_vector_new,
				(t_method)h_vector_free, sizeof(t_h_vector), 
				CLASS_DEFAULT, A_GIMME, 0);

  // a class for the proxy-inlet
  proxy_class = class_new(gensym("h_vector_proxy"), NULL, NULL, sizeof(t_proxy),
			  CLASS_PD|CLASS_NOINLET, A_NULL);

  class_addmethod(h_vector_class, (t_method)h_vector_set, 
		  gensym("set"), A_GIMME, 0);
  class_addmethod(h_vector_class, (t_method)h_vector_push_back, 
		  gensym("pushback"), A_GIMME, 0);
  class_addmethod(h_vector_class, (t_method)h_vector_insert, 
		  gensym("insert"), A_GIMME, 0);
  class_addanything(proxy_class, (t_method)h_vector_value); // the right inlet
  class_addmethod(h_vector_class, (t_method)h_vector_get, 
		  gensym("get"), A_GIMME, 0);
  class_addmethod(h_vector_class, (t_method)h_vector_pop_back, 
		  gensym("popback"), A_DEFFLOAT, 0);
  class_addmethod(h_vector_class, (t_method)h_vector_remove, 
		  gensym("remove"), A_GIMME, 0);
  class_addmethod(h_vector_class, (t_method)h_vector_resize, 
		  gensym("resize"), A_GIMME , 0);
  class_addmethod(h_vector_class, (t_method)h_vector_getsize, 
		  gensym("getsize"), A_DEFFLOAT , 0);
  class_addmethod(h_vector_class, (t_method)h_vector_set_namespace, 
		  gensym("namespace"), A_DEFSYMBOL , 0);
  class_addmethod(h_vector_class, (t_method)h_vector_get_namespace, 
		  gensym("getnamespace"), A_DEFFLOAT, 0);
  class_addmethod(h_vector_class, (t_method)h_vector_getall,
		  gensym("getall"), A_DEFFLOAT, 0);
  class_addmethod(h_vector_class, (t_method)h_vector_print,
		  gensym("print"), A_DEFFLOAT, 0);
  class_addmethod(h_vector_class, (t_method)h_vector_clear,  
		  gensym("clear"), A_DEFFLOAT, 0);
  class_addmethod(h_vector_class, (t_method)h_vector_clear_all,  
		  gensym("clearall"), A_DEFFLOAT, 0);
  class_addmethod(h_vector_class, (t_method)h_vector_save, 
		  gensym("save"), A_DEFSYMBOL, 0);
  class_addmethod(h_vector_class, (t_method)h_vector_read, 
		  gensym("read"), A_DEFSYMBOL, 0);
  class_addmethod(h_vector_class, (t_method)h_vector_read_at, 
		  gensym("readat"), A_GIMME, 0);
  class_addmethod(h_vector_class, (t_method)h_vector_save_xml, 
		  gensym("saveXML"), A_DEFSYMBOL , 0);
  class_addmethod(h_vector_class, (t_method)h_vector_read_xml, 
		  gensym("readXML"), A_DEFSYMBOL , 0);
  class_addmethod(h_vector_class, (t_method)h_vector_read_at_xml, 
		  gensym("readatXML"), A_GIMME, 0);

  // without an argument the following two methods wont work ??? why?? because of c++?
  class_addmethod(h_vector_class, (t_method)h_vector_help, gensym("help"),A_DEFFLOAT, 0);
}

#if defined(PDCONTAINER_SINGLE_OBJECT)
// for PD-Extended
}
#endif
