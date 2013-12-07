
/*=============================================================================*\
 * File: gfsmTrie.h
 * Author: Bryan Jurish <moocow@ling.uni-potsdam.de>
 * Description: finite state machine library
 *
 * Copyright (c) 2005-2007 Bryan Jurish.
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

/** \file gfsmTrie.h
 *  \brief Deterministic prefix tree automata
 */

#ifndef _GFSM_TRIE_H
#define _GFSM_TRIE_H

#include <gfsmAutomaton.h>

/*======================================================================
 * Types: Trie
 */
/** Alias for gfsmAutomaton */
typedef gfsmAutomaton gfsmTrie;

/*======================================================================
 * Constants
 */
/** Default initial Trie flags */
extern const gfsmAutomatonFlags gfsmTrieDefaultFlags;

/** Default initial Trie semiring */
extern const gfsmSRType gfsmTrieDefaultSRType;


/*======================================================================
 * Methods: Constructors etc.
 */
///\name Constructors etc.
//@{

//------------------------------
/** Create a new ::gfsmTrie automaton */
#define gfsm_trie_new() \
  gfsm_automaton_new_full(gfsmTrieDefaultFlags, gfsmTrieDefaultSRType, gfsmAutomatonDefaultSize)

//@}


/*======================================================================
 * Methods: Manipulation
 */
///\name Manipulation
//@{

//------------------------------
/** Add a string-pair \a (lo,hi) to the trie with weight \a w
 *  \param trie Trie
 *  \param lo   lower string (NULL for epsilon)
 *  \param trie upper string (NULL for epsilon)
 *  \param w    weight which is added (gfsm_sr_plus) to all arcs for this pair
 *  \returns Id of the final state of the added path
 *
 *  \note really just a wrapper for \a gfsm_trie_add_path_full() with
 *  \a add_to_arcs=true , \a add_to_state_final=true, \a add_to_path_final=true.
 */
gfsmStateId gfsm_trie_add_path(gfsmTrie        *trie,
			       gfsmLabelVector *lo,
			       gfsmLabelVector *hi,
			       gfsmWeight       w);

//------------------------------
/** Add a string-pair \a (lo,hi) to the trie with weight \a w
 *  \param trie Trie
 *  \param lo   lower string (NULL for epsilon)
 *  \param hi   upper string (NULL for epsilon)
 *  \param w    weight associated with this pair
 *  \param add_to_arcs  whether to add (gfsm_sr_plus) \a w to all arc-weights
 *  \param add_to_state_final whether to add (gfsm_sr_plus) \a w to all intermediate state final-weights;
 *                            implies that all states will be marked as final in the resulting automaton
 *  \param add_to_path_final  whether to add (gfsm_sr_plus) \a w to the final weight for the last node
 *                            in the path
 *  \param path_states If non-NULL, contains the state-path corresponding to \a (lo,hi) on return
 *  \returns Id of the final state of the added path
 */
gfsmStateId gfsm_trie_add_path_full(gfsmTrie          *trie,
				    gfsmLabelVector   *lo,
				    gfsmLabelVector   *hi,
				    gfsmWeight         w,
				    gboolean           add_to_arcs,
				    gboolean           add_to_state_final,
				    gboolean           add_to_path_final,
				    gfsmStateIdVector *path_states
				    );

/*======================================================================
 * Methods: find path
 */
/** Find state of longest prefix for a string-pair \a (lo,hi) in the trie.
 *  \param trie Trie
 *  \param lo   lower string (NULL for epsilon)
 *  \param hi   upper string (NULL for epsilon)
 *  \param lo_i on return holds number of labels in \a lo which were matched
 *  \param hi_i on return holds number of labels in \a hi which were matched
 *  \param w_last pointer to weight of last arc followed or final weight
 *  \param path_states if non-NULL, contains the state-path corresponding to the prefix on return
 *  \returns Id of the state matching the longest prefix of \a (lo,hi)
 */
gfsmStateId gfsm_trie_find_prefix(gfsmTrie          *trie,
				  gfsmLabelVector   *lo,
				  gfsmLabelVector   *hi,
				  guint             *lo_i,
				  guint             *hi_i,
				  gfsmWeight        *w_last,
				  gfsmStateIdVector *path_states
				  );


/*======================================================================
 * Methods: find arcs
 */
/** Find an arc from state \a qid with lower label \a lab in trie \a trie.
 *  \param trie Trie
 *  \param qid   outgoing state qid
 *  \param lab   lower label
 *  \returns gfsmArc* or NULL on failure
 */
gfsmArc* gfsm_trie_find_arc_lower(gfsmTrie *trie, gfsmStateId qid, gfsmLabelVal lab);

/** Find an arc from state \a qid with upper label \a lab in trie \a trie.
 *  \param trie  Trie
 *  \param qid    outgoing state qid
 *  \param lab   upper label id
 *  \returns gfsmArc* or NULL on failure
 */
gfsmArc* gfsm_trie_find_arc_upper(gfsmTrie *trie, gfsmStateId qid, gfsmLabelVal lab);

/** Find an arc from state \a qid with lower label \a lo and upper label \a hi in trie \a trie.
 *  \param trie  Trie
 *  \param qid  outgoing state qid
 *  \param lo   lower label id
 *  \param hi   upper label id
 *  \returns gfsmArc* or NULL on failure
 */
gfsmArc* gfsm_trie_find_arc_both(gfsmTrie *trie, gfsmStateId qid, gfsmLabelVal lo, gfsmLabelVal hi);


/*======================================================================
 * Methods: find or insert arcs
 */
/** Find or insert an arc from state \a qid with lower label \a lab in trie \a trie;
 *  adding weight \a w.
 *  \param trie Trie
 *  \param qid   outgoing state qid
 *  \param lab   lower label
 *  \param w    arc weight
 *  \param add_weight whether to add weight to the arc
 *  \returns gfsmStateId of the (unique) destination state
 */
gfsmStateId gfsm_trie_get_arc_lower(gfsmTrie *trie, gfsmStateId qid, gfsmLabelVal lab, gfsmWeight w, gboolean add_weight);

/** Find or insert an arc from state \a qid with upper label \a lab in trie \a trie;
 *  adding weight \a w.
 *  \param trie Trie
 *  \param qid   outgoing state qid
 *  \param lab   upper label
 *  \param w    arc weight
 *  \param add_weight whether to add weight to the arc
 *  \returns gfsmStateId of the (unique) destination state
 */
gfsmStateId gfsm_trie_get_arc_upper(gfsmTrie *trie, gfsmStateId qid, gfsmLabelVal lab, gfsmWeight w, gboolean add_weight);

/** Find or insert an arc from state \a qid with lower label \a lo and upper label \a hi
 *  with weight \a w in trie \a trie.
 *  \param trie  Trie
 *  \param qid  outgoing state qid
 *  \param lo   lower label id
 *  \param hi   upper label id
 *  \param w    arc weight
 *  \param add_weight whether to add weight to the arc
 *  \returns gfsmStateId of the (unique) destination state
 */
gfsmStateId gfsm_trie_get_arc_both(gfsmTrie *trie, gfsmStateId qid, gfsmLabelVal lo, gfsmLabelVal hi, gfsmWeight w, gboolean add_weight);

//@}


#endif /* _GFSM_LOOKUP_H */
