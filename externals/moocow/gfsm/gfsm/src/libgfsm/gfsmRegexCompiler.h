
/*=============================================================================*\
 * File: gfsmRegexCompiler.h
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

/** \file gfsmRegexCompiler.h
 *  \brief Regular expression compiler
 *
 *  \file gfsmRegex.lex.h
 *  \brief flex-generated lexer headers for gfsmRegexCompiler
 *
 *  \file gfsmRegex.tab.h
 *  \brief bison-generated parser headers for gfsmRegexCompiler
 *
 *  \union YYSTYPE
 *  \brief bison-generated parser rule-value union
 *
 *  \struct yy_buffer_state
 *  \brief flex-generated lexer input buffer state struct
 */

#ifndef _GFSM_REGEX_COMPILER_H
#define _GFSM_REGEX_COMPILER_H

#include <gfsmScanner.h>
#include <gfsmAlgebra.h>

/*======================================================================
 * Types
 */

/** \brief Type for a regular expression compiler */
typedef struct {
  gfsmScanner         scanner; ///< underlying scanner
  gfsmSRType          srtype;  ///< semiring type
  gfsmAutomaton      *fsm;     ///< regex automaton under construction
  gfsmAlphabet       *abet;    ///< alphabet
  GString            *gstr;    ///< string buffer
} gfsmRegexCompiler;

/*======================================================================
 * Regex Compiler: Constructors etc.
 */
///\name Regex Compiler: Constructors etc.
//@{

/** Create and return a new gfsmRegexCompiler */
gfsmRegexCompiler *gfsm_regex_compiler_new_full(const gchar  *name,
						gfsmAlphabet *abet,
						gfsmSRType    srtype,
						gboolean      emit_warnings);

/** Create and return a new gfsmRegexCompiler, no alphabet */
#define gfsm_regex_compiler_new() \
  gfsm_regex_compiler_new_full("gfsmRegexCompiler", NULL, gfsmAutomatonDefaultSRType, TRUE);

/** Destroy a gfsmRegexCompiler.
 *  \param free_automaton whether to free the stored alphabet, if present
 *  \param free_automaton whether to free the parsed automaton, if present
 */
void gfsm_regex_compiler_free(gfsmRegexCompiler *rec, gboolean free_alphabet, gboolean free_automaton);

/** Reset regex compiler; possibly freeing associated automaton */
void gfsm_regex_compiler_reset(gfsmRegexCompiler *rec, gboolean free_automaton);

//@}

/*======================================================================
 * Regex Compiler: Methods
 */
//@{

/**
 * Parse an automaton from the currently selected input source.
 * \returns parsed automaton, or \a NULL on error
 */
gfsmAutomaton *gfsm_regex_compiler_parse(gfsmRegexCompiler *rec);

//@}

/*======================================================================
 * Regex Compiler: Alphabet Utilities
 */
///\name Regex Compiler: Alphabet Utilities
//@{

/** Get a label value for a single character */
gfsmLabelVal gfsm_regex_compiler_char2label(gfsmRegexCompiler *rec, gchar c);

/** Get a label value for a GString* (implicitly frees \a gs) */
gfsmLabelVal gfsm_regex_compiler_gstring2label(gfsmRegexCompiler *rec, GString *gs);

//@}

/*======================================================================
 * Regex Compiler: Automaton Utilities
 */
///\name Regex Compiler: Automaton Utilities
//@{

//--------------------------------------------------------------
/** New full-fleded automaton */
gfsmAutomaton *gfsm_regex_compiler_new_fsm(gfsmRegexCompiler *rec);

/** New Epsilon recognizer */
gfsmAutomaton *gfsm_regex_compiler_epsilon_fsm(gfsmRegexCompiler *rec);

/** New single-character recognizer */
gfsmAutomaton *gfsm_regex_compiler_label_fsm(gfsmRegexCompiler *rec, gfsmLabelVal lab);


//--------------------------------------------------------------
/** Single-label concatenation: (low-level): append */
gfsmAutomaton *gfsm_regex_compiler_append_lab(gfsmRegexCompiler *rec,
					      gfsmAutomaton    *fsm,
					      gfsmLabelVal lab);

/** Single-label concatenation (low-level): prepend */
gfsmAutomaton *gfsm_regex_compiler_prepend_lab(gfsmRegexCompiler *rec,
					       gfsmLabelVal       lab,
					       gfsmAutomaton     *fsm);

/** General concatenation */
gfsmAutomaton *gfsm_regex_compiler_concat(gfsmRegexCompiler *rec,
					  gfsmAutomaton *fsm1,
					  gfsmAutomaton *fsm2);

//--------------------------------------------------------------
/** Closure */
gfsmAutomaton *gfsm_regex_compiler_closure(gfsmRegexCompiler *rec,
					   gfsmAutomaton *fsm,
					   gboolean is_plus);

/** Power (n-ary closure) */
gfsmAutomaton *gfsm_regex_compiler_power(gfsmRegexCompiler *rec,
					 gfsmAutomaton *fsm,
					 guint32 n);

/** Optionality */
gfsmAutomaton *gfsm_regex_compiler_optional(gfsmRegexCompiler *rec, gfsmAutomaton *fsm);

//--------------------------------------------------------------
/** Projection */
gfsmAutomaton *gfsm_regex_compiler_project(gfsmRegexCompiler *rec,
					   gfsmAutomaton *fsm,
					   gfsmLabelSide which);

//--------------------------------------------------------------
/** Complement */
gfsmAutomaton *gfsm_regex_compiler_complement(gfsmRegexCompiler *rec,
					      gfsmAutomaton *fsm);

//--------------------------------------------------------------
/** Union */
gfsmAutomaton *gfsm_regex_compiler_union(gfsmRegexCompiler *rec,
					 gfsmAutomaton *fsm1,
					 gfsmAutomaton *fsm2);

//--------------------------------------------------------------
/** Intersection */
gfsmAutomaton *gfsm_regex_compiler_intersect(gfsmRegexCompiler *rec,
					     gfsmAutomaton *fsm1,
					     gfsmAutomaton *fsm2);

//--------------------------------------------------------------
/** Product */
gfsmAutomaton *gfsm_regex_compiler_product(gfsmRegexCompiler *rec,
					   gfsmAutomaton *fsm1,
					   gfsmAutomaton *fsm2);

//--------------------------------------------------------------
/** Composition */
gfsmAutomaton *gfsm_regex_compiler_compose(gfsmRegexCompiler *rec,
					   gfsmAutomaton *fsm1,
					   gfsmAutomaton *fsm2);

//--------------------------------------------------------------
/** Difference */
gfsmAutomaton *gfsm_regex_compiler_difference(gfsmRegexCompiler *rec,
					      gfsmAutomaton *fsm1,
					      gfsmAutomaton *fsm2);

//--------------------------------------------------------------
/** Weight (final) */
gfsmAutomaton *gfsm_regex_compiler_weight(gfsmRegexCompiler *rec,
					  gfsmAutomaton *fsm1,
					  gfsmWeight w);

//--------------------------------------------------------------
/** Remove epsilons */
gfsmAutomaton *gfsm_regex_compiler_rmepsilon(gfsmRegexCompiler *rec, gfsmAutomaton *fsm);


//--------------------------------------------------------------
/** Determinize */
gfsmAutomaton *gfsm_regex_compiler_determinize(gfsmRegexCompiler *rec, gfsmAutomaton *fsm);

//--------------------------------------------------------------
/** Connect */
gfsmAutomaton *gfsm_regex_compiler_connect(gfsmRegexCompiler *rec, gfsmAutomaton *fsm);

//@}


#endif /* _GFSM_REGEX_COMPILER_H */
