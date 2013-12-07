
/*=============================================================================*\
 * File: gfsmArcIndex.c
 * Author: Bryan Jurish <moocow@ling.uni-potsdam.de>
 * Description: finite state machine library: arc indices
 *
 * Copyright (c) 2006-2007 Bryan Jurish.
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

#include <gfsmArcIndex.h>
#include <gfsmArcIter.h>

//-- no-inline definitions
#ifndef GFSM_INLINE_ENABLED
# include <gfsmArcIndex.hi>
#endif

/*======================================================================
 * gfsmReverseArcIndex
 */

/*--------------------------------------------------------------
 * automaton_reverse_arc_index()
 */
gfsmReverseArcIndex *gfsm_automaton_to_reverse_arc_index(gfsmAutomaton *fsm, gfsmReverseArcIndex *rarcs)
{
  gfsmStateId idfrom;
  gfsmArcIter ai;
  gfsmArc *arc;

  if (!rarcs) {
    rarcs = gfsm_reverse_arc_index_sized_new(fsm->states->len);
  }
  g_ptr_array_set_size(rarcs,fsm->states->len);

  for (idfrom=0; idfrom < fsm->states->len; idfrom++) {
    for (gfsm_arciter_open(&ai,fsm,idfrom); gfsm_arciter_ok(&ai); gfsm_arciter_next(&ai)) {
      arc  = gfsm_arciter_arc(&ai);
      g_ptr_array_index(rarcs,arc->target)
	//= gfsm_arclist_prepend(g_ptr_array_index(rarcs,arc->target), arc);
	= g_slist_prepend(g_ptr_array_index(rarcs,arc->target),arc);
    }
    gfsm_arciter_close(&ai);
  }

  return rarcs;
}

/*--------------------------------------------------------------
 * reverse_arc_index_free()
 */
void gfsm_reverse_arc_index_free(gfsmReverseArcIndex *rarcs, gboolean free_lists)
{
  guint i;
  if (free_lists) {
    //-- +free_lists, -free_arcs
    for (i=0; i < rarcs->len; i++) { g_slist_free(g_ptr_array_index(rarcs,i)); }
  }

  //-- free index array
  g_ptr_array_free(rarcs,TRUE);
}



/*======================================================================
 * gfsmWeightVector
 */

/*--------------------------------------------------------------
 * automaton_to_weight_vector()
 */
gfsmWeightVector *gfsm_automaton_to_final_weight_vector(gfsmAutomaton *fsm, gfsmWeightVector *wv)
{
  gfsmStateId qid;
  guint n_states = gfsm_automaton_n_states(fsm);
  gfsmWeight  *wp;

  if (wv==NULL) {
    wv = gfsm_weight_vector_sized_new(n_states);
  } else {
    gfsm_weight_vector_resize(wv,n_states);
  }
  wv->len = n_states;

  for (qid=0,wp=(gfsmWeight*)wv->data; qid < n_states; qid++,wp++) {
    gfsm_automaton_lookup_final(fsm,qid,wp);
  }

  return wv;
}

/*--------------------------------------------------------------
 * weight_vector_write_bin_handle()
 */
gboolean gfsm_weight_vector_write_bin_handle(gfsmWeightVector *wv, gfsmIOHandle *ioh, gfsmError **errp)
{
  guint32 len = wv->len;
  if (!gfsmio_write(ioh,&len,sizeof(guint32))) {
    g_set_error(errp, g_quark_from_static_string("gfsm"),                              //-- domain
		g_quark_from_static_string("weight_vector_write_bin_handle:len"), //-- code
		"could not store weight vector length");
    return FALSE;
  }
  if (!gfsmio_write(ioh,wv->data,wv->len*sizeof(gfsmWeight))) {
    g_set_error(errp, g_quark_from_static_string("gfsm"),                                  //-- domain
		g_quark_from_static_string("weight_vector_write_bin_handle:weights"), //-- code
		"could not store weight vector data");
    return FALSE;
  }
  return TRUE;
}

/*--------------------------------------------------------------
 * weight_vector_read_bin_handle()
 */
gboolean gfsm_weight_vector_read_bin_handle(gfsmWeightVector *wv, gfsmIOHandle *ioh, gfsmError **errp)
{
  guint32 len;
  if (!gfsmio_read(ioh, &len, sizeof(guint32))) {
    g_set_error(errp,
		g_quark_from_static_string("gfsm"),                                    //-- domain
		g_quark_from_static_string("weight_vector_read_bin_handle:len"),  //-- code
		"could not read weight vector length");
    return FALSE;
  }
  gfsm_weight_vector_resize(wv,len);
  if (!gfsmio_read(ioh, wv->data, len*sizeof(gfsmWeight))) {
    g_set_error(errp,
		g_quark_from_static_string("gfsm"),                                     //-- domain
		g_quark_from_static_string("weight_vector_read_bin_handle:data"),  //-- code
		"could not read weight vector data");
    return FALSE;
  }
  return TRUE;
}

/*======================================================================
 * gfsmArcTable
 */

/*--------------------------------------------------------------
 * automaton_to_arc_table()
 */
gfsmArcTable *gfsm_automaton_to_arc_table(gfsmAutomaton *fsm, gfsmArcTable *tab)
{
  gfsmStateId qid, n_states=gfsm_automaton_n_states(fsm);
  guint              n_arcs=gfsm_automaton_n_arcs(fsm);
  gfsmArcIter ai;
  gfsmArc  *arcp;

  //-- maybe allocate
  if (!tab) {
    tab = gfsm_arc_table_sized_new(n_arcs);
  } else {
    gfsm_arc_table_resize(tab, n_arcs);
  }
  tab->len = n_arcs;

  //-- populate arcs
  for (qid=0,arcp=(gfsmArc*)tab->data; qid < n_states; qid++) {
    if (!gfsm_automaton_has_state(fsm,qid)) continue;
    for (gfsm_arciter_open(&ai,fsm,qid); gfsm_arciter_ok(&ai); gfsm_arciter_next(&ai)) {
      gfsmArc *a = gfsm_arciter_arc(&ai);
      *(arcp++) = *a;
    }
    gfsm_arciter_close(&ai);
  }

  //-- return
  return tab;
}

/*--------------------------------------------------------------
 * arc_table_write_bin_handle()
 */
gboolean gfsm_arc_table_write_bin_handle(gfsmArcTable *tab, gfsmIOHandle *ioh, gfsmError **errp)
{
  guint32 len = tab->len;
  if (!gfsmio_write(ioh, &len, sizeof(guint32))) {
    g_set_error(errp,
		g_quark_from_static_string("gfsm"),
		g_quark_from_static_string("arc_table_write_bin_handle:len"),
		"could not write arc table length");
    return FALSE;
  }
  if (!gfsmio_write(ioh, tab->data, len*sizeof(gfsmArc))) {
    g_set_error(errp,
		g_quark_from_static_string("gfsm"),
		g_quark_from_static_string("arc_table_write_bin_handle:data"),
		"could not write arc table data");
    return FALSE;
  }
  return TRUE;
}

/*--------------------------------------------------------------
 * arc_table_read_bin_handle()
 */
gboolean gfsm_arc_table_read_bin_handle(gfsmArcTable *tab, gfsmIOHandle *ioh, gfsmError **errp)
{
  guint32 len;
  if (!gfsmio_read(ioh, &len, sizeof(guint32))) {
    g_set_error(errp,
		g_quark_from_static_string("gfsm"),
		g_quark_from_static_string("arc_table_read_bin_handle:len"),
		"could not read arc table length");
    return FALSE;
  }
  gfsm_arc_table_resize(tab,len);
  if (!gfsmio_read(ioh, tab->data, len*sizeof(gfsmArc))) {
    g_set_error(errp,
		g_quark_from_static_string("gfsm"),
		g_quark_from_static_string("arc_table_read_bin_handle:data"),
		"could not read arc table data");
    return FALSE;
  }
  return TRUE;
}



/*======================================================================
 * gfsmArcTableIndex
 */

/*--------------------------------------------------------------
 * arc_table_index_copy()
 */
gfsmArcTableIndex *gfsm_arc_table_index_copy(gfsmArcTableIndex *dst, gfsmArcTableIndex *src)
{
  gfsmStateId i;
  gfsm_arc_table_copy (dst->tab, src->tab);
  g_ptr_array_set_size(dst->first, src->first->len);

  for (i=0; i < src->first->len; i++) {
    gint offset = (gfsmArc*)g_ptr_array_index(src->first,i) - (gfsmArc*)src->tab->data;
    g_ptr_array_index(dst->first,i) = (gfsmArc*)dst->tab->data + offset;
  }

  return dst;
}


/*--------------------------------------------------------------
 * automaton_to_arc_table_index()
 */
gfsmArcTableIndex *gfsm_automaton_to_arc_table_index(gfsmAutomaton *fsm, gfsmArcTableIndex *tabx)
{
  gfsmStateId qid, n_states=gfsm_automaton_n_states(fsm);
  guint              n_arcs=gfsm_automaton_n_arcs(fsm);
  gfsmArc  *arcp, *arcp_max;
  gfsmArc **firstp;

  //-- maybe allocate
  if (!tabx) {
    tabx = gfsm_arc_table_index_sized_new(n_states, n_arcs);
  } else {
    gfsm_arc_table_index_resize(tabx, n_states, n_arcs);
  }

  //-- populate tabx->arcs
  gfsm_automaton_to_arc_table(fsm,tabx->tab);

  //-- populate tabx->first
  arcp     = (gfsmArc*)tabx->tab->data;
  arcp_max = arcp + n_arcs;
  for (qid=0,firstp=(gfsmArc**)tabx->first->pdata; qid<n_states; qid++,firstp++) {
    *firstp = arcp;
    for ( ; arcp<arcp_max && arcp->source==qid; arcp++) { ; }
  }
  *firstp = arcp_max;

  //-- return
  return tabx;
}

/*--------------------------------------------------------------
 * arc_table_index_sort_with_data()
 */
void gfsm_arc_table_index_sort_with_data(gfsmArcTableIndex *tabx, GCompareDataFunc compare_func, gpointer data)
{
  gfsmArc **firstp     = (gfsmArc**)tabx->first->pdata;
  gfsmArc **firstp_max = firstp + tabx->first->len - 1;
  for ( ; firstp < firstp_max; firstp++) {
    gfsmArc *min = *firstp;
    gfsmArc *max = *(firstp+1);
    g_qsort_with_data(min, max-min, sizeof(gfsmArc), compare_func, data);
  }
}

/*--------------------------------------------------------------
 * arc_table_index_write_bin_handle()
 */
gboolean gfsm_arc_table_index_write_bin_handle(gfsmArcTableIndex *tabx, gfsmIOHandle *ioh, gfsmError **errp)
{
  gfsmStateId first_len=tabx->first->len, qid;
  if (!gfsm_arc_table_write_bin_handle(tabx->tab, ioh, errp)) return FALSE;

  if (!gfsmio_write(ioh, &first_len, sizeof(gfsmStateId))) {
    g_set_error(errp,
		g_quark_from_static_string("gfsm"),
		g_quark_from_static_string("arc_table_index_write_bin_handle:len"),
		"could not write arc table index 'first' length");
    return FALSE;
  }
  for (qid=0; qid < first_len; qid++) {
    gfsmArc     *a = (gfsmArc*)g_ptr_array_index(tabx->first,qid);
    guint32 offset = a - ((gfsmArc*)tabx->tab->data);
    if (!gfsmio_write(ioh, &offset, sizeof(guint32))) {
      g_set_error(errp,
		  g_quark_from_static_string("gfsm"),
		  g_quark_from_static_string("arc_table_index_write_bin_handle:data"),
		  "could not write state arc offset for state '%u'", qid);
      return FALSE;
    }
  }
  return TRUE;
}

/*--------------------------------------------------------------
 * arc_table_index_read_bin_handle()
 */
gboolean gfsm_arc_table_index_read_bin_handle(gfsmArcTableIndex *tabx, gfsmIOHandle *ioh, gfsmError **errp)
{
  gfsmStateId first_len, qid;
  if (!gfsm_arc_table_read_bin_handle(tabx->tab, ioh, errp)) return FALSE;

  if (!gfsmio_read(ioh, &first_len, sizeof(gfsmStateId))) {
    g_set_error(errp,
		g_quark_from_static_string("gfsm"),
		g_quark_from_static_string("arc_table_index_read_bin_handle:len"),
		"could not read arc table index 'first' length");
    return FALSE;
  }
  g_ptr_array_set_size(tabx->first,first_len);
  for (qid=0; qid < first_len; qid++) {
    guint32 offset;
    if (!gfsmio_read(ioh, &offset, sizeof(guint32))) {
      g_set_error(errp,
		  g_quark_from_static_string("gfsm"),
		  g_quark_from_static_string("arc_table_index_write_bin_handle:data"),
		  "could not read state arc offset for state '%u'", qid);
      return FALSE;
    }
    g_ptr_array_index(tabx->first,qid) = &g_array_index(tabx->tab,gfsmArc,offset);
  }
  return TRUE;
}


/*======================================================================
 * gfsmArcLabelIndex [GONE]
 */
//--------------------------------------------------------------
// arc_label_index_compare_arcs()
/*
gint gfsm_arc_label_index_compare_arcs(gfsmArc *a1, gfsmArc *a2, gfsmArcLabelIndexSortData *sdata)
{ return gfsm_arc_label_index_compare_arcs_inline(a1,a2,sdata); }
*/


/*======================================================================
 * gfsmArcRange
 */

#undef GFSM_ARCRANGE_ENABLE_BSEARCH
#undef GFSM_ARCRANGE_ENABLE_SEEK

#ifdef GFSM_ARCRANGE_ENABLE_BSEARCH
/*--------------------------------------------------------------
 * arc_range_bsearch_*()
 *  + NOT WORTH IT (tested for out_degree {1,2,4,8,16,32,64,128,256,512})
 */
void gfsm_arcrange_bsearch_source(gfsmArcRange *range, gfsmStateId find)
{
  gfsmArc *min=range->min, *max=range->max;
  while (min < max) {
    gfsmArc *mid = min + (max-min)/2;
    if (mid->source < find) { min = mid+1; }
    else                    { max = mid; }
  }
  range->min = min;
}

//--------------------------------------------------------------
void gfsm_arcrange_bsearch_target(gfsmArcRange *range, gfsmStateId find)
{
  gfsmArc *min=range->min, *max=range->max;
  while (min < max) {
    gfsmArc *mid = min + (max-min)/2;
    if (mid->target < find) { min = mid+1; }
    else                    { max = mid; }
  }
  range->min = min;
}

//--------------------------------------------------------------
void gfsm_arcrange_bsearch_lower(gfsmArcRange *range, gfsmLabelId find)
{
  gfsmArc *min=range->min, *max=range->max;
  while (min < max) {
    gfsmArc *mid = min + (max-min)/2;
    if (mid->lower < find) { min = mid+1; }
    else                   { max = mid; }
  }
  range->min = min;
}

//--------------------------------------------------------------
void gfsm_arcrange_bsearch_upper(gfsmArcRange *range, gfsmLabelId find)
{
  gfsmArc *min=range->min, *max=range->max;
  while (min < max) {
    gfsmArc *mid = min + (max-min)/2;
    if (mid->upper < find) { min = mid+1; }
    else                    { max = mid; }
  }
  range->min = min;
}

//--------------------------------------------------------------
void gfsm_arcrange_bsearch_weight(gfsmArcRange *range, gfsmWeight find, gfsmSemiring *sr)
{
  gfsmArc *min=range->min, *max=range->max;
  while (min < max) {
    gfsmArc *mid = min + (max-min)/2;
    if (gfsm_sr_compare(sr,mid->weight,find) < 0) { range->min = mid+1; }
    else                                          { range->max = mid; }
  }
  range->min = min;
}
#endif /* GFSM_ARCRANGE_ENABLE_BSEARCH */

#ifdef GFSM_ARCRANGE_ENABLE_SEEK
//--------------------------------------------------------------
// arcrange_seek_X()
//  -- also not worth it

//----------------------------------------------
GFSM_INLINE
void gfsm_arcrange_seek_source(gfsmArcRange *range, gfsmStateId find)
{
  gfsm_assert(range != NULL);
  while (gfsm_arcrange_ok(range) && gfsm_arcrange_arc(range)->source < find)
    gfsm_arcrange_next(range);
}

//----------------------------------------------
GFSM_INLINE
void gfsm_arcrange_seek_target(gfsmArcRange *range, gfsmStateId find)
{
  gfsm_assert(range != NULL);
  while (gfsm_arcrange_ok(range) && gfsm_arcrange_arc(range)->target < find)
    gfsm_arcrange_next(range);
}

//----------------------------------------------
GFSM_INLINE
void gfsm_arcrange_seek_lower(gfsmArcRange *range, gfsmLabelId find)
{
  gfsm_assert(range != NULL);
  while (gfsm_arcrange_ok(range) && gfsm_arcrange_arc(range)->lower < find)
    gfsm_arcrange_next(range);
}

//----------------------------------------------
GFSM_INLINE
void gfsm_arcrange_seek_upper(gfsmArcRange *range, gfsmLabelId find)
{
  gfsm_assert(range != NULL);
  while (gfsm_arcrange_ok(range) && gfsm_arcrange_arc(range)->upper < find)
    gfsm_arcrange_next(range);
}

//----------------------------------------------
GFSM_INLINE
void gfsm_arcrange_seek_weight(gfsmArcRange *range, gfsmWeight find, gfsmSemiring *sr)
{
  gfsm_assert(range != NULL);
  while (gfsm_arcrange_ok(range) && gfsm_sr_compare(sr,gfsm_arcrange_arc(range)->weight,find) < 0)
    gfsm_arcrange_next(range);
}
#endif /* GFSM_ARCRANGE_ENABLE_SEEK */
