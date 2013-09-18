
/*=============================================================================*\
 * File: gfsmAssert.h
 * Author: Bryan Jurish <moocow@ling.uni-potsdam.de>
 * Description: finite state machine library: assertions
 *
 * Copyright (c) 2007 Bryan Jurish.
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

/** \file gfsmAssert.h
 *  \brief Assertions
 */

#ifndef _GFSM_ASSERT_H
#define _GFSM_ASSERT_H

#include <glib.h>

/**
 * \def    gfsm_assert_not_reached()
 * \detail If ever actually evaluated, aborts with an error message.
 *         Only available if libgfsm was configured with debugging enabled.
 *
 * \def     gfsm_assert(expr)
 * \detail  If \a expr does not evaluate to a true value, aborts with an error message.
 *          Only available if libgfsm was configured with debugging enabled.
 */

#ifdef GFSM_DEBUG_ENABLED
# define gfsm_assert_not_reached() g_assert_not_reached()
# define gfsm_assert(expr)         g_assert(expr)
#else
# define gfsm_assert_not_reached() 
# define gfsm_assert(expr)         
#endif

#endif /* _GFSM_ASSERT_H */
