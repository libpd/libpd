#include "defines.h"

/*--------------- vmax ---------------*/
/* maximum value in a list of float
   and its location (index)
*/

static t_class *vmax_class;

typedef struct _vmax
{
    t_object x_obj;
	t_outlet *m_out_maxi;
} t_vmax;


static void vmax_perform(t_vmax *x, t_symbol *s, int argc, t_atom *argv)
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

static void *vmax_new( t_float halfDecayTime)
{
	t_vmax *x=(t_vmax *)pd_new(vmax_class);
	outlet_new(&x->x_obj, gensym("float"));
	x->m_out_maxi=outlet_new(&x->x_obj, gensym("float"));
	return (void *)x;
}

void vmax_setup(void)
{
    vmax_class = class_new(gensym("vmax"),
    	(t_newmethod)vmax_new, 0,
		sizeof(t_vmax), 
		CLASS_DEFAULT,
	    0);
    class_addlist(vmax_class, (t_method)vmax_perform);
}

