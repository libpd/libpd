/* Copyright (c) 2003-2005 krzYszcz and others.
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

#include <stdio.h>
#include <string.h>
#ifdef UNIX
#include <unistd.h>
#endif
#ifdef NT
#include <io.h>
#endif
#include "m_pd.h"
#include "g_canvas.h"
#include "common/loud.h"
#include "hammer/file.h"
#include "toxy/plusbob.h"
#include "plustot.h"

typedef struct _plustot_env
{
    t_plusobject   x_plusobject;
    t_plustin     *x_tin;
    t_glist       *x_glist;
    t_hammerfile  *x_filehandle;
} t_plustot_env;

static t_class *plustot_env_class;

static void plustot_env_takeover(t_glist *glist, t_plusbob *defparent,
				 t_plusbob *newparent)
{
    t_gobj *g;
    for (g = glist->gl_list; g; g = g->g_next)
    {
	if (pd_class(&g->g_pd) == canvas_class)
	{
	    if (!plustin_glistfind((t_glist *)g, PLUSTIN_GLIST_THIS))
		plustot_env_takeover((t_glist *)g, defparent, newparent);
	}
	else plusbob_detachownedchildren(defparent, newparent, (t_pd *)g);
    }
}

static void plustot_env_evalfile(t_plustot_env *x, t_symbol *fname)
{
    char buf1[MAXPDSTRING], buf2[MAXPDSTRING], *nameptr, *dir;
    int fd;
    dir = canvas_getdir(x->x_glist)->s_name;
    if ((fd = open_via_path(dir, fname->s_name, "",
			    buf1, &nameptr, MAXPDSTRING, 0)) < 0)
    {
	loud_error((t_pd *)x, "file '%s' not found", fname->s_name);
    }
    else
    {
	Tcl_Interp *interp = plustin_getinterp(x->x_tin);
	FILE *fp;
    	close(fd);
	strcpy(buf2, buf1);
	strcat(buf2, "/");
	strcat(buf2, nameptr);
	sys_bashfilename(buf2, buf2);
	Tcl_Preserve(interp);
	if (Tcl_EvalFile(interp, buf2) != TCL_OK)
	{
	    strcpy(buf1, "evaluation failed (");
	    strncat(buf1, buf2, MAXPDSTRING - strlen(buf1) - 2);
	    strcat(buf1, ")");
	    plusloud_tclerror((t_pd *)x, interp, buf1);
	}
	Tcl_Release(interp);
    }
}

static void plustot_env_evalfilehook(t_pd *z, t_symbol *fn, int ac, t_atom *av)
{
    plustot_env_evalfile((t_plustot_env *)z, fn);
}

static void plustot_env_bang(t_plustot_env *x)
{
    outlet_plusbob(((t_object *)x)->ob_outlet, (t_plusbob *)x->x_tin);
}

static void plustot_env_source(t_plustot_env *x, t_symbol *s)
{
    if (s && s != &s_)
	plustot_env_evalfile(x, s);
    else
	hammerpanel_open(x->x_filehandle, 0);
}

static void plustot_env_free(t_plustot_env *x)
{
    t_plustin *tin = plustin_glistprovide(x->x_glist, PLUSTIN_GLIST_UP, 0);
    plusbob_detachchildren((t_plusbob *)x->x_tin, (t_plusbob *)tin);
    plusbob_release((t_plusbob *)x->x_tin);
    hammerfile_free(x->x_filehandle);
    plusobject_free(&x->x_plusobject);
}

void *plustot_env_new(t_symbol *s, int ac, t_atom *av)
{
    t_plustot_env *x = 0;
    t_glist *gl = canvas_getcurrent();
    t_plustin *oldtin = plustin_glistfind(gl, PLUSTIN_GLIST_THIS);
    t_plustin *deftin = (oldtin ? 0 : plustin_glistfind(gl, PLUSTIN_GLIST_ANY));
    t_plustin *tin = 0;
    if ((tin = oldtin)
	|| (tin = plustin_glistprovide(gl, PLUSTIN_GLIST_THIS, 1)))
    {
	int warned = 0;
	x = (t_plustot_env *)plusobject_new(plustot_env_class, s, ac, av, 0);
	x->x_tin = tin;
	plusbob_preserve((t_plusbob *)tin);
	x->x_glist = gl;
	plusoutlet_new(&x->x_plusobject, &s_symbol);
	if (deftin)
	    /* true if both oldtin == 0 (we are first in this glist)
	       and plustin_default != 0 (bobs exist already) */
	    plustot_env_takeover(x->x_glist,
				 (t_plusbob *)deftin, (t_plusbob *)tin);
	x->x_filehandle = hammerfile_new((t_pd *)x, 0,
					 plustot_env_evalfilehook, 0, 0);
	while (ac--)
	{
	    if (av->a_type == A_SYMBOL)
		plustot_env_evalfile(x, av->a_w.w_symbol);
	    else if (!warned)
	    {
		loud_warning((t_pd *)x, 0, "bad atom");
		warned = 1;
	    }
	    av++;
	}
    }
    else loud_error(0, "+env: cannot initialize");
    return (x);
}

void plustot_env_setup(void)
{
    plustot_env_class = class_new(gensym("+env"), 0,
				  (t_method)plustot_env_free,
				  sizeof(t_plustot_env), 0, 0);
    plusclass_inherit(plustot_env_class, gensym("+env"));
    class_addbang(plustot_env_class, plustot_env_bang);
    class_addmethod(plustot_env_class, (t_method)plustot_env_source,
		    gensym("source"), A_DEFSYM, 0);
    hammerfile_setup(plustot_env_class, 0);
}
