
/*=============================================================================*\
 * File: gfsmTrie.c
 * Author: Bryan Jurish <moocow@ling.uni-potsdam.de>
 * Description: finite state machine library
 *
 * Copyright (c) 2005-2006 Bryan Jurish.
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

#include <gfsmTrie.h>
#include <gfsmArcIter.h>

/*======================================================================
 * Constants
 */

const gfsmAutomatonFlags gfsmTrieDefaultFlags =
  {
    TRUE,         //-- is_transducer:1
    TRUE,         //-- is_weighted:1
    TRUE,         //-- is_deterministic:1
    gfsmASMLower, //-- sort_mode:24
    0             //-- unused:5
  };

const gfsmSRType gfsmTrieDefaultSRType = gfsmSRTReal;


/*======================================================================
 * Methods: ensure path (adding weight)
 */

//--------------------------------------------------------------
gfsmStateId gfsm_trie_add_path(gfsmTrie        *trie,
			       gfsmLabelVector *lo,
			       gfsmLabelVector *hi,
			       gfsmWeight       w)
{
  return gfsm_trie_add_path_full(trie,lo,hi,w,TRUE,TRUE,TRUE,NULL);
}


//--------------------------------------------------------------
gfsmStateId gfsm_trie_add_path_full(gfsmTrie          *trie,
				    gfsmLabelVector   *lo,
				    gfsmLabelVector   *hi,
				    gfsmWeight         w,
				    gboolean           add_to_arcs,
				    gboolean           add_to_state_final,
				    gboolean           add_to_path_final,
				    gfsmStateIdVector *path_states
				    )
{
  gfsmStateId  qid;
  guint i;

  //-- ensure trie has a root state
  if (!gfsm_automaton_has_state(trie,trie->root_id)) {
    trie->root_id = gfsm_automaton_add_state(trie);
  }
  qid = trie->root_id;

  //-- initialize state-path, if specified
  if (path_states) {
    g_ptr_array_set_size(path_states, (lo ? lo->len : 0) + (hi ? hi->len : 0));
    path_states->len = 0;
    g_ptr_array_add(path_states, GUINT_TO_POINTER(qid));
  }

  //-- add lower path
  for (i=0; lo && i < lo->len; i++) {
    if (add_to_state_final) {
      gfsm_automaton_set_final_state_full(trie, qid, TRUE,
					  gfsm_sr_plus(trie->sr, w, gfsm_automaton_get_final_weight(trie, qid)));
    }
    qid = gfsm_trie_get_arc_lower(trie, qid, ((gfsmLabelVal)GPOINTER_TO_UINT(g_ptr_array_index(lo,i))), w, add_to_arcs);
    if (path_states) g_ptr_array_add(path_states, GUINT_TO_POINTER(qid));
  }

  //-- add upper path
  for (i=0; hi && i < hi->len; i++) {
    if (add_to_state_final) {
      gfsm_automaton_set_final_state_full(trie, qid, TRUE,
					  gfsm_sr_plus(trie->sr, w, gfsm_automaton_get_final_weight(trie, qid)));
    }
    qid = gfsm_trie_get_arc_upper(trie, qid, ((gfsmLabelVal)GPOINTER_TO_UINT(g_ptr_array_index(hi,i))), w, add_to_arcs);
    if (path_states) g_ptr_array_add(path_states, GUINT_TO_POINTER(qid));
  }

  //-- add final epsilon-arc
  //qid = gfsm_trie_get_arc_both(trie, qid, gfsmEpsilon, gfsmEpsilon, w, add_to_arcs);
  //if (path_states) g_ptr_array_add(path_states,qid);

  if (add_to_state_final || add_to_path_final) {
    gfsm_automaton_set_final_state_full(trie, qid, TRUE,
					gfsm_sr_plus(trie->sr, w, gfsm_automaton_get_final_weight(trie, qid)));
  } else {
    gfsm_automaton_set_final_state(trie,qid,TRUE);
  }

  return qid;
}

/*======================================================================
 * Methods: find prefix
 */
gfsmStateId gfsm_trie_find_prefix(gfsmTrie          *trie,
				  gfsmLabelVector   *lo,
				  gfsmLabelVector   *hi,
				  guint             *lo_i,
				  guint             *hi_i,
				  gfsmWeight        *w_last,
				  gfsmStateIdVector *path_states
				  )
{
  gfsmStateId qid = trie->root_id;
  gfsmWeight fw, w = gfsm_sr_zero(trie->sr);
  guint i, j=0;
  gfsmArc *a;

  //-- initialize state-path, if specified
  if (path_states) {
    g_ptr_array_set_size(path_states, (lo ? lo->len : 0) + (hi ? hi->len : 0));
    path_states->len = 0;
    g_ptr_array_add(path_states, GUINT_TO_POINTER(qid));
  }

  //-- find lower path
  for (i=0; lo && i < lo->len; i++) {
    if ( !(a=gfsm_trie_find_arc_lower(trie, qid, ((gfsmLabelVal)GPOINTER_TO_UINT(g_ptr_array_index(lo,i))))) )
      break;

    qid = a->target;
    w = a->weight;
    if (path_states) g_ptr_array_add(path_states, GUINT_TO_POINTER(qid));
  }

  //-- find upper path
  if (i==lo->len) {
    for (j=0; hi && j < hi->len; j++) {
      if ( !(a = gfsm_trie_find_arc_upper(trie, qid, ((gfsmLabelVal)GPOINTER_TO_UINT(g_ptr_array_index(hi,j))))) )
	break;
      
      qid = a->target;
      w = a->weight;
      if (path_states) g_ptr_array_add(path_states, GUINT_TO_POINTER(qid));
    }

    //-- final state?
    if (j==hi->len && gfsm_automaton_lookup_final(trie, qid, &fw))
      w = fw;
  }

  //-- output variables
  if (lo_i)   *lo_i = i;
  if (hi_i)   *hi_i = j;
  if (w_last) *w_last = w;

  return qid;
}

/*======================================================================
 * Methods: find arcs
 */

//--------------------------------------------------------------
gfsmArc* gfsm_trie_find_arc_lower(gfsmTrie *trie, gfsmStateId qid, gfsmLabelVal lab)
{
  gfsmArcIter ai;
  gfsmArc     *a=NULL;
  for (gfsm_arciter_open(&ai, trie, qid), gfsm_arciter_seek_lower(&ai, lab);
       gfsm_arciter_ok(&ai); 
       gfsm_arciter_next(&ai), gfsm_arciter_seek_lower(&ai,lab))
    {
      a = gfsm_arciter_arc(&ai);
      break;
    }
  gfsm_arciter_close(&ai);
  return a;
}

//--------------------------------------------------------------
gfsmArc* gfsm_trie_find_arc_upper(gfsmTrie *trie, gfsmStateId qid, gfsmLabelVal lab)
{
  gfsmArcIter ai;
  gfsmArc     *a=NULL;
  for (gfsm_arciter_open(&ai, trie, qid), gfsm_arciter_seek_upper(&ai, lab);
       gfsm_arciter_ok(&ai); 
       gfsm_arciter_next(&ai), gfsm_arciter_seek_upper(&ai,lab))
    {
      a = gfsm_arciter_arc(&ai);
      break;
    }
  gfsm_arciter_close(&ai);
  return a;
}

//--------------------------------------------------------------
gfsmArc* gfsm_trie_find_arc_both(gfsmTrie *trie, gfsmStateId qid, gfsmLabelVal lo, gfsmLabelVal hi)
{
  gfsmArcIter ai;
  gfsmArc     *a=NULL;
  for (gfsm_arciter_open(&ai,trie,qid), gfsm_arciter_seek_both(&ai,lo,hi);
       gfsm_arciter_ok(&ai); 
       gfsm_arciter_next(&ai), gfsm_arciter_seek_both(&ai,lo,hi))
    {
      a = gfsm_arciter_arc(&ai);
      break;
    }
  gfsm_arciter_close(&ai);
  return a;
}


/*======================================================================
 * Methods: find or insert arcs
 */

//--------------------------------------------------------------
gfsmStateId gfsm_trie_get_arc_lower(gfsmTrie *trie, gfsmStateId qid, gfsmLabelVal lab, gfsmWeight w, gboolean add_weight)
{
  gfsmArc *a=gfsm_trie_find_arc_lower(trie,qid,lab);

  if (a==NULL) {
    gfsmStateId qid2 = gfsm_automaton_add_state(trie);
    gfsm_automaton_add_arc(trie,qid,qid2,lab,gfsmEpsilon, add_weight ? w : trie->sr->zero);
    return qid2;
  }

  //-- found an existing arc
  if (add_weight) a->weight = gfsm_sr_plus(trie->sr, a->weight, w);
  return a->target;
}

//--------------------------------------------------------------
gfsmStateId gfsm_trie_get_arc_upper(gfsmTrie *trie, gfsmStateId qid, gfsmLabelVal lab, gfsmWeight w, gboolean add_weight)
{
  gfsmArc *a=gfsm_trie_find_arc_upper(trie,qid,lab);

  if (a==NULL) {
    gfsmStateId qid2 = gfsm_automaton_add_state(trie);
    gfsm_automaton_add_arc(trie,qid,qid2,gfsmEpsilon,lab, add_weight ? w : trie->sr->zero);
    //trie->flags.is_deterministic = TRUE; //-- HACK
    return qid2;
  }

  //-- found an existing arc
  if (add_weight) a->weight = gfsm_sr_plus(trie->sr, a->weight, w);
  return a->target;
}

//--------------------------------------------------------------
gfsmStateId gfsm_trie_get_arc_both(gfsmTrie *trie, gfsmStateId qid, gfsmLabelVal lo, gfsmLabelVal hi, gfsmWeight w, gboolean add_weight)
{
  gfsmArc *a=gfsm_trie_find_arc_both(trie,qid,lo,hi);

  if (a==NULL) {
    gfsmStateId qid2 = gfsm_automaton_add_state(trie);
    gfsm_automaton_add_arc(trie,qid,qid2,lo,hi, add_weight ? w : trie->sr->zero);
    //trie->flags.is_deterministic = TRUE; //-- HACK
    return qid2;
  }

  //-- found an existing arc
  if (add_weight) a->weight = gfsm_sr_plus(trie->sr, a->weight, w);
  return a->target;
}

