/* Copyright (c) 2003-2005 krzYszcz and others.
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

#ifndef __BINPORT_H__
#define __BINPORT_H__

/* return values of binport_read() and import_max(), also passed to
   outlet_float() by cyclone library objects (cyclone, maxmode...) */
#define BINPORT_FAILED    -4  /* internal error */
#define BINPORT_CORRUPT   -3  /* file contents inconsistency */
#define BINPORT_INVALID   -2  /* file type not recognized */
#define BINPORT_NOFILE    -1  /* file not found */
#define BINPORT_MAXBINARY  0
#define BINPORT_MAXTEXT    1
#define BINPORT_MAXOLD     2
#define BINPORT_PDFILE     3

#ifndef MIXED_STANDALONE
int binport_read(t_binbuf *bb, char *filename, char *dirname);
void binport_write(t_binbuf *bb, char *filename, char *dirname);
#endif

#endif
