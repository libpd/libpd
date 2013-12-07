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
// h_multimap.cpp


#include "include/HMultiMap.h"


static t_class *h_multimap_class;
static t_class *proxy_class;

typedef struct _h_multimap 
{
  t_object  x_obj;
  t_canvas *x_canvas;
  t_outlet *out0, *out1, *out2;
  HMultiMap *hmultimap;
  Element value;
  bool event_set;
} t_h_multimap;

typedef struct proxy
{
  t_object obj;
  t_int index;      // number of proxy inlet(s)
  t_h_multimap *x;  // we'll put the other struct in here
} t_proxy;

static void h_multimap_add(t_h_multimap *x, t_symbol *s, int argc, t_atom *argv)
{
  if(!x->event_set)
    {
      post("h_multimap, add: you must first set a value at right inlet!");
      return;
    }
  
  if(argc)
    {
      Element key(argc,argv);
      x->hmultimap->add(key, x->value);
      x->event_set = false;
    }
  else
    post("h_multimap, add: no arguments"); 
}

static void h_multimap_value(t_proxy *p, t_symbol *s, int argc, t_atom *argv)
{
  t_h_multimap *x = (t_h_multimap *)(p->x);

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

  // "normal" input (list, float, symbol or pointer):
  if (argc)
  {
    x->value.setAtoms(argc, argv);
    x->event_set = true;
    return;
  }
}

static void h_multimap_get(t_h_multimap *x, t_symbol *s,  int argc, t_atom *argv)
{
  if(!argc)
    {
      post("h_multimap, get: no arguments"); 
      return;
    }  
  
  // outputs all the values of one key one after an other

  Element key(argc,argv);
  int value_nr = x->hmultimap->getNr( key );

  outlet_float(x->out1,value_nr);

  for(int i=0; i < value_nr; i++)
    {
      Element output;
      try
      { output = x->hmultimap->get(key, i); }
      catch(const char* s)
      {
	post("%s", s);
	continue;
      }

      if(output.getLength() == 1) // symbol, float or pointer
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
    }
}

static void h_multimap_remove(t_h_multimap *x, t_symbol *s, int argc, t_atom *argv)
{
  if(!argc)
    {
      post("h_multimap, remove: no arguments"); 
      return;
    }  
  
  Element key(argc,argv);
  try
  { x->hmultimap->remove( key ); }
  catch(const char* s)
  { post("%s", s); }
}

static void h_multimap_getsize(t_h_multimap *x)
{
  outlet_float(x->out2,x->hmultimap->getSize());
}

static void h_multimap_help(t_h_multimap *x)
{
  x->hmultimap->help();
}

static void h_multimap_set_namespace(t_h_multimap *x, t_symbol *s)
{
  x->hmultimap->setNamespace(s->s_name);
}

static void h_multimap_get_namespace(t_h_multimap *x)
{
  post("h_multimap current namespace: %s",x->hmultimap->getNamespace().c_str());
}

static void h_multimap_keys(t_h_multimap *x)
{
  multimap<Element,Element>::iterator iter = x->hmultimap->getAll().begin();
  
  while(iter != x->hmultimap->getAll().end())
  {
    Element output = (*iter).first;
 
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

static void h_multimap_values(t_h_multimap *x)
{
  multimap<Element,Element>::iterator iter  = x->hmultimap->getAll().begin();
  
  while(iter != x->hmultimap->getAll().end())
  {
    Element output = (*iter).second;
 
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

static void h_multimap_print(t_h_multimap *x)
{
  x->hmultimap->printAll();
}

static void h_multimap_clear(t_h_multimap *x)
{
  x->hmultimap->clearNamespace();
}

static void h_multimap_clear_all(t_h_multimap *x)
{
  x->hmultimap->clearAll();
}

static void h_multimap_save(t_h_multimap *x, t_symbol *s)
{
  // make correct path
  char filnam[MAXPDSTRING];
  char filename[MAXPDSTRING];
  canvas_makefilename(x->x_canvas, s->s_name, filnam, MAXPDSTRING);
  sys_bashfilename(filnam, filename);

  if(x->hmultimap->saveToFile(filename))
    post("h_multimap: data of namespace %s written to file %s",
	 x->hmultimap->getNamespace().c_str(),s->s_name);
  else
    post("h_multimap: couldn't write to file %s",s->s_name);
}

static void h_multimap_save_xml(t_h_multimap *x, t_symbol *s)
{
  // make correct path
  char filnam[MAXPDSTRING];
  char filename[MAXPDSTRING];
  canvas_makefilename(x->x_canvas, s->s_name, filnam, MAXPDSTRING);
  sys_bashfilename(filnam, filename);

  if(x->hmultimap->saveToFileXML(filename))
    post("h_multimap: data of namespace %s written to file %s",
         x->hmultimap->getNamespace().c_str(),s->s_name);
  else
    post("h_multimap: couldn't write to file %s",s->s_name);
}

static void h_multimap_read(t_h_multimap *x, t_symbol *s)
{
  // make correct path
  char filnam[MAXPDSTRING];
  char filename[MAXPDSTRING];
  canvas_makefilename(x->x_canvas, s->s_name, filnam, MAXPDSTRING);
  sys_bashfilename(filnam, filename);

  if(!x->hmultimap->readFromFile(filename))
    post("h_multimap: couldn't read from file %s",s->s_name);
}

static void h_multimap_read_xml(t_h_multimap *x, t_symbol *s)
{
  // make correct path
  char filnam[MAXPDSTRING];
  char filename[MAXPDSTRING];
  canvas_makefilename(x->x_canvas, s->s_name, filnam, MAXPDSTRING);
  sys_bashfilename(filnam, filename);

  if(!x->hmultimap->readFromFileXML(filename))
    post("h_multimap: couldn't read from file %s",s->s_name);
}

static void *h_multimap_new(t_symbol *s, int argc, t_atom *argv) 
{

  t_h_multimap *x = (t_h_multimap *)pd_new(h_multimap_class);
  t_proxy *inlet = (t_proxy *)pd_new(proxy_class); // for the proxy inlet

  inlet->x = x;	// make x visible to the proxy inlets
  
  switch(argc)
    {
    default:
      post("h_multimap warning: only one argument for namespace is possible!");
    case 1:
      x->hmultimap = new HMultiMap(atom_getsymbol(argv)->s_name);
      break;
    case 0:
      x->hmultimap = new HMultiMap();
      break;
    }

  // we are going to create a proxy inlet no. 0
  // it belongs to the object x but the destination is t_proxy
  inlet->index = 0;
  inlet_new(&x->x_obj, &inlet->obj.ob_pd, 0,0);

  x->out0 = outlet_new(&x->x_obj, 0);
  x->out1 = outlet_new(&x->x_obj, &s_float);
  x->out2 = outlet_new(&x->x_obj, &s_float);
  x->x_canvas = canvas_getcurrent();
    
  return (void *)x;
}

static void *h_multimap_free(t_h_multimap *x)
{
  delete x->hmultimap;
  return (void *)x;
}

#if defined(PDCONTAINER_SINGLE_OBJECT)
// for PD-Extended
extern "C" {
#endif

void h_multimap_setup(void) 
{
  h_multimap_class = class_new(gensym("h_multimap"), (t_newmethod)h_multimap_new,
				(t_method)h_multimap_free, sizeof(t_h_multimap), 
				CLASS_DEFAULT, A_GIMME, 0);

  // a class for the proxy-inlet
  proxy_class = class_new(gensym("h_map_proxy"), NULL, NULL, sizeof(t_proxy),
			  CLASS_PD|CLASS_NOINLET, A_NULL);

  class_addmethod(h_multimap_class, (t_method)h_multimap_add, 
		  gensym("add"), A_GIMME , 0);
  class_addanything(proxy_class, (t_method)h_multimap_value); // the right inlet
  class_addmethod(h_multimap_class, (t_method)h_multimap_get, 
		  gensym("get"), A_GIMME , 0);
  class_addmethod(h_multimap_class, (t_method)h_multimap_remove, 
		  gensym("remove"), A_GIMME , 0);
  class_addmethod(h_multimap_class, (t_method)h_multimap_getsize, 
		  gensym("getsize"), A_DEFFLOAT , 0);
  class_addmethod(h_multimap_class, (t_method)h_multimap_set_namespace, 
		  gensym("namespace"), A_DEFSYMBOL , 0);
  class_addmethod(h_multimap_class, (t_method)h_multimap_get_namespace, 
		  gensym("getnamespace"), A_DEFFLOAT, 0);
  class_addmethod(h_multimap_class, (t_method)h_multimap_keys,
		  gensym("keys"), A_DEFFLOAT, 0);
  class_addmethod(h_multimap_class, (t_method)h_multimap_values,
		  gensym("values"), A_DEFFLOAT, 0);
  class_addmethod(h_multimap_class, (t_method)h_multimap_print,
		  gensym("print"), A_DEFFLOAT, 0);
  class_addmethod(h_multimap_class, (t_method)h_multimap_clear,  
		  gensym("clear"), A_DEFFLOAT, 0);
  class_addmethod(h_multimap_class, (t_method)h_multimap_clear_all,  
		  gensym("clearall"), A_DEFFLOAT, 0);
  class_addmethod(h_multimap_class, (t_method)h_multimap_save, 
		  gensym("save"), A_DEFSYMBOL , 0);
  class_addmethod(h_multimap_class, (t_method)h_multimap_save_xml, 
      gensym("saveXML"), A_DEFSYMBOL , 0);  
  class_addmethod(h_multimap_class, (t_method)h_multimap_read, 
		  gensym("read"), A_DEFSYMBOL , 0);
  class_addmethod(h_multimap_class, (t_method)h_multimap_read_xml, 
      gensym("readXML"), A_DEFSYMBOL , 0);

  // without an argument the following two methods wont work ??? why?? because of c++?
  class_addmethod(h_multimap_class, (t_method)h_multimap_help, 
		  gensym("help"),A_DEFFLOAT, 0);
}

#if defined(PDCONTAINER_SINGLE_OBJECT)
// for PD-Extended
}
#endif

