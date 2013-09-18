
/*=============================================================================*\
 * File: gfsmLookup.h
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

/** \file gfsmLookup.h
 *  \brief Linear composition
 */

#ifndef _GFSM_LOOKUP_H
#define _GFSM_LOOKUP_H

#include <gfsmAutomaton.h>
#include <gfsmUtils.h>

/*======================================================================
 * Types: lookup
 */
/** \brief Type for gfsm_automaton_lookup() computation state */
typedef struct {
  gfsmStateId qt;  /**< current state in transducer */
  gfsmStateId qr;  /**< current state in result acceptor */
  guint32     i;   /**< current position in input vector */
} gfsmLookupConfig;

//------------------------------

/** Type for gfsm_automaton_lookup_viterbi(): Trellis (1 per call) */
typedef GPtrArray gfsmViterbiTable;

/** Viterbi algorithm best-successor accumulator
 *  \arg key   is a gfsmStateId (state in fst)
 *  \arg value is a gfsmStateId (state in trellis)
 */
typedef GTree gfsmViterbiMap;

/** Key type for gfsmViterbiMap (state-id in fst) */
typedef gfsmStateId gfsmViterbiMapKey;

/** Value type for gfsmViterbiMap (state-id in trellis) */
typedef gfsmStateId gfsmViterbiMapValue;

/** Type for Viterbi trellis column (1 per input index)
 *  \arg data  is a gfsmStateId in trellis automaton
 */
typedef GSList gfsmViterbiColumn;

/** Type for Viterbi trellis nodes: state in trellis automaton
 *  \arg state \a q has exactly one outgoing arc \a arc=((gfsmArc*)a->ars->data)
 *  \arg best preceeding state in trellis is \a arc->target
 *  \arg label of best arc from best preceeding state in trellis is \a arc->lower
 *  \arg total weight of best path to this state is \a arc->weight
 */
typedef gfsmState gfsmViterbiNode;


/*======================================================================
 * Constants
 */

/** Number of states to pre-allocate when extending state-map vector on lookup_full() (>= 1) */
extern const gfsmStateId gfsmLookupStateMapGet;


/*======================================================================
 * Methods: lookup
 */
///\name Lookup
//@{

//------------------------------
/** Compose linear automaton specified by \a input with the transducer
 *  \a fst , storing result in \a result.
 *  \param fst transducer (lower-upper)
 *  \param input input labels (lower)
 *  \param result output transducer or NULL
 *  \returns \a result if non-NULL, otherwise a new automaton.
 */
#define gfsm_automaton_lookup(fst,input,result) \
  gfsm_automaton_lookup_full((fst),(input),(result),NULL)

//------------------------------
/** Compose linear automaton specified by \a input with the transducer
 *  \a fst , storing result in \a result , and storing state-translation map \a statemap.
 *  \param fst transducer (lower-upper)
 *  \param input input labels (lower)
 *  \param result output transducer or NULL
 *  \param statemap if non-NULL, maps \a result StateIds (indices) to \a fst StateIds (values) on return.
 *                  Not implicitly created or cleared.
 *  \returns \a result if non-NULL, otherwise a new automaton.
 */
gfsmAutomaton *gfsm_automaton_lookup_full(gfsmAutomaton     *fst,
					  gfsmLabelVector   *input,
					  gfsmAutomaton     *result,
					  gfsmStateIdVector *statemap);

//@}


/*======================================================================
 * Methods: Viterbi
 */
///\name Viterbi Lookup
//@{

//------------------------------
/** Get the best path for input \a input in the transducer \a fst using the Viterbi algorithm.
 *  \param fst transducer (lower-upper)
 *  \param input input labels (lower)
 *  \param trellis output fsm or NULL
 *  \returns \a trellis if non-NULL, otherwise a new automaton representing the (reversed) Viterbi trellis.
 *           \arg labels (lower & upper) in \a trellis represent upper labels of \a fst
 *           \arg arc-weights in \a trellis represent Viterbi algorithm weights (gamma)
 *           \arg arc-targets in \a trellis represent the best preceeding state (psi)
 */
#define gfsm_automaton_lookup_viterbi(fst,input,trellis) \
   gfsm_automaton_lookup_viterbi_full((fst),(input),(trellis),NULL)

//------------------------------
/** Get the best path for input \a input in the transducer \a fst using the Viterbi algorithm.
 *  \param fst transducer (lower-upper)
 *  \param input input labels (lower)
 *  \param trellis output fsm or NULL
 *  \param trellis2fst if non-NULL, maps \a trellis StateIds (indices) to \a fst StateIds (values) on return.
 *                     If NULL, a temporary vector will be created & freed.
 *  \returns \a trellis if non-NULL, otherwise a new automaton representing the (reversed) Viterbi trellis.
 *           \arg lower-labels in \a trellis represent \a input labels
 *           \arg upper-labels of \a trellis represent upper labels of \a fst
 *           \arg arc-weights in \a trellis represent Viterbi algorithm weights (gamma)
 *           \arg arc-targets in \a trellis represent the best preceeding state (psi)
 *           \arg root state of \a trellis has arcs sorted by total path weight (best-first)
 */
gfsmAutomaton *gfsm_automaton_lookup_viterbi_full(gfsmAutomaton     *fst,
						  gfsmLabelVector   *input,
						  gfsmAutomaton     *trellis,
						  gfsmStateIdVector *trellis2fst);

//@}

/*======================================================================
 * Viterbi: Utilities
 */
///\name Viterbi Low-level Utilities
//@{


//------------------------------
// expand_column()

/** Expand lower-epsilon arcs from \a fst into \a col. */
void _gfsm_viterbi_expand_column(gfsmAutomaton      *fst,
				 gfsmAutomaton      *trellis,
				 gfsmViterbiColumn  *col,
				 gfsmStateIdVector  *trellis2fst,
				 gfsmViterbiMap     *fst2trellis);


//------------------------------
// gfsmViterbiMap

/** Create a new gfsmViterbiMap */
#define gfsm_viterbi_map_new() \
  g_tree_new_full((GCompareDataFunc)gfsm_uint_compare, NULL, NULL, NULL)


/** Free a gfsmViterbiMap */
#define gfsm_viterbi_map_free(vmap) g_tree_destroy(vmap)


/** Lookup stored value in a gfsmViterbiColumnMap
 *  \returns gpointer to the stored value for \a key in \a vmap
 */
#define gfsm_viterbi_map_lookup(vmap,key) g_tree_lookup((vmap),(key))

/** Insert a literal value into a gfsmViterbiColumnMap */
#define gfsm_viterbi_map_insert(vmap,key,val) g_tree_insert((vmap),(gpointer)(key),(gpointer)(val))


//------------------------------
// gfsmViterbiNode

/** gfsmViterbiNode: Accessor: unique outgoing arc for \a nod */
//#define gfsm_viterbi_node_arc(nod) ((gfsmArc*)((nod)->arcs->data))
#define gfsm_viterbi_node_arc(nod) gfsm_arclist_arc((nod)->arcs)

/** gfsmViterbiNode: Accessor: Best preceeding state accessor for \a nod */
#define gfsm_viterbi_node_best_prevstate(nod) gfsm_viterbi_node_arc(nod)->target

/** gfsmViterbiNode: Accessor: Total weight of best path to \a nod */
#define gfsm_viterbi_node_best_weight(nod) gfsm_viterbi_node_arc(nod)->weight

//@}

#endif /* _GFSM_LOOKUP_H */
