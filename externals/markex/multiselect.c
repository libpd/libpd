/*
 * Copyright (c) 1997-1999 Mark Danks.
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt" in this distribution.
 */

#include "m_pd.h"

/* -------------------------- multiselect ------------------------------ */

static t_class *multiselect_class;

typedef struct _multiselectElem
{
	t_float				val;
	t_outlet			*acceptOutlet;
	t_int				active;
} t_multiselectElem;

typedef struct _multiselect
{
    t_object			x_obj;
	t_multiselectElem	*x_elem;
	int					x_num;
    t_outlet			*x_rejectOutlet;
	t_inlet				*x_inlet;
} t_multiselect;

static void multiselect_float(t_multiselect *x, t_floatarg f)
{
	int i;
	int shotOut = 0;
	for (i = 0; i < x->x_num; i++)
	{
		if (x->x_elem[i].val == f && x->x_elem[i].active)
		{
			outlet_bang(x->x_elem[i].acceptOutlet);
			shotOut = 1;
		}
	}
	if (!shotOut)
		outlet_float(x->x_rejectOutlet, f);
}

static void multiselect_params(t_multiselect *x, t_symbol *s, int argc, t_atom *argv)
{
    int n;
	freebytes(x->x_elem, x->x_num * sizeof(t_multiselectElem));
   	for (n = 0; n < argc; n++)
   	{
		x->x_elem[n].val = atom_getfloatarg(n, argc, argv);
   		x->x_elem[n].active = 1;
	}
	for (; n < x->x_num; n++)
		x->x_elem[n].active = 0;		
}

static void multiselect_free(t_multiselect *x)
{
	int i;
	for (i = 0; i < x->x_num; i++)
	{
		outlet_free(x->x_elem[i].acceptOutlet);
	}
    freebytes(x->x_elem, x->x_num * sizeof(t_multiselectElem));
    inlet_free(x->x_inlet);
}

static void *multiselect_new(t_symbol *s, int argc, t_atom *argv)
{
   	int n;
   	t_multiselect *x = (t_multiselect *)pd_new(multiselect_class);
   	x->x_num = argc;
   	x->x_elem = (t_multiselectElem *)getbytes(argc * sizeof(t_multiselectElem));
   	for (n = 0; n < argc; n++)
   	{
		x->x_elem[n].val = atom_getfloatarg(n, argc, argv);
		x->x_elem[n].acceptOutlet = outlet_new(&x->x_obj, &s_bang);
		x->x_elem[n].active = 1;
   	}
   	x->x_rejectOutlet = outlet_new(&x->x_obj, &s_float);
	x->x_inlet = inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_list, gensym("params"));
   	return (x);
}

void multiselect_setup(void)
{
    multiselect_class = class_new(gensym("multiselect"), (t_newmethod)multiselect_new,
		(t_method)multiselect_free, sizeof(t_multiselect), 0, A_GIMME, 0);
	class_addcreator((t_newmethod)multiselect_new, gensym("multisel"),  A_GIMME, 0);

    class_addfloat(multiselect_class, multiselect_float);
    class_addmethod(multiselect_class, (t_method)multiselect_params,
    	    gensym("params"), A_GIMME, A_NULL); 

#if PD_MINOR_VERSION < 37 
	class_sethelpsymbol(multiselect_class, gensym("multiselect-help.pd"));
#endif
}
