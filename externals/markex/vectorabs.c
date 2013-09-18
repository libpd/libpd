/*
 * Copyright (c) 1997-1999 Mark Danks.
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt" in this distribution.
 */

#include "m_pd.h"

/* -------------------------- vectorabs ------------------------------ */

/* instance structure */
static t_class *vectorAbs_class;

typedef struct _vectorAbs
{
    t_object	x_obj;
	t_atom		*m_list;
	int			m_num;
	t_outlet    *t_out1;	    /* the outlet */
} t_vectorAbs;

static void doVectorAbs(t_vectorAbs *x, t_symbol *s, int argc, t_atom *argv)
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
      if (temp < 0.f)
	temp = temp * -1.f;
      SETFLOAT((&x->m_list[i]), temp);
    }
  outlet_list(x->t_out1, &s_list, argc, x->m_list);
}

static void *vectorAbs_new(void)
{
    t_vectorAbs *x = (t_vectorAbs *)pd_new(vectorAbs_class);
    x->t_out1 = outlet_new(&x->x_obj, 0);
	x->m_num = 3;
	x->m_list = (t_atom *)getbytes(sizeof(t_atom) * x->m_num);
    return (x);
}

void vectorabs_setup(void)
{
    vectorAbs_class = class_new(gensym("vectorabs"), (t_newmethod)vectorAbs_new, 0,
    	sizeof(t_vectorAbs), 0, A_NULL);
    class_addcreator((t_newmethod)vectorAbs_new, gensym("vabs"), A_NULL);
    class_addmethod(vectorAbs_class, (t_method)doVectorAbs,
		&s_list, A_GIMME, A_NULL); 
}

void vabs_setup(void){
  vectorabs_setup();
}
