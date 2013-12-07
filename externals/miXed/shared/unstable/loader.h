/* Copyright (c) 2003-2005 krzYszcz and others.
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

#ifndef __LOADER_H__
#define __LOADER_H__

enum { LOADER_OK, LOADER_FAILED,
       LOADER_NOFILE, LOADER_BADFILE, LOADER_NOENTRY };

int unstable_load_lib(char *dirname, char *classname);
int unstable_dirload_lib(char *dirname, char *classname);
int unstable_dirload_all(char *dirname, int beloud, int withclasses);

#endif
