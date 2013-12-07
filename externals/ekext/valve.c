/* takes a map like 0 1 3 4 7 and a route. if the route and the input are  */
/* non-zero in the map, then the route is output from the object */
#include "m_pd.h"
#include <math.h>
#include <string.h>
#define MAXENTRIES 512
#define LASTENTRY 511

static t_class *valve_class;

typedef struct _valve
{
  t_object x_obj;

  t_atom *map;
  int bufsize;

  t_float input, router, max;
  t_outlet *routed, *notrouted;

  int flush;
} t_valve;

static void valve_map(t_valve *x, t_symbol *s, int n, t_atom *map)
{
  if (x->map) {
    freebytes(x->map, x->bufsize * sizeof(t_atom));
    x->map = 0;
    x->bufsize = 0;
  }

  x->map = copybytes(map, n * sizeof(t_atom));
  x->bufsize = n;
}

void valve_float(t_valve *x, t_floatarg fin)
{
  if (x->map) {
    int arg, arga, argb;
    arga = argb = 0;
    float testa, testb;
    testa = testb = 0;
    x->input = fin;
    arg = (int)x->input;
    testa = fin < 0 ? 0 : atom_getfloatarg(arg, x->bufsize, x->map);
    testb = x->router < 0 ? 0 : atom_getfloatarg(x->router, x->bufsize, x->map);
    arga = (int)testa;
    argb = (int)testb;
    if(arga && argb) 
      {
	outlet_float(x->routed, x->router);
      }
    else if (!argb)
      {
	outlet_float(x->notrouted, argb);
      }
    else if (!arga && argb)
      {
	outlet_float(x->notrouted, arga);
      }
  }
}

void valve_set(t_valve *x, t_floatarg fmap, t_floatarg fval)
{
  if(fmap < x->bufsize && fmap >= 0)
    {
      int imap = (int)fmap;
      SETFLOAT(&x->map[imap], fval);
      x->max = fmap > x->max ? fmap : x->max;
    }
}

void valve_clear(t_valve *x)
{
  if (x->map) {
    freebytes(x->map, x->bufsize * sizeof(t_atom));
    x->map = 0;
    x->bufsize = 0;
  }
}

void valve_debug(t_valve *x)
{
  int i;
  for(i=0;i<x->bufsize;i++) {
    float element = atom_getfloatarg(i, x->bufsize, x->map);
    post("element %d = %d", i, element);
  }
  post("max = %d", x->max);
}  

void *valve_new(t_floatarg f) 
{
  t_valve *x = (t_valve *)pd_new(valve_class);
  x->max = 0;
  int i;
  x->map = 0;
  x->bufsize = 0;

  floatinlet_new(&x->x_obj, &x->router);

  x->routed = outlet_new(&x->x_obj, &s_float);
  x->notrouted = outlet_new(&x->x_obj, &s_float);
  return (void *)x;
}

void valve_setup(void) 
{
  valve_class = class_new(gensym("valve"),
  (t_newmethod)valve_new,
  0, sizeof(t_valve),
  0, A_DEFFLOAT, 0);
  post("|¬~¬~¬~¬~¬~¬valve~¬~¬~¬~¬~¬~¬|");
  post("|~>^^^integer map router^^^<¬|");
  post("|¬~¬~¬Edward Kelly 2007~¬~¬~¬|");

  class_addfloat(valve_class, valve_float);
  class_addmethod(valve_class, (t_method)valve_set, gensym("set"), A_DEFFLOAT, A_DEFFLOAT, 0);
  class_addmethod(valve_class, (t_method)valve_map, gensym("map"), A_GIMME, 0);
  class_addmethod(valve_class, (t_method)valve_clear, gensym("clear"), A_DEFFLOAT, 0);
  class_addmethod(valve_class, (t_method)valve_debug, gensym("debug"), A_DEFFLOAT, 0);
}
