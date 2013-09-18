/*=============================================================================*\
 * File: gfsmCompound.c
 * Author: Bryan Jurish <moocow@ling.uni-potsdam.de>
 * Description: finite state machine library: compound states
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

#include <gfsmCompound.h>

//-- no-inline definitions
#ifndef GFSM_INLINE_ENABLED
# include <gfsmCompound.hi>
#endif

/*======================================================================
 * Label Pair: Methods
 */

/*--------------------------------------------------------------
 * labelpair_compare()
 */
gint gfsm_labelpair_compare(gfsmLabelPair lp1, gfsmLabelPair lp2)
{ return gfsm_labelpair_compare_inline(lp1,lp2); }

/*--------------------------------------------------------------
 * labelpair_compare_with_data()
 */
gint gfsm_labelpair_compare_with_data(gfsmLabelPair lp1, gfsmLabelPair lp2, GFSM_UNUSED gpointer data)
{ return gfsm_labelpair_compare_inline(lp1,lp2); }


/*======================================================================
 * Methods: gfsmStatePair
 */

/*--------------------------------------------------------------
 * statepair_hash()
 */
guint gfsm_statepair_hash(gfsmStatePair *sp)
{
  //return 7*sp->id1 + sp->id2;
  //return 5039*sp->id1 + sp->id2;
  return 7949*sp->id1 + sp->id2;
}


/*--------------------------------------------------------------
 * statepair_compare()
 */
gint gfsm_statepair_compare(const gfsmStatePair *sp1, const gfsmStatePair *sp2)
{
  return (sp1->id1 < sp2->id1 ? -1
	  : (sp1->id1 > sp2->id1 ? 1
	     : (sp1->id2 < sp2->id2 ? -1
		: (sp1->id2 > sp2->id2 ? 1
		   : 0))));
}

/*--------------------------------------------------------------
 * statepair_equal()
 */
gboolean gfsm_statepair_equal(const gfsmStatePair *sp1, const gfsmStatePair *sp2)
{
  return sp1->id1==sp2->id1 && sp1->id2==sp2->id2;
}



/*======================================================================
 * Methods: gfsmComposeState
 */

/*--------------------------------------------------------------
 * compose_state_hash()
 */
guint gfsm_compose_state_hash(gfsmComposeState *sp)
{
  return 7949*sp->id1 + sp->id2 + 7963*sp->idf;
}


/*--------------------------------------------------------------
 * compose_state_compare()
 */
gint gfsm_compose_state_compare(const gfsmComposeState *sp1, const gfsmComposeState *sp2)
{
  return (sp1->id1 < sp2->id1 ? -1
	  : (sp1->id1 > sp2->id1 ? 1
	     : (sp1->id2 < sp2->id2 ? -1
		: (sp1->id2 > sp2->id2 ? 1
		   : (sp1->idf < sp2->idf ? -1
		      : (sp1->idf > sp2->idf ? 1
			 : 0))))));
}

/*--------------------------------------------------------------
 * compose_state_equal()
 */
gboolean gfsm_compose_state_equal(const gfsmComposeState *sp1, const gfsmComposeState *sp2)
{
  return sp1->id1==sp2->id1 && sp1->id2==sp2->id2 && sp1->idf==sp2->idf;
}



/*======================================================================
 * Methods: gfsmStateWeightPair
 */

//--------------------------------------------------------------
guint gfsm_state_weight_pair_hash(gfsmStateWeightPair *swp)
{
  return swp->id;
}

//--------------------------------------------------------------
gint gfsm_state_weight_pair_compare(const gfsmStateWeightPair *swp1, const gfsmStateWeightPair *swp2, gfsmSemiring *sr)
{
  return (swp1->id < swp2->id ? -1
	  : (swp1->id > swp2->id ? 1
	     : gfsm_sr_compare(sr,swp1->w,swp2->w)));
}

//--------------------------------------------------------------
gboolean gfsm_state_weight_pair_equal(const gfsmStateWeightPair *swp1, const gfsmStateWeightPair *swp2)
{
  return swp1->id==swp2->id && swp1->w==swp2->w;
}


/*======================================================================
 * Methods: gfsmStatePairEnum
 */


/*======================================================================
 * Methods: gfsmComposeStateEnum
 */

