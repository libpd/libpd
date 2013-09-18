/*=============================================================================*\
 * File: gfsmWeightmap.c
 * Author: Bryan Jurish <moocow@ling.uni-potsdam.de>
 * Description: finite state machine library: weight maps: extern definitions
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

#include <gfsmWeightMap.h>
#include <gfsmCompound.h>

//-- (no-)inline definitions
#ifndef GFSM_INLINE_ENABLED
# include <gfsmWeightMap.hi>
#endif

//--------------------------------------------------------------
// weightmap_to_array_foreach_func_()
static
gboolean gfsm_weightmap_to_array_foreach_func_(gpointer stateid_p, gpointer weight_p, gfsmStateWeightPairArray *array)
{
  gfsmStateWeightPair wp;
  wp.id = GPOINTER_TO_UINT(stateid_p);
  wp.w  = gfsm_ptr2weight(weight_p);
  g_array_append_val(array,wp);
  return FALSE; //-- continue traversal
}

//--------------------------------------------------------------
gfsmStateWeightPairArray *gfsm_weightmap_to_array(gfsmWeightMap *weightmap, gfsmStateWeightPairArray *array)
{
  if (!array) {
    array = g_array_sized_new(FALSE,FALSE,sizeof(gfsmStateWeightPair),gfsm_weightmap_size(weightmap));
  } else {
    g_array_set_size(array,gfsm_weightmap_size(weightmap));
    array->len = 0;
  }
  gfsm_weightmap_foreach(weightmap, (GTraverseFunc)gfsm_weightmap_to_array_foreach_func_, array);
  return array;
}
