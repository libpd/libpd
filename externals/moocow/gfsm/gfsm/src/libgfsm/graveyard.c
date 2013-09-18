//=================================================================================
// arc sorting (old)
//=================================================================================

/// Typedef for mode-dependent arc-sorting parameters
/** \see gfsm_arc_compare(), gfsm_arclist_sort(), gfsm_automaton_arcsort() */
typedef struct {
  gfsmArcSortMode  mode; /**< sort mode */
  gfsmSemiring    *sr;   /**< semiring for weight-mode, otherwise ignored */
} gfsmArcSortDataOLD;

/** Old-style default 3-way comparison on arcs (inline version)
 *  \param a1 first arc to compare
 *  \param a2 second arc to compare
 *  \param sdata specifies comparison mode
 *  \returns
 *    negative, zero, or positive integer depending on whether
 *    \a a1 is less-than, equal-to, or greater-than \a a2 according to \a sdata.
 *  \note
 *    \li Prefer gfsm_arc_compare_inline() for literal direct arc comparisons in new code
 *    \li Prefer gfsm_arc_compare() if you're passing function pointers around, since
 *        its address is guaranteed not to change between compilation units.
 */
GFSM_INLINE
gint gfsm_arc_compare_inline(gfsmArc *a1, gfsmArc *a2, gfsmArcSortDataOLD *sdata);

/** Old-style default 3-way comparison on arcs (extern version)
 *  Really just a wrapper for gfsm_arc_compare_inline()
 */
gint gfsm_arc_compare(gfsmArc *a1, gfsmArc *a2, gfsmArcSortDataOLD *sdata);

GFSM_INLINE
gint gfsm_arc_compare_inline(gfsmArc *a1, gfsmArc *a2, gfsmArcSortData *sdata)
{
  if (!a1) {
    if (!a2) return 0;
    return 1;
  }
  if (!a2) return -1;
  switch (sdata->mode) {
  case gfsmASMLower:
    if (a1->lower < a2->lower)   return -1;
    if (a1->lower > a2->lower)   return  1;
    if (a1->upper < a2->upper)   return -1;
    if (a1->upper > a2->upper)   return  1;
    //if (a1->source < a2->source) return -1;
    //if (a1->source > a2->source) return  1;
    if (a1->target < a2->target) return -1;
    if (a1->target > a2->target) return  1;
    return 0;
  case gfsmASMUpper:
    if (a1->upper < a2->upper)   return -1;
    if (a1->upper > a2->upper)   return  1;
    if (a1->lower < a2->lower)   return -1;
    if (a1->lower > a2->lower)   return  1;
    //if (a1->source < a2->source) return -1;
    //if (a1->source > a2->source) return  1;
    if (a1->target < a2->target) return -1;
    if (a1->target > a2->target) return  1;
    return 0;
  case gfsmASMWeight:
    return gfsm_sr_compare(sdata->sr, a1->weight, a2->weight);
  case gfsmASMNone:
  default:
    return (GPOINTER_TO_INT(a2)-GPOINTER_TO_INT(a1));
  }
  return 0;
}

/** Backwards-compatible wrapper for gfsm_arc_compare_bymask() */
gint gfsm_arc_compare(gfsmArc *a1, gfsmArc *a2, gfsmArcSortDataOLD *sdata);

/** Return symbolic name of an (old-style) arc-sort mode */
const gchar *gfsm_arc_sortmode_to_name(gfsmArcSortModeOLD mode);

/*--------------------------------------------------------------
 * compare()
 */
gint gfsm_arc_compare(gfsmArc *a1, gfsmArc *a2, gfsmArcSortDataOLD *sdata)
{
  gfsmArcCompData acdata = { 0, sdata->sr, NULL,NULL };
  switch (sdata->mode) {
  case gfsmASMLowerOLD:  acdata.mask = gfsmASMLower; break;
  case gfsmASMUpperOLD:  acdata.mask = gfsmASMUpper; break;
  case gfsmASMWeightOLD: acdata.mask = gfsmASMWeight; break;
  default:
    break;
  }
  return gfsm_arc_compare_bymask_inline(a1,a2,&acdata);
}

//=================================================================================
// statepair2weightXXX
//=================================================================================

/// Typedef for mapping (gfsmStatePair)s to (gfsmWeight)s
/// used by gfsm_automaton_rmepsilon()
typedef struct {
  gfsmWeightMap *wm; /**< underlying weight-map */
  gfsmSemiring  *sr; /**< semiring used for comparison */
} gfsmStatePair2WeightMap;

/// Typedef for mapping (gfsmStatePair)s to (gfsmWeight)s
/// used by gfsm_automaton_rmepsilon()
typedef struct {
  GHashTable    *h;  /**< underlying hash */
  gfsmSemiring  *sr; /**< semiring used for comparison */
} gfsmStatePair2WeightHash;

/*======================================================================
 * Methods: gfsmStatePair2WeightHash
 */
///\name gfsmStatePair2WeightHash Methods
//@{


/** create a new gfsmStatePair2WeightHash (copies & frees keys)
 *  \see gfsmWeightHash
 */
gfsmStatePair2WeightHash *gfsm_statepair2weighthash_new(gfsmSemiring *sr);

/** Hash \a sp to \a w in \a spw.
 *  \returns TRUE if \a sp was already present in \a spw with a less-than-or-equal weight */
gboolean gfsm_statepair2weighthash_insert(gfsmStatePair2WeightHash *sp2wh, gfsmStatePair *sp, gfsmWeight w);

/** Lookup weight for \a sp in \a sp2wh .
 *  \returns TRUE if \a sp was already present in \a sp2wh and sets \a *wp to its stored weight */
gboolean gfsm_statepair2weighthash_lookup(gfsmStatePair2WeightHash *sp2wh, gfsmStatePair *sp, gfsmWeight *wp);

/** Clear a gfsmStatePair2WeightHash */
#define gfsm_statepair2weighthash_clear(sp2wh) \
  g_hash_free((sp2wh)->h)

/** Free all memory allocated by a gfsmStatePair2WeightHash */
void gfsm_statepair2weighthash_free(gfsmStatePair2WeightHash *sp2wh);

/** Alias; \sa gfsm_weighthash_foreach() */
#define gfsm_statepair2weighthash_foreach(sp2wh,func,data) \
  gfsm_weighthash_foreach((sp2wh)->h,(func),(data))

//@}


/*======================================================================
 * Methods: gfsmStatePair2WeightMap
 */

/*--------------------------------------------------------------
 * statepair2weightmap_new()
 */
gfsmStatePair2WeightMap *gfsm_statepair2weightmap_new(gfsmSemiring *sr)
{
  gfsmStatePair2WeightMap *sp2wm = g_new(gfsmStatePair2WeightMap,1);
  sp2wm->wm = gfsm_weightmap_new_full((GCompareDataFunc)gfsm_statepair_compare,
				      sr,
				      (GDestroyNotify)g_free);
  sp2wm->sr = sr;
  return sp2wm;
}

/*--------------------------------------------------------------
 * statepair2weightmap_insert()
 */
gboolean gfsm_statepair2weightmap_insert(gfsmStatePair2WeightMap *spw, gfsmStatePair *sp, gfsmWeight w)
{
  gpointer orig_key;
  gpointer orig_val;
  if (g_tree_lookup_extended(spw->wm, sp, &orig_key, &orig_val)) {
    //-- already present: is the stored weight better?
    if (gfsm_sr_less(spw->sr, gfsm_ptr2weight(orig_val), w)) {
      return TRUE;
    }
  }
  //-- either not present or we need to update the stored weight because (w) is better
  gfsm_weightmap_insert(spw->wm, gfsm_statepair_clone(sp), w);
  return FALSE;
}

/*--------------------------------------------------------------
 * statepair2weightmap_lookup()
 */
gboolean gfsm_statepair2weightmap_lookup(gfsmStatePair2WeightMap *spw, gfsmStatePair *sp, gfsmWeight *wp)
{
  gpointer orig_key;
  return g_tree_lookup_extended(spw->wm, sp, &orig_key, &((gpointer*)wp));
}

/*--------------------------------------------------------------
 * statepair2weightmap_free()
 */
void gfsm_statepair2weightmap_free(gfsmStatePair2WeightMap *sp2wm)
{
  gfsm_weightmap_free(sp2wm->wm);
  g_free(sp2wm);
}

/*======================================================================
 * Methods: gfsmStatePair2WeightHash
 */

/*--------------------------------------------------------------
 * statepair2weighthash_new()
 */
gfsmStatePair2WeightHash *gfsm_statepair2weighthash_new(gfsmSemiring *sr)
{
  gfsmStatePair2WeightHash *sp2wh = g_new(gfsmStatePair2WeightHash,1);
  sp2wh->h = g_hash_table_new_full((GHashFunc)gfsm_statepair_hash,
				   (GEqualFunc)gfsm_statepair_equal,
				   (GDestroyNotify)gfsm_statepair_free,
				   NULL);
  sp2wh->sr = sr;
  return sp2wh;
}

/*--------------------------------------------------------------
 * statepair2weighthash_insert()
 */
gboolean gfsm_statepair2weighthash_insert(gfsmStatePair2WeightHash *sp2wh, gfsmStatePair *sp, gfsmWeight w)
{
  gpointer orig_key;
  gpointer orig_val;
  if (g_tree_lookup_extended(spw->h, sp, &orig_key, &orig_val)) {
    //-- already present: is (stored_weight+w) better than (stored_weight)?
    gfsmWeight ow = gfsm_ptr2weight(orig_val);
    w = gfsm_sr_plus(sp2wh->sr, ow, w);
    if (gfsm_sr_less(sp2wh->sr, ow, w)) {
      return TRUE;
    }
    //-- adding new weight is better
    g_hash_table_steal(sp2wh->h,  orig_key);
    g_hash_table_insert(sp2wh->h, orig_key, gfsm_weight2ptr(w));
  }
  else {
    //-- pair (sp) was not present: copy & insert
    g_hash_table_insert(sp2wh->h, gfsm_statepair_clone(sp), gfsm_weight2ptr(w));
  }
  return FALSE; //-- either not present or updated
}

/*--------------------------------------------------------------
 * statepair2weighthash_lookup()
 */
gboolean gfsm_statepair2weighthash_lookup(gfsmStatePair2WeightHash *sp2wh, gfsmStatePair *sp, gfsmWeight *wp)
{
  gpointer orig_key;
  return g_hash_table_lookup_extended(spw->wm, sp, &orig_key, &((gpointer*)wp));
}

/*--------------------------------------------------------------
 * statepair2weighthash_free()
 */
void gfsm_statepair2weighthash_free(gfsmStatePair2WeightHash *sp2wh)
{
  gfsm_weighthash_free(sp2wh->wm);
  g_free(sp2wh);
}


//=================================================================================
// DETERMINIZE (v1)
//=================================================================================

/*--------------------------------------------------------------
 * _determinize_lp2ec_foreach_func()
 */
typedef struct {
  gfsmAutomaton *nfa;
  gfsmAutomaton *dfa;
  gfsmStateId    dfa_src_id;
  gfsmEnum      *ec2id;
  gfsmStateSet  *ec_tmp;
} gfsmLp2EcForeachData;

gboolean _gfsm_determinize_lp2ec_foreach_func(gfsmLabelPair         lp,
					      gfsmWeightedStateSet *wss,
					      gfsmLp2EcForeachData *data)
{
  gfsmStateId    ec2id_val;
  gfsmStateSet  *ec2id_key;

  if ( gfsm_enum_lookup_extended(data->ec2id,
				 wss->set,
				 (gpointer)(&ec2id_key),
				 (gpointer)(&ec2id_val)) )
    {
      //-- target node-set is already present: just add an arc in @dfa
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
      _gfsm_determinize_visit_state(data->nfa,   data->dfa,
				    wss->set,    ec2id_val,
				    data->ec2id, data->ec_tmp);
    }
  return FALSE;
}


/*--------------------------------------------------------------
 * _determinize_visit_state()
 */
void _gfsm_determinize_visit_state(gfsmAutomaton *nfa,    gfsmAutomaton *dfa,
				   gfsmStateSet  *nfa_ec, gfsmStateId    dfa_id,
				   gfsmEnum      *ec2id,  gfsmStateSet  *ec_tmp)
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

	if (a->lower==gfsmEpsilon && a->upper==gfsmEpsilon) continue; //-- ignore eps arcs
	lp = gfsm_labelpair_new(a->lower, a->upper);

	//-- populate state-set with all nodes eps-reachable from this arc's target
	gfsm_stateset_clear(ec_tmp);
	gfsm_stateset_populate_eps(ec_tmp, nfa, a->target);

	//-- add equivalence class to local mapping
	if ( g_tree_lookup_extended(lp2ecw,
				    (gpointer)lp,
				    (gpointer)(&lp2ec_key),
				    (gpointer)(&lp2ec_val)) )
	  {
	    //-- already present: compute union and add new arc's weight
	    gfsm_stateset_union(lp2ec_val->set, ec_tmp);
	    lp2ec_val->weight = gfsm_sr_plus(nfa->sr, lp2ec_val->weight, a->weight);
	  }
	else
	  {
	    //-- not yet present: insert new value
	    lp2ec_val         = g_new(gfsmWeightedStateSet,1);
	    lp2ec_val->set    = gfsm_stateset_clone(ec_tmp);
	    lp2ec_val->weight = a->weight;
	    g_tree_insert(lp2ecw, (gpointer)lp, lp2ec_val);
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
  lp2ec_foreach_data.ec_tmp      = ec_tmp;
  g_tree_foreach(lp2ecw,
		 (GTraverseFunc)_gfsm_determinize_lp2ec_foreach_func,
		 (gpointer)(&lp2ec_foreach_data));

  //-- cleanup
  g_tree_destroy(lp2ecw);

  /*
  g_printerr("_gfsm_automaton_determinize_visit_state(): not yet implemented!");
  g_assert_not_reached();
  */
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
  gfsmStateSet  *ec_tmp; //-- (temp) equiv-class@nfa

  //-- sanity check(s)
  if (!nfa) return NULL;
  else if (nfa->flags.is_deterministic) {
    if (dfa) gfsm_automaton_copy(dfa,nfa);
    else     dfa = gfsm_automaton_clone(nfa);
    return dfa;
  }

  //-- initialization: nfa
  //gfsm_automaton_arcsort(nfa,gfsmASMLower);

  //-- initialization: dfa
  if (!dfa) {
    dfa = gfsm_automaton_shadow(nfa);
  } else {
    gfsm_automaton_clear(dfa);
    gfsm_automaton_copy_shallow(dfa,nfa);
  }
  dfa->flags.sort_mode = gfsmASMNone; //-- avoid "smart" arc-insertion

  //-- initialization: ec2id
  ec2id = gfsm_enum_new_full(NULL /*(gfsmDupFunc)gfsm_stateset_clone*/ ,
			     (GHashFunc)gfsm_stateset_hash,
			     (GEqualFunc)gfsm_stateset_equal,
			     (GDestroyNotify)gfsm_stateset_free);

  //-- initialization: nfa_ec
  nfa_ec = gfsm_stateset_sized_new(32);
  ec_tmp = gfsm_stateset_sized_new(32);
  gfsm_stateset_populate_eps(nfa_ec, nfa, nfa->root_id);

  //-- set root in dfa
  dfa_id = gfsm_automaton_ensure_state(dfa, gfsm_enum_insert(ec2id, nfa_ec));
  gfsm_automaton_set_root(dfa, dfa_id);

  //-- guts: determinize recursively outwards from root node
  _gfsm_determinize_visit_state(nfa, dfa, nfa_ec, dfa_id, ec2id, ec_tmp);

  //-- set flag in dfa
  dfa->flags.is_deterministic = TRUE;

  //-- cleanup
  //gfsm_stateset_free(nfa_ec); //-- this ought to be freed by gfsm_enum_free(ec2id)
  gfsm_stateset_free(ec_tmp);   //-- ... but not this
  gfsm_enum_free(ec2id);

  return dfa;
}

//=================================================================================
// END DETERMINIZE (v1)
//=================================================================================



//-------------------------------------------------------------------------------
#define GFSM_ALPHABET_CSET \
  "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz!\"#$%&'()*+,-./0123456789:;<=>?" \
  "\241\242\243\244\245\246\247\250\251\252\253\254\255\256\257\260\261\262\263\264" \
  "\265\266\267\270\271\272\273\274\275\276\277\300\301\302\303\304\305\306\307\310" \
  "\311\312\313\314\315\316\317\320\321\322\323\324\325\326\327\330\331\332\333\334" \
  "\335\336\337\340\341\342\343\344\345\346\347\350\351\352\353\354\355\356\357\360"

const GScannerConfig gfsm_alphabet_scanner_config = {
  /* Character sets */
  ( " \t\r\n" )             /* skip chars */,
  ( GFSM_ALPHABET_CSET )    /* identifier_first */,
  ( GFSM_ALPHABET_CSET )    /* identifier_nth */,
  "" 		            /* comment_single */,

  FALSE                     /* case_sensitive */, 
  FALSE                     /* skip_comment_multi */,
  FALSE                     /* skip_comment_single */,
  FALSE                     /* scan_comment_multi */,
  FALSE                     /* scan_comment_single */,
  TRUE                      /* scan_identifier */,
  TRUE                      /* scan_identifier_1char */,
  FALSE                     /* scan_identifier_NULL */,
  FALSE                     /* scan_symbols */,

  FALSE                     /* scan_binary */,
  TRUE                      /* scan_octal */,
  FALSE                     /* scan_float */,
  TRUE                      /* scan_hex */,
  FALSE                     /* scan_hex_dollar */,

  FALSE                     /* scan_string_sq : string: 'anything' */,
  FALSE 		    /* scan_string_dq : string: "\\-escapes!\n" */,
  TRUE                      /* numbers_2_int : bin, octal, hex => int */,
  FALSE		            /* int_2_float : int => G_TOKEN_FLOAT? */,
  FALSE 		    /* identifier_2_string : 1 */,
  FALSE                     /* 2_token : 1 : return G_TOKEN_CHAR? */,
  FALSE		            /* symbol_2_token : 1 */,
  FALSE                     /* scope_0_fallback : try scope 0 on lookups? */,
  //FALSE                     /* store_int64 : use value.v_int64 rather than v_int */,
  //0                         /* padding_dummy; */
};

gboolean gfsm_automaton_compile_file (gfsmAutomaton *fsm, FILE *f, const gchar *filename)
{
  GScanner *scanner = g_scanner_new(&gfsm_automaton_scanner_config);
  GTokenType typ;

  g_scanner_input_file(scanner, fileno(f));
  while ((typ = g_scanner_get_next_token(scanner)) != G_TOKEN_EOF) {
    g_printerr("<DEBUG:SCANNER:%u.%u> typ=%d : ",
	       g_scanner_cur_line(scanner), g_scanner_cur_position(scanner), typ);

    switch (typ) {
    case G_TOKEN_INT:
      g_printerr("(INT) value=%ld\n", g_scanner_cur_value(scanner).v_int);
      break;

    case G_TOKEN_FLOAT:
      g_printerr("(FLOAT) value=%g\n", g_scanner_cur_value(scanner).v_float);
      break;

    case G_TOKEN_CHAR:
      g_printerr("(CHAR) value=%d\n", g_scanner_cur_value(scanner).v_char);
      break;

    default:
      g_printerr("(?) text='%s'\n", scanner->text); //'text' is private: debugging use only!
      break;
    }
  }
  return TRUE;
}


/*--------------------------------------------------------------
 * scanner template for compile_file()
 */
const GScannerConfig gfsm_automaton_scanner_config = {
  /* Character sets */
  ( " \t\r" )             /* skip chars */,
  ( "" )                  /* identifier_first */,
  ( "" )                  /* identifier_nth */,
  "#\n" 	          /* comment_single */,

  FALSE                     /* case_sensitive */, 

  FALSE                     /* skip_comment_multi */,
  FALSE                     /* skip_comment_single */,
  FALSE                     /* scan_comment_multi */,

  FALSE                     /* scan_identifier */,
  FALSE                     /* scan_identifier_1char */,
  FALSE                     /* scan_identifier_NULL */,
  FALSE                     /* scan_symbols */,

  FALSE                     /* scan_binary */,
  TRUE                      /* scan_octal */, 
  TRUE                      /* scan_float */,
  TRUE                      /* scan_hex */,
  FALSE                     /* scan_hex_dollar */,

  FALSE                     /* scan_string_sq : string: 'anything' */,
  FALSE 		    /* scan_string_dq : string: "\\-escapes!\n" */,
  TRUE                      /* numbers_2_int : bin, octal, hex => int */,
  FALSE		            /* int_2_float : int => G_TOKEN_FLOAT? */,
  FALSE 		    /* identifier_2_string : 1 */,
  FALSE                     /* 2_token : 1 : return G_TOKEN_CHAR? */,
  FALSE		            /* symbol_2_token : 1 */,
  FALSE                     /* scope_0_fallback : try scope 0 on lookups? */,
  FALSE                     /* store_int64 : use value.v_int64 rather than v_int */,
  //0                         /* padding_dummy; */
};

/*--------------------------------------------------------------
 * gfsm_automaton_scan_field()
 */
gboolean gfsm_automaton_scan_field(GScanner  *scanner,
				   GTokenType expect1,
				   GTokenType expect2,
				   gboolean   complain_on_newline)
{
  GTokenType typ;
  while (1) {
    typ = g_scanner_get_next_token(scanner);
    if (expect1 != typ
	&& expect2 != typ
	&& (typ == G_TOKEN_CHAR || typ == G_TOKEN_EOF)
	&& g_scanner_cur_value(scanner).v_char == '\n')
      {
	//-- return but don't complain on unexpected newlines
	if (complain_on_newline)
	  g_scanner_unexp_token(scanner, expect1, NULL, NULL, NULL, NULL, TRUE);
	return FALSE;
      }

    if (expect1 != typ && expect2 != typ) {
      g_scanner_unexp_token(scanner, expect1, NULL, NULL, NULL, NULL, TRUE);
      return FALSE;
    }
    return TRUE;
  }
  return FALSE;
}
