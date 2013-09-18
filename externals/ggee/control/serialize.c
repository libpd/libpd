/* (C) Guenter Geiger <geiger@epy.co.at> */


#include <m_pd.h>
#ifdef _MSC_VER
#pragma warning( disable : 4244 )
#pragma warning( disable : 4305 )
#endif

/* ------------------------ serialize ----------------------------- */

#define MAX_ELEMENTS 256

static t_class *serialize_class;


typedef struct _serialize
{
     t_object x_obj;
     t_atom   x_abuf[MAX_ELEMENTS];
     t_int    x_count;
     t_float    x_elements;
} t_serialize;


void serialize_float(t_serialize *x,t_floatarg f)
{
     SETFLOAT(&x->x_abuf[x->x_count],f);
     x->x_count++;

     if (x->x_count == x->x_elements) {
	  outlet_list(x->x_obj.ob_outlet,0,x->x_count,x->x_abuf);
	  x->x_count = 0;
     }
}


static void *serialize_new(t_floatarg f)
{
    t_serialize *x = (t_serialize *)pd_new(serialize_class);
    outlet_new(&x->x_obj,&s_float);
    x->x_elements = f;
    x->x_count=0;
    if ((f <= 0) || (f > MAX_ELEMENTS)) x->x_elements = 1;
    floatinlet_new(&x->x_obj, &x->x_elements);
    return (x);
}



void serialize_setup(void)
{
    serialize_class = class_new(gensym("serialize"), (t_newmethod)serialize_new, 0,
				sizeof(t_serialize),0, A_DEFFLOAT,0);
    class_addfloat(serialize_class,serialize_float);
}


