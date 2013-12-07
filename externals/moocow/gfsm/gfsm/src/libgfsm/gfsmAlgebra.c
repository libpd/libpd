
/*=============================================================================*\
 * File: gfsmAlgebra.c
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

#include <glib.h>
#include <gfsmAlgebra.h>
#include <gfsmAssert.h>
#include <gfsmArcIter.h>
#include <gfsmStateSet.h>
#include <gfsmEnum.h>
#include <gfsmUtils.h>
#include <gfsmCompound.h>

/*======================================================================
 * Methods: algebra
 */

/*--------------------------------------------------------------
 * closure_final_func()
 *  + called for each final @id of @fsm during closure(@fsm)
 */
static
gboolean gfsm_automaton_closure_final_func_(gfsmStateId id, gpointer pw, gfsmAutomaton *fsm)
{
  gfsmWeight w = gfsm_ptr2weight(pw);
  if (id != fsm->root_id)
    gfsm_automaton_add_arc(fsm, id, fsm->root_id, gfsmEpsilon, gfsmEpsilon, w);
  return FALSE;
}


/*--------------------------------------------------------------
 * closure()
 */
gfsmAutomaton *gfsm_automaton_closure(gfsmAutomaton *fsm, gboolean is_plus)
{
  //-- sanity check(s)
  if (!fsm || fsm->root_id == gfsmNoState) return fsm;

  //-- add epsilon arcs from old final states to translated new root
  gfsm_automaton_finals_foreach(fsm, (GTraverseFunc)gfsm_automaton_closure_final_func_, fsm);

  //-- reflexive+transitive or reflexive?
  if (!is_plus) gfsm_automaton_optional(fsm);

  return fsm;
}


/*--------------------------------------------------------------
 * n_closure()
 */
gfsmAutomaton *gfsm_automaton_n_closure(gfsmAutomaton *fsm, guint n)
{
  //-- sanity check(s)
  if (!fsm || fsm->root_id == gfsmNoState) return fsm;

  //-- check for simple closures
  if (n == 0)      return gfsm_automaton_closure(fsm, FALSE);
  else if (n == 1) return gfsm_automaton_closure(fsm, TRUE);
  else {
    gfsm_automaton_n_concat(fsm, fsm, n-1);
  }

  return gfsm_automaton_closure(fsm, TRUE);
}


/*--------------------------------------------------------------
 * complement()
 */
gfsmAutomaton *gfsm_automaton_complement(gfsmAutomaton *fsm)
{
  gfsmAlphabet *alph = gfsm_identity_alphabet_new();
  gfsm_automaton_get_alphabet(fsm, gfsmLSLower, alph);
  gfsm_automaton_complement_full(fsm,alph);
  gfsm_alphabet_free(alph);
  return fsm;
}

/*--------------------------------------------------------------
 * complement_full()
 */
gfsmAutomaton *gfsm_automaton_complement_full(gfsmAutomaton *fsm, gfsmAlphabet *alph)
{
  gfsmStateId id, sink_id;
  gfsm_automaton_complete(fsm, alph, &sink_id);

  //-- flip final states (no weights here)
  for (id = 0; id < fsm->states->len; id++) {
    gfsmState  *s = gfsm_automaton_find_state(fsm,id);
    if (!s || !s->is_valid) continue;
    gfsm_automaton_set_final_state(fsm, id, !s->is_final);
  }

  return fsm;
}

/*--------------------------------------------------------------
 * complete()
 */
gfsmAutomaton *gfsm_automaton_complete(gfsmAutomaton *fsm, gfsmAlphabet *alph, gfsmStateId *sinkp)
{
  gfsmStateId  id, sinkid;
  GPtrArray    *alabels;

  if (!fsm->flags.is_deterministic) fsm = gfsm_automaton_determinize(fsm);
  if (gfsm_acmask_nth(fsm->flags.sort_mode,0) != gfsmACLower) {
    gfsm_automaton_arcsort(fsm,gfsmACLower);
  }
  //-- avoid "smart" arc insertion
  fsm->flags.sort_mode = gfsmASMNone;

  //-- add sink-id
  sinkid = gfsm_automaton_add_state(fsm);
  if (sinkp) *sinkp = sinkid;

  //-- get alphabet label-vector
  alabels = g_ptr_array_sized_new(gfsm_alphabet_size(alph));
  gfsm_alphabet_labels_to_array(alph,alabels);

  for (id = 0; id < fsm->states->len; id++) {
    gfsmState    *s = gfsm_automaton_find_state(fsm,id);
    gfsmArcList *al;
    gfsmArc      *a;
    guint       labi;
    if (!s || !s->is_valid) continue;

    al = s->arcs;
    a  = gfsm_arclist_arc(al);
    for (labi=0; labi < alabels->len; ) {
      gfsmLabelVal lab = (gfsmLabelVal)GPOINTER_TO_UINT(g_ptr_array_index(alabels,labi));

      if (lab==gfsmEpsilon) {
	++labi;
      }
      else if (!a || a->lower > lab) {
	//-- no arc for this label: route it to sink
	gfsm_automaton_add_arc(fsm, id, sinkid, lab, lab, fsm->sr->one);
	++labi;
      }
      else if (a->lower == lab) {
	++labi;
      }
      else {
	while (al != NULL && a->lower < lab) {
	  al = al->next;
	  a  = gfsm_arclist_arc(al);
	}
      }
    }
  }

  //-- mark fsm as (still) deterministic
  fsm->flags.is_deterministic = TRUE;

  //-- cleanup
  //g_array_free(alabels,TRUE);
  g_ptr_array_free(alabels,TRUE);

  return fsm;
}


/*--------------------------------------------------------------
 * compose()
 */
gfsmAutomaton *gfsm_automaton_compose(gfsmAutomaton *fsm1, gfsmAutomaton *fsm2)
{
  gfsmAutomaton *fsm = gfsm_automaton_compose_full(fsm1,fsm2, NULL,NULL);
  gfsm_automaton_swap(fsm1,fsm);
  gfsm_automaton_free(fsm);
  return fsm1;
}

/*--------------------------------------------------------------
 * compose_visit_()
 */
//#define GFSM_DEBUG_COMPOSE_VISIT 1
#ifdef GFSM_DEBUG_COMPOSE_VISIT
# include <stdio.h>
#endif
gfsmStateId gfsm_automaton_compose_visit_(gfsmComposeState   sp,
					  gfsmAutomaton     *fsm1,
					  gfsmAutomaton     *fsm2,
					  gfsmAutomaton     *fsm,
					  gfsmComposeStateEnum *spenum,
					  gfsmComposeFlags   flags)
{
  gfsmState   *q1, *q2;
  gfsmStateId qid = gfsm_enum_lookup(spenum,&sp);
  gfsmStateId qid2;
  gfsmArcList *al1, *al2, *ai1, *ai2;
  gfsmArcList *ai1_noneps, *ai2_noneps, *ai2_continue;
  gfsmArc     *a1,*a2;

#ifdef GFSM_DEBUG_COMPOSE_VISIT
  fprintf(stderr, "compose(): visit : (q%u,f%u,q%u) => q%d\n", sp.id1, sp.idf, sp.id2,
	  (int)(qid==gfsmEnumNone ? -1 : qid));
#endif

  //-- ignore already-visited states
  if (qid != gfsmEnumNone) return qid;

  //-- get state pointers for input automata
  q1 = gfsm_automaton_find_state(fsm1,sp.id1);
  q2 = gfsm_automaton_find_state(fsm2,sp.id2);

  //-- sanity check
  if ( !(q1 && q2 && q1->is_valid && q2->is_valid) ) {
#ifdef GFSM_DEBUG_COMPOSE_VISIT
      fprintf(stderr, "compose(): BAD   : (q%u,f%u,q%u)     XXXXX\n", sp.id1, sp.idf, sp.id2);
#endif
    return gfsmNoState;
  }

  //-- insert new state into output automaton
  qid = gfsm_automaton_add_state(fsm);
  gfsm_enum_insert_full(spenum,&sp,qid);

#ifdef GFSM_DEBUG_COMPOSE_VISIT
      fprintf(stderr, "compose(): CREATE: (q%u,f%u,q%u) => q%u ***\n", sp.id1, sp.idf, sp.id2, qid);
#endif

  //-- check for final states
  if (q1->is_final && q2->is_final) {
    gfsm_automaton_set_final_state_full(fsm,qid,TRUE,
					gfsm_sr_times(fsm->sr,
						      gfsm_automaton_get_final_weight(fsm1,sp.id1),
						      gfsm_automaton_get_final_weight(fsm2,sp.id2)));
  }

  //-------------------------------------------
  // recurse on outgoing arcs

  //--------------------------------
  // recurse: arcs: sort

  //-- arcs: sort arclists: fsm1
  if (flags&gfsmCFEfsm1NeedsArcSort) {
    gfsmArcCompData sortdata = { gfsmACUpper,NULL,NULL,NULL };
    al1 = gfsm_arclist_sort(gfsm_arclist_clone(q1->arcs), &sortdata);
  }
  else { al1 = q1->arcs; }

  //-- arcs: sort arclists: fsm2
  if (flags&gfsmCFEfsm2NeedsArcSort) {
    gfsmArcCompData sortdata = { gfsmACLower,NULL,NULL,NULL };
    al2 = gfsm_arclist_sort(gfsm_arclist_clone(q2->arcs), &sortdata);
  }
  else { al2 = q2->arcs; }

  //--------------------------------
  // recusrse: arcs: handle epsilons
  for (ai1_noneps=al1; ai1_noneps!=NULL && ai1_noneps->arc.upper==gfsmEpsilon; ai1_noneps=ai1_noneps->next) {;}
  for (ai2_noneps=al2; ai2_noneps!=NULL && ai2_noneps->arc.lower==gfsmEpsilon; ai2_noneps=ai2_noneps->next) {;}

  //-- (eps,NULL): case fsm1(q1 --a:eps(~eps2)--> q1b), filter:({0,2} --eps2:eps2--> 2), fsm2(q2 --(NULL~eps2:eps)--> q2)
  if (sp.idf != 1) {
    for (ai1=al1; ai1!=ai1_noneps; ai1=ai1->next) {
      a1   = &(ai1->arc);
#ifdef GFSM_DEBUG_COMPOSE_VISIT
      fprintf(stderr,
	      "compose(): MATCH[e,NULL]: (q%u --%d:eps(e2)--> q%u) ~ ({0,2}--(e2:e2)-->2) ~ (q%u --(NULL~e2:eps)--> q%u) ***\n",
	      sp.id1, a1->lower, a1->target,
	      sp.id2, sp.id2);
#endif
      qid2 = gfsm_automaton_compose_visit_((gfsmComposeState){a1->target, sp.id2, 2}, fsm1,fsm2,fsm, spenum,flags);
      if (qid2 != gfsmNoState)
	gfsm_automaton_add_arc(fsm, qid, qid2, a1->lower, gfsmEpsilon, a1->weight);
    }
  }
  //-- (NULL,eps): case fsm1(q1 --(NULL~eps:eps1)--> q1), filter:({0,1} --eps1:eps1--> 1), fsm2(q2 --eps(~eps1):b--> q2b)
  if (sp.idf != 2) {
    for (ai2=al2; ai2!=ai2_noneps; ai2=ai2->next) {
      a2   = &(ai2->arc);
#ifdef GFSM_DEBUG_COMPOSE_VISIT
      fprintf(stderr,
	      "compose(): MATHC[NULL,e]: (q%u --(NULL~eps:e1)--> q%u) ~ ({0,1}--(e1:e1)-->1) ~ (q%u --eps(e1):%d--> q%u) ***\n",
	      sp.id1, sp.id1,
	      sp.id2, a2->upper, a2->target);
#endif
      qid2 = gfsm_automaton_compose_visit_((gfsmComposeState){sp.id1, a2->target, 1}, fsm1,fsm2,fsm, spenum,flags);
      if (qid2 != gfsmNoState)
	gfsm_automaton_add_arc(fsm, qid, qid2, gfsmEpsilon, a2->upper, a2->weight);
    }
  }
  //-- (eps,eps): case fsm1(q1 --a:eps(~eps2)--> q1b), filter:({0} --eps2:eps1--> 0), fsm2(q2 --eps:b--> q2b)
  if (sp.idf == 0) {
    for (ai1=al1; ai1!=ai1_noneps; ai1=ai1->next) {
      a1 = &(ai1->arc);
      for (ai2=al2; ai2!=ai2_noneps; ai2=ai2->next) {
	a2   = &(ai2->arc);
#ifdef GFSM_DEBUG_COMPOSE_VISIT
	fprintf(stderr,
		"compose(): MATCH[e,e]: (q%u --%d:eps(e2)--> q%u) ~ ({0}--(e2:e1)-->0) ~ (q%u --eps(e1):%d--> q%u) ***\n",
		sp.id1, a1->lower, a1->target,
		sp.id2, a2->upper, a2->target);
#endif
	qid2 = gfsm_automaton_compose_visit_((gfsmComposeState){a1->target, a2->target, 0},
					     fsm1,fsm2,fsm, spenum,flags);
	if (qid2 != gfsmNoState)
	  gfsm_automaton_add_arc(fsm, qid, qid2, a1->lower, a2->upper,
				 gfsm_sr_times(fsm->sr, a1->weight, a2->weight));
      }
    }
  }

  //--------------------------------
  // recurse: arcs: non-eps: iterate
  for (ai1=ai1_noneps, ai2_continue=ai2_noneps; ai1!=NULL; ai1=ai1->next) {
    a1 = &(ai1->arc);

    for (ai2=ai2_continue; ai2!=NULL; ai2=ai2->next) {
      a2 = &(ai2->arc);

#ifdef GFSM_DEBUG_COMPOSE_VISIT
      fprintf(stderr,
	      "compose(): check[x,x]: (q%u --%d:%d--> q%u) ~ ({0,1,2}--(x:x)-->0) ~ (q%u --%d:%d--> q%u)\n",
	      sp.id1, a1->lower, a1->upper, a1->target,
	      sp.id2, a2->lower, a2->upper, a2->target);
#endif

      if      (a2->lower < a1->upper) { ai2_continue=ai2->next; continue; }
      else if (a2->lower > a1->upper) { break; }

#ifdef GFSM_DEBUG_COMPOSE_VISIT
      fprintf(stderr,
	      "compose(): MATCH[x,x]: (q%u --%d:%d--> q%u) ~ ({0,1,2}--(x:x)-->0) ~ (q%u --%d:%d--> q%u) ***\n",
	      sp.id1, a1->lower, a1->upper, a1->target,
	      sp.id2, a2->lower, a2->upper, a2->target);
#endif

      //-- non-eps: case fsm1:(q1 --a:b--> q1'), fsm2:(q2 --b:c-->  q2')
      qid2 = gfsm_automaton_compose_visit_((gfsmComposeState){a1->target,a2->target,0},
					   fsm1,fsm2,fsm, spenum,flags);
      if (qid2 != gfsmNoState)
	gfsm_automaton_add_arc(fsm, qid, qid2, a1->lower, a2->upper,
			       gfsm_sr_times(fsm1->sr, a1->weight, a2->weight));
    }
  }

  //-- maybe cleanup temporary arc-lists
  if (flags&gfsmCFEfsm1NeedsArcSort) gfsm_arclist_free(al1);
  if (flags&gfsmCFEfsm2NeedsArcSort) gfsm_arclist_free(al2);

  return qid;
}

/*--------------------------------------------------------------
 * compose_full()
 */
//#define GFSM_DEBUG_COMPOSE
#ifdef GFSM_DEBUG_COMPOSE
# include <gfsmAutomatonIO.h>
#endif
gfsmAutomaton *gfsm_automaton_compose_full(gfsmAutomaton *fsm1,
					   gfsmAutomaton *fsm2,
					   gfsmAutomaton *composition,
					   gfsmComposeStateEnum *spenum
					   )
{
  gboolean          spenum_is_temp;
  gfsmComposeState  rootpair;
  gfsmStateId       rootid;
  gfsmComposeFlags  flags = 0;
#ifdef GFSM_DEBUG_COMPOSE
  gfsmError *err =NULL;
#endif

  //-- setup: output fsm
  if (!composition) {
    composition=gfsm_automaton_shadow(fsm1);
  } else {
    gfsm_automaton_clear(composition);
    gfsm_automaton_copy_shallow(composition,fsm1);
  }
  composition->flags.sort_mode     = gfsmASMNone;
  composition->flags.is_transducer = 1;

  //-- setup: ComposeStateEnum
  if (spenum==NULL) {
    spenum_is_temp=TRUE;
    spenum = gfsm_compose_state_enum_new();
  } else {
    spenum_is_temp=FALSE;
    gfsm_enum_clear(spenum);
  }

  //-- setup: flags
  if (gfsm_acmask_nth(fsm1->flags.sort_mode,0) != gfsmACUpper) flags |= gfsmCFEfsm1NeedsArcSort;
  if (gfsm_acmask_nth(fsm2->flags.sort_mode,0) != gfsmACLower) flags |= gfsmCFEfsm2NeedsArcSort;

  //-- guts: recursively visit states depth-first from root
  rootpair.id1 = fsm1->root_id;
  rootpair.id2 = fsm2->root_id;
  rootpair.idf = 0;
  rootid = gfsm_automaton_compose_visit_(rootpair, fsm1, fsm2, composition, spenum, flags);

  //-- finalize: set new root state
  if (rootid != gfsmNoState) {
    gfsm_automaton_set_root(composition, rootid);
  } else {
    composition->root_id = gfsmNoState;
  }
  //-- cleanup
  if (spenum_is_temp) gfsm_enum_free(spenum);

  return composition;
}



/*--------------------------------------------------------------
 * concat_final_func()
 *  + called for each final @id of @fsm during concat(@fsm,@fsm2)
 *  + during the call @fsm1->root_id should be set to the translated root of @fsm2
 */
static
gboolean gfsm_automaton_concat_final_func_(gfsmStateId id, gpointer pw, gfsmAutomaton *fsm)
{
  gfsmWeight w = gfsm_ptr2weight(pw);
  gfsm_automaton_add_arc(fsm, id, fsm->root_id, gfsmEpsilon, gfsmEpsilon, w);
  gfsm_automaton_find_state(fsm,id)->is_final = FALSE;
  return FALSE;
}

/*--------------------------------------------------------------
 * concat_final_func_1()
 *  + called for singleton final @id of @fsm during concat(@fsm,@fsm2)
 *  + BAD if singleton final of @fsm has outgoing arcs!
 */
#if 0
struct gfsm_automaton_concat_1_final_data_ {
  gfsmStateId *rootxp;
  gfsmWeight  *weightp;
};
static
gboolean gfsm_automaton_concat_final_func_1_(gfsmStateId id,
					     gpointer pw,
					     struct gfsm_automaton_concat_1_final_data_ *data)
{
  *(data->rootxp) = id;
  *(data->weightp) = gfsm_ptr2weight(pw);
  return TRUE;
}
#endif

/*--------------------------------------------------------------
 * concat()
 */
gfsmAutomaton *gfsm_automaton_concat(gfsmAutomaton *fsm1, gfsmAutomaton *_fsm2)
{
  gfsmAutomaton *fsm2;
  gfsmStateId    offset;
  gfsmStateId    id2;
  gfsmStateId    size2;
  gfsmStateId    rootx;
  gfsmWeightMap *finals2 = NULL;

  //-- sanity check(s)
  if (!_fsm2 || _fsm2->root_id == gfsmNoState) return fsm1;
  if (_fsm2==fsm1) fsm2 = gfsm_automaton_clone(fsm1);
  else             fsm2 = _fsm2;

  if (fsm1->finals == fsm2->finals) {
    finals2 = gfsm_weightmap_new(gfsm_uint_compare);
    gfsm_weightmap_copy(finals2, fsm2->finals);
  }

  offset = fsm1->states->len;
  size2  = fsm2->states->len;
  gfsm_automaton_reserve(fsm1, offset + size2);

  //-- concatenative arcs
  if (fsm1->root_id != gfsmNoState) {
    //-- multiple final states: add epsilon arcs from old finals to mapped root2
    gfsmStateId root_tmp = fsm1->root_id;
    rootx                = fsm2->root_id+offset;
    fsm1->root_id        = rootx;
    gfsm_automaton_finals_foreach(fsm1, (GTraverseFunc)gfsm_automaton_concat_final_func_, fsm1);
    fsm1->root_id        = root_tmp;
  } else /*if (fsm2->root_id != gfsmNoState)*/ {
    fsm1->root_id = rootx = fsm2->root_id + offset;
  }
  gfsm_weightmap_clear(fsm1->finals);

  //-- adopt states from fsm2 into fsm1
  for (id2 = 0; id2 < size2; id2++) {
    gfsmStateId      id1;
    const gfsmState *s2;
    gfsmState       *s1;
    gfsmArcIter      ai;
    gfsmWeight       s2fw;

    s2 = gfsm_automaton_find_state_const(fsm2,id2);
    id1 = id2+offset;
    s1 = gfsm_automaton_find_state(fsm1, id1);

    //-- sanity check(s)
    if (!s1 || !s2 || !s2->is_valid) continue;

    //-- copy state
    gfsm_state_copy(s1,s2);

    //-- translate targets for adopted arcs
    for (gfsm_arciter_open_ptr(&ai,fsm1,s1); gfsm_arciter_ok(&ai); gfsm_arciter_next(&ai))
      {
	gfsmArc *a = gfsm_arciter_arc(&ai);
	a->target += offset;
      }

    //-- check for new final states: get weight & mark state is_final flag
    if ( (finals2 && gfsm_weightmap_lookup(finals2, GUINT_TO_POINTER(id2), &s2fw))
	 ||
	 (!finals2 && gfsm_weightmap_lookup(fsm2->finals, GUINT_TO_POINTER(id2), &s2fw)) )
      {
	s1->is_final = TRUE;
	gfsm_weightmap_insert(fsm1->finals, GUINT_TO_POINTER(id1), s2fw);
      }
  }

  //-- mark as unsorted
  fsm1->flags.sort_mode = gfsmASMNone;

  //-- cleanup
  if (finals2) gfsm_weightmap_free(finals2);
  if (fsm2 != _fsm2) gfsm_automaton_free(fsm2);

  return fsm1;
}

/*--------------------------------------------------------------
 * n_concat()
 */
gfsmAutomaton *gfsm_automaton_n_concat(gfsmAutomaton *fsm1, gfsmAutomaton *_fsm2, guint n)
{
  gfsmAutomaton *fsm2 = _fsm2;

  //-- sanity check(s)
  if (!_fsm2 || _fsm2->root_id == gfsmNoState) return fsm1;
  if (_fsm2==fsm1) fsm2 = gfsm_automaton_clone(fsm1);

  for ( ; n > 0; n--) { gfsm_automaton_concat(fsm1, fsm2); }

  if (fsm2 != _fsm2) gfsm_automaton_free(fsm2);

  return fsm1;
}


/*--------------------------------------------------------------
 * connect()
 */
gfsmAutomaton *gfsm_automaton_connect(gfsmAutomaton *fsm)
{
  gfsmBitVector *wanted;

  //-- sanity check
  if (!fsm || gfsm_automaton_n_states(fsm)==0) return fsm;

  wanted = gfsm_bitvector_sized_new(fsm->states->len);
  gfsm_automaton_connect_fw(fsm, wanted);

  gfsm_bitvector_zero(wanted);
  gfsm_automaton_connect_bw(fsm, NULL, wanted);

  gfsm_bitvector_free(wanted);
  return fsm;
}


/*--------------------------------------------------------------
 * connect_fw_visit_state()
 *  + marks all states on a path from (id) in (visited)
 */
void gfsm_connect_fw_visit_state(gfsmAutomaton *fsm,
				 gfsmStateId    id,
				 gfsmBitVector *visited)
{
  gfsmState *s;
  gfsmArcIter ai;

  //-- already visited
  if (gfsm_bitvector_get(visited,id)) return;

  s = gfsm_automaton_find_state(fsm,id);
  if (!s || !s->is_valid) return;                    //-- ignore invalid states

  //-- mark node as visited on this path
  gfsm_bitvector_set(visited,id,1);

  //-- visit targets of outgoing arcs
  for (gfsm_arciter_open_ptr(&ai,fsm,s); gfsm_arciter_ok(&ai); gfsm_arciter_next(&ai)) {
    gfsm_connect_fw_visit_state(fsm, gfsm_arciter_arc(&ai)->target, visited);
  }

  return;
}

/*--------------------------------------------------------------
 * connect_fw()
 */
gfsmAutomaton *gfsm_automaton_connect_fw(gfsmAutomaton *fsm, gfsmBitVector *visited)
{
  gboolean  visited_is_temp = FALSE;

  //-- sanity check
  if (!fsm || fsm->root_id == gfsmNoState)
    return gfsm_automaton_prune_states(fsm,NULL);

  //-- traversal record
  if (visited==NULL) {
    visited = gfsm_bitvector_sized_new(fsm->states->len);
    visited_is_temp = TRUE;
  }

  //-- traverse
  gfsm_connect_fw_visit_state(fsm, fsm->root_id, visited);
  gfsm_automaton_prune_states(fsm, visited);

  //-- cleanup
  if (visited_is_temp) gfsm_bitvector_free(visited);

  return fsm;
}

/*--------------------------------------------------------------
 * connect_bw(): final_foreach()
 */
struct gfsm_connect_bw_data_ {
  gfsmAutomaton *fsm;
  GPtrArray     *rarcs;
  gfsmBitVector *finalizable;
};

gboolean gfsm_connect_bw_visit_state(gfsmStateId id,
				     gpointer    pw,
				     struct      gfsm_connect_bw_data_ *data)
{
  GSList *rl;

  //-- already visited
  if (gfsm_bitvector_get(data->finalizable,id)     //-- already visited?
      || !gfsm_automaton_has_state(data->fsm, id)) //-- bad state?
    return FALSE;                                  //-----> continue traversal

  //-- mark state as finalizable
  gfsm_bitvector_set(data->finalizable,id,1);

  //-- visit sources of incoming arcs
  for (rl=g_ptr_array_index(data->rarcs,id); rl != NULL; rl=rl->next) {
    gfsmArc *arc = (gfsmArc*)rl->data;
    gfsm_connect_bw_visit_state(arc->source,pw,data);
  }

  return FALSE; //-- continue traversal
}

/*--------------------------------------------------------------
 * connect_bw()
 */
gfsmAutomaton *gfsm_automaton_connect_bw(gfsmAutomaton       *fsm,
					 gfsmReverseArcIndex *rarcs,
					 gfsmBitVector       *finalizable)
{
  gboolean rarcs_is_temp = FALSE;
  gboolean finalizable_is_temp = FALSE;
  struct gfsm_connect_bw_data_ data = {fsm,rarcs,finalizable};

  //-- sanity check(s)
  if (!fsm || gfsm_automaton_n_final_states(fsm)==0)
    return gfsm_automaton_prune_states(fsm,NULL);

  //-- reverse arc-index
  if (rarcs==NULL) {
    rarcs = data.rarcs = gfsm_automaton_reverse_arc_index(fsm,NULL);
    rarcs_is_temp = TRUE;
  }

  //-- traversal record
  if (finalizable==NULL) {
    finalizable = data.finalizable = gfsm_bitvector_sized_new(fsm->states->len);
    finalizable_is_temp = TRUE;
  }

  //-- traverse
  gfsm_automaton_finals_foreach(fsm, (GTraverseFunc)gfsm_connect_bw_visit_state,  &data);
  gfsm_automaton_prune_states(fsm, finalizable);

  //-- cleanup
  if (finalizable_is_temp) gfsm_bitvector_free(finalizable);
  if (rarcs_is_temp) gfsm_reverse_arc_index_free(rarcs,TRUE);

  return fsm;
}


/*--------------------------------------------------------------
 * prune_states()
 */
gfsmAutomaton *gfsm_automaton_prune_states(gfsmAutomaton *fsm, gfsmBitVector *wanted)
{
  gfsmStateId id, maxwanted=gfsmNoState;
  gfsmArcIter ai;

  for (id=0; id < fsm->states->len; id++) {
    if (!wanted || !gfsm_bitvector_get(wanted,id)) {
      //-- unwanted state: chuck it
      gfsm_automaton_remove_state(fsm,id);
    }
    else {
      maxwanted = id;
      //-- prune outgoing arcs to any unwanted states, too
      for (gfsm_arciter_open(&ai, fsm, id); gfsm_arciter_ok(&ai); ) {
	gfsmArc *arc = gfsm_arciter_arc(&ai);
	if (!wanted || !gfsm_bitvector_get(wanted,arc->target)) {
	  gfsm_arciter_remove(&ai);
	} else {
	  gfsm_arciter_next(&ai);
	}
      }
    }
  }

  //-- update number of states
  if (maxwanted != gfsmNoState) {
    g_array_set_size(fsm->states, maxwanted+1);
  } else {
    g_array_set_size(fsm->states, 0);
  }

  return fsm;
}

/*--------------------------------------------------------------
 * determinize_lp2ec_foreach_func_()
 */
typedef struct {
  gfsmAutomaton *nfa;
  gfsmAutomaton *dfa;
  gfsmStateId    dfa_src_id;
  gfsmEnum      *ec2id;
} gfsmLp2EcForeachData;

static
gboolean gfsm_determinize_lp2ec_foreach_func_(gfsmLabelPair         lp,
					      gfsmWeightedStateSet *wss,
					      gfsmLp2EcForeachData *data)
{
  gfsmStateId    ec2id_val;
  gpointer       ec2id_val_as_ptr;
  gfsmStateSet  *ec2id_key;

  if ( gfsm_enum_lookup_extended(data->ec2id,
				 wss->set,
				 (gpointer)(&ec2id_key),
				 (gpointer)(&ec2id_val_as_ptr)) )
    {
      //-- target node-set is already present: just add an arc in @dfa
      ec2id_val = GPOINTER_TO_UINT(ec2id_val_as_ptr);
      gfsm_automaton_add_arc(data->dfa,
			     data->dfa_src_id,
			     ec2id_val,
			     gfsm_labelpair_lower(lp),
			     gfsm_labelpair_upper(lp),
			     wss->weight);

      //-- ... and maybe free the embedded state set
      if (wss->set != ec2id_key) gfsm_stateset_free(wss->set);
      wss->set = NULL;
    }
  else
    {
      //-- image of equiv-class (wss->set) was not yet present: make a new one
      ec2id_val = gfsm_automaton_ensure_state(data->dfa,
					      gfsm_enum_insert(data->ec2id, wss->set));

      //-- ... add @dfa arc
      gfsm_automaton_add_arc(data->dfa,
			     data->dfa_src_id,
			     ec2id_val,
			     gfsm_labelpair_lower(lp),
			     gfsm_labelpair_upper(lp),
			     wss->weight);

      //-- ... and recurse
      gfsm_determinize_visit_state_(data->nfa,   data->dfa,
				    wss->set,    ec2id_val,
				    data->ec2id);
    }
  return FALSE;
}

/*--------------------------------------------------------------
 * determinize_visit_state_()
 */
void gfsm_determinize_visit_state_(gfsmAutomaton *nfa,    gfsmAutomaton *dfa,
				   gfsmStateSet  *nfa_ec, gfsmStateId    dfa_id,
				   gfsmEnum      *ec2id)
{
  GTree            *lp2ecw;  //-- maps label-pairs@nfa.src.ec => (eq-class@nfa.sink, sum(weight))
  gfsmStateSetIter  eci;
  gfsmStateId       ecid;
  gfsmLp2EcForeachData lp2ec_foreach_data;
  gfsmWeight           fw;

  //-- check for final state
  if (gfsm_stateset_lookup_final_weight(nfa_ec,nfa,&fw)) {
    gfsm_automaton_set_final_state_full(dfa, dfa_id, TRUE, fw);
  }

  //-- build label-pair => (sink-eqc, sum(weight)) mapping 'lp2ecw' for node-set nfa_ec
  lp2ecw = g_tree_new_full(((GCompareDataFunc)
			    gfsm_labelpair_compare_with_data), //-- key_comp_func
			   NULL, //-- key_comp_data
			   NULL, //-- key_free_func
			   (GDestroyNotify)g_free);            //-- val_free_func

  for (eci=gfsm_stateset_iter_begin(nfa_ec);
       (ecid=gfsm_stateset_iter_id(eci)) != gfsmNoState;
       eci=gfsm_stateset_iter_next(nfa_ec,eci))
    {
      gfsmArcIter  ai;
      for (gfsm_arciter_open(&ai, nfa, ecid); gfsm_arciter_ok(&ai); gfsm_arciter_next(&ai)) {
	gfsmArc *a = gfsm_arciter_arc(&ai);
	gfsmLabelPair lp;
	gfsmLabelPair *lp2ec_key;
	gfsmWeightedStateSet *lp2ec_val;

	//if (a->lower==gfsmEpsilon && a->upper==gfsmEpsilon) continue; //-- ignore eps arcs
	lp = gfsm_labelpair_new(a->lower, a->upper);

	//-- add equivalence class to local mapping
	if ( g_tree_lookup_extended(lp2ecw,
				    GUINT_TO_POINTER(lp),
				    (gpointer)(&lp2ec_key),
				    (gpointer)(&lp2ec_val)) )
	  {
	    //-- already present: compute union and add new arc's weight
	    gfsm_stateset_insert(lp2ec_val->set, a->target);
	    lp2ec_val->weight = gfsm_sr_plus(nfa->sr, lp2ec_val->weight, a->weight);
	  }
	else
	  {
	    //-- not yet present: insert new value
	    lp2ec_val         = g_new(gfsmWeightedStateSet,1);
	    lp2ec_val->set    = gfsm_stateset_new_singleton(a->target);
	    lp2ec_val->weight = a->weight;
	    g_tree_insert(lp2ecw, GUINT_TO_POINTER(lp), lp2ec_val);
	  }
      }

      //-- tmp-cleanup
      gfsm_arciter_close(&ai);
    }

  //-- stateset-iter (eci) cleanup
  //(none)

  //-- insert computed arcs into @dfa
  lp2ec_foreach_data.nfa         = nfa;
  lp2ec_foreach_data.dfa         = dfa;
  lp2ec_foreach_data.dfa_src_id  = dfa_id;
  lp2ec_foreach_data.ec2id       = ec2id;
  g_tree_foreach(lp2ecw,
		 (GTraverseFunc)gfsm_determinize_lp2ec_foreach_func_,
		 (gpointer)(&lp2ec_foreach_data));

  //-- cleanup
  g_tree_destroy(lp2ecw);
}


/*--------------------------------------------------------------
 * determinize()
 */
gfsmAutomaton *gfsm_automaton_determinize(gfsmAutomaton *nfa)
{
  if (!nfa->flags.is_deterministic) {
    gfsmAutomaton *dfa = gfsm_automaton_determinize_full(nfa,NULL);
    gfsm_automaton_swap(nfa,dfa);
    gfsm_automaton_free(dfa);
  }
  return nfa;
}

/*--------------------------------------------------------------
 * determinize_full()
 */
gfsmAutomaton *gfsm_automaton_determinize_full(gfsmAutomaton *nfa, gfsmAutomaton *dfa)
{
  gfsmEnum      *ec2id;  //-- (global) maps literal(equiv-class@nfa) => node-id@dfa
  gfsmStateSet  *nfa_ec; //-- (temp) equiv-class@nfa
  gfsmStateId    dfa_id; //-- (temp) id @ dfa

  //-- sanity check(s)
  if (!nfa) return NULL;
  else if (nfa->flags.is_deterministic) {
    if (dfa) gfsm_automaton_copy(dfa,nfa);
    else     dfa = gfsm_automaton_clone(nfa);
    return dfa;
  }

  //-- initialization: dfa
  if (!dfa) {
    dfa = gfsm_automaton_shadow(nfa);
  } else {
    gfsm_automaton_clear(dfa);
    gfsm_automaton_copy_shallow(dfa,nfa);
  }
  //-- avoid "smart" arc-insertion
  dfa->flags.sort_mode = gfsmASMNone;

  //-- initialization: ec2id
  ec2id = gfsm_enum_new_full(NULL /*(gfsmDupFunc)gfsm_stateset_clone*/ ,
			     (GHashFunc)gfsm_stateset_hash,
			     (GEqualFunc)gfsm_stateset_equal,
			     (GDestroyNotify)gfsm_stateset_free);

  //-- initialization: nfa_ec
  nfa_ec = gfsm_stateset_sized_new(32);
  gfsm_stateset_insert(nfa_ec, nfa->root_id);

  //-- set root in dfa
  dfa_id = gfsm_automaton_ensure_state(dfa, gfsm_enum_insert(ec2id, nfa_ec));
  gfsm_automaton_set_root(dfa, dfa_id);

  //-- guts: determinize recursively outwards from root node
  gfsm_determinize_visit_state_(nfa, dfa, nfa_ec, dfa_id, ec2id);

  //-- set flag in dfa
  dfa->flags.is_deterministic = TRUE;

  //-- cleanup
  //gfsm_stateset_free(nfa_ec); //-- this ought to be freed by gfsm_enum_free(ec2id)
  gfsm_enum_free(ec2id);

  return dfa;
}



/*--------------------------------------------------------------
 * difference()
 */
gfsmAutomaton *gfsm_automaton_difference(gfsmAutomaton *fsm1, gfsmAutomaton *fsm2)
{
  gfsmAutomaton *fsm = gfsm_automaton_difference_full(fsm1,fsm2,NULL);
  gfsm_automaton_swap(fsm1,fsm);
  gfsm_automaton_free(fsm);
  return fsm1;
}

/*--------------------------------------------------------------
 * difference_full()
 */
gfsmAutomaton *gfsm_automaton_difference_full(gfsmAutomaton *fsm1,
					      gfsmAutomaton *fsm2,
					      gfsmAutomaton *diff)
{
  gfsmAutomaton *not_fsm2;
  gfsmAlphabet *alph1 = gfsm_identity_alphabet_new();

  gfsm_automaton_get_alphabet(fsm1, gfsmLSLower, alph1);
  not_fsm2 = gfsm_automaton_clone(fsm2);
  gfsm_automaton_complement_full(not_fsm2, alph1);
  diff = gfsm_automaton_intersect_full(fsm1, not_fsm2, diff, NULL);

  gfsm_automaton_free(not_fsm2);
  gfsm_alphabet_free(alph1);

  return diff;
}



/*--------------------------------------------------------------
 * intersect()
 */
gfsmAutomaton *gfsm_automaton_intersect(gfsmAutomaton *fsm1, gfsmAutomaton *fsm2)
{
  gfsmAutomaton *fsm = gfsm_automaton_intersect_full(fsm1,fsm2,NULL,NULL);
  gfsm_automaton_swap(fsm1,fsm);
  gfsm_automaton_free(fsm);
  return fsm1;
}

/*--------------------------------------------------------------
 * intersect_full()
 */
gfsmAutomaton *gfsm_automaton_intersect_full(gfsmAutomaton *fsm1,
					     gfsmAutomaton *fsm2,
					     gfsmAutomaton *intersect,
					     gfsmStatePairEnum *spenum)
{
  gboolean      spenum_is_temp;
  gfsmStatePair rootpair;
  gfsmStateId   rootid;
  gfsmComposeFlags flags = 0;

  //-- setup: output fsm
  if (!intersect) {
    intersect=gfsm_automaton_shadow(fsm1);
  } else {
    gfsm_automaton_clear(intersect);
    gfsm_automaton_copy_shallow(intersect,fsm1);
  }
  //-- avoid "smart" arc-insertion
  intersect->flags.sort_mode     = gfsmASMNone;
  intersect->flags.is_transducer = 0;

  //-- setup: StatePairEnum
  if (spenum==NULL) {
    spenum_is_temp=TRUE;
    spenum = gfsm_statepair_enum_new();
  } else {
    spenum_is_temp=FALSE;
    gfsm_enum_clear(spenum);
  }

  //-- setup: flags
  if (gfsm_acmask_nth(fsm1->flags.sort_mode,0) != gfsmACLower) flags |= gfsmCFEfsm1NeedsArcSort;
  if (gfsm_acmask_nth(fsm2->flags.sort_mode,0) != gfsmACLower) flags |= gfsmCFEfsm2NeedsArcSort;

  //-- guts
  rootpair.id1 = fsm1->root_id;
  rootpair.id2 = fsm2->root_id;
  rootid = gfsm_automaton_intersect_visit_(rootpair, fsm1, fsm2, intersect, spenum,flags);

  //-- finalize: set root state
  if (rootid != gfsmNoState) {
    gfsm_automaton_set_root(intersect, rootid);
  } else {
    intersect->root_id = gfsmNoState;
  }

  //-- cleanup
  if (spenum_is_temp) gfsm_enum_free(spenum);

  return intersect;
}

/*--------------------------------------------------------------
 * intersect_visit()
 */
gfsmStateId gfsm_automaton_intersect_visit_(gfsmStatePair sp,
					    gfsmAutomaton *fsm1,
					    gfsmAutomaton *fsm2,
					    gfsmAutomaton *fsm,
					    gfsmStatePairEnum *spenum,
					    gfsmComposeFlags flags)
{
  gfsmState   *q1, *q2;
  gfsmStateId qid = gfsm_enum_lookup(spenum,&sp);
  gfsmStateId qid2;
  gfsmArcList *al1, *al2, *ai1, *ai2, *ai2eps;
  gfsmArc     *a1,*a2;

  //-- ignore already-visited states
  if (qid != gfsmEnumNone) return qid;

  //-- get state pointers for input automata
  q1 = gfsm_automaton_find_state(fsm1,sp.id1);
  q2 = gfsm_automaton_find_state(fsm2,sp.id2);

  //-- sanity check
  if ( !(q1 && q2 && q1->is_valid && q2->is_valid) ) return gfsmNoState;

  //-- insert new state into output automaton
  qid = gfsm_automaton_add_state(fsm);
  gfsm_enum_insert_full(spenum,&sp,qid);
  //q   = gfsm_automaton_get_state(fsm,qid);

  //-- check for final states
  if (q1->is_final && q2->is_final) {
    gfsm_automaton_set_final_state_full(fsm,qid,TRUE,
					gfsm_sr_times(fsm->sr,
						      gfsm_automaton_get_final_weight(fsm1,sp.id1),
						      gfsm_automaton_get_final_weight(fsm2,sp.id2)));
  }

  //-------------------------------------------
  // recurse on outgoing arcs

  //--------------------------------
  // recurse: arcs: sort

  //-- arcs: sort arclists: fsm1
  if (flags&gfsmCFEfsm1NeedsArcSort) {
    gfsmArcCompData sortdata = { (gfsmACLower|(gfsmACUpper<<gfsmACShift)),NULL,NULL,NULL };
    al1 = gfsm_arclist_sort(gfsm_arclist_clone(q1->arcs), &sortdata);
  }
  else { al1 = q1->arcs; }

  //-- arcs: sort arclists: fsm2
  if (flags&gfsmCFEfsm2NeedsArcSort) {
    gfsmArcCompData sortdata = { (gfsmACLower|(gfsmACUpper<<gfsmACShift)),NULL,NULL,NULL };
    al2 = gfsm_arclist_sort(gfsm_arclist_clone(q2->arcs), &sortdata);
  }
  else { al2 = q2->arcs; }

  //--------------------------------
  // recurse: arcs: iterate
  for (ai1=al1, ai2=al2; ai1 != NULL; ai1=ai1->next) {
    a1 = &(ai1->arc);
    if (a1->lower == gfsmEpsilon) {
      //-- handle epsilon arcs

      //-- eps: case fsm1:(q1 --eps-->  q1'), fsm2:(q2)
      qid2 = gfsm_automaton_intersect_visit_((gfsmStatePair){a1->target,sp.id2},
						fsm1, fsm2, fsm, spenum, flags);
      if (qid2 != gfsmNoState)
	gfsm_automaton_add_arc(fsm, qid, qid2, gfsmEpsilon, gfsmEpsilon, a1->weight);

      //-- eps: case fsm1:(q1 --eps-->  q1'), fsm2:(q2 --eps-->  q2')
      for (ai2eps=al2; ai2eps != NULL; ai2eps=ai2eps->next) {
	a2 = &(ai2eps->arc);
	if (a2->lower != gfsmEpsilon) break;

	qid2 = gfsm_automaton_intersect_visit_((gfsmStatePair){a1->target,a2->target},
						  fsm1, fsm2, fsm, spenum, flags);
	if (qid2 != gfsmNoState)
	  gfsm_automaton_add_arc(fsm, qid, qid2, gfsmEpsilon, gfsmEpsilon,
				 gfsm_sr_times(fsm1->sr, a1->weight, a2->weight));
      }
    }
    else {
      //-- handle non-epsilon arcs
      for ( ; ai2 != NULL; ai2=ai2->next) {
	a2 = &(ai2->arc);

	if      (a2->lower < a1->lower) continue;
	else if (a2->lower > a1->lower) break;

	qid2 = gfsm_automaton_intersect_visit_((gfsmStatePair){a1->target,a2->target},
						  fsm1, fsm2, fsm, spenum, flags);
	if (qid2 != gfsmNoState)
	  gfsm_automaton_add_arc(fsm, qid, qid2, a1->lower, a1->lower,
				 gfsm_sr_times(fsm1->sr, a1->weight, a2->weight));
      }
    }
  }

  //-- handle epsilon-arcs on fsm2
  for (ai2=al2 ; ai2 != NULL; ai2=ai2->next) {
    a2 = &(ai2->arc);
    if (a2->lower != gfsmEpsilon) break;

    //-- eps: case fsm1:(q1), fsm2:(q2 --eps-->  q2')
    qid2 = gfsm_automaton_intersect_visit_((gfsmStatePair){sp.id1,a2->target},
					   fsm1, fsm2, fsm, spenum, flags);
    if (qid2 != gfsmNoState)
      gfsm_automaton_add_arc(fsm, qid, qid2, gfsmEpsilon, gfsmEpsilon, a2->weight);
  }

  //-- cleanup
  if (flags&gfsmCFEfsm1NeedsArcSort) gfsm_arclist_free(al1);
  if (flags&gfsmCFEfsm2NeedsArcSort) gfsm_arclist_free(al2);

  return qid;
}


/*--------------------------------------------------------------
 * invert()
 */
gfsmAutomaton *gfsm_automaton_invert(gfsmAutomaton *fsm)
{
  gfsmStateId id;
  gfsmArcIter ai;
  gfsmArcCompMask acmask_old=fsm->flags.sort_mode, acmask_new=gfsmACNone;;
  gint aci;

  //-- invert arcs
  for (id=0; id < fsm->states->len; id++) {
    for (gfsm_arciter_open(&ai,fsm,id); gfsm_arciter_ok(&ai); gfsm_arciter_next(&ai)) {
      gfsmArc *a = gfsm_arciter_arc(&ai);
      gfsmLabelId tmp = a->lower;
      a->lower        = a->upper;
      a->upper        = tmp;
    }
  }

  //-- adjust sort mask (translate "lower"<->"upper")
  for (aci=0; aci < gfsmACMaxN; aci++) {
    gfsmArcCompMask cmp = gfsm_acmask_nth(acmask_old,aci);
    switch (cmp) {
    case gfsmACLower:  cmp=gfsmACUpper; break;
    case gfsmACUpper:  cmp=gfsmACLower; break;
    case gfsmACLowerR: cmp=gfsmACUpperR; break;
    case gfsmACUpperR: cmp=gfsmACLowerR; break;
    default: break;
    }
    acmask_new |= gfsm_acmask_new(cmp,aci);
  }
  fsm->flags.sort_mode = acmask_new;
  
  return fsm;
}

/*--------------------------------------------------------------
 * optional()
 */
gfsmAutomaton *gfsm_automaton_optional(gfsmAutomaton *fsm)
{
  if (!gfsm_automaton_is_final_state(fsm,fsm->root_id))
    gfsm_automaton_set_final_state_full(fsm,fsm->root_id,TRUE,fsm->sr->one);
  return fsm;
}

/*--------------------------------------------------------------
 * product() (single-destructive)
 */
gfsmAutomaton *gfsm_automaton_product(gfsmAutomaton *fsm1, gfsmAutomaton *fsm2)
{
  gfsmAutomaton *fsm2_tmp = gfsm_automaton_clone(fsm2);
  gfsm_automaton_product2(fsm1,fsm2_tmp);
  gfsm_automaton_free(fsm2_tmp);
  return fsm1;
}

/*--------------------------------------------------------------
 * _product() (dual-destructive)
 */
gfsmAutomaton *gfsm_automaton_product2(gfsmAutomaton *fsm1, gfsmAutomaton *fsm2)
{
  gfsmStateId  qid;
  gfsmState   *qp;
  gfsmArcIter  ai;
  gfsmArc     *a;

  //-- chuck out all upper-labels from fsm1
  for (qid=0; qid < fsm1->states->len; qid++) {
    qp = gfsm_automaton_find_state(fsm1,qid);
    if (!qp || !qp->is_valid) continue;
    for (gfsm_arciter_open_ptr(&ai,fsm1,qp); gfsm_arciter_ok(&ai); gfsm_arciter_next(&ai)) {
      a = gfsm_arciter_arc(&ai);
      a->upper = gfsmEpsilon;
    }
  }

  //-- chuck out all upper-labels from fsm2
  for (qid=0; qid < fsm2->states->len; qid++) {
    qp = gfsm_automaton_find_state(fsm2,qid);
    if (!qp || !qp->is_valid) continue;
    for (gfsm_arciter_open_ptr(&ai,fsm2,qp); gfsm_arciter_ok(&ai); gfsm_arciter_next(&ai)) {
      a = gfsm_arciter_arc(&ai);
      a->lower = gfsmEpsilon;
    }
  }

  //-- concatenate
  gfsm_automaton_concat(fsm1,fsm2);
  
  //-- mark output fsm as transducer
  fsm1->flags.is_transducer = 1;

  return fsm1;
}

/*--------------------------------------------------------------
 * project()
 */
gfsmAutomaton *gfsm_automaton_project(gfsmAutomaton *fsm, gfsmLabelSide which)
{
  gfsmStateId id;
  gfsmArcIter ai;
  if (which==gfsmLSBoth) return fsm;

  for (id=0; id < fsm->states->len; id++) {
    for (gfsm_arciter_open(&ai,fsm,id); gfsm_arciter_ok(&ai); gfsm_arciter_next(&ai)) {
      gfsmArc *a = gfsm_arciter_arc(&ai);
      if (which==gfsmLSLower) a->upper = a->lower;
      else                    a->lower = a->upper;
    }
  }
  fsm->flags.is_transducer = FALSE;
  return fsm;
}

/*--------------------------------------------------------------
 * replace()
 */
gfsmAutomaton *gfsm_automaton_replace(gfsmAutomaton *fsm1, gfsmLabelVal lo, gfsmLabelVal hi, gfsmAutomaton *fsm2)
{
  gfsmStateId id;
  gfsmArcIter ai;
  gfsmStateId nstates = fsm1->states->len;

  for (id=0; id < nstates; id++) {
    if (!gfsm_automaton_has_state(fsm1,id)) continue;
    for (gfsm_arciter_open(&ai,fsm1,id), gfsm_arciter_seek_both(&ai,lo,hi);
	 gfsm_arciter_ok(&ai);
	 gfsm_arciter_seek_both(&ai,lo,hi))
      {
	gfsmArc *a = gfsm_arciter_arc(&ai);
	gfsm_automaton_insert_automaton(fsm1, id, a->target, fsm2, a->weight);
	gfsm_arciter_remove(&ai); //-- implies gfsm_arciter_next()
      }
    //gfsm_arciter_close(&ai);
  }

  return fsm1;
}

/*--------------------------------------------------------------
 * insert_automaton()
 */
gfsmAutomaton *gfsm_automaton_insert_automaton(gfsmAutomaton *fsm1,
					       gfsmStateId    q1from,
					       gfsmStateId    q1to,
					       gfsmAutomaton *fsm2,
					       gfsmWeight     w)
{
  gfsmStateId offset;
  gfsmStateId size2;
  gfsmStateId id2;
  gfsmStateId      id1;
  const gfsmState *s2;
  gfsmState       *s1;
  gfsmArcIter      ai;
  gfsmWeight       s2fw;

  //-- reserve size
  offset = fsm1->states->len;
  size2  = fsm2->states->len;
  gfsm_automaton_reserve(fsm1, offset + size2);

  //-- avoid "smart" arc-insertion
  fsm1->flags.sort_mode = gfsmASMNone;

  //-- adopt states from fsm2 into fsm1
  for (id2 = 0; id2 < size2; id2++) {

    s2 = gfsm_automaton_find_state_const(fsm2,id2);
    id1 = id2+offset;
    s1 = gfsm_automaton_find_state(fsm1, id1);

    //-- sanity check(s)
    if (!s1 || !s2 || !s2->is_valid) continue;

    //-- copy state
    gfsm_state_copy(s1,s2);

    //-- translate targets for adopted arcs
    for (gfsm_arciter_open_ptr(&ai,fsm1,s1); gfsm_arciter_ok(&ai); gfsm_arciter_next(&ai))
      {
	gfsmArc *a = gfsm_arciter_arc(&ai);
	a->target += offset;
      }

    //-- check for fsm2-final states: get weight & add arc to our sink state
    if (gfsm_weightmap_lookup(fsm2->finals, GUINT_TO_POINTER(id2), &s2fw)) {
      s1->is_final = FALSE;
      gfsm_automaton_add_arc(fsm1,id1,q1to,gfsmEpsilon,gfsmEpsilon, s2fw);
    }
  }

  //-- add arc to new state
  gfsm_automaton_add_arc(fsm1, q1from, fsm2->root_id+offset, gfsmEpsilon, gfsmEpsilon, w);

  return fsm1;
}

/*--------------------------------------------------------------
 * rmepsilon_foreach_func()
 */
static
void gfsm_automaton_rmeps_pass2_foreach_func_(gfsmStatePair *sp, gpointer pw, gfsmAutomaton *fsm)
{
  gfsmWeight  w = gfsm_ptr2weight(pw);
  gfsmWeight  fw2;
  gfsmArcIter ai;
  gfsmArc     *a;
  if (sp->id1==sp->id2) return; //-- sanity check

  //-- adopt final weights (plus)
  if (gfsm_automaton_lookup_final(fsm, sp->id2, &fw2)) {
    gfsm_automaton_set_final_state_full(fsm, sp->id1, TRUE,
					gfsm_sr_plus(fsm->sr,
						     gfsm_automaton_get_final_weight(fsm, sp->id1),
						     gfsm_sr_times(fsm->sr, w, fw2)));
  }

  //-- adopt non-epsilon arcs
  for (gfsm_arciter_open(&ai,fsm,sp->id2); gfsm_arciter_ok(&ai); gfsm_arciter_next(&ai)) {
    a = gfsm_arciter_arc(&ai);
    if (a->lower != gfsmEpsilon || a->upper != gfsmEpsilon) {
      gfsm_automaton_add_arc(fsm, sp->id1, a->target, a->lower, a->upper,
			     gfsm_sr_times(fsm->sr, a->weight, w));
    }
  }
}

/*--------------------------------------------------------------
 * rmepsilon()
 */
gfsmAutomaton *gfsm_automaton_rmepsilon(gfsmAutomaton *fsm)
{
  gfsmStatePair2WeightHash *sp2wh = gfsm_statepair2weighthash_new();
  gfsmArcIter ai;
  gfsmStateId qid;
  gfsmArc *a;

  //-- pass-1: populate sp2wh with epsilon-reachable states
  for (qid=0; qid < fsm->states->len; qid++) {
    gfsm_automaton_rmeps_visit_state_(fsm, qid, qid, fsm->sr->one, sp2wh);
  }

  //-- pass-2: adopt non-epsilon arcs & final weights from eps-reachable states
  gfsm_weighthash_foreach(sp2wh, (GHFunc)gfsm_automaton_rmeps_pass2_foreach_func_, fsm);

  //-- pass-3: actual removal of now-redundant epsilon arcs
  for (qid=0; qid < fsm->states->len; qid++) {
    for (gfsm_arciter_open(&ai,fsm,qid); gfsm_arciter_ok(&ai); ) {
      a = gfsm_arciter_arc(&ai);
      if (a->lower==gfsmEpsilon && a->upper==gfsmEpsilon) {
	gfsm_arciter_remove(&ai);
      } else {
	gfsm_arciter_next(&ai);
      }
    }
  }

  //-- cleanup
  gfsm_weighthash_free(sp2wh);

  return fsm;
}

/*--------------------------------------------------------------
 * rmepsilon_visit_state()
 */
void gfsm_automaton_rmeps_visit_state_(gfsmAutomaton *fsm,
				       gfsmStateId qid_noeps, //-- state reachable by non-eps arcs
				       gfsmStateId qid_eps,   //-- eps-reachable state from qid_noeps
				       gfsmWeight weight_eps, //-- total weight of followed eps-arcs
				       gfsmStatePair2WeightHash *sp2wh //-- maps (qid_noeps,qid_noeps)=>sum_weight_eps
				       )
{
  gfsmState *q_noeps, *q_eps;
  gfsmStatePair sp = {qid_noeps,qid_eps};
  gfsmArcIter ai;
  gfsmArc *a;

  //-- visited check, mark
  if (!gfsm_weighthash_insert_sum_if_less(sp2wh, &sp, weight_eps, fsm->sr))
    return; //-- no update required

  //-- sanity check
  q_noeps = gfsm_automaton_find_state(fsm,qid_noeps);
  q_eps   = gfsm_automaton_find_state(fsm,qid_eps);
  if (!q_noeps || !q_noeps->is_valid || !q_eps || !q_eps->is_valid) return;

  //-- visit epsilon-reachable states from q_eps
  for (gfsm_arciter_open_ptr(&ai, fsm, q_eps), gfsm_arciter_seek_both(&ai,gfsmEpsilon,gfsmEpsilon);
       gfsm_arciter_ok(&ai);
       gfsm_arciter_next(&ai), gfsm_arciter_seek_both(&ai,gfsmEpsilon,gfsmEpsilon))
    {
      a = gfsm_arciter_arc(&ai);
      gfsm_automaton_rmeps_visit_state_(fsm, qid_noeps, a->target,
					gfsm_sr_times(fsm->sr, weight_eps, a->weight),
					sp2wh);
    }
}


/*--------------------------------------------------------------
 * reverse()
 */
gfsmAutomaton *gfsm_automaton_reverse(gfsmAutomaton *fsm)
{
  gfsmStateId new_root = gfsm_automaton_add_state(fsm);
  gfsmStateId id;
  gfsmState   *s, *ts;
  gfsmArcList *al, *al_next, *al_prev;
  gfsmWeight   w;
  //gfsmArcSortMode sm = gfsm_automaton_sortmode(fsm);

  //-- mark automaton as unsorted (avoid "smart" arc-insertion)
  fsm->flags.sort_mode = gfsmASMNone;

  //-- reverse arc directions, keeping old "source" and "target" values
  //   intact as sentinels
  for (id = 0; id < new_root; id++) {
    s = gfsm_automaton_find_state(fsm,id);
    if (!s || !s->is_valid) continue;

    //-- check for old final states
    if (gfsm_automaton_lookup_final(fsm,id,&w)) {
      s->is_final = FALSE;
      gfsm_weightmap_remove(fsm->finals, GUINT_TO_POINTER(id));
      gfsm_automaton_add_arc(fsm, new_root, id, gfsmEpsilon, gfsmEpsilon, w);
    }

    //-- reverse arcs
    for (al_prev=NULL, al=s->arcs; al != NULL; al=al_next) {
      gfsmArc *a = gfsm_arclist_arc(al);
      al_next    = al->next;
      if (a->target==id) {
	//-- already reversed (or a single-arc loop, which doesn't need reversal)
	al_prev = al;
	continue; 
      }

      //-- steal arc
      if (al_prev) al_prev->next = al->next;
      else               s->arcs = al->next;
      al->next = NULL;

      //-- move arc
      ts = gfsm_automaton_find_state(fsm,a->target);
      gfsm_automaton_add_arc_node(fsm, ts, al);
    }
  }

  //-- sanitize: swap 'source' and 'target' fields
  for (id=0; id < new_root; id++) {
    gfsmArcIter ai;
    for (gfsm_arciter_open(&ai,fsm,id); gfsm_arciter_ok(&ai); gfsm_arciter_next(&ai)) {
      gfsmArc     *a  = gfsm_arciter_arc(&ai);
      gfsmStateId tmp = a->target;
      a->target = a->source;
      a->source = tmp;
    }
  }

  //-- root flop
  gfsm_automaton_set_final_state_full(fsm,fsm->root_id,TRUE,fsm->sr->one);
  fsm->root_id = new_root;

  return fsm;
}

#if 0
gfsmAutomaton *gfsm_automaton_reverse_old(gfsmAutomaton *fsm)
{
  gfsmStateId new_root = gfsm_automaton_add_state(fsm);
  gfsmStateId id;
  gfsmState   *s, *ts;
  gfsmArcList *al, *al_next, *al_prev;
  gfsmWeight   w;
  //gfsmArcSortMode sm = gfsm_automaton_sortmode(fsm);

  //-- mark automaton as unsorted (avoid "smart" arc-insertion)
  fsm->flags.sort_mode = gfsmASMNone;

  //-- reverse arc directions, assigning reversed arcs 'target' values as 'old_src+new_root'
  for (id = 0; id < new_root; id++) {
    s = gfsm_automaton_find_state(fsm,id);
    if (!s || !s->is_valid) continue;

    //-- check for old final states
    if (gfsm_automaton_lookup_final(fsm,id,&w)) {
      s->is_final = FALSE;
      gfsm_weightmap_remove(fsm->finals, GUINT_TO_POINTER(id));
      gfsm_automaton_add_arc(fsm, new_root, id, gfsmEpsilon, gfsmEpsilon, w);
    }

    //-- reverse arcs
    for (al_prev=NULL, al=s->arcs; al != NULL; al=al_next) {
      gfsmArc *a = gfsm_arclist_arc(al);
      al_next    = al->next;
      if (a->target >= new_root) {
	//-- already moved
	al_prev = al;
	continue; 
      }

      //-- steal arc
      if (al_prev) al_prev->next = al->next;
      else               s->arcs = al->next;
      al->next = NULL;

      //-- move arc
      ts = gfsm_automaton_find_state(fsm,a->target);
      gfsm_automaton_add_arc_link(fsm, ts, al);
      
      //-- flag as reversed
      a->target = id + new_root;
    }
  }

  //-- sanitize
  for (id=0; id < new_root; id++) {
    gfsmArcIter ai;
    for (gfsm_arciter_open(&ai,fsm,id); gfsm_arciter_ok(&ai); gfsm_arciter_next(&ai)) {
      gfsmArc *a = gfsm_arciter_arc(&ai);
      a->target -= new_root;
    }
  }

  //-- root flop
  gfsm_automaton_set_final_state_full(fsm,fsm->root_id,TRUE,fsm->sr->one);
  fsm->root_id = new_root;

  return fsm;
}
#endif

/*--------------------------------------------------------------
 * sigma()
 */
gboolean gfsm_automaton_sigma_foreach_func_(GFSM_UNUSED gfsmAlphabet *abet, GFSM_UNUSED gpointer key, gfsmLabelVal lab, gfsmAutomaton *fsm)
{
  gfsm_automaton_add_arc(fsm,0,1,lab,lab,fsm->sr->one);
  return FALSE;
}

gfsmAutomaton *gfsm_automaton_sigma(gfsmAutomaton *fsm, gfsmAlphabet *abet)
{
  gfsm_automaton_clear(fsm);
  fsm->flags.sort_mode = gfsmASMNone; //-- avoid "smart" arc-insertion
  fsm->root_id = gfsm_automaton_add_state_full(fsm,0);
  gfsm_automaton_add_state_full(fsm,1);
  gfsm_automaton_set_final_state_full(fsm,1,TRUE,fsm->sr->one);
  gfsm_alphabet_foreach(abet, (gfsmAlphabetForeachFunc)gfsm_automaton_sigma_foreach_func_, fsm);
  return fsm;
}

/*--------------------------------------------------------------
 * union_()
 */
gfsmAutomaton *gfsm_automaton_union(gfsmAutomaton *fsm1, gfsmAutomaton *fsm2)
{
  gfsmStateId offset;
  gfsmStateId id2;
  gfsmStateId oldroot1;
  gfsmArcCompData sortdata = {0,0,0,0};

  //-- sanity check
  if (!fsm2 || fsm2->root_id==gfsmNoState) return fsm1;

  offset = fsm1->states->len + 1;
  gfsm_automaton_reserve(fsm1, offset + fsm2->states->len);


  //-- add new root and eps-arc to old root for fsm1
  oldroot1 = fsm1->root_id;
  fsm1->root_id = gfsm_automaton_add_state(fsm1);
  if (oldroot1 != gfsmNoState) {
    gfsm_automaton_add_arc(fsm1, fsm1->root_id, oldroot1, gfsmEpsilon, gfsmEpsilon, fsm1->sr->one);
  }

  //-- avoid "smart" arc-insertion (temporary)
  sortdata.mask = fsm1->flags.sort_mode;
  sortdata.sr   = fsm1->sr;
  fsm1->flags.sort_mode = gfsmASMNone;

  //-- adopt states from fsm2 into fsm1
  for (id2 = 0; id2 < fsm2->states->len; id2++) {
    const gfsmState *s2 = gfsm_automaton_find_state_const(fsm2,id2);
    gfsmState       *s1 = gfsm_automaton_find_state(fsm1,id2+offset);
    gfsmArcIter      ai;
    gfsm_state_copy(s1,s2);
    for (gfsm_arciter_open_ptr(&ai, fsm1, s1); gfsm_arciter_ok(&ai); gfsm_arciter_next(&ai)) {
      gfsmArc *a = gfsm_arciter_arc(&ai);
      a->target += offset;
    }
    //-- index final states from @fsm2
    if (s2->is_final) {
      gfsm_automaton_set_final_state_full(fsm1, id2+offset, TRUE, gfsm_automaton_get_final_weight(fsm2, id2));
    }
    //-- maybe sort new arcs
    if (sortdata.mask != gfsmASMNone
	&& (fsm2->flags.sort_mode != sortdata.mask
	    || (sortdata.mask == gfsmASMWeight && fsm2->sr->type != fsm1->sr->type)))
      {
	s1->arcs = gfsm_arclist_sort(s1->arcs, &sortdata);
      }
  }

  //-- re-instate "smart" arc-insertion
  fsm1->flags.sort_mode = sortdata.mask;

  //-- add epsilon arc to translated root(fsm2) in fsm1
  gfsm_automaton_add_arc(fsm1,
			 fsm1->root_id,
			 offset + fsm2->root_id,
			 gfsmEpsilon,
			 gfsmEpsilon,
			 fsm1->sr->one);

  return fsm1;
}


/*--------------------------------------------------------------
 * dummy()
 */
gfsmAutomaton *gfsm_automaton_dummy(gfsmAutomaton *fsm)
{
  g_assert_not_reached(); /*-- NOT gfsm_assert_not_reached() ! */
  return fsm;
}
