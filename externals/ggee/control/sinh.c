/* (C) Guenter Geiger <geiger@epy.co.at> */


#include <m_pd.h>
#ifdef _MSC_VER
#pragma warning( disable : 4244 )
#pragma warning( disable : 4305 )
#endif

#include <math.h>



/* ------------------------ sinh ----------------------------- */

static t_class *sinh_class;


typedef struct _sinh
{
     t_object x_obj;
} t_sinh;


void sinh_bang(t_sinh *x)
{
     post("bang");
}


void sinh_float(t_sinh *x,t_floatarg f)
{

    outlet_float(x->x_obj.ob_outlet,sinh(f));
}


static void *sinh_new()
{
    t_sinh *x = (t_sinh *)pd_new(sinh_class);

    outlet_new(&x->x_obj,&s_float);
    return (x);
}

void sinh_setup(void)
{
    sinh_class = class_new(gensym("sinh"), (t_newmethod)sinh_new, 0,
				sizeof(t_sinh), 0,0);
    class_addbang(sinh_class,sinh_bang);
    class_addfloat(sinh_class,sinh_float);
}


