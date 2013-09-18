/*
 * Copyright (c) 1997-1999 Mark Danks.
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt" in this distribution.
 */

#include "m_pd.h"

/* -------------------------- vector- ------------------------------ */

/* instance structure */
static t_class *vectorMinus_class;

typedef struct _vectorMinus
{
    t_object	x_obj;
    t_float		x_minus;
	t_atom		*m_list;
	int			m_num;
	t_outlet    *t_out1;	    /* the outlet */
} t_vectorMinus;

static void doVectorMinus(t_vectorMinus *x, t_symbol *s, int argc, t_atom *argv)
{
	int i;
	if (argc > x->m_num)
	{
		x->m_list = (t_atom *)resizebytes(x->m_list, sizeof(t_atom) * x->m_num, sizeof(t_atom) * argc);
		x->m_num = argc;
	}
	for (i = 0; i < argc; i++)
	{
		float temp = atom_getfloat(&argv[i]);
		temp -= x->x_minus;
		SETFLOAT((&x->m_list[i]), temp);
	}
	outlet_list(x->t_out1, &s_list, argc, x->m_list);
}

static void *vectorMinus_new(t_floatarg n)
{
    t_vectorMinus *x = (t_vectorMinus *)pd_new(vectorMinus_class);
    x->x_minus = (float)n;
    floatinlet_new(&x->x_obj, &x->x_minus);
    x->t_out1 = outlet_new(&x->x_obj, 0);
	x->m_num = 3;
	x->m_list = (t_atom *)getbytes(sizeof(t_atom) * x->m_num);
    return (x);
}

static void vectorMinus_setup(void)
{
    vectorMinus_class = class_new(gensym("vector-"), (t_newmethod)vectorMinus_new, 0,
    	sizeof(t_vectorMinus), 0, A_DEFFLOAT, 0);
    class_addcreator((t_newmethod)vectorMinus_new, gensym("v-"), A_DEFFLOAT, 0);
    class_addmethod(vectorMinus_class, (t_method)doVectorMinus,
    	    &s_list, A_GIMME, A_NULL); 
}


void setup_vector0x2d(void){
  vectorMinus_setup();
}

void setup_v0x2d(void){
  vectorMinus_setup();
}
