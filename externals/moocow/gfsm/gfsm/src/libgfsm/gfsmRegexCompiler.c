
/*=============================================================================*\
 * File: gfsmRegexCompiler.c
 * Author: Bryan Jurish <moocow@ling.uni-potsdam.de>
 * Description: finite state machine library
 *
 * Copyright (c) 2005 Bryan Jurish.
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

#include <gfsmRegexCompiler.h>
#include <gfsmArith.h>
#include <gfsmUtils.h>

#include "gfsmRegex.tab.h"
#include "gfsmRegex.lex.h"

extern int gfsmRegex_yyparse(gfsmRegexCompiler *rec);

/*======================================================================
 * Regex Compiler: Constructors etc.
 */

//--------------------------------------------------------------
gfsmRegexCompiler *gfsm_regex_compiler_new_full(const gchar  *name,
						gfsmAlphabet *abet,
						gfsmSRType    srtype,
						gboolean      emit_warnings)
{
  gfsmRegexCompiler *rec = g_new0(gfsmRegexCompiler,1);
  char *myname = (name ? ((char*)name) : "gfsmRegexCompiler");
  gfsm_scanner_init(&(rec->scanner), myname, gfsmRegex_yy);
  rec->fsm   = NULL;
  rec->abet  = abet;
  rec->srtype = srtype;
  rec->scanner.emit_warnings = emit_warnings;
  rec->gstr = g_string_new("");
  return rec;
}

//--------------------------------------------------------------
void gfsm_regex_compiler_free(gfsmRegexCompiler *rec, gboolean free_alphabet, gboolean free_automaton)
{
  if (free_alphabet && rec->abet) gfsm_alphabet_free(rec->abet);
  if (free_automaton && rec->fsm) gfsm_automaton_free(rec->fsm);
  g_string_free(rec->gstr,TRUE);
  gfsm_scanner_free(&(rec->scanner)); //-- ought to free the rest
}

//--------------------------------------------------------------
void gfsm_regex_compiler_reset(gfsmRegexCompiler *rec, gboolean free_automaton)
{
  if (free_automaton && rec->fsm) gfsm_automaton_free(rec->fsm);
  g_string_truncate(rec->gstr,0);
  rec->fsm = NULL;
  g_clear_error(&(rec->scanner.err));
}

/*======================================================================
 * Regex Compiler: Methods
 */

//--------------------------------------------------------------
gfsmAutomaton *gfsm_regex_compiler_parse(gfsmRegexCompiler *rec)
{
  gfsmRegex_yyparse(rec);
  if (rec->scanner.err) {
    if (rec->fsm) gfsm_automaton_free(rec->fsm);
    rec->fsm = NULL;
  }
  return rec->fsm;
}



/*======================================================================
 * Regex Compiler: Alphabet Utilities
 */

//--------------------------------------------------------------
gfsmLabelVal gfsm_regex_compiler_char2label(gfsmRegexCompiler *rec, gchar c)
{
  gchar cs[2] = {c,'\0'};
  gfsmLabelVal lab = gfsm_alphabet_find_label(rec->abet, cs);
  if (lab==gfsmNoLabel) {
    gfsm_scanner_carp(&(rec->scanner),
		      "Warning: no label for character '%c' in alphabet: using gfsmNoLabel", c);
    g_clear_error(&(rec->scanner.err));
  }
  return lab;
}

//--------------------------------------------------------------
gfsmLabelVal gfsm_regex_compiler_gstring2label(gfsmRegexCompiler *rec, GString *gs)
{
  gfsmLabelVal lab = gfsm_alphabet_find_label(rec->abet, gs->str);
  if (lab==gfsmNoLabel) {
    gfsm_scanner_carp(&(rec->scanner),
		      "Warning: no label for string '%s' in alphabet: using gfsmNoLabel", gs->str);
    g_clear_error(&(rec->scanner.err));
  }
  g_string_free(gs,TRUE);
  return lab;
}


/*======================================================================
 * Regex Compiler: Automaton Utilities
 */
#define RETURN(rec,_fsm) (rec)->fsm=(_fsm); return (_fsm);

//--------------------------------------------------------------
gfsmAutomaton *gfsm_regex_compiler_new_fsm(gfsmRegexCompiler *rec)
{
  gfsmAutomaton *fsm = gfsm_automaton_new_full(gfsmAutomatonDefaultFlags,
					       rec->srtype,
					       gfsmAutomatonDefaultSize);
  fsm->flags.is_transducer = FALSE;
  return fsm;
}

//--------------------------------------------------------------
gfsmAutomaton *gfsm_regex_compiler_epsilon_fsm(gfsmRegexCompiler *rec)
{
  gfsmAutomaton *fsm = gfsm_regex_compiler_new_fsm(rec);
  fsm->root_id = gfsm_automaton_add_state(fsm);
  gfsm_automaton_set_final_state(fsm,fsm->root_id,TRUE);
  RETURN(rec,fsm);
}

//--------------------------------------------------------------
gfsmAutomaton *gfsm_regex_compiler_label_fsm(gfsmRegexCompiler *rec, gfsmLabelVal lab)
{
  gfsmAutomaton *fsm = gfsm_regex_compiler_new_fsm(rec);
  gfsmStateId    labid;
  fsm->root_id = gfsm_automaton_add_state(fsm);
  labid        = gfsm_automaton_add_state(fsm);
  gfsm_automaton_add_arc(fsm, fsm->root_id, labid, lab, lab, fsm->sr->one);
  gfsm_automaton_set_final_state(fsm,labid,TRUE);
  RETURN(rec,fsm);
}

//--------------------------------------------------------------
gfsmAutomaton *gfsm_regex_compiler_concat(gfsmRegexCompiler *rec,
					      gfsmAutomaton *fsm1,
					      gfsmAutomaton *fsm2)
{
  gfsm_automaton_concat(fsm1, fsm2);
  gfsm_automaton_free(fsm2);
  RETURN(rec,fsm1);
}


//--------------------------------------------------------------
struct gfsm_regex_append_lab_data_ {
  gfsmAutomaton     *fsm;
  gfsmLabelVal       lab;
  gfsmStateId        newid;
};

gboolean _gfsm_regex_append_lab_foreach_func(gfsmStateId qid, gpointer pw,
					     struct gfsm_regex_append_lab_data_ *data)
{
  gfsm_automaton_get_state(data->fsm,qid)->is_final = FALSE;
  gfsm_automaton_add_arc(data->fsm, qid, data->newid, data->lab, data->lab, gfsm_ptr2weight(pw));
  return FALSE;
}

gfsmAutomaton *gfsm_regex_compiler_append_lab(gfsmRegexCompiler *rec, gfsmAutomaton *fsm, gfsmLabelVal lab)
{
  struct gfsm_regex_append_lab_data_ data = { fsm, lab, gfsm_automaton_add_state(fsm) };
  gfsm_weightmap_foreach(fsm->finals,
			 (GTraverseFunc)_gfsm_regex_append_lab_foreach_func,
			 &data);
  gfsm_weightmap_clear(fsm->finals);
  gfsm_automaton_set_final_state(fsm, data.newid, TRUE);
  RETURN(rec,fsm);
}

//--------------------------------------------------------------
gfsmAutomaton *gfsm_regex_compiler_prepend_lab(gfsmRegexCompiler *rec, gfsmLabelVal lab, gfsmAutomaton *fsm)
{
  gfsmStateId qid = gfsm_automaton_add_state(fsm);
  gfsm_automaton_add_arc(fsm, qid, fsm->root_id, lab, lab, fsm->sr->one);
  fsm->root_id = qid;
  RETURN(rec,fsm);
}


//--------------------------------------------------------------
gfsmAutomaton *gfsm_regex_compiler_closure(gfsmRegexCompiler *rec, gfsmAutomaton *fsm, gboolean is_plus)
{
  gfsm_automaton_closure(fsm,is_plus);
  RETURN(rec,fsm);
}

//--------------------------------------------------------------
gfsmAutomaton *gfsm_regex_compiler_power(gfsmRegexCompiler *rec, gfsmAutomaton *fsm, guint32 n)
{
  gfsm_automaton_n_closure(fsm,n);
  RETURN(rec,fsm);
}

//--------------------------------------------------------------
gfsmAutomaton *gfsm_regex_compiler_project(gfsmRegexCompiler *rec,
						gfsmAutomaton *fsm,
						gfsmLabelSide which)
{
  gfsm_automaton_project(fsm,which);
  RETURN(rec,fsm);
}

//--------------------------------------------------------------
gfsmAutomaton *gfsm_regex_compiler_optional(gfsmRegexCompiler *rec, gfsmAutomaton *fsm)
{
  gfsm_automaton_optional(fsm);
  RETURN(rec,fsm);
}

//--------------------------------------------------------------
gfsmAutomaton *gfsm_regex_compiler_complement(gfsmRegexCompiler *rec, gfsmAutomaton *fsm)
{
  gfsm_automaton_complement_full(fsm,rec->abet);
  RETURN(rec,fsm);
}

//--------------------------------------------------------------
gfsmAutomaton *gfsm_regex_compiler_union(gfsmRegexCompiler *rec, gfsmAutomaton *fsm1, gfsmAutomaton *fsm2)
{
  gfsm_automaton_union(fsm1,fsm2);
  gfsm_automaton_free(fsm2);
  RETURN(rec,fsm1);
}

//--------------------------------------------------------------
gfsmAutomaton *gfsm_regex_compiler_intersect(gfsmRegexCompiler *rec, gfsmAutomaton *fsm1, gfsmAutomaton *fsm2)
{
  gfsm_automaton_intersect(fsm1,fsm2);
  gfsm_automaton_free(fsm2);
  RETURN(rec,fsm1);
}

//--------------------------------------------------------------
gfsmAutomaton *gfsm_regex_compiler_product(gfsmRegexCompiler *rec, gfsmAutomaton *fsm1, gfsmAutomaton *fsm2)
{
  gfsm_automaton_product2(fsm1,fsm2);
  gfsm_automaton_free(fsm2);
  RETURN(rec,fsm1);
}

//--------------------------------------------------------------
gfsmAutomaton *gfsm_regex_compiler_compose(gfsmRegexCompiler *rec, gfsmAutomaton *fsm1, gfsmAutomaton *fsm2)
{
  gfsm_automaton_compose(fsm1,fsm2);
  gfsm_automaton_free(fsm2);
  RETURN(rec,fsm1);
}

//--------------------------------------------------------------
gfsmAutomaton *gfsm_regex_compiler_difference(gfsmRegexCompiler *rec, gfsmAutomaton *fsm1, gfsmAutomaton *fsm2)
{
  gfsm_automaton_difference(fsm1,fsm2);
  gfsm_automaton_free(fsm2);
  RETURN(rec,fsm1);
}

//--------------------------------------------------------------
/** Weight */
gfsmAutomaton *gfsm_regex_compiler_weight(gfsmRegexCompiler *rec, gfsmAutomaton *fsm, gfsmWeight w)
{
  gfsm_automaton_arith_final(fsm, gfsmAOSRTimes, w, FALSE);
  RETURN(rec,fsm);
}


//--------------------------------------------------------------
gfsmAutomaton *gfsm_regex_compiler_rmepsilon(gfsmRegexCompiler *rec, gfsmAutomaton *fsm)
{
  gfsm_automaton_rmepsilon(fsm);
  //gfsm_automaton_connect(fsm);
  //gfsm_automaton_renumber_states(fsm);
  RETURN(rec,fsm);
}


//--------------------------------------------------------------
gfsmAutomaton *gfsm_regex_compiler_determinize(gfsmRegexCompiler *rec, gfsmAutomaton *fsm)
{
  gfsm_automaton_determinize(fsm);
  RETURN(rec,fsm);
}

//--------------------------------------------------------------
gfsmAutomaton *gfsm_regex_compiler_connect(gfsmRegexCompiler *rec, gfsmAutomaton *fsm)
{
  gfsm_automaton_connect(fsm);
  gfsm_automaton_renumber_states(fsm);
  RETURN(rec,fsm);
}

#undef RETURN

