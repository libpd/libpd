
/*=============================================================================*\
 * File: gfsmDebug.h
 * Author: Bryan Jurish <moocow@ling.uni-potsdam.de>
 * Description: finite state machine library: debugging
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

/** \file gfsmDebug.h
 *  \brief Debugging utilities
 */

#ifndef _GFSM_DEBUG_H
#define _GFSM_DEBUG_H

#include <glib.h>

/* Define these to enable verbose memory debugging */
/*#define GFSM_MEM_DEBUG*/
/*#define GFSM_ALLOC_DEBUG*/

/** Initialize debugging -- should be called before any other gfsm operations */
void gfsm_debug_init(void);

/** Finish debugging -- be nice and clean up after ourselves */
void gfsm_debug_finish(void);

/** Print memory debugging trace information */
void gfsm_debug_print(void);

#endif /* _GFSM_DEBUG_H */
