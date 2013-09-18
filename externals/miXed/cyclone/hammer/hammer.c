/* Copyright (c) 2002-2005 krzYszcz and others.
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

#include "m_pd.h"
#include "unstable/fragile.h"
#include "common/loud.h"
#include "common/port.h"
#include "hammer/file.h"
#include "../build_counter"
void allhammers_setup(void);

typedef struct _hammer
{
    t_object       x_ob;
    t_hammerfile  *x_filehandle;
} t_hammer;

static t_class *hammer_class;
static int hammer_firstndx;
static int hammer_lastndx;

static void hammer_readhook(t_pd *z, t_symbol *fn, int ac, t_atom *av)
{
    int result = import_max(fn->s_name, "");
    outlet_float(((t_object *)z)->ob_outlet, (t_float)result);
}

static void hammer_doimport(t_hammer *x, t_symbol *fn)
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

static void hammer_click(t_hammer *x, t_floatarg xpos, t_floatarg ypos,
			 t_floatarg shift, t_floatarg ctrl, t_floatarg alt)
{
    hammer_doimport(x, 0);
}

static void hammer_import(t_hammer *x, t_symbol *fn)
{
    hammer_doimport(x, fn);
}

static void hammer_cd(t_hammer *x, t_symbol *dir)
{
    hammerpanel_setopendir(x->x_filehandle, dir);
}

static void hammer_pwd(t_hammer *x, t_symbol *s)
{
    t_symbol *dir;
    if (s && s->s_thing && (dir = hammerpanel_getopendir(x->x_filehandle)))
	pd_symbol(s->s_thing, dir);
}

static void hammer_bang(t_hammer *x)
{
    fragile_class_printnames("hammer classes are: ",
			     hammer_firstndx, hammer_lastndx);
}

static void hammer_free(t_hammer *x)
{
    hammerfile_free(x->x_filehandle);
}

static void *hammer_new(void)
{
    t_hammer *x = (t_hammer *)pd_new(hammer_class);
    x->x_filehandle = hammerfile_new((t_pd *)x, 0, hammer_readhook, 0, 0);
    outlet_new((t_object *)x, &s_float);
    return (x);
}

void hammer_setup(void)
{
    if (canvas_getcurrent())
    {
	/* Loading the library by object creation is banned, because of a danger
	   of having some of the classes already loaded. LATER rethink. */
	loud_error(0, "apparently an attempt to create a 'hammer' object");
	loud_errand(0, "without having hammer library preloaded");
	return;
    }
    if (zgetfn(&pd_objectmaker, gensym("hammer")))
    {
	loud_error(0, "hammer is already loaded");
	return;
    }
    if (!zgetfn(&pd_objectmaker, gensym("cyclone")))
	post("this is hammer %s, %s %s build",
	     CYCLONE_VERSION, loud_ordinal(CYCLONE_BUILD), CYCLONE_RELEASE);
    hammer_class = class_new(gensym("hammer"),
			     (t_newmethod)hammer_new,
			     (t_method)hammer_free,
			     sizeof(t_hammer), 0, 0);
    class_addbang(hammer_class, hammer_bang);
    class_addmethod(hammer_class, (t_method)hammer_cd,
		    gensym("cd"), A_DEFSYM, 0);
    class_addmethod(hammer_class, (t_method)hammer_pwd,
		    gensym("pwd"), A_SYMBOL, 0);
    class_addmethod(hammer_class, (t_method)hammer_import,
		    gensym("import"), A_DEFSYM, 0);
    class_addmethod(hammer_class, (t_method)hammer_click,
		    gensym("click"),
		    A_FLOAT, A_FLOAT, A_FLOAT, A_FLOAT, A_FLOAT, 0);
    hammerfile_setup(hammer_class, 0);
    hammer_firstndx = fragile_class_count();
    allhammers_setup();
    hammer_lastndx = fragile_class_count() - 1;
}
