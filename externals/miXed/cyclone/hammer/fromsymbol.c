/* Copyright (c) 2002-2003 krzYszcz and others.
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

#include <string.h>
#include "m_pd.h"

typedef struct _fromsymbol
{
    t_object   x_ob;
} t_fromsymbol;

static t_class *fromsymbol_class;

static void fromsymbol_bang(t_fromsymbol *x)
{
    outlet_bang(((t_object *)x)->ob_outlet);  /* CHECKED */
}

static void fromsymbol_float(t_fromsymbol *x, t_float f)
{
    /* CHECKED: fromsymbol: doesn't understand "int", "float" */
}

static void fromsymbol_symbol(t_fromsymbol *x, t_symbol *s)
{
    static char zero = 0;
    char *sname = &zero;
    if (s)
    {
	sname = s->s_name;
	while (*sname == ' ' || *sname == '\t'
	       || *sname == '\n' || *sname == '\r') sname++;
    }
    if (*sname)
    {
	t_binbuf *bb = binbuf_new();
	int ac;
	t_atom *av;
	binbuf_text(bb, sname, strlen(sname));
	ac = binbuf_getnatom(bb);
	av = binbuf_getvec(bb);
	if (ac)
	{
	    if (av->a_type == A_SYMBOL)
		outlet_anything(((t_object *)x)->ob_outlet,
				av->a_w.w_symbol, ac - 1, av + 1);
	    else if (av->a_type == A_FLOAT)
	    {
		if (ac > 1)
		    outlet_list(((t_object *)x)->ob_outlet, &s_list, ac, av);
		else
		    outlet_float(((t_object *)x)->ob_outlet, av->a_w.w_float);
	    }
	}
	binbuf_free(bb);
    }
}

static void fromsymbol_list(t_fromsymbol *x, t_symbol *s, int ac, t_atom *av)
{
    /* CHECKED: fromsymbol: doesn't understand "int", "float",
       'list <symbol>' ignored without complaining. */
}

static void fromsymbol_anything(t_fromsymbol *x, t_symbol *s, int ac, t_atom *av)
{
    fromsymbol_symbol(x, s);  /* CHECKED */
}

static void *fromsymbol_new(void)
{
    t_fromsymbol *x = (t_fromsymbol *)pd_new(fromsymbol_class);
    outlet_new((t_object *)x, &s_anything);
    return (x);
}

void fromsymbol_setup(void)
{
    fromsymbol_class = class_new(gensym("fromsymbol"),
				 (t_newmethod)fromsymbol_new, 0,
				 sizeof(t_fromsymbol), 0, 0);
    class_addbang(fromsymbol_class, fromsymbol_bang);
    class_addfloat(fromsymbol_class, fromsymbol_float);
    class_addsymbol(fromsymbol_class, fromsymbol_symbol);
    class_addlist(fromsymbol_class, fromsymbol_list);
    class_addanything(fromsymbol_class, fromsymbol_anything);
}
