
/*=============================================================================*\
 * File: gfsmArclist.c
 * Author: Bryan Jurish <moocow@ling.uni-potsdam.de>
 * Description: finite state machine library: arc lists
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

#include <gfsmArcList.h>

//-- no-inline definitions
#ifndef GFSM_INLINE_ENABLED
# include <gfsmArcList.hi>
#endif

/*======================================================================
 * Methods: Arc lists
 */

/*--------------------------------------------------------------
 * arclist_insert_node_sorted()
 */
gfsmArcList *gfsm_arclist_insert_node_sorted(gfsmArcList *al, gfsmArcList *nod, gfsmArcCompData *acdata)
{
  gfsmArcList *al_first=al;
  gfsmArcList *al_prev=NULL;

  for (; al != NULL; al_prev=al, al=al->next) {
    if (gfsm_arc_compare_bymask_inline(&(nod->arc), &(al->arc), acdata) <= 0) break;
  }
  if (al_prev == NULL) return gfsm_arclist_prepend_node(al,nod);
  al_prev->next = gfsm_arclist_prepend_node(al,nod);

  return al_first;
}

/*--------------------------------------------------------------
 * arclist_clone()
 */
gfsmArcList *gfsm_arclist_clone(gfsmArcList *src)
{
  gfsmArcList *dst = NULL, *prev=NULL;
  while (src != NULL) {
    gfsmArcList *nod = gfsm_arclist_new_full(src->arc.source,
					     src->arc.target,
					     src->arc.lower,
					     src->arc.upper,
					     src->arc.weight,
					     NULL);
    if (prev==NULL) {
      dst = nod;
    } else {
      prev->next = nod;
    }
    prev = nod;
    src  = src->next;
  }
  return dst;
}

/*--------------------------------------------------------------
 * arclist_concat()
 */
gfsmArcList *gfsm_arclist_concat(gfsmArcList *al1, gfsmArcList *al2)
{
  if (al1==NULL) { return al2; }
  else {
    gfsmArcList *nod=al1;
    while (nod->next != NULL) { nod=nod->next; }
    nod->next = al2;
  }
  return al1;
}

/*--------------------------------------------------------------
 * arclist_length()
 */
guint gfsm_arclist_length(gfsmArcList *al)
{
  guint len=0;
  while (al != NULL) { ++len; al=al->next; }
  return len;
}

/*--------------------------------------------------------------
 * arclist_free()
 */
void gfsm_arclist_free(gfsmArcList *al)
{
  while (al != NULL) {
    gfsmArcList *nxt = al->next;
    g_free(al);
    al = nxt;
  }
}

/*--------------------------------------------------------------
 * arclist_sort_with_data() & friends
 *  + adapted from code in GLib glib/gslist.c:
 *    Copyright (C) 1995-1997  Peter Mattis, Spencer Kimball and Josh MacDonald,
 *    Modified by the GLib Team and others 1997-2000.
 */
static
gfsmArcList *gfsm_arclist_sort_merge (gfsmArcList   *l1, 
				      gfsmArcList   *l2,
				      GFunc     compare_func,
				      gpointer  user_data)
{
  gfsmArcList list, *l;
  gint cmp;

  l=&list;

  while (l1 && l2)
    {
      cmp = ((GCompareDataFunc) compare_func) (&(l1->arc), (&l2->arc), user_data);

      if (cmp <= 0)
        {
	  l=l->next=l1;
	  l1=l1->next;
        } 
      else 
	{
	  l=l->next=l2;
	  l2=l2->next;
        }
    }
  l->next= l1 ? l1 : l2;
  
  return list.next;
}


gfsmArcList *gfsm_arclist_sort_real (gfsmArcList   *list,
				     GFunc     compare_func,
				     gpointer  user_data)
{
  gfsmArcList *l1, *l2;

  if (!list) 
    return NULL;
  if (!list->next) 
    return list;

  l1 = list; 
  l2 = list->next;

  while ((l2 = l2->next) != NULL)
    {
      if ((l2 = l2->next) == NULL) 
	break;
      l1=l1->next;
    }
  l2 = l1->next; 
  l1->next = NULL;

  return gfsm_arclist_sort_merge (gfsm_arclist_sort_real (list, compare_func, user_data),
				  gfsm_arclist_sort_real (l2, compare_func, user_data),
				  compare_func,
				  user_data);
}


/*--------------------------------------------------------------
 * arclist_remove_node()
 *  + adapted from _g_slist_remove_link()
 */
gfsmArcList *gfsm_arclist_remove_node(gfsmArcList *al, gfsmArcList *nod)
{
  gfsmArcList *tmp;
  gfsmArcList *prev;

  prev = NULL;
  tmp = al;

  while (tmp)
    {
      if (tmp == nod)
	{
	  if (prev)
	    prev->next = tmp->next;
	  if (al == tmp)
	    al = al->next;

	  tmp->next = NULL;
	  break;
	}

      prev = tmp;
      tmp = tmp->next;
    }

  return al;
}


/*--------------------------------------------------------------
 * arclist_reverse()
 */
gfsmArcList* gfsm_arclist_reverse(gfsmArcList *al)
{
  gfsmArcList *prev=NULL;
  while (al) {
    gfsmArcList *next = al->next;
    al->next = prev;
    prev     = al;
    al       = next;
  }
  return prev;
}
