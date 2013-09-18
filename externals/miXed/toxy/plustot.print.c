/* Copyright (c) 2003-2005 krzYszcz and others.
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

#include "m_pd.h"
#include "common/loud.h"
#include "toxy/plusbob.h"
#include "plustot.h"

typedef struct _plustot_print
{
    t_plusobject  x_plusobject;
    t_symbol     *x_label;
    t_binbuf     *x_bb;
} t_plustot_print;

static t_class *plustot_print_class;

static char *plustot_print_symbolname(t_symbol *s)
{
    return (s && s != &s_ ? s->s_name : "???");
}

static void plustot_print_symbol(t_plustot_print *x, t_symbol *s)
{
    Tcl_Obj *ob = plustag_tobvalue(s, (t_pd *)x);
    if (ob)
    {
	int len;
	char *ptr;
	Tcl_IncrRefCount(ob);
	ptr = Tcl_GetStringFromObj(ob, &len);
	if (ptr && len)
	{
	    int ac;
	    binbuf_text(x->x_bb, ptr, len);
	    if (ac = binbuf_getnatom(x->x_bb))
	    {
		t_plustin *tin = plustag_tobtin(s, (t_pd *)x);
		t_symbol *glname = (tin ? plustin_getglistname(tin) : 0);
		t_atom *av = binbuf_getvec(x->x_bb);
		if (av->a_type == A_SYMBOL || av->a_type == A_FLOAT)
		{
		    char *lstring = (x->x_label ? x->x_label->s_name :
				     plustot_print_symbolname(
					 plustag_typename(s, 1, (t_pd *)x)));
		    if (glname)
			startpost("%s (%s):", lstring, glname->s_name);
		    else
			startpost("%s:", lstring);
		}
		/* FIXME {1.0, 1.0}, etc. */
		if (av->a_type == A_SYMBOL)
		{
		    startpost(" %s", av->a_w.w_symbol->s_name);
		    postatom(ac - 1, av + 1);
		    endpost();
		}
		else if (av->a_type == A_FLOAT)
		{
		    if (ac > 1)
		    {
			postatom(ac, av);
			endpost();
		    }
		    else post(" %g", av->a_w.w_float);
		}
	    }
	    /* LATER consider printing empty list as 'bang' */
	}
	Tcl_DecrRefCount(ob);
    }
}

static void plustot_print_free(t_plustot_print *x)
{
    binbuf_free(x->x_bb);
    plusobject_free(&x->x_plusobject);
}

void *plustot_print_new(t_symbol *s, int ac, t_atom *av)
{
    t_plustot_print *x =
	(t_plustot_print *)plusobject_new(plustot_print_class, s, ac, av, 0);
    x->x_label = (ac && av->a_type == A_SYMBOL ? av->a_w.w_symbol : 0);
    x->x_bb = binbuf_new();
    return (x);
}

void plustot_print_setup(void)
{
    plustot_print_class = class_new(gensym("+print"), 0,
				    (t_method)plustot_print_free,
				    sizeof(t_plustot_print), 0, 0);
    plusclass_inherit(plustot_print_class, gensym("+print"));
    class_addsymbol(plustot_print_class, plustot_print_symbol);
}
