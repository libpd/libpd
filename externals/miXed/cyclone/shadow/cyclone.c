/* Copyright (c) 2003-2005 krzYszcz and others.
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

/* Never use forked calls in shadow code... */

/* LATER support multi-atom dir ('cd' message), and fn ('import' message)
   (same in maxmode, hammer and sickle) */

#include "m_pd.h"
#include "common/loud.h"
#include "common/port.h"
#include "hammer/file.h"
#include "unstable/fragile.h"
#include "unstable/loader.h"
#include "shadow.h"
#include "../build_counter"

typedef struct _cyclone
{
    t_object       x_ob;
    t_hammerfile  *x_filehandle;
} t_cyclone;

static t_class *cyclone_class;
static int cyclone_hammerndx;
static int cyclone_sicklendx;
static int cyclone_nettlesndx;
static int cyclone_lastndx;

static void cyclone_readhook(t_pd *z, t_symbol *fn, int ac, t_atom *av)
{
    int result = import_max(fn->s_name, "");
    outlet_float(((t_object *)z)->ob_outlet, (t_float)result);
}

static void cyclone_doimport(t_cyclone *x, t_symbol *fn)
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

static void cyclone_click(t_cyclone *x, t_floatarg xpos, t_floatarg ypos,
			  t_floatarg shift, t_floatarg ctrl, t_floatarg alt)
{
    cyclone_doimport(x, 0);
}

static void cyclone_import(t_cyclone *x, t_symbol *fn)
{
    cyclone_doimport(x, fn);
}

static void cyclone_cd(t_cyclone *x, t_symbol *dir)
{
    hammerpanel_setopendir(x->x_filehandle, dir);
}

static void cyclone_pwd(t_cyclone *x, t_symbol *s)
{
    t_symbol *dir;
    if (s && s->s_thing && (dir = hammerpanel_getopendir(x->x_filehandle)))
	pd_symbol(s->s_thing, dir);
}

static void cyclone_bang(t_cyclone *x)
{
    if (cyclone_hammerndx && cyclone_sicklendx)
    {
	fragile_class_printnames("hammer classes are: ",
				 cyclone_hammerndx, cyclone_sicklendx - 1);
	fragile_class_printnames("sickle classes are: ",
				 cyclone_sicklendx, cyclone_nettlesndx - 1);
    }
    fragile_class_printnames("nettles are: ",
			     cyclone_nettlesndx, cyclone_lastndx - 1);
}

static void cyclone_free(t_cyclone *x)
{
    hammerfile_free(x->x_filehandle);
}

static void *cyclone_new(void)
{
    t_cyclone *x = (t_cyclone *)pd_new(cyclone_class);
    x->x_filehandle = hammerfile_new((t_pd *)x, 0, cyclone_readhook, 0, 0);
    outlet_new((t_object *)x, &s_float);
    return (x);
}

void cyclone_setup(void)
{
    int hresult, sresult;
    hresult = sresult = LOADER_OK;
    if (zgetfn(&pd_objectmaker, gensym("cyclone")))
    {
	loud_error(0, "cyclone is already loaded");
	return;
    }
    post("this is cyclone %s, %s %s build",
	 CYCLONE_VERSION, loud_ordinal(CYCLONE_BUILD), CYCLONE_RELEASE);
    cyclone_class = class_new(gensym("cyclone"),
			      (t_newmethod)cyclone_new,
			      (t_method)cyclone_free,
			      sizeof(t_cyclone), 0, 0);
    class_addbang(cyclone_class, cyclone_bang);
    class_addmethod(cyclone_class, (t_method)cyclone_cd,
		    gensym("cd"), A_DEFSYM, 0);
    class_addmethod(cyclone_class, (t_method)cyclone_pwd,
		    gensym("pwd"), A_SYMBOL, 0);
    class_addmethod(cyclone_class, (t_method)cyclone_import,
		    gensym("import"), A_DEFSYM, 0);
    class_addmethod(cyclone_class, (t_method)cyclone_click,
		    gensym("click"),
		    A_FLOAT, A_FLOAT, A_FLOAT, A_FLOAT, A_FLOAT, 0);
    hammerfile_setup(cyclone_class, 0);

    if (canvas_getcurrent())
    {
	/* Loading hammer and sickle by cyclone object creation is banned,
	   because of a danger of having some of the classes already loaded. */
	loud_warning(0, "cyclone",
 "creating cyclone object without loading cyclone components");
	cyclone_hammerndx = cyclone_sicklendx = 0;
    }
    else
    {
	cyclone_hammerndx = fragile_class_count();
	if (zgetfn(&pd_objectmaker, gensym("hammer")))
	    loud_warning(0, "cyclone", "hammer is already loaded");
	else
	    hresult = unstable_load_lib("", "hammer");

	cyclone_sicklendx = fragile_class_count();
	if (zgetfn(&pd_objectmaker, gensym("sickle")))
	    loud_warning(0, "cyclone", "sickle is already loaded");
	else
	    sresult = unstable_load_lib("", "sickle");
    }
    cyclone_nettlesndx = fragile_class_count();
    allnettles_setup();
    cyclone_lastndx = fragile_class_count() - 1;

    if (hresult == LOADER_NOFILE)
	loud_error(0, "hammer library is missing");
    else if (sresult == LOADER_NOFILE)
	loud_error(0, "sickle library is missing");
    else if (cyclone_hammerndx &&
	     (!zgetfn(&pd_objectmaker, gensym("hammer")) ||
	      !zgetfn(&pd_objectmaker, gensym("sickle"))))
    {
	loud_error(0, "version mismatch");
	loud_errand(0,
		    "use a more recent Pd release (or recompile the cyclone).");
    }
}
