
/*=============================================================================*\
 * File: gfsmVersion.c
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

#include <gfsmVersion.h>
#include <gfsmUtils.h>

#ifdef HAVE_CONFIG_H
#include <gfsmConfig.h>
#endif

/*======================================================================
 * Constants
 */
const gfsmVersionInfo gfsm_version =
  {
    GFSM_VERSION_MAJOR,
    GFSM_VERSION_MINOR,
    GFSM_VERSION_MICRO
  };

const char *gfsm_version_string = PACKAGE_VERSION;

/*======================================================================
 * Comparison
 */
int gfsm_version_compare(gfsmVersionInfo v1, gfsmVersionInfo v2)
{
  int rc;
  if      ((rc=gfsm_uint_compare(GUINT_TO_POINTER(v1.major),GUINT_TO_POINTER(v2.major)))) return rc;
  else if ((rc=gfsm_uint_compare(GUINT_TO_POINTER(v1.minor),GUINT_TO_POINTER(v2.minor)))) return rc;
  return gfsm_uint_compare(GUINT_TO_POINTER(v1.micro),GUINT_TO_POINTER(v2.micro));
}
