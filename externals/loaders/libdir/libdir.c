#include "m_pd.h"
#include "s_stuff.h"
#include "g_canvas.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>

/* this object requires Pd 0.40.3 or later */

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


static char *version = "1.9";

/* This loader opens a directory with a -meta.pd file as a library.  In the
 * long run, the idea is that one folder will have all of objects files, all
 * of the related *-help.pd files, a file with meta data for the help system,
 * etc.  Then to install the lib, it would just be dropped into extra, or
 * anywhere in the global classpath.
 *
 * Ultimately, the meta file will be read for meta data, specifically for
 * the auto-generated Help system, but for other things too.  Right now,
 * its just used as a marker that a directory is meant to be a library.
 * Plus its much easier to implement it this way, I can use
 * open_via_path() instead of writing a new function.  The grand plan is
 * to have one directory hold the objects, help files, manuals,
 * etc. making it a self-contained library. <hans@at.or.at>
 */

static int libdir_loader(t_canvas *canvas, char *classname)
{
    int fd = -1;
    char helppathname[FILENAME_MAX];
    char fullclassname[FILENAME_MAX], dirbuf[FILENAME_MAX];
    char *nameptr;
    t_canvasenvironment *canvasenvironment;

/* look for meta file (classname)/(classname)-meta.pd */
    strncpy(fullclassname, classname, FILENAME_MAX - 6);
    strcat(fullclassname, "/");
    strncat(fullclassname, classname, FILENAME_MAX - strlen(fullclassname) - 6);
    strcat(fullclassname, "-meta");
    
    /* if this is being called from a canvas, then add the library path to the
     * canvas-local path */
    if(canvas) 
    {
        canvasenvironment = canvas_getenv(canvas);
        /* setting the canvas to NULL causes it to ignore any canvas-local path */
        if ((fd = canvas_open(NULL, fullclassname, ".pd",
                              dirbuf, &nameptr, FILENAME_MAX, 0)) < 0) 
        {
            return (0);
        }
        close(fd);
        if(sys_isabsolutepath(dirbuf)) // only include actual full paths
            canvasenvironment->ce_path = namelist_append(canvasenvironment->ce_path, 
                                                         dirbuf, 0);
        logpost(NULL, 3, "libdir_loader: added '%s' to the canvas-local objectclass path",
                classname);
    }
    else
    {
        if ((fd = open_via_path(".", fullclassname, ".pd",
                                dirbuf, &nameptr, FILENAME_MAX, 0)) < 0) 
        {
            return (0);
        }
        close(fd);
        sys_searchpath = namelist_append(sys_searchpath, dirbuf, 0);
        strncpy(helppathname, sys_libdir->s_name, FILENAME_MAX-30);
        helppathname[FILENAME_MAX-30] = 0;
        strcat(helppathname, "/doc/5.reference/");
        strcat(helppathname, classname);
        sys_helppath = namelist_append(sys_helppath, helppathname, 0);
        logpost(NULL, 3, "libdir_loader: added '%s' to the global objectclass path", 
                classname);
//        post("\tThis is deprecated behavior.");
    }
    /* post("libdir_loader loaded fullclassname: '%s'\n", fullclassname); */
    logpost(NULL, 14, "Loaded libdir '%s' from '%s'", classname, dirbuf);

    return (1);
}

void libdir_setup(void)
{
/* relies on t.grill's loader functionality, fully added in 0.40 */
    sys_register_loader(libdir_loader);
    logpost(NULL, 3, "libdir loader %s",version);  
    logpost(NULL, 3, "\tcompiled on "__DATE__" at "__TIME__ " ");
    logpost(NULL, 3, "\tcompiled against Pd version %d.%d.%d.%s", 
            PD_MAJOR_VERSION, PD_MINOR_VERSION, PD_BUGFIX_VERSION, PD_TEST_VERSION);
}

void setup(void)
{
    libdir_setup();
}
