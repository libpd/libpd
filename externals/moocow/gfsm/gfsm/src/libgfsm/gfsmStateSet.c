
/*=============================================================================*\
 * File: gfsmStateSet.c
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

#include <gfsmStateSet.h>
#include <gfsmArcIter.h>

//-- no-inline definitions
#ifndef GFSM_INLINE_ENABLED
# include <gfsmStateSet.hi>
#endif

/*======================================================================
 * Constants
 */
const guint gfsmStateSetDefaultSize = 2;

/*======================================================================
 * Methods: Constructors etc.
 */
//-- inlined

/*======================================================================
 * Methods: Accessors
 */

//--------------------------------------------------------------
// find()
gfsmStateSetIter gfsm_stateset_find(gfsmStateSet *sset, gfsmStateId id)
{
  gfsmStateSetIter sseti;
  gfsmStateId      iid;
  for (sseti = gfsm_stateset_iter_begin(sset);
       (iid=gfsm_stateset_iter_id(sseti)) != gfsmNoState;
       sseti = gfsm_stateset_iter_next(sset,sseti))
    {
      if (id == iid) return sseti;
      else if (id < iid) return NULL;
    }
  return NULL;
}

//--------------------------------------------------------------
// insert()
gboolean gfsm_stateset_insert(gfsmStateSet *sset, gfsmStateId id)
{
  guint i;
  for (i = 0; i < sset->len && id > g_array_index(sset,gfsmStateId,i); i++) ;

  if (i == sset->len) {
    g_array_append_val(sset,id);
  }
  else if (id == g_array_index(sset,gfsmStateId,i)) {
    return TRUE;
  }
  else {
    g_array_insert_val(sset,i,id);
  }
  return FALSE;
}

//--------------------------------------------------------------
// union()
gfsmStateSet *gfsm_stateset_union(gfsmStateSet *sset1, gfsmStateSet *sset2)
{
  guint i1=0, i2;
  for (i2=0; i2 < sset2->len; i2++) {
    gfsmStateId id = g_array_index(sset2,gfsmStateId,i2);
    for (; i1 < sset1->len && id > g_array_index(sset1,gfsmStateId,i1); i1++) ;

    if (i1 == sset1->len) g_array_append_val(sset1,id);
    else if (id == g_array_index(sset1,gfsmStateId,i1)) continue;
    else g_array_insert_val(sset1,i1,id);
  }
  return sset1;
}

//--------------------------------------------------------------
// remove()
gboolean gfsm_stateset_remove(gfsmStateSet *sset, gfsmStateId id) {
  guint i;
  for (i = 0; i < sset->len && id > g_array_index(sset,gfsmStateId,i); i++) ;
  if (i != sset->len && id == g_array_index(sset,gfsmStateId,i)) {
    g_array_remove_index(sset,i);
    return TRUE;
  }
  return FALSE;
}


//--------------------------------------------------------------
// equal()
gboolean gfsm_stateset_equal(gfsmStateSet *sset1, gfsmStateSet *sset2)
{
  guint i;
  if (sset1->len != sset2->len) return FALSE;
  for (i=0; i < sset1->len; i++) {
    if (g_array_index(sset1,gfsmStateId,i) != g_array_index(sset2,gfsmStateId,i)) return FALSE;
  }
  return TRUE;
}

//--------------------------------------------------------------
// foreach()
void gfsm_stateset_foreach(gfsmStateSet *sset, gfsmStateSetForeachFunc func, gpointer data)
{
  guint i;
  for (i = 0; i < sset->len; i++) {
    if ((*func)(g_array_index(sset,gfsmStateId,i), data)) break;
  }
}


/*======================================================================
 * Methods: iterators
 */
//-- inlined

/*======================================================================
 * Methods: Utilities
 */

/*--------------------------------------------------------------
 * hash()
 */
guint gfsm_stateset_hash(gfsmStateSet *sset)
{
  guint hv = 0;
  gfsmStateSetIter sseti;
  gfsmStateId      iid;
  for (sseti = gfsm_stateset_iter_begin(sset);
       (iid=gfsm_stateset_iter_id(sseti)) != gfsmNoState;
       sseti = gfsm_stateset_iter_next(sset,sseti))
    {
      hv += 5*iid;
    }
  return hv;
}


/*======================================================================
 * Methods: Automaton access
 */

//--------------------------------------------------------------
// has_final_state()
gboolean gfsm_stateset_has_final_state(gfsmStateSet *sset, gfsmAutomaton *fsm)
{
  guint i;
  for (i = 0; i < sset->len; i++) {
    if (gfsm_automaton_is_final_state(fsm, g_array_index(sset,gfsmStateId,i))) return TRUE;
  }
  return FALSE;
}

//--------------------------------------------------------------
// lookup_final_weight()
gboolean gfsm_stateset_lookup_final_weight(gfsmStateSet *sset, gfsmAutomaton *fsm, gfsmWeight *wp)
{
  guint i;
  gboolean rc=FALSE;
  *wp = fsm->sr->one;
  gfsmWeight w;
  for (i = 0; i < sset->len; i++) {
    gfsmStateId id = g_array_index(sset,gfsmStateId,i);
    if (gfsm_automaton_lookup_final(fsm,id,&w)) {
      *wp = gfsm_sr_plus(fsm->sr, *wp, w);
      rc  = TRUE;
    }
  }
  return rc;
}

/*--------------------------------------------------------------
 * populate()
 */
void gfsm_stateset_populate(gfsmStateSet *sset,
			    gfsmAutomaton *fsm,
			    gfsmStateId     id,
			    gfsmLabelVal    lo,
			    gfsmLabelVal    hi)
{
  gfsmArcIter ai;
  if (gfsm_stateset_insert(sset,id)) return;
  
  for (gfsm_arciter_open(&ai,fsm,id), gfsm_arciter_seek_both(&ai,lo,hi);
       gfsm_arciter_ok(&ai);
       gfsm_arciter_next(&ai), gfsm_arciter_seek_both(&ai,lo,hi))
    {
      gfsm_stateset_populate(sset,fsm,gfsm_arciter_arc(&ai)->target,lo,hi);
    }
  gfsm_arciter_close(&ai);
}

/*--------------------------------------------------------------
 * has_final_state()
 */
//--inlined

/*--------------------------------------------------------------
 * lookup_final_weight()
 */
//--inlined
