#include "defines.h"

/*--------------- vnmax ---------------*/
/* maximum n values in a list of float
   and their locations (indices)
*/

//// UNCOMPLETE


static t_class *vnmax_class;

typedef struct _vnmax
{
    t_object x_obj;
	t_outlet *m_out_maxi;
} t_vnmax;


static void vnmax_perform(t_vnmax *x, t_symbol *s, int argc, t_atom *argv)
{
	int i;
	int maxi;
	t_float max=-MAXFLOAT;
	for (i = 0; i < argc; i++)
	{
		t_float f=atom_getfloat(&argv[i]);
		if (f>max)
		{ 
			max=f;
			maxi=i;
		}
	}
	outlet_float(x->x_obj.ob_outlet, max);
	outlet_float(x->m_out_maxi, (t_float)(maxi+1));
}

static void *vnmax_new( t_float halfDecayTime)
{
	t_vnmax *x=(t_vnmax *)pd_new(vnmax_class);
	outlet_new(&x->x_obj, gensym("list"));
	x->m_out_maxi=outlet_new(&x->x_obj, gensym("list"));
	return (void *)x;
}

void vnmax_setup(void)
{
    vnmax_class = class_new(gensym("vnmax"),
    	(t_newmethod)vnmax_new, 0,
		sizeof(t_vnmax), 
		CLASS_DEFAULT,
	    0);
    class_addlist(vnmax_class, (t_method)vnmax_perform);
}

