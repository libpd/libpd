/*=============================================================================*\
 * File: gfsmIO.c
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

#include <gfsmConfig.h>

#include <glib.h>
#include <gfsmIO.h>
//#include <gfsmCompat.h>
#include <gfsmUtils.h>

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#if 0
#include <fcntl.h>
#endif
#include <errno.h>

#ifdef GFSM_ZLIB_ENABLED
# include <zlib.h>
# define GFSM_DEFAULT_COMPRESSION Z_DEFAULT_COMPRESSION
#endif

#include "vasprintf.h"
#include "getdelim.h"

/*======================================================================
 * Protos: I/O: Handles: Methods: Instatiations: C FILE*
 */
void gfsmio_close_cfile(FILE *f);
void gfsmio_flush_cfile(FILE *f);
gboolean gfsmio_eof_cfile(FILE *f);
gboolean gfsmio_read_cfile(FILE *f, void *buf, size_t nbytes);
ssize_t gfsmio_getdelim_cfile(FILE *f, char **lineptr, size_t *n, int delim);
gboolean gfsmio_write_cfile(FILE *f, const void *buf, size_t nbytes);
#ifdef HAVE_VFPRINTF
int gfsmio_vprintf_cfile(FILE *f, const char *fmt, va_list *app);
#endif

/*======================================================================
 * Protos: I/O: Handles: Methods: Instatiations: gzFile
 */
#ifdef GFSM_ZLIB_ENABLED
void gfsmio_close_zfile(gzFile zf);
void gfsmio_flush_zfile(gzFile zf);
gboolean gfsmio_eof_zfile(gzFile zf);
gboolean gfsmio_read_zfile(gzFile zf, void *buf, size_t nbytes);
gboolean gfsmio_write_zfile(gzFile zf, const void *buf, size_t nbytes);
#endif

/*======================================================================
 * Protos: I/O: Handles: Methods: Instatiations: GString*
 */
void gfsmio_close_gstring(gfsmPosGString *pgs);
gboolean gfsmio_eof_gstring(gfsmPosGString *pgs);
gboolean gfsmio_read_gstring(gfsmPosGString *pgs, void *buf, size_t nbytes);
gboolean gfsmio_write_gstring(gfsmPosGString *pgs, const void *buf, size_t nbytes);

/*======================================================================
 * I/O: Handles: Constructors etc.
 */

/*--------------------------------------------------------------*/
gfsmIOHandle *gfsmio_handle_new(gfsmIOHandleType typ, void *handle_data)
{
  gfsmIOHandle *ioh = g_new0(gfsmIOHandle,1);
  ioh->iotype = typ;
  ioh->handle = handle_data;

  switch (typ) {
  //--------------------------------
  case gfsmIOTCFile:
#ifndef GFSM_ZLIB_ENABLED
  case gfsmIOTZFile:
#endif
    ioh->read_func    = (gfsmIOReadFunc)gfsmio_read_cfile;
    ioh->getdelim_func= (gfsmIOGetdelimFunc)gfsmio_getdelim_cfile;

    ioh->write_func   = (gfsmIOWriteFunc)gfsmio_write_cfile;
#ifdef HAVE_VFPRINTF
    ioh->vprintf_func = (gfsmIOVprintfFunc)gfsmio_vprintf_cfile;
#endif

    ioh->flush_func   = (gfsmIOFlushFunc)gfsmio_flush_cfile;
    ioh->close_func   = (gfsmIOCloseFunc)gfsmio_close_cfile;
    ioh->eof_func     = (gfsmIOEofFunc)gfsmio_eof_cfile;
    break;

#ifdef GFSM_ZLIB_ENABLED
  //--------------------------------
  case gfsmIOTZFile:
    ioh->read_func    = (gfsmIOReadFunc)gfsmio_read_zfile;
    //ioh->getdelim_func= (gfsmIOReadFunc)gfsmio_getdelim_zfile;

    ioh->write_func   = (gfsmIOWriteFunc)gfsmio_write_zfile;
    //ioh->vprintf_func = (gfsmIOReadFunc)gfsmio_vprintf_zfile;

    ioh->flush_func   = (gfsmIOFlushFunc)gfsmio_flush_zfile;
    ioh->close_func   = (gfsmIOCloseFunc)gfsmio_close_zfile;
    ioh->eof_func     = (gfsmIOEofFunc)gfsmio_eof_zfile;
    break;
#endif

  //--------------------------------
  case gfsmIOTGString:
    ioh->read_func    = (gfsmIOReadFunc)gfsmio_read_gstring;
    //ioh->getdelim_func= gfsmio_getdelim_gstring;

    ioh->write_func   = (gfsmIOWriteFunc)gfsmio_write_gstring;
    //ioh->vprintf_func = gfsmio_vprintf_gstring;

    //ioh->flush_func   = gfsmio_flush_gstring;
    ioh->close_func   = (gfsmIOCloseFunc)gfsmio_close_gstring;
    ioh->eof_func     = (gfsmIOEofFunc)gfsmio_eof_gstring;
    break;

  //--------------------------------
  case gfsmIOTUser:
  default:
    break;
  }

  return ioh;
}

/*--------------------------------------------------------------*/
void gfsmio_handle_free(gfsmIOHandle *ioh)
{
  g_free(ioh);
}

/*--------------------------------------------------------------*/
gfsmIOHandle *gfsmio_new_file(FILE *f)
{
  return gfsmio_handle_new(gfsmIOTCFile, f);
}

/*--------------------------------------------------------------*/
#undef GFSM_ZFILE_USE_FCNTL
gfsmIOHandle *gfsmio_new_zfile(FILE *f, const char *mode, int compress_level)
{
#ifdef GFSM_ZLIB_ENABLED
# ifdef GFSM_ZFILE_USE_FCNTL
  int fd = fileno(f);
  int flags = fcntl(fd, F_GETFL);
  gzFile zf;
#  if 0 /* DEBUG */
  //-- DEBUG
  const int o_rdwr = O_RDWR;
  const int o_rdonly = O_RDONLY;
  const int o_wronly = O_WRONLY;
  //-- /DEBUG
#  endif /* DEBUG */
  if ( (flags&O_RDWR) == O_RDWR ) {
    zf = gzdopen(dup(fd),"rwb");
    gzsetparams(zf, compress_level, Z_DEFAULT_STRATEGY);
  }
  else
  if ( (flags&O_WRONLY) == O_WRONLY ) {
    zf = gzdopen(fd,"wb");
    gzsetparams(zf, compress_level, Z_DEFAULT_STRATEGY);
  }
  else { // if ( (flags&O_RDONLY) == O_RDONLY ) 
    zf = gzdopen(fd,"rb");
    gzsetparams(zf, compress_level, Z_DEFAULT_STRATEGY);
  }
  return gfsmio_handle_new(gfsmIOTZFile, zf);

# else /* !defined(GFSM_ZFILE_USE_FCNTL) */

  if (compress_level != 0) {
    //-- use compression
    gzFile zf = gzdopen(fileno(f), mode);
    if (strchr(mode,'w')) gzsetparams(zf, compress_level, Z_DEFAULT_STRATEGY);
    return gfsmio_handle_new(gfsmIOTZFile, zf);
  } else {
    return gfsmio_new_file( fdopen(dup(fileno(f)), mode) );
  }

# endif /* GFSM_ZFILE_USE_FCNTL */

#else /* !defined(GFSM_ZLIB_ENABLED) */

  return gfsmio_new_file( fdopen(dup(fileno(f)), mode) );
#endif
}

/*--------------------------------------------------------------*/
gfsmIOHandle *gfsmio_new_filename(const char *filename, const char *mode, int compress_level, gfsmError **errp)
{
#ifdef GFSM_ZLIB_ENABLED
  if (compress_level != 0) {
    gzFile zf;
    if (strcmp(filename,"-")==0) {
      if (strchr(mode,'w')) zf = gzdopen(dup(fileno(stdout)), mode);
      else                  zf = gzdopen(dup(fileno(stdin)), mode);
    }
    else if (!(zf = gzopen(filename,mode))) {
      int errnum;
      const char *zerror = gzerror(zf,&errnum);
      g_set_error(errp,
		  g_quark_from_static_string("gfsm"),  //-- domain
		  g_quark_from_static_string("gzopen"), //-- code
		  "gzopen() failed for file '%s': %s",
		  filename, 
		  errnum==Z_ERRNO ? strerror(errno) : zerror);
      return NULL;
    }

    //-- set compression level
    if (compress_level < 0) compress_level = GFSM_DEFAULT_COMPRESSION;
    if (strchr(mode,'w')) {
      gzsetparams(zf, compress_level, Z_DEFAULT_STRATEGY);
    }

    return gfsmio_handle_new(gfsmIOTZFile,zf);
  }
  else {
#endif
    FILE *f;
    if (strcmp(filename,"-")==0) {
      if (strchr(mode,'w')) f = stdout;
      else f = stdin;
    }
    else if (!(f = fopen(filename,mode))) {
      g_set_error(errp,
		  g_quark_from_static_string("gfsm"),  //-- domain
		  g_quark_from_static_string("fopen"), //-- code
		  "open failed for file '%s': %s",
		  filename, strerror(errno));
    }
    return gfsmio_new_file(f);
#ifdef GFSM_ZLIB_ENABLED
  }
#endif
}

/*--------------------------------------------------------------*/
gfsmIOHandle *gfsmio_new_gstring(gfsmPosGString *pgs)
{
  return gfsmio_handle_new(gfsmIOTGString, pgs);
}


/*======================================================================
 * I/O: Handles: Methods: Basic
 */

/*--------------------------------------------------------------*/
/** flush all data to an output handle (calls \a flush_func) */
void gfsmio_close(gfsmIOHandle *ioh)
{
  if (ioh->close_func) (*ioh->close_func)(ioh->handle);
}

/*--------------------------------------------------------------*/
/** flush all data to an output handle (calls \a flush_func) */
void gfsmio_flush(gfsmIOHandle *ioh)
{
  if (ioh->flush_func) (*ioh->flush_func)(ioh->handle);
}

/*--------------------------------------------------------------*/
/** returns true if \a h is at EOF, false otherwise or if no \a eof_func is defined */
gboolean gfsmio_eof(gfsmIOHandle *ioh)
{
  if (ioh->eof_func) return (*ioh->eof_func)(ioh->handle);
  return FALSE;
}



/*======================================================================
 * I/O: Handles: Methods: Read
 */

/*--------------------------------------------------------------*/
int gfsmio_getc(gfsmIOHandle *ioh)
{
  if (gfsmio_eof(ioh)) return GFSMIO_EOF;
  else {
    //-- getc() --> read()
    unsigned char c = 0;
    if (gfsmio_read(ioh, &c, 1)) return (int)c;
  }
  return GFSMIO_EOF;
}

/*--------------------------------------------------------------*/
gboolean gfsmio_read(gfsmIOHandle *ioh, void *buf, size_t nbytes)
{
  if (ioh->read_func) return (*ioh->read_func)(ioh->handle, buf, nbytes);

  g_printerr("gfsmio_read(): no method defined for handle of type %d\n", ioh->iotype);
  return FALSE;
}

/*--------------------------------------------------------------*/
ssize_t gfsmio_getline(gfsmIOHandle *ioh, char **lineptr, size_t *n)
{
  return gfsmio_getdelim(ioh, lineptr, n, '\n');
}

/*--------------------------------------------------------------*/
ssize_t gfsmio_getdelim(gfsmIOHandle *ioh, char **lineptr, size_t *n, int delim)
{
  if (ioh->getdelim_func) {
    return (*ioh->getdelim_func)(ioh->handle, lineptr, n, delim);
  }
  else {
    //-- getdelim() --> getc()
    ssize_t i = 0;
    int c = -2;
    GString *gs=NULL;

    while ( *n > 0 && i < (((ssize_t)(*n))-1) && (c=gfsmio_getc(ioh)) != GFSMIO_EOF ) {
      (*lineptr)[i++] = c;
#ifdef GFSM_DEBUG_GETDELIM
      fprintf(stderr, "---> getdelim(i=%d) got char %d ~ '%c' to linebuf\n", i, (char)c, c);//--DEBUG
#endif
      if ((char)c == (char)delim) {
	(*lineptr)[i] = '\0';
	return i;
      }
    }
    if (c == GFSMIO_EOF) {
#ifdef GFSM_DEBUG_GETDELIM
      fprintf(stderr, "---> getdelim(i=%d) got EOF reading to linebuf\n", i);//--DEBUG
#endif
      (*lineptr)[i] = '\0';
      return i == 0 ? GFSMIO_EOF : i;
    }

    //-- oops: buffer overflow
    gs = g_string_new_len((i>0 ? *lineptr : ""), i);
    while ( (c=gfsmio_getc(ioh)) != GFSMIO_EOF ) {
      g_string_append_c(gs,c);
      i++;
#ifdef GFSM_DEBUG_GETDELIM
      fprintf(stderr, "---> getdelim(i=%d) got char %d ~ '%c' to GString*\n", i, (char)c, c);//--DEBUG
#endif
      if ((char)c == (char)delim) break;
    }

#ifdef GFSM_DEBUG_GETDELIM
    if (c==GFSMIO_EOF) { fprintf(stderr, "---> getdelim(i=%d) got EOF reading to GString*\n", i); }//--DEBUG
#endif

    //-- maybe free old line buffer
    if (*lineptr) free(*lineptr);

    //-- set up new buffer
    g_string_append_c(gs,0); //-- this shouldn't be necessary, but weird things happen otherwise (bug?)

    //-- the following code breaks in Perl on OpenSuSE 11.0 [maybe GString doesn't use malloc ?!], --moocow 2008-10-31
    /*
      *lineptr = gs->str;            //-- copy literal GString data buffer
      *n       = gs->allocated_len;  //-- ... and its length
      g_string_free(gs,FALSE);       //-- ... and only free GString wrapper struct; not the data buffer
    */
    //-- ...so we do this instead (ugly but functional):
    *lineptr = (char *)malloc(gs->allocated_len);  //-- malloc a copy of GString data buffer
    memcpy(*lineptr, gs->str, gs->allocated_len);  //-- ... and copy the data
    *n = gs->allocated_len;                        //-- ... and its length
    g_string_free(gs,TRUE);                        //-- ... and free the whole GString and its data buffer

    return i==0 && c==GFSMIO_EOF ? GFSMIO_EOF : i;
  }
  return GFSMIO_EOF;
}

/*======================================================================
 * I/O: Handles: Methods: Write
 */

/*--------------------------------------------------------------*/
gboolean gfsmio_putc(gfsmIOHandle *ioh, int c)
{
  return gfsmio_write(ioh, &c, 1);
}

/*--------------------------------------------------------------*/
gboolean gfsmio_puts(gfsmIOHandle *ioh, const char *s)
{
  return gfsmio_write(ioh, s, strlen(s));
}


/*--------------------------------------------------------------*/
gboolean gfsmio_write(gfsmIOHandle *ioh, const void *buf, size_t nbytes)
{
  if (ioh->write_func) return (*ioh->write_func)(ioh->handle, buf, nbytes);

  g_printerr("gfsmio_read(): no method defined for handle of type %d\n", ioh->iotype);
  return FALSE;
}

/*--------------------------------------------------------------*/
int gfsmio_printf(gfsmIOHandle *io, const char *fmt, ...)
{
  int len;
  va_list ap;

  va_start(ap,fmt);
  len = gfsmio_vprintf(io, fmt, &ap);
  va_end(ap);

  return len;
}

/*--------------------------------------------------------------*/
int gfsmio_vprintf(gfsmIOHandle *io, const char *fmt, va_list *app)
{
  char *obuf = NULL;
  size_t len = 0;
  gboolean rc;
  len = vasprintf(&obuf, fmt, *app);
  rc = gfsmio_write(io, obuf, len);
  if (obuf) free(obuf);
  return rc ? len : 0;
}


/*======================================================================
 * I/O: Handles: Methods: FILE*
 */

/*--------------------------------------------------------------
 * FILE*: Basic Methods
 */
void gfsmio_flush_cfile(FILE *f)
{ if (f) fflush(f); }

void gfsmio_close_cfile(FILE *f)
{
  if (f && f != stdin && f != stdout && f != stderr) fclose(f);
}

gboolean gfsmio_eof_cfile(FILE *f)
{ return f ? feof(f) : FALSE; }

/*--------------------------------------------------------------
 * FILE*: Read Methods
 */
gboolean gfsmio_read_cfile(FILE *f, void *buf, size_t nbytes)
{ return f ? (fread(buf,nbytes,1,f)==1) : FALSE; }

ssize_t gfsmio_getdelim_cfile(FILE *f, char **lineptr, size_t *n, int delim)
{ return f ? getdelim(lineptr, n, delim, f) : 0; }

/*--------------------------------------------------------------
 * FILE*: Write Methods
 */
gboolean gfsmio_write_cfile(FILE *f, const void *buf, size_t nbytes)
{ return f ? (fwrite(buf, nbytes, 1, f)==1) : FALSE; }

#ifdef HAVE_VFPRINTF
int gfsmio_vprintf_cfile(FILE *f, const char *fmt, va_list *app)
{ return f ? vfprintf(f, fmt, *app) : 0; }
#endif

/*======================================================================
 * I/O: Handles: Methods: gzFile
 */
#ifdef GFSM_ZLIB_ENABLED

/*--------------------------------------------------------------
 * gzFile: Basic Methods
 */
void gfsmio_close_zfile(gzFile zf)
{ if (zf) gzclose(zf); }

void gfsmio_flush_zfile(gzFile zf)
{ if (zf) gzflush(zf,Z_SYNC_FLUSH); }

gboolean gfsmio_eof_zfile(gzFile zf)
{ return zf ? gzeof(zf) : FALSE; }

/*--------------------------------------------------------------
 * gzFile: Read Methods
 */
gboolean gfsmio_read_zfile(gzFile zf, void *buf, size_t nbytes)
{ return zf ? (gzread(zf,buf,nbytes)==(int)nbytes) : FALSE; }

/*--------------------------------------------------------------
 * gzFile: Write Methods
 */
gboolean gfsmio_write_zfile(gzFile zf, const void *buf, size_t nbytes)
{ return zf ? (gzwrite(zf, buf, nbytes)==(int)nbytes) : FALSE; }

#endif /* GFSM_ZLIB_ENABLED */


/*======================================================================
 * I/O: Handles: Methods: GString*
 */

/*--------------------------------------------------------------
 * GString*: Basic Methods
 */

void gfsmio_close_gstring(gfsmPosGString *pgs)
{ if (pgs) pgs->pos = 0; }

gboolean gfsmio_eof_gstring(gfsmPosGString *pgs)
{ return pgs && pgs->gs ? (pgs->pos >= pgs->gs->len) : TRUE; }

/*--------------------------------------------------------------
 * GString*: Read Methods
 */
gboolean gfsmio_read_gstring(gfsmPosGString *pgs, void *buf, size_t nbytes)
{
  if (!pgs || !pgs->gs || pgs->pos > pgs->gs->len) return FALSE;
  if (pgs->pos+nbytes <= pgs->gs->len) {
    //-- normal case: read it in
    memcpy(buf, pgs->gs->str + pgs->pos, nbytes);
    pgs->pos += nbytes;
    return TRUE;
  }
  //-- overflow: grab what we can
  memcpy(buf, pgs->gs->str + pgs->pos, pgs->gs->len-pgs->pos);
  pgs->pos = pgs->gs->len;
  return FALSE;
}

/*--------------------------------------------------------------
 * GString*: Write Methods
 */
gboolean gfsmio_write_gstring(gfsmPosGString *pgs, const void *buf, size_t nbytes)
{
  if (pgs && pgs->gs) {
    g_string_append_len(pgs->gs, buf, nbytes);
    return TRUE;
  }
  return FALSE;
}
