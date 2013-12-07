/*=============================================================================*\
 * File: gfsmSet.c
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

#include <gfsmSet.h>
#include <gfsmCommon.h>

//-- no-inline definitions
#ifndef GFSM_INLINE_ENABLED
# include <gfsmSet.hi>
#endif

/*======================================================================
 * Constructors etc.
 */

/*--------------------------------------------------------------
 * copy_foreach_func()
 */
gboolean gfsm_set_copy_foreach_func(gpointer key, gpointer value, gfsmSet *data)
{
  if (data) g_tree_insert(data,key,value);
  return FALSE; // don't stop iterating
}

/*--------------------------------------------------------------
 * clear()
 */
void gfsm_set_clear(gfsmSet *set)
{
  guint i;
  GPtrArray *keys = g_ptr_array_sized_new(gfsm_set_size(set));
  gfsm_set_to_ptr_array(set,keys);
  for (i=0; i < keys->len; i++) {
    g_tree_remove(set, g_ptr_array_index(keys,i));
  }
  g_ptr_array_free(keys,TRUE);
}


/*======================================================================
 * Algebra
 */

/*--------------------------------------------------------------
 * union_func()
 */
gboolean gfsm_set_union_func(gpointer key, GFSM_UNUSED gpointer value, gfsmSetUnionData *data)
{
  if (!data->dupfunc) {
    //-- no memory hairiness: just insert
    gfsm_set_insert(data->dst, key);
  } else {
    if (!gfsm_set_contains(data->dst, key))
      gfsm_set_insert(data->dst, (*data->dupfunc)(key));
  }
  return FALSE;
}

/*--------------------------------------------------------------
 * difference_func()
 */
gboolean gfsm_set_difference_func(gpointer key, GFSM_UNUSED gpointer value, gfsmSet *set1)
{
  gfsm_set_remove(set1,key);
  return FALSE;
}

/*--------------------------------------------------------------
 * intersection()
 */
gfsmSet *gfsm_set_intersection(gfsmSet *set1, gfsmSet *set2)
{
  guint i;
  GPtrArray *elts1 = g_ptr_array_sized_new(gfsm_set_size(set1));
  gfsm_set_to_ptr_array(set1,elts1);
  for (i=0; i < elts1->len; i++) {
    gpointer elt = g_ptr_array_index(elts1,i);
    if (!gfsm_set_contains(set2,elt)) gfsm_set_remove(set1,elt);
  }
  g_ptr_array_free(elts1,TRUE);
  return set1;
}


/*======================================================================
 * Converters
 */

/*--------------------------------------------------------------
 * to_slist_foreach_func()
 */
gboolean gfsm_set_to_slist_foreach_func(gpointer key, GFSM_UNUSED gpointer value, GSList **dst)
{
  *dst = g_slist_prepend(*dst, key);
  return FALSE; //-- don't stop iterating
}

/*--------------------------------------------------------------
 * to_ptr_array_foreach_func()
 */
gboolean gfsm_set_to_ptr_array_foreach_func(gpointer key, GFSM_UNUSED gpointer value, GPtrArray *dst)
{
  g_ptr_array_add(dst,key);
  return FALSE;
}

/*======================================================================
 * Debugging
 */
#ifdef GFSM_DEBUG_ENABLED

gboolean gfsm_set_print_foreach_func(gpointer key, gpointer data, FILE *f)
{
  fprintf(f, " %u", GPOINTER_TO_UINT(key));
  return FALSE;
}

void gfsm_set_print_uint(gfsmSet *set, FILE *f)
{
  fputc('{',f);
  g_tree_foreach(set, (GTraverseFunc)gfsm_set_print_foreach_func, f);
  fputs(" }", f);
}

#endif /*GFSM_DEBUG_ENABLED*/
