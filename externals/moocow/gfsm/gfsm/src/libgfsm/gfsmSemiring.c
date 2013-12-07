/*=============================================================================*\
 * File: gfsmSemiring.c
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

#include <gfsmSemiring.h>

//-- no-inline definitions
#ifndef GFSM_INLINE_ENABLED
# include <gfsmSemiring.hi>
#endif

/*======================================================================
 * Semiring: methods: constructors etc.
 */



/*======================================================================
 * Semiring: methods: general accessors
 */

/*--------------------------------------------------------------
 * compare()
 */
gint gfsm_sr_compare(gfsmSemiring *sr, gfsmWeight x, gfsmWeight y)
{
  switch (sr->type) {
  case gfsmSRTLog:
  case gfsmSRTTropical: return (x < y ? -1 : (x > y ? 1 : 0));
  case gfsmSRTTrivial:  return 0;

  case gfsmSRTPLog: return (x < y ? 1 : (x > y ? -1 : 0));

  case gfsmSRTUser:
    return (gfsm_sr_compare(sr,x,y) ? -1 : (gfsm_sr_compare(sr,y,x) ? 1 : 0));
    if (((gfsmUserSemiring*)sr)->less_func) {
      if ((*((gfsmUserSemiring*)sr)->less_func)(sr,x,y)) return -1;
      if ((*((gfsmUserSemiring*)sr)->less_func)(sr,y,x)) return  1;
      return 0;
    }

  case gfsmSRTBoolean:
  case gfsmSRTReal:
  default:             return (x > y ? -1 : (x < y ? 1 : 0));
  }
  return 0; //-- should never happen
}

/*======================================================================
 * Semiring: string utilities
 */

/*--------------------------------------------------------------
 * name_to_type()
 */
gfsmSRType gfsm_sr_name_to_type(const char *name)
{
  if      (strcmp(name,"boolean")  ==0) return gfsmSRTBoolean;
  else if (strcmp(name,"log")      ==0) return gfsmSRTLog;
  else if (strcmp(name,"plog")     ==0) return gfsmSRTPLog;
  else if (strcmp(name,"real")     ==0) return gfsmSRTReal;
  else if (strcmp(name,"trivial")  ==0) return gfsmSRTTrivial;
  else if (strcmp(name,"tropical") ==0) return gfsmSRTTropical;
  else if (strcmp(name,"user")     ==0) return gfsmSRTUser;
  return gfsmSRTUnknown;
}

/*--------------------------------------------------------------
 * type_to_name()
 */
gchar *gfsm_sr_type_to_name(gfsmSRType type)
{
  switch (type) {
  case gfsmSRTBoolean:  return "boolean";
  case gfsmSRTLog:      return "log";
  case gfsmSRTPLog:     return "plog";
  case gfsmSRTTrivial:  return "trivial";
  case gfsmSRTTropical: return "tropical";
  case gfsmSRTReal:     return "real";
  default:              return "unknown";
  }
}

/*======================================================================
 * Semiring: general utilities
 */
