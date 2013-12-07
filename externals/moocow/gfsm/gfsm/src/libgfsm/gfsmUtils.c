/*=============================================================================*\
 * File: gfsmUtils.c
 * Author: Bryan Jurish <moocow@ling.uni-potsdam.de>
 * Description: finite state machine library
 *
 * Copyright (c) 2004-2008 Bryan Jurish.
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

#include <glib.h>
#include <gfsmUtils.h>
#include <gfsmError.h>
#include <gfsmCommon.h>
#include <string.h>
#include <errno.h>

/*======================================================================
 * glib utility functions
 */

/*--------------------------------------------------------------
 * int_compare()
 */
gint gfsm_int_compare(gconstpointer a, gconstpointer b)
{
  return (GPOINTER_TO_INT(b) - GPOINTER_TO_INT(a));
}

/*--------------------------------------------------------------
 * int_compare_data()
 */
gint gfsm_int_compare_data(gconstpointer a, gconstpointer b, GFSM_UNUSED gpointer data)
{
  return (GPOINTER_TO_INT(b) - GPOINTER_TO_INT(a));
}

/*--------------------------------------------------------------
 * uint_compare()
 */
#define gfsm_uint_compare_code(a,b) \
    ( GPOINTER_TO_UINT(a) < GPOINTER_TO_UINT(b) ? (-1) : (GPOINTER_TO_UINT(a) == GPOINTER_TO_UINT(b) ? 0 : 1) )
    //( (gint) ((glong((guint)b)) - (glong((guint)a))) );

gint gfsm_uint_compare(gconstpointer a, gconstpointer b)
{
  return gfsm_uint_compare_code(GPOINTER_TO_UINT(a),GPOINTER_TO_UINT(b));
}

/*--------------------------------------------------------------
 * uint_compare_data()
 */
gint gfsm_uint_compare_data(gconstpointer a, gconstpointer b, GFSM_UNUSED gpointer data)
{
  return gfsm_uint_compare_code(a,b);
}

/*======================================================================
 * Hash Utilties
 */
/*--------------------------------------------------------------
 * gfsm_hash_clear_func()
 */
gboolean gfsm_hash_clear_func (GFSM_UNUSED gpointer key, GFSM_UNUSED gpointer value, GFSM_UNUSED gpointer user_data)
{ return TRUE; };
	      

/*======================================================================
 * File Utilties
 */

/*--------------------------------------------------------------
 * open_filename()
 */
FILE *gfsm_open_filename(const char *filename, const char *mode, gfsmError **errp)
{
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
  return f;
}
