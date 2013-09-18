
/*=============================================================================*\
 * File: gfsmScanner.c
 * Author: Bryan Jurish <moocow@ling.uni-potsdam.de>
 * Description: finite state machine library
 *
 * Copyright (c) 2005-2008 Bryan Jurish.
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

#include <gfsmScanner.h>
#include <gfsmUtils.h>
#include <gfsmCommon.h>
#include <string.h>

/*======================================================================
 * Constants
 */
const char *gfsmScannerDefaultName = "gfsmScanner";

const char *gfsmScannerDefaultFilename = "input";

/*======================================================================
 * gfsmScanner: Constructors etc.
 */

//--------------------------------------------------------------
gfsmScanner *gfsm_scanner_new_full(const char                  *name,
				   gfsmFlexScannerInitFunc      yyinit_func,
				   gfsmFlexScannerFreeFunc      yyfree_func,
				   gfsmFlexScannerScanFileFunc  yyscan_file_func,
				   gfsmFlexScannerScanBytesFunc yyscan_bytes_func,
				   gfsmFlexScannerScanPopFunc   yyscan_pop_func,
				   gfsmFlexScannerGetTextFunc   yyget_text_func,
				   gfsmFlexScannerGetPosFunc    yyget_lineno_func,
				   gfsmFlexScannerSetPosFunc    yyset_lineno_func,
				   gfsmFlexScannerSetExtraFunc  yyset_extra_func)
{
  return gfsm_scanner_init_full(g_new0(gfsmScanner,1),
				name,
				yyinit_func,
				yyfree_func,
				yyscan_file_func,
				yyscan_bytes_func,
				yyscan_pop_func,
				yyget_text_func,
				yyget_lineno_func,
				yyset_lineno_func,
				yyset_extra_func);
}

//--------------------------------------------------------------
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
				    gfsmFlexScannerSetExtraFunc  yyset_extra_func)
{
  scanner->data          = NULL;
  scanner->name          = (char *)name;
  scanner->filename      = NULL;
  scanner->err           = NULL;

  //-- funcs: init, free
  scanner->yyfree_func   = yyfree_func;

  //-- data & funcs: buffer switching
  scanner->infile = NULL;
  scanner->yyscan_file_func  = yyscan_file_func;
  scanner->yyscan_bytes_func = yyscan_bytes_func;
  scanner->yyscan_pop_func = yyscan_pop_func;

  //-- funcs: text
  scanner->yyget_text_func = yyget_text_func;

  //-- funcs: position
  scanner->yyget_lineno_func = yyget_lineno_func;
  scanner->yyset_lineno_func = yyset_lineno_func;

  //-- initialize underlying scanner
  if (yyinit_func) (*yyinit_func)(&(scanner->yyscanner));

  //-- set extra data
  if (yyset_extra_func) (*yyset_extra_func)(scanner, scanner->yyscanner);

  return scanner;
}

//--------------------------------------------------------------
void gfsm_scanner_free(gfsmScanner *scanner)
{
  gfsm_scanner_close(scanner);
  if (scanner->yyfree_func) (*(scanner->yyfree_func))(scanner->yyscanner);
  g_clear_error(&(scanner->err));
  g_free(scanner);
}


/*======================================================================
 * gfsmScanner: I/O Selection
 */

//--------------------------------------------------------------
void gfsm_scanner_close(gfsmScanner *scanner)
{
  (*(scanner->yyscan_pop_func))(scanner->yyscanner);
  if (scanner->infile) {
    fclose(scanner->infile);
    scanner->infile = NULL;
  }
  if (scanner->filename) {
    g_free(scanner->filename);
    scanner->filename = NULL;
  }
}

//--------------------------------------------------------------
void gfsm_scanner_scan_file(gfsmScanner *scanner, FILE *f)
{
  gfsm_scanner_close(scanner);
  (*(scanner->yyscan_file_func))(f, scanner->yyscanner);
  (*(scanner->yyset_lineno_func))(1, scanner->yyscanner);
}

//--------------------------------------------------------------
void gfsm_scanner_scan_filename(gfsmScanner *scanner, const char *filename)
{
  g_clear_error(&(scanner->err));
  FILE *f = gfsm_open_filename(filename, "r", &(scanner->err));
  gfsm_scanner_scan_file(scanner,f);
  scanner->filename = g_strdup(filename);
  scanner->infile = f;
}

//--------------------------------------------------------------
void gfsm_scanner_scan_bytes(gfsmScanner *scanner, const char *bytes, int len)
{
  gfsm_scanner_close(scanner);
  (*(scanner->yyscan_bytes_func))(bytes, len, scanner->yyscanner);
  (*(scanner->yyset_lineno_func))(1, scanner->yyscanner);
}

//--------------------------------------------------------------
void gfsm_scanner_scan_gstring(gfsmScanner *scanner, GString *gstr)
{
  gfsm_scanner_scan_bytes(scanner, gstr->str, gstr->len);
}

//--------------------------------------------------------------
void gfsm_scanner_scan_string(gfsmScanner *scanner, const char *str)
{
  gfsm_scanner_scan_bytes(scanner, str, strlen(str));
}



/*======================================================================
 * gfsmScanner: Flex utilities
 */

//--------------------------------------------------------------
int gfsm_scanner_yywrap(GFSM_UNUSED gfsmScanner *scanner)
{ return 1; }


/*======================================================================
 * gfsmScanner: error reporting
 */

//--------------------------------------------------------------
void gfsm_scanner_carp(gfsmScanner *scanner, const char *fmt, ...)
{
  va_list ap;
  va_start(ap,fmt);
  gfsm_scanner_carp_full_v(scanner,
			   g_quark_from_static_string("gfsm"),
			   g_quark_from_static_string("scanner_error"),
			   fmt, ap);
  va_end(ap);
}

//--------------------------------------------------------------
void gfsm_scanner_carp_full(gfsmScanner        *scanner,
			    GQuark              domain,
			    gint                code,
			    const char         *fmt,
			    ...)
{
  va_list ap;
  va_start(ap,fmt);
  gfsm_scanner_carp_full_v(scanner,domain,code,fmt,ap);
  va_end(ap);
}

//--------------------------------------------------------------
void gfsm_scanner_carp_full_v(gfsmScanner        *scanner,
			      GQuark              domain,
			      gint                code,
			      const char         *fmt,
			      va_list             ap)
{
  char *msg = g_strdup_vprintf(fmt, ap);

  if (scanner->err) {
    g_error_free(scanner->err);
    scanner->err = NULL;
  }

  g_set_error(&(scanner->err), domain, code,
	      "%s: %s in %s%s%s at line %u%s%s%s",
	      (scanner->name     ? scanner->name     : gfsmScannerDefaultName),
	      msg,
	      (scanner->filename ? "file \""           : ""),
	      (scanner->filename ? scanner->filename : gfsmScannerDefaultFilename),
	      (scanner->filename ? "\""                : ""),
	      (scanner->yyget_lineno_func ? (*(scanner->yyget_lineno_func))(scanner->yyscanner) : 0),
	      //--
	      (scanner->yyget_text_func ? ", near \"" : ""),
	      (scanner->yyget_text_func ? (*(scanner->yyget_text_func))(scanner->yyscanner) : ""),
	      (scanner->yyget_text_func ? "\"" : "")
	      );

  if (scanner->emit_warnings) {
    fprintf(stderr, "%s\n", scanner->err->message);
  }

  g_free(msg);
}
