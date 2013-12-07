
/*=============================================================================*\
 * File: gfsmPaths.c
 * Author: Bryan Jurish <moocow@ling.uni-potsdam.de>
 * Description: finite state machine library
 *
 * Copyright (c) 2005-2008 Bryan Jurish.
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

#include <gfsmPaths.h>
#include <gfsmArc.h>
#include <gfsmArcIter.h>


/*======================================================================
 * Methods: Path Utilities: gfsmLabelVector
 */

//--------------------------------------------------------------
gfsmLabelVector *gfsm_label_vector_copy(gfsmLabelVector *dst, gfsmLabelVector *src)
{
  guint i;
  g_ptr_array_set_size(dst, src->len);
  for (i=0; i < src->len; i++) {
    g_ptr_array_index(dst,i) = g_ptr_array_index(src,i);
  }
  return dst;
}

//--------------------------------------------------------------
gfsmLabelVector *gfsm_label_vector_reverse(gfsmLabelVector *v)
{
  guint i, mid;
  gpointer tmp;
  mid = v->len/2;
  for (i=0; i < mid; i++) {
    tmp = g_ptr_array_index(v,i);
    g_ptr_array_index(v,i) = g_ptr_array_index(v,v->len-i-1);
    g_ptr_array_index(v,v->len-i-1) = tmp;
  }
  return v;
}

/*======================================================================
 * Methods: Path Utilities: gfsmPath
 */

//--------------------------------------------------------------
gfsmPath *gfsm_path_new_full(gfsmLabelVector *lo, gfsmLabelVector *hi, gfsmWeight w)
{
  gfsmPath *p = g_new(gfsmPath,1);
  p->lo = lo ? lo : g_ptr_array_new();
  p->hi = hi ? hi : g_ptr_array_new();
  p->w  = w;
  return p;
}

//--------------------------------------------------------------
gfsmPath *gfsm_path_new_copy(gfsmPath *p1)
{
  gfsmPath *p = g_new(gfsmPath,1);

  p->lo = g_ptr_array_sized_new(p1->lo->len);
  p->hi = g_ptr_array_sized_new(p1->hi->len);

  gfsm_label_vector_copy(p->lo, p1->lo);
  gfsm_label_vector_copy(p->hi, p1->hi);

  p->w  = p1->w;

  return p;
}

//--------------------------------------------------------------
gfsmPath *gfsm_path_new_append(gfsmPath *p1, gfsmLabelVal lo, gfsmLabelVal hi, gfsmWeight w, gfsmSemiring *sr)
{
  gfsmPath *p = g_new(gfsmPath,1);

  if (lo != gfsmEpsilon) {
    p->lo = g_ptr_array_sized_new(p1->lo->len+1);
    gfsm_label_vector_copy(p->lo, p1->lo);
    g_ptr_array_add(p->lo, GUINT_TO_POINTER(lo));
  } else {
    p->lo = g_ptr_array_sized_new(p1->lo->len);
    gfsm_label_vector_copy(p->lo, p1->lo);
  }

  if (hi != gfsmEpsilon) {
    p->hi = g_ptr_array_sized_new(p1->hi->len+1);
    gfsm_label_vector_copy(p->hi, p1->hi);
    g_ptr_array_add(p->hi, GUINT_TO_POINTER(hi));
  } else {
    p->hi = g_ptr_array_sized_new(p1->hi->len);
    gfsm_label_vector_copy(p->hi, p1->hi);
  }

  p->w  = gfsm_sr_times(sr, p1->w, w);

  return p;
}

//--------------------------------------------------------------
gfsmPath *gfsm_path_new_times_w(gfsmPath *p1, gfsmWeight w, gfsmSemiring *sr)
{
  gfsmPath *p = g_new(gfsmPath,1);

  p->lo = g_ptr_array_sized_new(p1->lo->len);
  gfsm_label_vector_copy(p->lo, p1->lo);

  p->hi = g_ptr_array_sized_new(p1->hi->len);
  gfsm_label_vector_copy(p->hi, p1->hi);

  p->w = gfsm_sr_times(sr, p1->w, w);

  return p;
}

//--------------------------------------------------------------
void gfsm_path_push(gfsmPath *p, gfsmLabelVal lo, gfsmLabelVal hi, gfsmWeight w, gfsmSemiring *sr)
{
  if (lo != gfsmEpsilon) g_ptr_array_add(p->lo, GUINT_TO_POINTER(lo));
  if (hi != gfsmEpsilon) g_ptr_array_add(p->hi, GUINT_TO_POINTER(hi));
  p->w = gfsm_sr_times(sr, p->w, w);
}


//--------------------------------------------------------------
void gfsm_path_pop(gfsmPath *p, gfsmLabelVal lo, gfsmLabelVal hi)
{
  if (lo != gfsmEpsilon) g_ptr_array_remove_index_fast(p->lo, p->lo->len-1);
  if (hi != gfsmEpsilon) g_ptr_array_remove_index_fast(p->hi, p->hi->len-1);
}

//--------------------------------------------------------------
int gfsm_label_vector_compare(const gfsmLabelVector *v1, const gfsmLabelVector *v2)
{
  guint i;
  gfsmLabelVal lab1, lab2;
  if (v1==v2) return 0;

  for (i=0; i < v1->len && i < v2->len; i++) {
    lab1 = (gfsmLabelVal)GPOINTER_TO_UINT(g_ptr_array_index(v1,i));
    lab2 = (gfsmLabelVal)GPOINTER_TO_UINT(g_ptr_array_index(v2,i));
    if (lab1 < lab2) return -1;
    if (lab1 > lab2) return  1;
  }
  if (v1->len <  v2->len) return -1;
  if (v1->len >  v2->len) return  1;
  return 0;
}

//--------------------------------------------------------------
int gfsm_path_compare_data(const gfsmPath *p1, const gfsmPath *p2, gfsmSemiring *sr)
{
  int cmp;
  if (p1==p2) return 0;
  if ((cmp=gfsm_sr_compare(sr, p1->w, p2->w))!=0) return cmp;
  if ((cmp=gfsm_label_vector_compare(p1->lo,p2->lo))!=0) return cmp;
  if ((cmp=gfsm_label_vector_compare(p1->hi,p2->hi))!=0) return cmp;
  return 0;
}

//--------------------------------------------------------------
gfsmPath *gfsm_path_reverse(gfsmPath *p)
{
  if (p->lo) gfsm_label_vector_reverse(p->lo);
  if (p->hi) gfsm_label_vector_reverse(p->hi);
  return p;
}

//--------------------------------------------------------------
void gfsm_path_free(gfsmPath *p)
{
  if (!p) return;
  if (p->lo) g_ptr_array_free(p->lo,TRUE);
  if (p->hi) g_ptr_array_free(p->hi,TRUE);
  g_free(p);
}

/*======================================================================
 * Methods: Automaton Serialization: paths()
 */

//--------------------------------------------------------------
gfsmSet *gfsm_automaton_paths(gfsmAutomaton *fsm, gfsmSet *paths)
{
  return gfsm_automaton_paths_full(fsm, paths, (fsm->flags.is_transducer ? gfsmLSBoth : gfsmLSLower));
}

//--------------------------------------------------------------
gfsmSet *gfsm_automaton_paths_full(gfsmAutomaton *fsm, gfsmSet *paths, gfsmLabelSide which)
{
  gfsmPath *tmp = gfsm_path_new(fsm->sr);
  if (paths==NULL) {
    paths = gfsm_set_new_full((GCompareDataFunc)gfsm_path_compare_data,
			      (gpointer)fsm->sr,
			      (GDestroyNotify)gfsm_path_free);
  }
  _gfsm_automaton_paths_r(fsm, paths, which, fsm->root_id, tmp);
  gfsm_path_free(tmp);
  return paths;
}

//--------------------------------------------------------------
gfsmSet *_gfsm_automaton_paths_r(gfsmAutomaton *fsm,
				 gfsmSet       *paths,
				 gfsmLabelSide  which, 
				 gfsmStateId    q,
				 gfsmPath      *path)
{
  gfsmArcIter ai;
  gfsmWeight  fw;

  //-- if final state, add to set of full paths
  if (gfsm_automaton_lookup_final(fsm,q,&fw)) {
    gfsmWeight path_w = path->w;
    path->w = gfsm_sr_times(fsm->sr, fw, path_w);

    if (!gfsm_set_contains(paths,path)) {
      gfsm_set_insert(paths, gfsm_path_new_copy(path));
    }
    path->w = path_w;
  }

  //-- investigate all outgoing arcs
  for (gfsm_arciter_open(&ai, fsm, q); gfsm_arciter_ok(&ai); gfsm_arciter_next(&ai)) {
    gfsmArc    *arc = gfsm_arciter_arc(&ai);
    gfsmWeight    w = path->w;
    gfsmLabelVal lo,hi;

    if (which==gfsmLSLower) {
      lo = arc->lower;
      hi = gfsmEpsilon;
    } else if (which==gfsmLSUpper) {
      lo = gfsmEpsilon;
      hi = arc->upper;
    } else {
      lo = arc->lower;
      hi = arc->upper;
    }

    gfsm_path_push(path, lo, hi, arc->weight, fsm->sr);
    _gfsm_automaton_paths_r(fsm, paths, which, arc->target, path);

    gfsm_path_pop(path, lo, hi);
    path->w = w;
  }
  gfsm_arciter_close(&ai);

  return paths;
}

/*======================================================================
 * Methods: Automaton Serialization: paths_to_strings()
 */

//--------------------------------------------------------------
GSList *gfsm_paths_to_strings(gfsmSet *paths,
			      gfsmAlphabet *abet_lo,
			      gfsmAlphabet *abet_hi,
			      gfsmSemiring *sr,
			      gboolean warn_on_undefined,
			      gboolean att_style,
			      GFSM_UNUSED GSList *strings)
{
  gfsmPathsToStringsOptions opts =
    {
      abet_lo,
      abet_hi,
      sr,
      warn_on_undefined,
      att_style,
      NULL
    };

  gfsm_set_foreach(paths, (GTraverseFunc)_gfsm_paths_to_strings_foreach_func, &opts);

  return g_slist_reverse(opts.strings);
}

//--------------------------------------------------------------
gboolean _gfsm_paths_to_strings_foreach_func(gfsmPath *path,
					     GFSM_UNUSED gpointer value_dummy,
					     gfsmPathsToStringsOptions *opts)
{
  GString *gs = gfsm_path_to_gstring(path, NULL,
				     opts->abet_lo, opts->abet_hi, opts->sr,
				     opts->warn_on_undefined, opts->att_style);
  opts->strings = g_slist_prepend(opts->strings, gs->str);
  g_string_free(gs,FALSE);

  return FALSE;
}

//--------------------------------------------------------------
GString *gfsm_path_to_gstring(gfsmPath     *path,
			      GString      *gs,
			      gfsmAlphabet *abet_lo,
			      gfsmAlphabet *abet_hi,
			      gfsmSemiring *sr,
			      gboolean      warn_on_undefined,
			      gboolean      att_style)
{
  if (!gs) gs = g_string_new("");
  if (abet_lo && path->lo->len > 0) {
    gfsm_alphabet_labels_to_gstring(abet_lo, path->lo, gs, warn_on_undefined, att_style);
  }
  if (abet_hi && path->hi->len > 0) {
    g_string_append(gs," : ");
    gfsm_alphabet_labels_to_gstring(abet_hi, path->hi, gs, warn_on_undefined, att_style);
  }
  if (gfsm_sr_compare(sr, path->w, sr->one) != 0) {
    g_string_append_printf(gs," <%g>",path->w);
  }
  return gs;
}

//--------------------------------------------------------------
char *gfsm_path_to_string(gfsmPath     *path,
			  gfsmAlphabet *abet_lo,
			  gfsmAlphabet *abet_hi,
			  gfsmSemiring *sr,
			  gboolean      warn_on_undefined,
			  gboolean      att_style)
{
  GString *gs = gfsm_path_to_gstring(path,NULL,abet_lo,abet_hi,sr,warn_on_undefined,att_style);
  char    *s  = gs->str;
  g_string_free(gs,FALSE);
  return s;
}


/*======================================================================
 * Methods: Viterbi trellis: paths
 */

//--------------------------------------------------------------
gfsmSet *gfsm_viterbi_trellis_paths_full(gfsmAutomaton *trellis, gfsmSet *paths, gfsmLabelSide which)
{
  gfsmArcIter ai;

  //-- sanity check: create path-set if given as NULL
  if (!paths) {
    paths = gfsm_set_new_full((GCompareDataFunc)gfsm_path_compare_data,
			      (gpointer)trellis->sr,
			      (GDestroyNotify)gfsm_path_free);
  }

  //-- get & follow pseudo-root of all paths
  for (gfsm_arciter_open(&ai, trellis, trellis->root_id); gfsm_arciter_ok(&ai); gfsm_arciter_next(&ai)) {
    gfsmArc  *arc  = gfsm_arciter_arc(&ai);
    gfsmPath *path = gfsm_path_new(trellis->sr);

    _gfsm_viterbi_trellis_bestpath_r(trellis, path, which, arc->target);
    path->w = arc->weight;

    //-- reverse the path we've created
    gfsm_path_reverse(path);

    //-- ... and maybe insert it
    if (gfsm_set_contains(paths,path)) {
      //-- oops: we've already got this one: free it
      gfsm_path_free(path);
    } else {
      //-- it's a bona-fide new path: insert it
      gfsm_set_insert(paths,path);
    }
  }

  return paths;
}

//--------------------------------------------------------------
gfsmPath *gfsm_viterbi_trellis_bestpath_full(gfsmAutomaton *trellis, gfsmPath *path, gfsmLabelSide which)
{
  gfsmArcIter ai;

  //-- sanity check: create path if NULL
  if (!path) { path = gfsm_path_new(trellis->sr); }

  //-- get & follow pseudo-root of best path
  gfsm_arciter_open(&ai, trellis, trellis->root_id);
  if (gfsm_arciter_ok(&ai)) {
    gfsmArc *arc = gfsm_arciter_arc(&ai);
    _gfsm_viterbi_trellis_bestpath_r(trellis, path, which, arc->target);
    path->w = arc->weight;
  } else {
    path->w = trellis->sr->zero;
  }

  //-- reverse the path we've created
  gfsm_path_reverse(path);

  return path;
}

//--------------------------------------------------------------
void _gfsm_viterbi_trellis_bestpath_r(gfsmAutomaton *trellis,
				      gfsmPath      *path,
				      gfsmLabelSide  which,
				      gfsmStateId    qid)
{
  while (TRUE) {
    gfsmArcIter ai;
    gfsm_arciter_open(&ai, trellis, qid);

    if (gfsm_arciter_ok(&ai)) {
      gfsmArc *arc = gfsm_arciter_arc(&ai);
      gfsm_path_push(path,
		     (which!=gfsmLSUpper ? arc->lower : gfsmEpsilon),
		     (which!=gfsmLSLower ? arc->upper : gfsmEpsilon),
		     trellis->sr->one, trellis->sr);
      qid = arc->target;
    }
    else break;
  }
}
