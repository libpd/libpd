#include "defines.h"

/*--------------- vmin ---------------*/
/* maximum value in a list of float
   and its location (index)
*/

static t_class *vmin_class;

typedef struct _vmin
{
    t_object x_obj;
	t_outlet *m_out_maxi;
} t_vmin;


static void vmin_perform(t_vmin *x, t_symbol *s, int argc, t_atom *argv)
{
	int i;
	int mini;
	t_float min=MAXFLOAT;
	for (i = 0; i < argc; i++)
	{
		t_float f=atom_getfloat(&argv[i]);
		if (f<min)
		{ 
			min=f;
			mini=i;
		}
	}
	outlet_float(x->x_obj.ob_outlet, min);
	outlet_float(x->m_out_maxi, (t_float)(mini+1));
}

static void *vmin_new( t_float halfDecayTime)
{
	t_vmin *x=(t_vmin *)pd_new(vmin_class);
	outlet_new(&x->x_obj, gensym("float"));
	x->m_out_maxi=outlet_new(&x->x_obj, gensym("float"));
	return (void *)x;
}

void vmin_setup(void)
{
    vmin_class = class_new(gensym("vmin"),
    	(t_newmethod)vmin_new, 0,
		sizeof(t_vmin), 
		CLASS_DEFAULT,
	    0);
    class_addlist(vmin_class, (t_method)vmin_perform);
}

