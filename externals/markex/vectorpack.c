/*
 * Copyright (c) 1997-1999 Mark Danks.
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt" in this distribution.
 */

#include "m_pd.h"


/* -------------------------- vectorpack ------------------------------ */

static t_class *vectorPack_class;

typedef struct _vectorPack
{
    t_object	x_obj;
    t_float		x_val;
	t_atom		*m_list;
	int			m_num;
	t_outlet    *t_out1;	    /* the outlet */
} t_vectorPack;

static void doVectorPack(t_vectorPack *x, t_symbol *s, int argc, t_atom *argv)
{
	int i;
	if ((argc + 1) > x->m_num)
	{
		x->m_list = (t_atom *)resizebytes(x->m_list, sizeof(t_atom) * x->m_num, sizeof(t_atom) * (argc + 1));
		x->m_num = argc;
	}
	for (i = 0; i < argc; i++)
	{
		float temp = atom_getfloat(&argv[i]);
		SETFLOAT((&x->m_list[i]), temp);
	}
	SETFLOAT((&x->m_list[argc]), x->x_val);
	outlet_list(x->t_out1, &s_list, (argc + 1), x->m_list);
}

static void *vectorPack_new(t_floatarg n)
{
    t_vectorPack *x = (t_vectorPack *)pd_new(vectorPack_class);
    x->x_val = (float)n;
    floatinlet_new(&x->x_obj, &x->x_val);
    x->t_out1 = outlet_new(&x->x_obj, 0);
	x->m_num = 4;
	x->m_list = (t_atom *)getbytes(sizeof(t_atom) * x->m_num);
    return (x);
}

void vectorpack_setup(void)
{
    vectorPack_class = class_new(gensym("vectorpack"), (t_newmethod)vectorPack_new, 0,
    	sizeof(t_vectorPack), 0, A_DEFFLOAT, 0);
    class_addcreator((t_newmethod)vectorPack_new, gensym("vpack"), A_DEFFLOAT, 0);
    class_addmethod(vectorPack_class, (t_method)doVectorPack,
    	    &s_list, A_GIMME, A_NULL); 
}

void vpack_setup(void){
  vectorpack_setup();
}
