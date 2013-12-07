
/*=============================================================================*\
 * File: gfsmUtils.h
 * Author: Bryan Jurish <moocow@ling.uni-potsdam.de>
 * Description: finite state machine library
 *
 * Copyright (c) 2004-2007 Bryan Jurish.
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

/** \file gfsmUtils.h
 *  \brief Miscellaneous utilities
 */

#ifndef _GFSM_UTILS_H
#define _GFSM_UTILS_H

#include <stdio.h>
#include <gfsmError.h>

/*======================================================================
 * Constants
 */
/*(none)*/

/*======================================================================
 * glib utility functions
 */
/** 3-way comparison predicate for integers */
gint gfsm_int_compare(gconstpointer a, gconstpointer b);

/** 3-way comparison predicate for integers, with user-data slot */
gint gfsm_int_compare_data(gconstpointer a, gconstpointer b, gpointer data);

/** 3-way comparison predicate for unsigned integers */
gint gfsm_uint_compare(gconstpointer a, gconstpointer b);

/** 3-way comparison predicate for unsigned integers, with user-data slot */
gint gfsm_uint_compare_data(gconstpointer a, gconstpointer b, gpointer data);

/*======================================================================
 * Hash Utilties
 */
/** Utility function to clear hash tables */
gboolean gfsm_hash_clear_func (gpointer key, gpointer value, gpointer user_data);

/*======================================================================
 * File Utilties
 */
/* Open a named file.
 * The filename \a "-" may be used to indicate stdin or stdout,
 * depending on \a mode.
 *
 * If the file cannot be opened, **errp is set (if non-NULL) and NULL is returned.
 */
FILE *gfsm_open_filename(const char *filename, const char *mode, gfsmError **errp);


#endif /* _GFSM_UTILS_H */
