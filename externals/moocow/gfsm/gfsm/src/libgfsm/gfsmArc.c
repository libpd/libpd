/*=============================================================================*\
 * File: gfsmArc.c
 * Author: Bryan Jurish <moocow@ling.uni-potsdam.de>
 * Description: finite state machine library: arcs
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

#include <gfsmArc.h>
#include <stdlib.h>

#ifndef GFSM_INLINE_ENABLED
//-- no inline definitions
# include <gfsmArc.hi>
#endif

/*======================================================================
 * Constants (none)
 */

/*======================================================================
 * Methods: Arcs: Constructors etc.
 */
//--inline

/*======================================================================
 * Arc Comparisons (old)
 */


/*--------------------------------------------------------------
 * sortmode_to_name()
 */
#if 0
const gchar *gfsm_arc_sortmode_to_name(gfsmArcSortMode mode)
{
  switch (mode) {
  case gfsmASMNone:   return "none";
  case gfsmASMLower:  return "lower";
  case gfsmASMUpper:  return "upper";
  case gfsmASMWeight: return "weight";
  default:            return "unknown";
  }
}
#endif


/*======================================================================
 * Arc Comparisons (new)
 */

/*--------------------------------------------------------------
 * acmask_from_chars()
 */
gfsmArcCompMask gfsm_acmask_from_chars(const char *maskchars)
{
  gfsmArcCompMask m = 0;
  gint  i;
  gint  nth=0;
  for (i=0; maskchars && maskchars[i] && nth < gfsmACMaxN; i++) {
    switch (maskchars[i]) {
    case 'l' : m |= gfsm_acmask_new(gfsmACLower, nth++); break;
    case 'L' : m |= gfsm_acmask_new(gfsmACLowerR,nth++); break;

    case 'u' : m |= gfsm_acmask_new(gfsmACUpper, nth++); break;
    case 'U' : m |= gfsm_acmask_new(gfsmACUpperR,nth++); break;

    case 'w' : m |= gfsm_acmask_new(gfsmACWeight, nth++); break;
    case 'W' : m |= gfsm_acmask_new(gfsmACWeightR,nth++); break;

    case 's' : m |= gfsm_acmask_new(gfsmACSource, nth++); break;
    case 'S' : m |= gfsm_acmask_new(gfsmACSourceR,nth++); break;

    case 't' : m |= gfsm_acmask_new(gfsmACTarget, nth++); break;
    case 'T' : m |= gfsm_acmask_new(gfsmACTargetR,nth++); break;

    case 'x' : m |= gfsm_acmask_new(gfsmACUser, nth++); break;
    case 'X' : m |= gfsm_acmask_new(gfsmACUserR,nth++); break;

      //-- silently ignore these
    case '_':
    case '-':
    case ',':
    case ' ':
    case '\t':
    case '\n':
      break;

    default:
      g_printerr("libgfsm: character '%c' is not in [stluwxSTLUWX_] in mode string '%s' - skipping\n",
		 maskchars[i], maskchars);
      break;
    }
  }
  return m;
}

/*--------------------------------------------------------------
 * acmask_from_args()
 */
gfsmArcCompMask gfsm_acmask_from_args(gfsmArcComp cmp0, ...)
{
  gfsmArcCompMask m=0;
  gfsmArcComp cmp;
  gint nth=0;
  va_list ap;

  va_start(ap,cmp0);
  for (cmp=cmp0; cmp!=0 && nth < gfsmACMaxN; nth++, cmp=va_arg(ap,gfsmArcComp)) {
    m |= gfsm_acmask_new(cmp,nth);
  }
  va_end(ap);

  return m;
}

/*--------------------------------------------------------------
 * compare_bymask()
 */
gint gfsm_arc_compare_bymask(gfsmArc *a1, gfsmArc *a2, gfsmArcCompData *acdata)
{ return gfsm_arc_compare_bymask_inline(a1,a2,acdata); }


/*--------------------------------------------------------------
 * acmask_to_chars()
 */
gchar *gfsm_acmask_to_chars(gfsmArcCompMask m, gchar *chars)
{
  gint nth;
  if (!chars) { chars = g_new0(gchar,gfsmACMaxN+1); }
  for (nth=0; nth < gfsmACMaxN; nth++) {
    chars[nth] = gfsm_acmask_nth_char(m,nth);
  }
  chars[gfsmACMaxN] = '\0';
  return chars;
}

/*--------------------------------------------------------------
 * acmask_nth_string()
 */
const gchar *gfsm_acmask_nth_string(gfsmArcCompMask m, gint nth)
{
  switch (gfsm_acmask_nth(m,nth)) {
  case gfsmACLower:   return "lower";
  case gfsmACUpper:   return "upper";
  case gfsmACWeight:  return "weight";
  case gfsmACSource:  return "source";
  case gfsmACTarget:  return "target";
    //
  case gfsmACLowerR:  return "reverse_lower";
  case gfsmACUpperR:  return "reverse_upper";
  case gfsmACWeightR: return "reverse_weight";
  case gfsmACSourceR: return "reverse_source";
  case gfsmACTargetR: return "reverse_target";
    //
  case gfsmACUser:    return "user";
  case gfsmACUserR:   return "reverse_user";
  case gfsmACNone:    return "none";
  case gfsmACReverse: return "reverse_none";
  default:            return "?";
  }
  return "?";
}

/*--------------------------------------------------------------
 * acmask_to_gstring()
 */
GString *gfsm_acmask_to_gstring(gfsmArcCompMask m, GString *gstr)
{
  gint nth;
  if (!gstr) { gstr = g_string_sized_new(96); }
  else { g_string_truncate(gstr,0); }
  for (nth=0; nth < gfsmACMaxN; nth++) {
    if (nth) { g_string_append(gstr, ", "); }
    g_string_append(gstr, gfsm_acmask_nth_string(m,nth));
  }
  return gstr;
}
