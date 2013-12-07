/*
 * This object loads libraries and libdirs from within a patch. It is the
 * first small step towards a patch-specific namespace.  Currently, it just
 * adds things to the global path.  It is a reimplementation of a similar/same
 * idea from Guenter Geiger's [using] object.   <hans@at.or.at>
 *
 * This object currently depends on the packages/patches/libdir-0.38-4.patch
 * for sys_load_lib_dir().
 */

#include "m_pd.h"
#include "s_stuff.h"
#include "g_canvas.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* WARNING: KLUDGE!  */
/*
 * this struct is not publically defined (its in g_canvas.c) so I need to
 * include this here.  Its from Pd 0.41-test03 2006-11-19. */
struct _canvasenvironment
{
    t_symbol *ce_dir;      /* directory patch lives in */
    int ce_argc;           /* number of "$" arguments */
    t_atom *ce_argv;       /* array of "$" arguments */
    int ce_dollarzero;     /* value of "$0" */
    t_namelist *ce_path;   /* search path */
};

static char *version = "$Revision: 1.2 $";

t_int import_instance_count;

t_namelist *loaded_libs = NULL;

#define DEBUG(x)
//#define DEBUG(x) x 

/*------------------------------------------------------------------------------
 *  CLASS DEF
 */
t_class *import_class;

typedef struct _import
{
    t_object            x_obj;
    t_canvas            *x_canvas;
    t_namelist          *x_current;
    char                x_classpath_root[FILENAME_MAX];
    t_outlet            *x_data_outlet;
    t_outlet            *x_status_outlet;
} t_import;


static int load_library(t_import *x, char *library_name)
{
    DEBUG(post("load_library"););
    if (!sys_load_lib(x->x_canvas, library_name)) 
        return 0;
    else
        return 1;
}


static void load_arguments(t_import *x, int argc, t_atom *argv)
{
    t_symbol *library_name;
    
    while (argc--) {
        switch (argv->a_type) {
        case A_FLOAT:
            logpost(x, 1, "[import]: floats not supported: '%f'",
                     atom_getfloat(argv));
            break;
        case A_SYMBOL:
            library_name = atom_getsymbol(argv);
            if (!load_library(x,library_name->s_name))
                logpost(x, 1, "[import]: can't load library in '%s'",
                         library_name->s_name);
            else
            {
                loaded_libs = namelist_append(loaded_libs, library_name->s_name, 0);
                logpost(x, 3, "[import] loaded library: '%s'", library_name->s_name);
            }
            break;
        default:
            logpost(x, 1, "[import]: Unsupported atom type");
        }
        argv++;
    }
}


static void import_output(t_import* x)
{
    DEBUG(post("import_output"););
    char buffer[MAXPDSTRING];

/* TODO: think about using x->x_current->nl_next so that if [import] is at
 * the end of its list, and another element gets added to the local
 * namespace, [import] will output the new element on the next bang. */
    if(x->x_current)
    {
        strncpy(buffer, x->x_current->nl_string, MAXPDSTRING);
        outlet_symbol( x->x_data_outlet, gensym(buffer));
        x->x_current = x->x_current->nl_next;
    }
    else 
        outlet_bang(x->x_status_outlet);
}


static void import_rewind(t_import* x) 
{
    x->x_current = loaded_libs;
}


static void *import_new(t_symbol *s, int argc, t_atom *argv)
{
    t_import *x = (t_import *)pd_new(import_class);
    t_symbol *currentdir;

    x->x_data_outlet = outlet_new(&x->x_obj, &s_symbol);
    x->x_status_outlet = outlet_new(&x->x_obj, 0);

    x->x_canvas = canvas_getcurrent();
    load_arguments(x,argc,argv);
    import_rewind(x);

    import_instance_count++;
     
    return (x);
}


static void import_free(t_import *x)
{
    import_instance_count--;
/* TODO perhaps loaded_libs should be an instance variable */
    if(import_instance_count == 0)
    {
        if(loaded_libs)
        {
            namelist_free(loaded_libs);
            loaded_libs = NULL;
        }
    }
/* TODO: perhaps this should remove any libs that this instance had added to
 * the namespace */
}


void import_setup(void)
{
    import_class = class_new(gensym("import"), (t_newmethod)import_new,
                             (t_method)import_free,
                             sizeof(t_import), 
                             CLASS_DEFAULT, 
                             A_GIMME, 
                             0);
    /* add inlet atom methods */
    class_addbang(import_class,(t_method) import_output);
    
    /* add inlet selector methods */
    class_addmethod(import_class,(t_method) import_rewind,
                    gensym("rewind"), 0);

    logpost(NULL, 3, "[import] %s", version);
    logpost(NULL, 4, "\t[import] is still in development, the interface could change!");
    logpost(NULL, 4, "\tcompiled against Pd version %d.%d.%d", PD_MAJOR_VERSION,
            PD_MINOR_VERSION, PD_BUGFIX_VERSION);
}
