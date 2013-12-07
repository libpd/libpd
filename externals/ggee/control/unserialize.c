/* (C) Guenter Geiger <geiger@epy.co.at> */


#include <m_pd.h>
#ifdef _MSC_VER
#pragma warning( disable : 4244 )
#pragma warning( disable : 4305 )
#endif

/* ------------------------ unserialize ----------------------------- */

#define MAX_ELEMENTS 256

static t_class *unserialize_class;


typedef struct _unserialize
{
     t_object x_obj;
     t_atom   x_abuf[MAX_ELEMENTS];
     t_int    x_count;
     t_int    x_elements;
} t_unserialize;


void unserialize_float(t_unserialize *x,t_floatarg f)
{
     SETFLOAT(&x->x_abuf[x->x_count],f);
     x->x_count++;

     if (x->x_count == x->x_elements) {
	  outlet_list(x->x_obj.ob_outlet,0,x->x_count,x->x_abuf);
	  x->x_count = 0;
     }
}


static void *unserialize_new(t_floatarg f)
{
    t_unserialize *x = (t_unserialize *)pd_new(unserialize_class);
    outlet_new(&x->x_obj,&s_float);
    x->x_elements = f;
    x->x_count=0;
    if ((f <= 0) || (f > MAX_ELEMENTS)) x->x_elements = 1;
    return (x);
}



void unserialize_setup(void)
{
    unserialize_class = class_new(gensym("unserialize"), (t_newmethod)unserialize_new, 
				  0,sizeof(t_unserialize),0, A_DEFFLOAT,0);
    class_addfloat(unserialize_class,unserialize_float);
}


