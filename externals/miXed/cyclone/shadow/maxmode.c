/* Copyright (c) 2004-2005 krzYszcz and others.
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

#include <stdio.h>
#include <string.h>
#include "m_pd.h"
#include "common/loud.h"
#include "common/fitter.h"
#include "common/port.h"
#include "hammer/file.h"
#include "unstable/fragile.h"
#include "unstable/loader.h"
#include "../build_counter"

typedef struct _maxmode
{
    t_object       x_ob;
    t_hammerfile  *x_filehandle;
    t_outlet      *x_modeout;
} t_maxmode;

static t_class *maxmode_class;
static int maxmode_dummiesndx;
static int maxmode_lastndx;
static t_pd *maxmode_dproxy = 0;
static int maxmode_withbanner = 0;

static void maxmode_readhook(t_pd *z, t_symbol *fn, int ac, t_atom *av)
{
    int result = import_max(fn->s_name, "");
    outlet_float(((t_object *)z)->ob_outlet, (t_float)result);
}

static void maxmode_doimport(t_maxmode *x, t_symbol *fn)
{
    if (fn && fn != &s_)
    {
	t_symbol *dir = hammerpanel_getopendir(x->x_filehandle);
	int result =
	    import_max(fn->s_name, (dir && dir != &s_ ? dir->s_name : ""));
	outlet_float(((t_object *)x)->ob_outlet, (t_float)result);
    }
    else hammerpanel_open(x->x_filehandle, 0);

}

static void maxmode_click(t_maxmode *x, t_floatarg xpos, t_floatarg ypos,
			  t_floatarg shift, t_floatarg ctrl, t_floatarg alt)
{
    maxmode_doimport(x, 0);
}

static void maxmode_import(t_maxmode *x, t_symbol *fn)
{
    maxmode_doimport(x, fn);
}

static void maxmode_cd(t_maxmode *x, t_symbol *dir)
{
    hammerpanel_setopendir(x->x_filehandle, dir);
}

static void maxmode_pwd(t_maxmode *x, t_symbol *s)
{
    t_symbol *dir;
    if (s && s->s_thing && (dir = hammerpanel_getopendir(x->x_filehandle)))
	pd_symbol(s->s_thing, dir);
}

static void maxmode_set(t_maxmode *x, t_symbol *s)
{
    if (!s || s == &s_)
	s = gensym("none");
    if (s != fitter_getmode())
	fitter_setmode(s);
}

static void maxmode_get(t_maxmode *x)
{
    t_symbol *mode;
    if (mode = fitter_getmode())
	outlet_symbol(x->x_modeout, mode);
}

static void maxmode_bang(t_maxmode *x)
{
    if (maxmode_dproxy)
	pd_bang(maxmode_dproxy);
    else
	post("no replacement abstractions");
    if (maxmode_lastndx > maxmode_dummiesndx)
	fragile_class_printnames("dummies are: ",
				 maxmode_dummiesndx, maxmode_lastndx);
    else
	post("no dummies");
}

static void maxmode_free(t_maxmode *x)
{
    /* FIXME cancel registration */
    hammerfile_free(x->x_filehandle);
}

static void *maxmode_new(t_symbol *s, int ac, t_atom *av)
{
    t_maxmode *x = (t_maxmode *)pd_new(maxmode_class);
    int selective = ac;
    if (maxmode_withbanner && !ac)
    {
	post("this is maxmode %s, %s %s build",
	     CYCLONE_VERSION, loud_ordinal(CYCLONE_BUILD), CYCLONE_RELEASE);
	loud_warning(0, "maxmode",
 "creating maxmode object without loading cyclone components");
	maxmode_withbanner = 0;
    }
    if (selective)
    {
	/* a numeric argument is valid -- transparent object is created
	   (global mode is not set, nothing is registered) */
	while (ac--) if (av->a_type == A_SYMBOL)
	{
	    /* FIXME register into fitter for per-patch-file, selective
	       compatibility control */
	    av++;
	}
    }
    else if (!fittermax_get())
	fittermax_set();
    x->x_filehandle = hammerfile_new((t_pd *)x, 0, maxmode_readhook, 0, 0);
    outlet_new((t_object *)x, &s_float);
    x->x_modeout = outlet_new((t_object *)x, &s_symbol);
    return (x);
}

void maxmode_setup(void)
{
    int dresult = LOADER_OK;
    if (zgetfn(&pd_objectmaker, gensym("maxmode")))
    {
	loud_error(0, "maxmode is already loaded");
	return;
    }
    maxmode_class = class_new(gensym("maxmode"),
			      (t_newmethod)maxmode_new,
			      (t_method)maxmode_free,
			      sizeof(t_maxmode), 0, A_GIMME, 0);
    class_addbang(maxmode_class, maxmode_bang);
    class_addmethod(maxmode_class, (t_method)maxmode_set,
		    gensym("set"), A_DEFSYM, 0);
    class_addmethod(maxmode_class, (t_method)maxmode_get,
		    gensym("get"), 0);
    class_addmethod(maxmode_class, (t_method)maxmode_cd,
		    gensym("cd"), A_DEFSYM, 0);
    class_addmethod(maxmode_class, (t_method)maxmode_pwd,
		    gensym("pwd"), A_SYMBOL, 0);
    class_addmethod(maxmode_class, (t_method)maxmode_import,
		    gensym("import"), A_DEFSYM, 0);
    class_addmethod(maxmode_class, (t_method)maxmode_click,
		    gensym("click"),
		    A_FLOAT, A_FLOAT, A_FLOAT, A_FLOAT, A_FLOAT, 0);
    hammerfile_setup(maxmode_class, 0);

    if (canvas_getcurrent())
    {
	fitter_setup(0, 0);
	if (!zgetfn(&pd_objectmaker, gensym("cyclone")))
	    /* cycloneless maxmode -- banner is posted by the oldest
	       maxmode object with no creation arguments */
	    maxmode_withbanner = 1;
    }
    else
    {
	fittermax_set();
	if (zgetfn(&pd_objectmaker, gensym("cyclone")))
	    loud_warning(0, "maxmode", "cyclone is already loaded");
	else
	{
	    if (unstable_load_lib("", "cyclone") == LOADER_NOFILE)
		loud_error(0, "cyclone library is missing");
	    else if (!zgetfn(&pd_objectmaker, gensym("cyclone")))
		loud_error(0, "miXed/Pd version mismatch");
	}
    }
    maxmode_dummiesndx = fragile_class_count();
    if (zgetfn(&pd_objectmaker, gensym("dummies")))
	loud_warning(0, "maxmode", "dummies are already loaded");
    else
	dresult = unstable_load_lib("", "dummies");
    maxmode_lastndx = fragile_class_count() - 1;
    if (dresult == LOADER_NOFILE)
	loud_warning(0, "maxmode", "dummies not found");
    else
    {
	t_symbol *s = gensym("_cc.dummies");
	if (s->s_thing && !s->s_next
	    && !strcmp(class_getname(*s->s_thing), "_cc.dummies"))
	    maxmode_dproxy = s->s_thing;
	else
	    loudbug_bug("maxmode_setup");  /* FIXME */
    }
}
