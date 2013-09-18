#include <m_pd.h>
#ifdef _MSC_VER
#pragma warning( disable : 4244 )
#pragma warning( disable : 4305 )
#endif

/* this is taken from ggee, where the file was hanging around but the object was not
   funtional. i keep it here for reference since i dont wnat to fix it over and over ;)
   but its not included in the makefile to avoid namespace clash with ggee.prepend
   anyhow, i ll just rename it cxc_prepend
*/

/* ------------------------ prepend ----------------------------- */

static t_class *prepend_class;


typedef struct _prepend
{
     t_object x_obj;
     t_symbol* x_s;
} t_prepend;


void prepend_anything(t_prepend *x,t_symbol* s,t_int argc,t_atom* argv)
{
     int i = argc;
     t_symbol* cur;
     t_atom a_out[256];
     int    c_out = 0;
     t_atom* a = a_out;

#if 1
     //     post("sym: %s",s->s_name);
     SETSYMBOL(a,s);
     a++;
     c_out++;
#endif

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
	 post("prepend.c: unknown type");
       }
       argv++;
     }
     
     outlet_anything(x->x_obj.ob_outlet,x->x_s,c_out,(t_atom*)&a_out);
     //post("done");
}

void prepend_list(t_prepend *x,t_symbol* s,t_int argc,t_atom* argv)
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
	 post("prepend.c: unknown type");
       }
       argv++;
     }
     
     outlet_anything(x->x_obj.ob_outlet,x->x_s,c_out,(t_atom*)&a_out);
     //post("done");
}

static void *prepend_new(t_symbol* s)
{
    t_prepend *x = (t_prepend *)pd_new(prepend_class);
    outlet_new(&x->x_obj, &s_float);
    if (s != &s_)
	 x->x_s = s;
    else
	 x->x_s = gensym("cxc_prepend");
    return (x);
}

static void prepend_set(t_prepend *x, t_symbol *s)
{
  x->x_s = s;
}

void cxc_prepend_setup(void)
{
    prepend_class = class_new(gensym("cxc_prepend"), (t_newmethod)prepend_new, 0,
				sizeof(t_prepend), 0,A_DEFSYM,NULL);
    class_addlist(prepend_class, prepend_list);
    class_addanything(prepend_class,prepend_anything);
    class_addmethod(prepend_class, (t_method)prepend_set, gensym("set"), A_SYMBOL, 0);
}


