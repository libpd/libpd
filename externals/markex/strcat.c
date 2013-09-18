/*
 * Copyright (c) 1997-1999 Mark Danks.
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt" in this distribution.
 */

#include "m_pd.h"
#include <stdio.h>

/* -------------------------- strcat ------------------------------ */

/* instance structure */

static t_class *strcat_class;

typedef struct _strcat
{
	t_object    x_obj;	        /* obligatory object header */
	t_symbol    *a_text;	    /* the first part of the text */
	t_symbol    *a_symbol;	    /* the last symbol sent out */
	t_outlet    *t_out1;	    /* the outlet */
} t_strcat;

void strcat_sym(t_strcat *x, t_symbol *sym)
{
    char temp[64];
    sprintf(temp, "%s%s", x->a_text->s_name, sym->s_name);
    x->a_symbol = gensym(temp);
    outlet_symbol(x->t_out1, x->a_symbol);
}

void strcat_float(t_strcat *x, t_floatarg n)
{
    char temp[64];
    sprintf(temp, "%s%f", x->a_text->s_name, n);
    x->a_symbol = gensym(temp);
    outlet_symbol(x->t_out1, x->a_symbol);
}

void strcat_bang(t_strcat *x)
{
    outlet_symbol(x->t_out1, x->a_symbol);
}

void *strcat_new(t_symbol *textname) /* init vals in struc */
{
    t_strcat *x = (t_strcat *)pd_new(strcat_class);
    x->t_out1 = outlet_new(&x->x_obj, 0);
    x->a_text = textname;
    x->a_symbol = textname;
    return (x);
}

void strcat_setup(void)
{
    strcat_class = class_new(gensym("strcat"), (t_newmethod)strcat_new, 0,
    	    	    	sizeof(t_strcat), 0, A_SYMBOL, 0);
    class_addsymbol(strcat_class, (t_method)strcat_sym);
    class_addfloat(strcat_class, (t_method)strcat_float);
    class_addbang(strcat_class, (t_method)strcat_bang);

#if PD_MINOR_VERSION < 37 
	class_sethelpsymbol(strcat_class, gensym("strcat-help.pd"));
#endif
}

