/* (C) Guenter Geiger <geiger@epy.co.at> */


#include <m_pd.h>
#include <math.h>
#include <string.h>
#ifdef _MSC_VER
#pragma warning( disable : 4244 )
#pragma warning( disable : 4305 )
#endif

/* ------------------------ constant ----------------------------- */
#ifndef M_PI
#define M_PI 3.141593f
#endif

static t_class *constant_class;


typedef struct _constant
{
     t_object x_obj;
     t_float  x_constant;
} t_constant;


void constant_bang(t_constant *x)
{
     outlet_float(x->x_obj.ob_outlet, x->x_constant);
}

static void *constant_new(t_symbol* s)
{
    t_constant *x = (t_constant *)pd_new(constant_class);
    
    if (s == &s_)
        x->x_constant = M_PI;
    else if (!strcmp(s->s_name,"pi"))
        x->x_constant = M_PI;
    else if (!strcmp(s->s_name,"2pi"))
        x->x_constant = 2*M_PI;
    else if (!strcmp(s->s_name,"e"))
        x->x_constant = exp(1.0);
    else if (!strcmp(s->s_name,"M_PI"))
        x->x_constant = M_PI;
    else if (!strcmp(s->s_name,"PI"))
        x->x_constant = M_PI;
    else if (!strcmp(s->s_name,"TWOPI"))
        x->x_constant = 2*M_PI;
    else
        pd_error(x, "Unsupported constant '%s'", s->s_name);

    outlet_new(&x->x_obj, &s_float);
    return (x);
}

void constant_setup(void)
{
    constant_class = class_new(gensym("constant"),
                               (t_newmethod)constant_new,
                               0,
                               sizeof(t_constant),
                               0,
                               A_DEFSYMBOL, 
                               0);
    class_addbang(constant_class,constant_bang);
}


