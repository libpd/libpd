/*
 * This object appends new directories to the canvas-local path. It is the
 * first small step towards a patch-specific namespace.
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

#define DEBUG(x)
//#define DEBUG(x) x 

/*------------------------------------------------------------------------------
 *  CLASS DEF
 */
t_class *path_class;

typedef struct _path
{
    t_object            x_obj;
    t_canvas            *x_canvas;
    t_namelist          *x_current;
    char                x_classpath_root[FILENAME_MAX];
    t_outlet            *x_data_outlet;
    t_outlet            *x_status_outlet;
} t_path;


static void path_append(t_path *x, t_symbol *s)
{
    t_canvasenvironment *e = canvas_getenv(x->x_canvas);
    e->ce_path = namelist_append(e->ce_path, s->s_name, 0);
}

static void load_arguments(t_path *x, int argc, t_atom *argv)
{
    t_symbol *path_name;
    while (argc--) {
        switch (argv->a_type) {
        case A_FLOAT:
            logpost(x, 1, "[path]: floats not supported: '%f'",
                     atom_getfloat(argv));
            break;
        case A_SYMBOL:
            path_name = atom_getsymbol(argv);
            path_append(x, path_name);
            logpost(x, 3, "[path] appended path: '%s'", path_name->s_name);
            break;
        default:
            logpost(x, 1, "[path]: Unsupported atom type");
        }
        argv++;
    }
}


static void path_output(t_path* x)
{
    DEBUG(post("path_output"););
    char buffer[MAXPDSTRING];

/* TODO: think about using x->x_current->nl_next so that if [path] is at
 * the end of its list, and another element gets added to the local
 * namespace, [path] will output the new element on the next bang. */
    if(x->x_current)
    {
        strncpy(buffer, x->x_current->nl_string, MAXPDSTRING);
        outlet_symbol( x->x_data_outlet, gensym(buffer));
        x->x_current = x->x_current->nl_next;
    }
    else 
        outlet_bang(x->x_status_outlet);
}


static void path_rewind(t_path* x) 
{
    t_canvasenvironment *e = canvas_getenv(x->x_canvas);
    x->x_current = e->ce_path;
}


static void *path_new(t_symbol *s, int argc, t_atom *argv)
{
    t_path *x = (t_path *)pd_new(path_class);
    t_symbol *currentdir;

    x->x_data_outlet = outlet_new(&x->x_obj, &s_symbol);
    x->x_status_outlet = outlet_new(&x->x_obj, 0);

    x->x_canvas = canvas_getcurrent();
    load_arguments(x,argc,argv);
    path_rewind(x);
     
    return (x);
}


static void path_free(t_path *x)
{
/* TODO: perhaps this should remove any libs that this instance had added to
 * the namespace */
}


void path_setup(void)
{
    path_class = class_new(gensym("path"), (t_newmethod)path_new,
                             (t_method)path_free,
                             sizeof(t_path), 
                             CLASS_DEFAULT, 
                             A_GIMME, 
                             0);
    /* add inlet atom methods */
    class_addbang(path_class,(t_method) path_output);
    
    /* add inlet selector methods */
    class_addmethod(path_class,(t_method) path_rewind,
                    gensym("rewind"), 0);
    class_addmethod(path_class,(t_method) path_append,
                    gensym("append"), A_DEFSYMBOL, 0);

    logpost(NULL, 3, "[path] %s", version);
    logpost(NULL, 4, "\t[path] is still in development, the interface could change!");
    logpost(NULL, 4, "\tcompiled against Pd version %d.%d.%d", PD_MAJOR_VERSION,
            PD_MINOR_VERSION, PD_BUGFIX_VERSION);
}
