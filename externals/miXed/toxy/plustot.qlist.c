/* Copyright (c) 2003-2005 krzYszcz and others.
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

#include <string.h>
#include "m_pd.h"
#include "unstable/fragile.h"
#include "common/loud.h"
#include "toxy/plusbob.h"
#include "plustot.h"

#ifdef KRZYSZCZ
#define PLUSTOT_QLIST_DEBUG
#endif

/* Need only an access to x_binbuf field. */
typedef struct _qlist
{
    t_object x_ob;
    t_outlet *x_bangout;
    void *x_binbuf;
} t_qlist;

typedef struct _plusproxy_qlist
{
    t_pd                    pp_pd;
    struct _plustot_qlist  *pp_master;
} t_plusproxy_qlist;

typedef struct _plustot_qlist
{
    t_plusobject        x_plusobject;
    t_glist            *x_glist;
    t_plustob          *x_tob;
    t_outlet           *x_rightout;
    t_plusproxy_qlist  *x_proxy;
} t_plustot_qlist;

static t_class *plusproxy_qlist_class;
static t_class *plustot_qlist_class;

static t_binbuf *plustot_qlist_usurp(t_plustot_qlist *x)
{
    static t_symbol *types[2];
    static int ntypes = 0;
    t_object *booty;
    if (ntypes == 0)
    {
	types[0] = gensym("qlist");
	types[1] = gensym("textfile");
	ntypes = 2;
    }
    if (booty = fragile_outlet_destination(
	((t_object *)x)->ob_outlet, ntypes, types,
	(t_pd *)x, "(connect left outlet to a qlist or textfile)"))
    {
	t_binbuf *bb = ((t_qlist *)booty)->x_binbuf;
#ifdef PLUSTOT_QLIST_DEBUG
	loudbug_post("booty '%s' at %x:",
		     class_getname(*(t_pd *)booty), (int)booty);
	loudbug_postbinbuf(bb);
#endif
	return (bb);
    }
    else return (0);
}

static t_plusproxy_qlist *plusproxy_qlist_new(t_plustot_qlist *master)
{
    t_plusproxy_qlist *pp = (t_plusproxy_qlist *)pd_new(plusproxy_qlist_class);
    pp->pp_master = master;
    return (pp);
}

static void plusproxy_qlist_symbol(t_plusproxy_qlist *pp, t_symbol *s)
{
    t_plustot_qlist *x = pp->pp_master;
    Tcl_Interp *interp = 0;
    if (plustag_isvalid(s, 0))
    {
	t_plustin *tin;
	Tcl_Obj *ob;
	if ((tin = plustag_tobtin(s, PLUSBOB_OWNER)) &&
	    (ob = plustob_getvalue((t_plustob *)s)))
	{
	    t_binbuf *bb;
	    if (bb = plustot_qlist_usurp(x))
	    {
		int nlists;
		Tcl_Obj **lists;
		interp = plustin_getinterp(tin);
		if (Tcl_ListObjGetElements(interp, ob,
					   &nlists, &lists) == TCL_OK)
		{
		    int lc;
		    Tcl_Obj **lp;
		    binbuf_clear(bb);
		    for (lc = 0, lp = lists; lc < nlists; lc++, lp++)
		    {
			int natoms;
			Tcl_Obj **atoms;
			if (Tcl_ListObjGetElements(interp, *lp,
						   &natoms, &atoms) == TCL_OK)
			{
			    int ac;
			    Tcl_Obj **ap;
			    for (ac = 0, ap = atoms; ac < natoms; ac++, ap++)
			    {
				double d;
				int len;
				char *ptr;
				Tcl_IncrRefCount(*ap);
				if (Tcl_GetDoubleFromObj(interp,
							 *ap, &d) == TCL_OK)
				{
				    t_atom at;
				    SETFLOAT(&at, (float)d);
				    binbuf_add(bb, 1, &at);
				}
				else if ((ptr = Tcl_GetStringFromObj(*ap, &len))
					 && len)
				{
				    t_atom at;
				    if (ptr[len - 1])
				    {
					char buf[MAXPDSTRING];
					if (len > MAXPDSTRING - 1)
					    len = MAXPDSTRING - 1;
					strncpy(buf, ptr, len);
					buf[len] = 0;
					ptr = buf;
				    }
				    SETSYMBOL(&at, gensym(ptr));
				    binbuf_add(bb, 1, &at);
				}
				/* FIXME else */
				Tcl_DecrRefCount(*ap);
			    }
			    binbuf_addsemi(bb);
			}
			else
			{
			    binbuf_clear(bb);
			    goto notalist;
			}
		    }
		}
		else goto notalist;
	    }
	}
    }
    return;
notalist:
    if (interp) plusloud_tclerror((t_pd *)x, interp, "not a list");
}

static void plustot_qlist_bang(t_plustot_qlist *x)
{
    t_binbuf *bb;
    if (bb = plustot_qlist_usurp(x))
    {
	if (plustob_setbinbuf(x->x_tob, bb))
	    outlet_plusbob(x->x_rightout, (t_plusbob *)x->x_tob);
    }
}

static void plustot_qlist_free(t_plustot_qlist *x)
{
    plusbob_release((t_plusbob *)x->x_tob);
    if (x->x_proxy) pd_free((t_pd *)x->x_proxy);
    plusobject_free(&x->x_plusobject);
}

void *plustot_qlist_new(t_symbol *s, int ac, t_atom *av)
{
    t_plustot_qlist *x = 0;
    t_glist *glist = canvas_getcurrent();
    t_plustin *tin = 0;
    t_plustob *tob = 0;
    if ((tin = plustin_glistprovide(glist, PLUSTIN_GLIST_ANY, 0)) &&
	(tob = plustob_new(tin, 0)))
    {
	x = (t_plustot_qlist *)
	    plusobject_new(plustot_qlist_class, s, ac, av, 0);
	plusbob_preserve((t_plusbob *)tob);
	plusbob_setowner((t_plusbob *)tob, (t_pd *)x);
	plustob_setlist(tob, ac, av);
	x->x_glist = glist;
	x->x_tob = tob;
	x->x_proxy = plusproxy_qlist_new(x);
	plusinlet_new(&x->x_plusobject, (t_pd *)x->x_proxy, 0, 0);
	plusoutlet_new(&x->x_plusobject, &s_anything);
	x->x_rightout = outlet_new((t_object *)x, &s_symbol);
    }
    else
    {
	loud_error(0, "+qlist: cannot initialize");
	if (tin)
	{
	    plusbob_preserve((t_plusbob *)tin);
	    plusbob_release((t_plusbob *)tin);
	}
    }
    return (x);
}

void plustot_qlist_setup(void)
{
    plustot_qlist_class = class_new(gensym("+qlist"), 0,
				    (t_method)plustot_qlist_free,
				    sizeof(t_plustot_qlist), 0, 0);
    plusclass_inherit(plustot_qlist_class, gensym("+qlist"));
    class_addbang(plustot_qlist_class, plustot_qlist_bang);

    plusproxy_qlist_class = class_new(gensym("+qlist proxy"), 0, 0,
				      sizeof(t_plusproxy_qlist), CLASS_PD, 0);
    class_addsymbol(plusproxy_qlist_class, plusproxy_qlist_symbol);
}
