/* which: an external for pure data */
/* Will output the path to the object named as its first argument */
/* Martin Peach 20090225 */
#include "m_pd.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h> /* for strncpy strncat */
#ifdef _WIN32
#include <io.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <share.h>
#include <errno.h>
#define close _close
#else
#include <unistd.h>
#endif

static t_class *which_class;

typedef struct _which
{
    t_object    x_obj;
    t_symbol    *x_object_name;
    t_symbol    *x_object_path;
    t_canvas    *x_canvas;
} t_which;

/* We took this (sys_dllextent) from s_loader.c since it's not global */
/* Naming convention for externs.  The names are kept distinct for those
who wich to make "fat" externs compiled for many platforms.  Less specific
fallbacks are provided, primarily for back-compatibility; these suffice if
you are building a package which will run with a single set of compiled
objects.  The specific name is the letter b, l, d, or m for  BSD, linux,
darwin, or microsoft, followed by a more specific string, either "fat" for
a fat binary or an indication of the instruction set. */

#ifdef __FreeBSD__
static char sys_dllextent[] = ".b_i386", sys_dllextent2[] = ".pd_freebsd";
#endif
#ifdef __linux__
#ifdef __x86_64__
static char sys_dllextent[] = ".l_ia64", sys_dllextent2[] = ".pd_linux";
#else
static char sys_dllextent[] = ".l_i386", sys_dllextent2[] = ".pd_linux";
#endif
#endif
#ifdef __APPLE__
#ifndef MACOSX3
static char sys_dllextent[] = ".d_fat", sys_dllextent2[] = ".pd_darwin";
#else
static char sys_dllextent[] = ".d_ppc", sys_dllextent2[] = ".pd_darwin";
#endif
#endif
#ifdef _WIN32
static char sys_dllextent[] = ".m_i386", sys_dllextent2[] = ".dll";
#endif

static void which_any(t_which *x, t_symbol *s, int argc, t_atom *argv);
static void which_bang(t_which *x);
static void which_symbol(t_which *x, t_symbol *s);
static void which_set(t_which *x, t_symbol *s);
static void *which_new(t_symbol *s, int argc, t_atom *argv);
void which_setup(void);

static void which_set(t_which *x, t_symbol *s)
{
    x->x_object_name = s;
}

static void which_symbol(t_which *x, t_symbol *s)
{
    x->x_object_name = s;
}

static void which_any(t_which *x, t_symbol *s, int argc, t_atom *argv)
{
    x->x_object_name = s;
    which_bang(x);
}

static void which_bang(t_which *x)
{
    int     fd = -1, result = 0;
    char    *nameptr = 0;
    char filename[MAXPDSTRING];
    char dirbuf[MAXPDSTRING];
    /*
    EXTERN int canvas_open(t_canvas *x, const char *name, const char *ext,
    char *dirresult, char **nameresult, unsigned int size, int bin);
    */
    /* canvas_open is a utility function to read a file, looking first down
    the canvas's search path (set with "declare" objects in the patch and
    recursively in calling patches), then down the system one.
    The filename is the concatenation of "name" and "ext".
    "Name" may be absolute, or may be relative with slashes.
    If anything can be opened, the true directory is put in the buffer
    dirresult (provided by caller), which should be "size" bytes.
    The "nameresult" pointer will be set somewhere in
    the interior of "dirresult" and will give the file basename (with
    slashes trimmed).  If "bin" is set a 'binary' open is
    attempted, otherwise ASCII (this only matters on Microsoft.) 
    If "x" is zero, the file is sought in the directory "." or in the
    global path.
    
    */
    /* try looking in the path for (objectname).(sys_dllextent) ... */

    x->x_canvas = canvas_getcurrent();
//post("which_bang: canvas is %p", x->x_canvas);
//post("which_bang: name is %s", x->x_object_name->s_name);
//post("which_bang: ext is %s", sys_dllextent);
//post("which_bang: ext2 is %s", sys_dllextent2);
    fd = canvas_open(x->x_canvas, x->x_object_name->s_name, sys_dllextent,
        dirbuf, &nameptr, MAXPDSTRING, 1);
//post("which_bang 1: fd is %d", fd);
//    if (fd >= 0) post("1");
    if (fd < 0)
    {/* same, with the more generic sys_dllextent2 */
        fd = canvas_open(x->x_canvas, x->x_object_name->s_name, sys_dllextent2,
            dirbuf, &nameptr, MAXPDSTRING, 1);
//post("which_bang 2: fd is %d", fd);
//        if (fd >= 0) post("2");
    }
    if (fd < 0)
    {
//post("which_bang: not found");
        outlet_symbol(x->x_obj.te_outlet, gensym("not found"));
        return;
    }
    result = close(fd);
//post("which_bang: dirbuf: %s", dirbuf);
//post("which_bang: nameptr: %s", nameptr);
    /* rebuild the absolute pathname */
    strncpy(filename, dirbuf, MAXPDSTRING);
    filename[MAXPDSTRING-2] = 0;
    strcat(filename, "/");
    strncat(filename, nameptr, MAXPDSTRING-strlen(filename));
    filename[MAXPDSTRING-1] = 0;
//post("which_bang: filename: %s", filename);
    x->x_object_path = gensym(filename);
    outlet_symbol(x->x_obj.te_outlet, x->x_object_path);
}


static void *which_new(t_symbol *s, int argc, t_atom *argv)
{
    t_which *x = (t_which *)pd_new(which_class);
    x->x_object_name = s;
    if ((argc >= 1)&&(argv[0].a_type == A_SYMBOL)) x->x_object_name = argv[0].a_w.w_symbol;
    outlet_new(&x->x_obj, &s_anything);
///    x->x_canvas = canvas_getcurrent();/* canvas_getcurrent only seems to work in the _new function: why? */
///    post("which_new: canvas is %p", x->x_canvas);
    return (x);
}

void which_setup(void)
{
    which_class = class_new(gensym("which"), (t_newmethod)which_new,
        0, sizeof(t_which), 0, A_DEFSYM, 0);
    class_addbang(which_class, (t_method)which_bang);
    class_addsymbol(which_class, (t_method)which_symbol);
    class_addanything(which_class, (t_method)which_any);
    class_addmethod(which_class, (t_method)which_set, gensym("set"), A_DEFSYM, 0);
}

/* end which.c */
