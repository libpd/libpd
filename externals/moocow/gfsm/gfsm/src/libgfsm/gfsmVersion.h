
/*=============================================================================*\
 * File: gfsmVersion.h
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

/** \file gfsmVersion.h
 *  \brief Library version information
 */

#ifndef _GFSM_VERSION_H
#define _GFSM_VERSION_H

#include <glib.h>

/*======================================================================
 * Types
 */
/// Library Version information
typedef struct {
  guint32 major;   /**< major version */
  guint32 minor;   /**< minor version */
  guint32 micro;   /**< micro version */
} gfsmVersionInfo;

/*======================================================================
 * Constants
 */
/** Current version information */
extern const gfsmVersionInfo gfsm_version;

/** Current version string */
extern const char *gfsm_version_string;

/*======================================================================
 * Comparison
 */
/** 3-way comparison two gfsmVersionInfo structures */
int gfsm_version_compare(gfsmVersionInfo v1, gfsmVersionInfo v2);

/** Equality check for gfsmVersionInfo structures */
#define gfsm_version_eq(v1,v2) (gfsm_version_compare((gfsmVersionInfo)(v1),(gfsmVersionInfo)(v2))==0)

/** Less-than comparison for gfsmVersionInfo structures: v1 < v2 */
#define gfsm_version_less(v1,v2) (gfsm_version_compare((gfsmVersionInfo)(v1),(gfsmVersionInfo)(v2))<0)

/** Less-than-or-equal comparison for gfsmVersionInfo structures: v1 <= v2 */
#define gfsm_version_le(v1,v2) (gfsm_version_compare((gfsmVersionInfo)(v1),(gfsmVersionInfo)(v2))<=0)

/** Greater-than comparison for gfsmVersionInfo structures: v1 > v2 */
#define gfsm_version_greater(v1,v2) (gfsm_version_compare((gfsmVersionInfo)(v1),(gfsmVersionInfo)(v2))>0)

/** Greater-than-or-equal comparison for gfsmVersionInfo structures: v1 >= v2 */
#define gfsm_version_ge(v1,v2) (gfsm_version_compare((gfsmVersionInfo)(v1),(gfsmVersionInfo)(v2))>=0)

#endif /* _GFSM_VERSION_H */
