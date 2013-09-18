/*=============================================================================*\
 * File: gfsmIO.h
 * Author: Bryan Jurish <moocow@ling.uni-potsdam.de>
 * Description: finite state machine library: I/O
 *
 * Copyright (c) 2006-2008 Bryan Jurish.
 *
 * For information on usage and redistribution, and for a DISCLAIMER
 * OF ALL WARRANTIES, see the file "COPYING" in this distribution.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *=============================================================================*/

/** \file gfsmIO.h
 *  \brief Abstract I/O routines
 */

#ifndef _GFSM_IO_H
#define _GFSM_IO_H

#include <gfsmConfig.h>
#include <gfsmError.h>

#include <stdarg.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>

#define GFSMIO_EOF ((int)-1)

/*======================================================================
 * I/O: types
 */

/** Builtin I/O types */
typedef enum {
  gfsmIOTCFile,         ///< I/O on a C FILE*
  gfsmIOTZFile,         ///< I/O on a zlib gzFile* (only if GFSM_ZLIB_ENABLED is defined)
  gfsmIOTGString,       ///< I/O on a GString*
  gfsmIOTUser = 255     ///< user I/O
} gfsmIOHandleType;

/*======================================================================
 * I/O: Handles: Function types
 */

/** Generic I/O Handle function type: fflush() and friends */
typedef void (*gfsmIOFlushFunc) (void *handle);

/** Generic I/O Handle function type: fclose() and friends */
typedef void (*gfsmIOCloseFunc) (void *handle);

/** Generic I/O Handle function type: feof() and friends */
typedef gboolean (*gfsmIOEofFunc) (void *handle);


/** Generic I/O Handle function type: fread() and friends */
typedef gboolean (*gfsmIOReadFunc)  (void *handle, void *buf, size_t nbytes);

/** Generic I/O Handle function type: getdelim() and friends */
typedef ssize_t (*gfsmIOGetdelimFunc)  (void *handle, char **lineptr, size_t *n, int delim);


/** Generic I/O Handle function type: fwrite() and friends */
typedef gboolean (*gfsmIOWriteFunc) (void *handle, const void *buf, size_t nbytes);

/** Generic I/O Handle function type: vprintf() and friends */
typedef int (*gfsmIOVprintfFunc) (void *handle, const char *fmt, va_list *app);



/*======================================================================
 * I/O: Handles: structs
 */

/** \brief Generic I/O handle struct */
typedef struct {
  gfsmIOHandleType    iotype;  ///< I/O class of this handle
  void               *handle;  ///< underlying handle data

  gfsmIOReadFunc     read_func;       /** fread() and friends (either read or getc must be defined) */
  gfsmIOGetdelimFunc getdelim_func;   /** getdelim() and friends (optional) */

  gfsmIOWriteFunc    write_func;     /** fwrite() and friends (either write or putc must be defined) */
  gfsmIOVprintfFunc  vprintf_func;   /** vprintf() and friends (optional) */

  gfsmIOFlushFunc    flush_func;     /** fflush() and friends (optional) */
  gfsmIOCloseFunc    close_func;     /** fclose() and friends (optional) */
  gfsmIOEofFunc      eof_func;       /** eof() and friends (optional) */
} gfsmIOHandle;


/** \brief GString with an associated index (read head) */
typedef struct {
  GString *gs;  ///< associated string
  size_t   pos; ///< (read-)position
} gfsmPosGString;

/*======================================================================
 * I/O: Handles: Constructors etc.
 */

/** create, initialize, and return a new gfsmIOHandle
 *  \param typ type of this handle
 *  \param handle_data value of the \a handle structure datum:
 *    \li for \a typ==gfsmIOTCFile , \a handle_data should be a FILE*
 *    \li for \a typ==gfsmIOTGString , \a handle_data should be a gfsmPosGString*
 *    \li for \a typ==gfsmIOTZFile   , \a handle_data should be a gzFile
 *    \li for \a typ==gfsmIOTUser , \a handle_data is whatever you want
 *
 * \returns new gfsmIOHandle
 */
gfsmIOHandle *gfsmio_handle_new(gfsmIOHandleType typ, void *handle_data);

/** destroy a gfsmIOHandle: does NOT implicitly call \a close or anything else */
void gfsmio_handle_free(gfsmIOHandle *ioh);

/* TODO: utilities ? file_handle_new, zfile_handle_new, gstring_handle_new, user_handle_new ? */

/** Create and return a new gfsmIOHandle to an uncompressed C FILE*
 *  Caller is responsible for closing the handle.
 */
gfsmIOHandle *gfsmio_new_file(FILE *f);

/** Create and return a new gfsmIOHandle to a C FILE* using compression (if available)
 *  Caller is responsible for closing the handle.
 *  The handle returned can always be closed without closing \a f itself.
 */
gfsmIOHandle *gfsmio_new_zfile(FILE *f, const char *mode, int compress_level);

/** Create and return a new gfsmIOHandle to a named file.
 *  Uses gzFile if zlib support was enabled, otherwise C FILE* (uncompressed)
 *  Caller is responsible for closing the handle.
 */
gfsmIOHandle *gfsmio_new_filename(const char *filename, const char *mode, int compress_level, gfsmError **errp);

/** Create and return a new gfsmIOHandle for a PosGString*
 *  Caller is responsible for allocation and de-allocation of the PosGString*.
 */
gfsmIOHandle *gfsmio_new_gstring(gfsmPosGString *pgs);

/*======================================================================
 * I/O: Handles: Methods: Basic
 */

/** close an open I/O handle (calls \a close_func) */
void gfsmio_close(gfsmIOHandle *ioh);

/** flush all data to an output handle (calls \a flush_func) */
void gfsmio_flush(gfsmIOHandle *ioh);

/** returns true if \a h is at EOF, false otherwise (or if no \a eof_func is defined) */
gboolean gfsmio_eof(gfsmIOHandle *ioh);



/*======================================================================
 * I/O: Handles: Methods: Read
 */

/** read a single byte of data from \a h, should return GFSMIO_EOF on EOF */
int gfsmio_getc(gfsmIOHandle *ioh);

/** read \a nbytes of data from \a io into \a buf, as \a fread() */
gboolean gfsmio_read(gfsmIOHandle *ioh, void *buf, size_t nbytes);

/** wrapper for getline(), returns number of bytes read (0 on error) */
ssize_t gfsmio_getline(gfsmIOHandle *ioh, char **lineptr, size_t *n);


/** wrapper for getdelim(), returns number of bytes read (0 on error) */
ssize_t gfsmio_getdelim(gfsmIOHandle *io, char **lineptr, size_t *n, int delim);



/*======================================================================
 * I/O: Handles: Methods: Write
 */

/** write a single byte to handle \a ioh, as \a fputc() */
gboolean gfsmio_putc(gfsmIOHandle *ioh, int c);

/** wrapper for puts() */
gboolean gfsmio_puts(gfsmIOHandle *io, const char *s);

/** write \a nbytes of data from \a buf into \a io, as \a fwrite() */
gboolean gfsmio_write(gfsmIOHandle *io, const void *buf, size_t nbytes);

/** wrapper for printf(): calls \a gfsmio_vprintf() */
int gfsmio_printf(gfsmIOHandle *io, const char *fmt, ...);

/** wrapper for vprintf(): calls \a vprintf_func */
int gfsmio_vprintf(gfsmIOHandle *io, const char *fmt, va_list *app);


#endif /* _GFSM_IO_H */
