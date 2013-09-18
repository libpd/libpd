/*=============================================================================*\
 * File: gfsmScanner.h
 * Author: Bryan Jurish <moocow@ling.uni-potsdam.de>
 * Description: finite state machine library
 *
 * Copyright (c) 2005-2007 Bryan Jurish.
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

/** \file gfsmScanner.h
 *  \brief  flex scanner utilities
 */

#ifndef _GFSM_SCANNER_H
#define _GFSM_SCANNER_H

#include <stdio.h>
#include <stdarg.h>

#include <gfsmError.h>

/*======================================================================
 * Types
 */

//------------------------------------------------------
/// Opaque type for generic flex lexers
typedef void* gfsmFlexScanner;

//------------------------------------------------------
/// Opaque type for generic flex lexer buffers
typedef void* gfsmFlexBufferState;

//------------------------------------------------------
/// typedef for flex scanner init() functions
typedef int (*gfsmFlexScannerInitFunc) (gfsmFlexScanner yyscanner);

/// typedef for flex scanner destroy() functions
typedef int (*gfsmFlexScannerFreeFunc) (gfsmFlexScanner yyscanner);

//------------------------------------------------------
/// typedef for flex scanner lineno() and column() accessors
typedef int (*gfsmFlexScannerGetPosFunc) (gfsmFlexScanner yyscanner);

/// typedef for flex scanner lineno() and column() manipulators
typedef void (*gfsmFlexScannerSetPosFunc) (int pos, gfsmFlexScanner yyscanner);

//------------------------------------------------------
/// typedef for flex scanner set_extra() function
typedef void (*gfsmFlexScannerSetExtraFunc) (void *extra, gfsmFlexScanner yyscanner);

/// typedef for flex scanner get_extra() function
typedef void* (*gfsmFlexScannerGetExtraFunc) (gfsmFlexScanner yyscanner);

//------------------------------------------------------
/// typedef for flex scanner get_text() function
typedef char* (*gfsmFlexScannerGetTextFunc) (gfsmFlexScanner yyscanner);

//------------------------------------------------------
/// typedef for flex scanner restart() function
typedef void  (*gfsmFlexScannerScanFileFunc) (FILE *in, gfsmFlexScanner yyscanner);

/// typedef for flex scanner scan_bytes() function
typedef void  (*gfsmFlexScannerScanBytesFunc) (const char *str, int len, gfsmFlexScanner yyscanner);

/// typedef for flex scanner pop_buffer_state() function
typedef void  (*gfsmFlexScannerScanPopFunc) (gfsmFlexScanner yyscanner);


//------------------------------------------------------

/** \brief Extra data struct for generic flex scanners.
 *  \detail To use, add the line
 *          \code #define YY_EXTRA_TYPE gfsmScannerExtra*
 *          to scanner.l
 */
typedef struct {
  gfsmFlexScanner yyscanner;          ///< underlying flex scanner
  void *data;                         ///< user data
  char *name;                         ///< name of this scanner, for errors & warnings; may be NULL
  FILE *infile;                       ///< current input file if we opened it ourselves, else NULL
  char *filename;                     ///< name of input file or NULL
  gfsmError *err;                     ///< Holds scanner error if something goes wrong
  gboolean emit_warnings;             ///< write warnings to stderr

  //-- funcs: init, free
  gfsmFlexScannerFreeFunc yyfree_func;    ///< ${PREFIX}lex_destroy(yyscanner)

  //-- funcs & data: buffer switching
  gfsmFlexScannerScanFileFunc  yyscan_file_func;  ///< ${PREFIX}restart(FILE*,yyscanner)
  gfsmFlexScannerScanBytesFunc yyscan_bytes_func; ///< ${PREFIX}_scan_bytes(bytes,len,yyscanner)
  gfsmFlexScannerScanPopFunc   yyscan_pop_func;   ///< ${PREFIX}pop_buffer_state(yyscanner)

  //-- funcs: text
  gfsmFlexScannerGetTextFunc  yyget_text_func; ///< ${PREFIX}get_text(yyscanner)

  //-- funcs: position
  gfsmFlexScannerGetPosFunc  yyget_lineno_func;  ///< ${PREFIX}get_lineno(yyscanner)
  gfsmFlexScannerSetPosFunc  yyset_lineno_func;  ///< ${PREFIX}set_lineno(yyscanner)

  //-- funcs: extra data
  gfsmFlexScannerSetExtraFunc yyset_extra_func; ///< ${PREFIX}get_extra(yyscanner)

} gfsmScanner;


/*======================================================================
 * Constants
 */
/** Default scanner name */
extern const char *gfsmScannerDefaultName; 

/** Default scanner filename */
extern const char *gfsmScannerDefaultFilename;

/*======================================================================
 * gfsmScanner: Constructors etc.
 */

/// \name gfsmScanner: Constructors etc.
//@{


/** Initialize and return a gfsmScanner given the flex prefix.
 *  Scanner is initialized for a reentrant flex scanner with prefix PREFIX.
 */
#define gfsm_scanner_init(scanner,name,PREFIX) \
  gfsm_scanner_init_full((scanner), \
			 (name), \
                         (gfsmFlexScannerInitFunc)      ( PREFIX ## lex_init ), \
                         (gfsmFlexScannerFreeFunc)      ( PREFIX ## lex_destroy ), \
                         (gfsmFlexScannerScanFileFunc)  ( PREFIX ## restart ), \
                         (gfsmFlexScannerScanBytesFunc) ( PREFIX ## _scan_bytes ), \
                         (gfsmFlexScannerScanPopFunc)   ( PREFIX ## pop_buffer_state ), \
                         (gfsmFlexScannerGetTextFunc)   ( PREFIX ## get_text ), \
                         (gfsmFlexScannerGetPosFunc)    ( PREFIX ## get_lineno ), \
                         (gfsmFlexScannerSetPosFunc)    ( PREFIX ## set_lineno ), \
                         (gfsmFlexScannerSetExtraFunc)  ( PREFIX ## set_extra ) )

/** Create, initialize, and return a new gfsmScanner given the a flex prefix.
 *  Scanner is initialized for a reentrant flex scanner with prefix PREFIX.
 */
#define gfsm_scanner_new(name,PREFIX) \
  gfsm_scanner_init( g_new0(gfsmScanner,1), (name), PREFIX )


/** Create, initialize, and return a new gfsmFlexScanner \a scanner.
 *  Underlying flex scanner will be available as \a scanner->yyscanner.
 */
gfsmScanner *gfsm_scanner_new_full(const char                  *name,
				   gfsmFlexScannerInitFunc      yyinit_func,
				   gfsmFlexScannerFreeFunc      yyfree_func,
				   gfsmFlexScannerScanFileFunc  yyscan_file_func,
				   gfsmFlexScannerScanBytesFunc yyscan_bytes_func,
				   gfsmFlexScannerScanPopFunc   yyscan_pop_func,
				   gfsmFlexScannerGetTextFunc   yyget_text_func,
				   gfsmFlexScannerGetPosFunc    yyget_lineno_func,
				   gfsmFlexScannerSetPosFunc    yyset_lineno_func,
				   gfsmFlexScannerSetExtraFunc  yyset_extra_func);



/** Initialize a gfsmScanner. */
gfsmScanner *gfsm_scanner_init_full(gfsmScanner                 *scanner,
				    const char                  *name,
				    gfsmFlexScannerInitFunc      yyinit_func,
				    gfsmFlexScannerFreeFunc      yyfree_func,
				    gfsmFlexScannerScanFileFunc  yyscan_file_func,
				    gfsmFlexScannerScanBytesFunc yyscan_bytes_func,
				    gfsmFlexScannerScanPopFunc   yyscan_pop_func,
				    gfsmFlexScannerGetTextFunc   yyget_text_func,
				    gfsmFlexScannerGetPosFunc    yyget_lineno_func,
				    gfsmFlexScannerSetPosFunc    yyset_lineno_func,
				    gfsmFlexScannerSetExtraFunc  yyset_extra_func);

/** Frees memory associated with a gfsmScanner.
 *  Calls \a scanner->yyfree_func() to destroy the underlying flex scanner.
 */
void gfsm_scanner_free(gfsmScanner *scanner);
					  

/*======================================================================
 * gfsmScanner: I/O Selection
 */

/// \name gfsmScanner: I/O Selection
//@{

/** Close any file associated with a scanner */
void gfsm_scanner_close(gfsmScanner *scanner);

/** Scan from an open FILE* */
void gfsm_scanner_scan_file(gfsmScanner *scanner, FILE *f);

/** Scan from a named file */
void gfsm_scanner_scan_filename(gfsmScanner *scanner, const char *filename);

/** Scan an in-memory buffer */
void gfsm_scanner_scan_bytes(gfsmScanner *scanner, const char *bytes, int len);

/** Scan from a GString* */
void gfsm_scanner_scan_gstring(gfsmScanner *scanner, GString *gstr);

/** Scan a NUL-terminated string */
void gfsm_scanner_scan_string(gfsmScanner *scanner, const char *str);

//@}

/*======================================================================
 * Scanner Methods: Flex scanner utilities
 */
/// \name  gfsmScanner: Flex Scanner Utilities
//@{

/** Can be used as an input wrapper; doesn't wrap input at all */
int gfsm_scanner_yywrap(gfsmScanner *scanner);

/** Use default yywrap */
#define GFSM_SCANNER_YYWRAP(PREFIX) \
  int PREFIX ## wrap(gfsmFlexScanner yyscanner) \
  { return gfsm_scanner_yywrap( yyscanner ); }

//@}

/*======================================================================
 * Scanner Methods: Errors
 */
/// \name gfsmScanner: Error Reporting
//@{

/** Wrapper for gfsm_scanner_carp_full_v(); sets \a scanner->err */
void gfsm_scanner_carp(gfsmScanner *scanner, const char *fmt, ...);

/** Wrapper for gfsm_scanner_carp_full_v(); sets \a scanner->err */
void gfsm_scanner_carp_full(gfsmScanner        *scanner,
			    GQuark              domain,
			    gint                code,
			    const char         *fmt,
			    ...);

/** Warning function - sets \a scanner->err */
void gfsm_scanner_carp_full_v(gfsmScanner        *scanner,
			      GQuark              domain,
			      gint                code,
			      const char         *fmt,
			      va_list             ap);
//@}

#endif /* _GFSM_SCANNER_H */
