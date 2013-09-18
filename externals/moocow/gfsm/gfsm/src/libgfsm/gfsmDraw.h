
/*=============================================================================*\
 * File: gfsmDraw.h
 * Author: Bryan Jurish <moocow@ling.uni-potsdam.de>
 * Description: finite state machine library: automata: visualization
 *
 * Copyright (c) 2004-2007 Bryan Jurish.
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

/** \file gfsmDraw.h
 *  \brief Automaton visualization utilities
 */

#ifndef _GFSM_DRAW_H
#define _GFSM_DRAW_H

#include <gfsmAutomaton.h>

/*======================================================================
 * Automaton Methods: Visualization: vcg
 */
/// \name Automaton Methods: Visualization: vcg
//@{

/** Draw an automaton in VCG format to a FILE*  */
gboolean gfsm_automaton_draw_vcg_file_full (gfsmAutomaton *fsm,
					    FILE          *f,
					    gfsmAlphabet  *lo_alphabet,
					    gfsmAlphabet  *hi_alphabet,
					    gfsmAlphabet  *state_alphabet,
					    const gchar   *title,
					    int            xspace, // ?
					    int            yspace, // ?
					    const gchar   *orientation, // "(top|bottom|left|right)_to_(ditto)"
					    const gchar   *state_shape,
					    const gchar   *state_color,
					    const gchar   *final_color,
					    gfsmError    **errp);

/** Draw an automaton in VCG format to a named file */
gboolean gfsm_automaton_draw_vcg_filename_full (gfsmAutomaton *fsm,
						const gchar   *filename,
						gfsmAlphabet  *lo_alphabet,
						gfsmAlphabet  *hi_alphabet,
						gfsmAlphabet  *state_alphabet,
						const gchar   *title,
						int            xspace, // ?
						int            yspace, // ?
						const gchar   *orientation, // "(top|bottom|left|right)_to_(ditto)"
						const gchar   *state_shape,
						const gchar   *state_color,
						const gchar   *final_color,
						gfsmError    **errp);
//@}


/*======================================================================
 * Automaton Methods: Visualization: dot
 */
///\name Automaton Methods: Visualization: dot
//@{

/** Draw an automaton in Ma-Bell .dot format to a FILE*  */
gboolean gfsm_automaton_draw_dot_file_full (gfsmAutomaton *fsm,
					    FILE          *f,
					    gfsmAlphabet  *lo_alphabet,
					    gfsmAlphabet  *hi_alphabet,
					    gfsmAlphabet  *state_alphabet,
					    const gchar   *title,
					    float          width,
					    float          height,
					    int            fontsize,
					    const gchar   *fontname,
					    gboolean       portrait,
					    gboolean       vertical,
					    float          nodesep,
					    float          ranksep,
					    gfsmError    **errp);

/** Draw an automaton in Ma-Bell .dot format to a named file */
gboolean gfsm_automaton_draw_dot_filename_full (gfsmAutomaton *fsm,
						const gchar   *filename,
						gfsmAlphabet  *lo_alphabet,
						gfsmAlphabet  *hi_alphabet,
						gfsmAlphabet  *state_alphabet,
						const gchar   *title,
						float          width,
						float          height,
						int            fontsize,
						const gchar   *fontname,
						gboolean       portrait,
						gboolean       vertical,
						float          nodesep,
						float          ranksep,
						gfsmError    **errp);
//@}


#endif /* _GFSM_DRAW_H */
