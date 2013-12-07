#include "defines.h"

/*--------------- vcog ---------------*/

static t_class *vcog_class;

typedef struct _vcog
{
    t_object x_obj;
} t_vcog;


static void vcog_perform(t_vcog *x, t_symbol *s, int argc, t_atom *argv)
{
	t_float sum=0.;
	t_float wsum=0.0;
	int i;
	for (i = 0; i < argc; i++)
	{
		t_float tmp=atom_getfloat(&argv[i]);
		sum+= tmp;
		wsum+= tmp*i;
	}
	if (sum!=0.0f) outlet_float(x->x_obj.ob_outlet, 1.0+(wsum/sum));
}

static void *vcog_new( t_float halfDecayTime)
{
	t_vcog *x=(t_vcog *)pd_new(vcog_class);
	outlet_new(&x->x_obj, gensym("float"));
	return (void *)x;
}

void vcog_setup(void)
{
    vcog_class = class_new(gensym("vcog"),
    	(t_newmethod)vcog_new, 0,
		sizeof(t_vcog), 
		CLASS_DEFAULT,
	    0);
    class_addlist(vcog_class, (t_method)vcog_perform);
}

