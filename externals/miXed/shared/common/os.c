/* Copyright (c) 2004-2005 krzYszcz and others.
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

#ifdef MSW
#include <io.h>
#else
#include <unistd.h>
#include <dirent.h>
#endif
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "m_pd.h"
#include "os.h"

static int ospath_doabsolute(char *path, char *cwd, char *result)
{
    if (*path == 0)
    {
	if (result)
	    strcpy(result, cwd);
	else
	    return (strlen(cwd));
    }
    else if (*path == '~')
    {
	path++;
	if (*path == '/' || *path == 0)
	{
#ifdef UNIX
	    char *home = getenv("HOME");
	    if (home)
	    {
		if (result)
		{
		    strcpy(result, home);
		    if (*path)
			strcat(result, path);
		}
		else return (strlen(home) + strlen(path));
	    }
	    else goto badpath;
#else
	    goto badpath;
#endif
	}
	else goto badpath;
    }
    else if (*path == '/')
    {
#ifdef MSW
	/* path is absolute, drive is implicit, LATER UNC? */
	if (*cwd && cwd[1] == ':')
	{
	    if (result)
	    {
		*result = *cwd;
		result[1] = ':';
		strcpy(result + 2, path);
	    }
	    else return (2 + strlen(path));
	}
	else goto badpath;
#else
	/* path is absolute */
	if (result)
	    strcpy(result, path);
	else
	    return (strlen(path));
#endif
    }
    else
    {
#ifdef MSW
	if (path[1] == ':')
	{
	    if (path[2] == '/')
	    {
		/* path is absolute */
		if (result)
		    strcpy(result, path);
		else
		    return (strlen(path));
	    }
	    else if (*cwd == *path)
	    {
		/* path is relative, drive is explicitly current */
		if (result)
		{
		    int ndx = strlen(cwd);
		    strcpy(result, cwd);
		    result[ndx++] = '/';
		    strcpy(result + ndx, path + 2);
		}
		else return (strlen(cwd) + strlen(path) - 1);
	    }
	    /* we do not maintain per-drive cwd, LATER rethink */
	    else goto badpath;
	}
	/* LATER devices? */
	else
	{
	    /* path is relative */
	    if (result)
	    {
		int ndx = strlen(cwd);
		strcpy(result, cwd);
		result[ndx++] = '/';
		strcpy(result + ndx, path);
	    }
	    else return (strlen(cwd) + 1 + strlen(path));
	}
#else
	/* path is relative */
	if (result)
	{
	    int ndx = strlen(cwd);
	    strcpy(result, cwd);
	    result[ndx++] = '/';
	    strcpy(result + ndx, path);
	}
	else return (strlen(cwd) + 1 + strlen(path));
#endif
    }
    if (result && *result && *result != '.')
    {
	/* clean-up */
	char *inptr, *outptr = result;
	int ndx = strlen(result);
	if (result[ndx - 1] == '.')
	{
	    result[ndx] = '/';  /* guarding slash */
	    result[ndx + 1] = 0;
	}
	for (inptr = result + 1; *inptr; inptr++)
	{
	    if (*inptr == '/')
	    {
		if (*outptr == '/')
		    continue;
		else if (*outptr == '.')
		{
		    if (outptr[-1] == '/')
		    {
			outptr--;
			continue;
		    }
		    else if (outptr[-1] == '.' && outptr[-2] == '/')
		    {
			outptr -= 2;
			if (outptr == result)
			    continue;
			else for (outptr--; outptr != result; outptr--)
			    if (*outptr == '/')
				break;
			continue;
		    }
		}
	    }
	    *++outptr = *inptr;
	}
	if (*outptr == '/' && outptr != result)
	    *outptr = 0;
	else
	    outptr[1] = 0;
    }
    else bug("ospath_doabsolute 1");
    return (0);
badpath:
    if (result)
	bug("ospath_doabsolute 2");
    return (0);
}

/* Returns an estimated length of an absolute path made up from the first arg.
   The actual ospath_absolute()'s length may be shorter (since it erases
   superfluous slashes and dots), but not longer.  Both args should be unbashed
   (system-independent), cwd should be absolute.  Returns 0 in case of any
   error (LATER revisit). */
int ospath_length(char *path, char *cwd)
{
    /* one extra byte used internally (guarding slash) */
    return (ospath_doabsolute(path, cwd, 0) + 1);
}

/* Copies an absolute path to result.  Arguments: path and cwd, are the same
   as in ospath_length().  Caller should first consult ospath_length(), and
   allocate at least ospath_length() + 1 bytes to the result buffer.
   Should never fail (failure is a bug). */
char *ospath_absolute(char *path, char *cwd, char *result)
{
    ospath_doabsolute(path, cwd, result);
    return (result);
}

FILE *fileread_open(char *filename, t_canvas *cv, int textmode)
{
    int fd;
    char path[MAXPDSTRING+2], *nameptr;
    t_symbol *dirsym = (cv ? canvas_getdir(cv) : 0);
    /* path arg is returned unbashed (system-independent) */
    if ((fd = open_via_path((dirsym ? dirsym->s_name : ""), filename,
			    "", path, &nameptr, MAXPDSTRING, 1)) < 0)
    	return (0);
    /* Closing/reopening dance.  This is unnecessary under linux, and we
       could have tried to convert fd to fp, but under windows open_via_path()
       returns what seems to be an invalid fd.
       LATER try to understand what is going on here... */
    close(fd);
    if (path != nameptr)
    {
	char *slashpos = path + strlen(path);
	*slashpos++ = '/';
	/* try not to be dependent on current open_via_path() implementation */
	if (nameptr != slashpos)
	    strcpy(slashpos, nameptr);
    }
    return (sys_fopen(path, (textmode ? "r" : "rb")));
}

FILE *filewrite_open(char *filename, t_canvas *cv, int textmode)
{
    char path[MAXPDSTRING+2];
    if (cv)
	/* path arg is returned unbashed (system-independent) */
	canvas_makefilename(cv, filename, path, MAXPDSTRING);
    else
    {
    	strncpy(path, filename, MAXPDSTRING);
    	path[MAXPDSTRING-1] = 0;
    }
    return (sys_fopen(path, (textmode ? "w" : "wb")));
}

/* FIXME add MSW */

struct _osdir
{
#ifndef MSW
    DIR            *dir_handle;
    struct dirent  *dir_entry;
#endif
    int             dir_flags;
};

/* returns 0 on error, a caller is then expected to call
   loud_syserror(owner, "cannot open \"%s\"", dirname) */
t_osdir *osdir_open(char *dirname)
{
#ifndef MSW
    DIR *handle = opendir(dirname);
    if (handle)
    {
#endif
	t_osdir *dp = getbytes(sizeof(*dp));
#ifndef MSW
	dp->dir_handle = handle;
	dp->dir_entry = 0;
#endif
	dp->dir_flags = 0;
	return (dp);
#ifndef MSW
    }
    else return (0);
#endif
}

void osdir_setmode(t_osdir *dp, int flags)
{
    if (dp)
	dp->dir_flags = flags;
}

void osdir_close(t_osdir *dp)
{
    if (dp)
    {
#ifndef MSW
	closedir(dp->dir_handle);
#endif
	freebytes(dp, sizeof(*dp));
    }
}

void osdir_rewind(t_osdir *dp)
{
    if (dp)
    {
#ifndef MSW
	rewinddir(dp->dir_handle);
	dp->dir_entry = 0;
#endif
    }
}

char *osdir_next(t_osdir *dp)
{
#ifndef MSW
    if (dp)
    {
	while (dp->dir_entry = readdir(dp->dir_handle))
	{
	    if (!dp->dir_flags ||
		(dp->dir_entry->d_type == DT_REG
		 && (dp->dir_flags & OSDIR_FILEMODE)) ||
		(dp->dir_entry->d_type == DT_DIR
		 && (dp->dir_flags & OSDIR_DIRMODE)))
		return (dp->dir_entry->d_name);
	}
    }
#endif
    return (0);
}

int osdir_isfile(t_osdir *dp)
{
#ifndef MSW
    return (dp && dp->dir_entry && dp->dir_entry->d_type == DT_REG);
#else
    return (0);
#endif
}

int osdir_isdir(t_osdir *dp)
{
#ifndef MSW
    return (dp && dp->dir_entry && dp->dir_entry->d_type == DT_DIR);
#else
    return (0);
#endif
}
