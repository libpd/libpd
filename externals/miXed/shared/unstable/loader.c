/* Copyright (c) 1997-2005 Miller Puckette, krzYszcz, and others.
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

/* This is just a not-yet-in-the-API-sys_load_lib() duplication
   (modulo differentiating the error return codes).  LATER use the original. */

/* this is for GNU/Linux and also Debian GNU/Hurd and GNU/kFreeBSD */
#if defined(__linux__) || defined(__FreeBSD_kernel__) || defined(__GNU__) || defined(__GLIBC__)
#include <dlfcn.h>
#endif
#ifdef UNIX
#include <stdlib.h>
#include <unistd.h>
#endif
#ifdef NT
#include <io.h>
#include <windows.h>
#endif
#ifdef MACOSX
#include <mach-o/dyld.h> 
#endif
#include <string.h>
#include <stdio.h>
#include "m_pd.h"
#include "common/loud.h"
#include "common/os.h"
#include "loader.h"

typedef void (*t_xxx)(void);

static char sys_dllextent[] = 
#ifdef __FreeBSD__
    ".pd_freebsd";
#endif
#ifdef IRIX
#ifdef N32
    ".pd_irix6";
#else
    ".pd_irix5";
#endif
#endif
/* this is for GNU/Linux and also Debian GNU/Hurd and GNU/kFreeBSD */
#if defined(__linux__) || defined(__FreeBSD_kernel__) || defined(__GNU__) || defined(__GLIBC__)
    ".pd_linux";
#endif
#ifdef MACOSX
    ".pd_darwin";
#endif
#ifdef NT
    ".dll";
#endif

static int unstable_doload_lib(char *dirname, char *classname)
{
    char symname[MAXPDSTRING], filename[MAXPDSTRING], *lastdot;
    void *dlobj;
    t_xxx makeout;
#ifdef NT
    HINSTANCE ntdll;
#endif
    /* refabricate the pathname */
    strcpy(filename, dirname);
    strcat(filename, "/");
    strcat(filename, classname);
    /* extract the setup function name */
    if (lastdot = strrchr(classname, '.'))
	*lastdot = 0;

#ifdef MACOSX
    strcpy(symname, "_");
    strcat(symname, classname);
#else
    strcpy(symname, classname);
#endif
    /* if the last character is a tilde, replace with "_tilde" */
    if (symname[strlen(symname) - 1] == '~')
	strcpy(symname + (strlen(symname) - 1), "_tilde");
    /* and append _setup to form the C setup function name */
    strcat(symname, "_setup");
/* this is for GNU/Linux and also Debian GNU/Hurd and GNU/kFreeBSD */
#if defined(__linux__) || defined(__FreeBSD_kernel__) || defined(__GNU__) || defined(__GLIBC__)
    dlobj = dlopen(filename, RTLD_NOW | RTLD_GLOBAL);
    if (!dlobj)
    {
	post("%s: %s", filename, dlerror());
	return (LOADER_BADFILE);
    }
    makeout = (t_xxx)dlsym(dlobj,  symname);
#endif
#ifdef NT
    sys_bashfilename(filename, filename);
    ntdll = LoadLibrary(filename);
    if (!ntdll)
    {
	post("%s: couldn't load", filename);
	return (LOADER_BADFILE);
    }
    makeout = (t_xxx)GetProcAddress(ntdll, symname);  
#endif
#ifdef MACOSX
    {
	NSObjectFileImage image; 
	void *ret;
	NSSymbol s; 
	if ( NSCreateObjectFileImageFromFile( filename, &image) != NSObjectFileImageSuccess )
	{
	    post("%s: couldn't load", filename);
	    return (LOADER_BADFILE);
	}
	ret = NSLinkModule( image, filename,
			    NSLINKMODULE_OPTION_BINDNOW
			    + NSLINKMODULE_OPTION_PRIVATE);
            
	s = NSLookupSymbolInModule(ret, symname); 
        
	if (s)
	    makeout = (t_xxx)NSAddressOfSymbol( s);
	else makeout = 0;
    }
#endif
    if (!makeout)
    {
	post("load_object: Symbol \"%s\" not found", symname);
	return (LOADER_NOENTRY);
    }
    (*makeout)();
    return (LOADER_OK);
}

/* start searching from dirname, then search the path */
int unstable_load_lib(char *dirname, char *classname)
{
    char dirbuf[MAXPDSTRING], *nameptr;
    int fd;
    if ((fd = open_via_path(dirname, classname, sys_dllextent,
    	dirbuf, &nameptr, MAXPDSTRING, 1)) < 0)
    {
    	return (LOADER_NOFILE);
    }
    else
    {
    	close(fd);
	return (unstable_doload_lib(dirbuf, nameptr));
    }
}

/* only dirname is searched */
int unstable_dirload_lib(char *dirname, char *classname)
{
    if (strlen(dirname) + strlen(classname) + strlen(sys_dllextent) + 3 <
	MAXPDSTRING)
    {
	char namebuf[MAXPDSTRING], *slash, *nameptr;
	strcpy(namebuf, dirname);
	if (*dirname && namebuf[strlen(namebuf)-1] != '/')
	    strcat(namebuf, "/");
	strcat(namebuf, classname);
	strcat(namebuf, sys_dllextent);
	slash = strrchr(namebuf, '/');
	if (slash)
	{
	    *slash = 0;
	    nameptr = slash + 1;
	}
	else nameptr = namebuf;
	return (unstable_doload_lib(namebuf, nameptr));
    }
    else return (LOADER_FAILED);
}

/* return the number of successfully loaded libraries, or -1 on error */
int unstable_dirload_all(char *dirname, int beloud, int withclasses)
{
    t_osdir *dp = osdir_open(dirname);
    if (dp)
    {
	int result = 0;
	char namebuf[MAXPDSTRING], *name;
	osdir_setmode(dp, OSDIR_FILEMODE);
	while (name = osdir_next(dp))
	{
	    int namelen = strlen(name), extlen = strlen(sys_dllextent);
	    if ((namelen -= extlen) > 0 &&
		strcmp(name + namelen, sys_dllextent) == 0)
	    {
		strncpy(namebuf, name, namelen);
		namebuf[namelen] = 0;
		if (zgetfn(&pd_objectmaker, gensym(namebuf)))
		{
		    if (beloud)
			loud_warning(0, "xeq", "plugin \"%s\" already loaded",
				     namebuf);
		}
		else
		{
		    int err;
		    if (beloud)
			post("loading xeq plugin \"%s\"", namebuf);
		    err = unstable_dirload_lib(dirname, namebuf);
		    if (err == LOADER_NOFILE)
		    {
			if (beloud)
			    loud_error(0, "xeq plugin \"%s\" disappeared",
				       namebuf);
		    }
		    else if (!zgetfn(&pd_objectmaker, gensym(namebuf)))
		    {
			if (beloud)
			    loud_error(0, "library \"%s\" not compatible",
				       namebuf);
		    }
		    else result++;
		}
	    }
	}
	osdir_close(dp);
	return (result);
    }
    else
    {
	if (beloud)
	    loud_syserror(0, "cannot open \"%s\"", dirname);
	return (-1);
    }
}
