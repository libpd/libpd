/* Copyright (c) 2004-2005 krzYszcz and others.
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

#ifndef __OS_H__
#define __OS_H__

EXTERN_STRUCT _osdir;
#define t_osdir  struct _osdir

#define OSDIR_FILEMODE  1
#define OSDIR_DIRMODE   2

int ospath_length(char *path, char *cwd);
char *ospath_absolute(char *path, char *cwd, char *result);

FILE *fileread_open(char *filename, t_canvas *cv, int textmode);
FILE *filewrite_open(char *filename, t_canvas *cv, int textmode);

t_osdir *osdir_open(char *dirname);
void osdir_setmode(t_osdir *dp, int flags);
void osdir_close(t_osdir *dp);
void osdir_rewind(t_osdir *dp);
char *osdir_next(t_osdir *dp);
int osdir_isfile(t_osdir *dp);
int osdir_isdir(t_osdir *dp);

#endif
