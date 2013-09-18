
/*=============================================================================*\
 * File: gfsmError.h
 * Author: Bryan Jurish <moocow@ling.uni-potsdam.de>
 * Description: finite state machine library
 *
 * Copyright (c) 2004 Bryan Jurish.
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

/** \file gfsmError.h
 *  \brief Error utilities
 */

#ifndef _GFSM_ERROR_H
#define _GFSM_ERROR_H

#include <glib.h>

/*======================================================================
 * Types
 */
/// struct for errors
typedef GError gfsmError;

/*======================================================================
 * Constants
 */
/** Constant indicating no error condition */
#define gfsmErrorNone NULL

/*======================================================================
 * Error Methods
 */
/** Report a warning and continue; does nothing if error==NULL */
void gfsm_carp_named(gfsmError *error, const gchar *myname);

/** Report a warning and continue; does nothing if error==NULL */
#define gfsm_carp(e) gfsm_carp_named(e,NULL)

/** carp and exit with code error->code ; does nothing if error==NULL */
void gfsm_die_named(gfsmError *error, const gchar *myname);

/** die using default diagnostic name "gfsm"; does nothing if error==NULL */
#define gfsm_die(e) gfsm_die_named(e,NULL)

/** do or die */
#define gfsm_do_or_die(e) gfsm_die_named(e,NULL)

#endif /* _GFSM_UTILS_H */
