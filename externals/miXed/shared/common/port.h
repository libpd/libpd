/* Copyright (c) 2003-2005 krzYszcz and others.
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

#ifndef __PORT_H__
#define __PORT_H__

int import_max(char *fn, char *dir);
void import_setmapping(int size, char **mapping);
char **import_getmapping(int *sizep);
char *port_usemapping(char *from, int mapsize, char **mapping);

#endif
