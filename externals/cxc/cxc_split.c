#include <m_pd.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#ifdef _MSC_VER
#pragma warning( disable : 4244 )
#pragma warning( disable : 4305 )
#endif

/* this is taken from ggee, where the file was hanging around but the object was not
   funtional. i keep it here for reference since i dont wnat to fix it over and over ;)
   but its not included in the makefile to avoid namespace clash with ggee.prepend
   anyhow, i ll just rename it cxc.prepend
*/

/* ------------------------ split ----------------------------- */

// why have to do this?
void split_anything();

static t_class *split_class;


typedef struct _split
{
     t_object x_obj;
     t_symbol* x_splitter;
} t_split;


void split_symbol(t_split *x, t_symbol *s)
{
  t_atom* a;
  split_anything(x, s, 0, a);
}


// move these to generic location later on
int split_string_isnum(char* s)
{
  // int isnum = 1;
  char tc;
  while((tc = *s++) != '\0') {
    // tc= s;
    switch(tc) {
    case 46: case 48: case 49: case 50: case 51: case 52: case 53: case 54: case 55: case 56: case 57:
      // post("yo numba: %c", tc);
      break;
    default:
      return 0;
      break;
    }
  }
  return 1;
}

void split_anything(t_split *x,t_symbol* s,t_int argc,t_atom* argv)
{
  int i = argc; int j;
  t_symbol* cur;
  t_atom a_out[256];
  int    c_out = 0;
  t_atom* a = a_out;
  // char *str;
  char u[MAXPDSTRING];
  char v[MAXPDSTRING];
  // char *v;
  u[0] = '\0';
  v[0] = '\0';
  int isnum = 1;
  // float tf;
  
  for(j=0; j<strlen(s->s_name); j++) {
    u[0] = s->s_name[j];
    if(u[0] == x->x_splitter->s_name[0]) {
      if(v[0] != '\0') { // delimiter is first character
	// check if string is digits only
	if(split_string_isnum(v)) {
	  SETFLOAT(a, (float)atof(v));
	}
	else {
	  SETSYMBOL(a, gensym(v));
	}
	a++; c_out++;
	// reset stuff
	v[0] = '\0';
	isnum = 1;
      } // v[0] != '\0'
    } else {
      strncat(v, u, 1);
    } // char matches splitter
  }
  
  // have to do this again here, damn.
  if(split_string_isnum(v)) {
    SETFLOAT(a, (float)atof(v));
  }
  else {
    SETSYMBOL(a, gensym(v));
  }
  a++, c_out++;
  
  outlet_list(x->x_obj.ob_outlet, &s_list, c_out, (t_atom*)&a_out);
  // outlet_anything(x->x_obj.ob_outlet,gensym("list"),c_out,(t_atom*)&a_out);
}

void split_list(t_split *x,t_symbol* s,t_int argc,t_atom* argv)
{
     int i = argc;
     t_symbol* cur;
     t_atom a_out[256];
     int    c_out = 0;
     t_atom* a = a_out;

     while (i--) {
       switch( argv->a_type) {
       case A_FLOAT:
	 //	 post("flo: %f",atom_getfloat(argv));
	 SETFLOAT(a,atom_getfloat(argv));
	 a++;
	 c_out++;
	 break;
       case A_SYMBOL:
	 //	 post("sym: %s",atom_getsymbol(argv)->s_name);
	 SETSYMBOL(a,atom_getsymbol(argv));
	 a++;
	 c_out++;
	 break;
       default:
	 post("split.c: unknown type");
       }
       argv++;
     }
     
     outlet_anything(x->x_obj.ob_outlet,x->x_splitter,c_out,(t_atom*)&a_out);
     //post("done");
}

static void *split_new(t_symbol* s)
{
    t_split *x = (t_split *)pd_new(split_class);
    outlet_new(&x->x_obj, &s_float);
    if (s != &s_)
	 x->x_splitter = s;
    else
	 x->x_splitter = gensym("cxc_split");
    return (x);
}

static void split_set(t_split *x, t_symbol *s)
{
  t_symbol *t;
  // init temp splitter
  char u[1]; u[0] = '\0';

  if(strlen(s->s_name) > 1) {
    // t = gensym((char*)s->s_name[0]);
    // post("%d", s->s_name[0]);
    strncat(u, s->s_name, 1);
    t = gensym(u);
  } else 
    t = s;
  x->x_splitter = t;
}

void cxc_split_setup(void)
{
    split_class = class_new(gensym("cxc_split"), (t_newmethod)split_new, 0,
				sizeof(t_split), 0,A_DEFSYM,NULL);
    // class_addlist(split_class, split_list);
    class_addanything(split_class,split_anything);
    class_addmethod(split_class, (t_method)split_set, gensym("set"), A_SYMBOL, 0);
    class_addsymbol(split_class, split_symbol);
}
