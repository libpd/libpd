
/*=============================================================================*\
 * File: gfsmIndexedIO.h
 * Author: Bryan Jurish <moocow@ling.uni-potsdam.de>
 * Description: finite state machine library: indexed automata: I/O
 *
 * Copyright (c) 2007 Bryan Jurish.
 *
 * For information on usage and redistribution, and for a DISCLAIMER
 * OF ALL WARRANTIES, see the file "COPYING" in this distribution.
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

/** \file gfsmIndexedIO.h
 *  \brief Librarian routines for indexed automata.
 */

#ifndef _GFSM_INDEXED_IO_H
#define _GFSM_INDEXED_IO_H

#include <gfsmAutomatonIO.h>
#include <gfsmIndexed.h>

/*======================================================================
 * Types
 */
/// Header info for binary files
typedef struct {
  gchar              magic[16];    /**< magic header string "gfsm_indexed" */
  gfsmVersionInfo    version;      /**< gfsm version which created the stored file */
  gfsmVersionInfo    version_min;  /**< minimum gfsm version required to load the file */
  gfsmAutomatonFlags flags;        /**< automaton flags */
  gfsmStateId        root_id;      /**< Id of root node */
  gfsmStateId        n_states;     /**< number of stored states */
  gfsmStateId        n_arcs;       /**< number of stored arcs */
  guint32            srtype;       /**< semiring type (cast to ::gfsmSRType) */
  guint32            sort_mask;    /**< arc-sort priorities (a ::gfsmArcCompMask) */
  guint32            reserved2;    /**< reserved */
  guint32            reserved3;    /**< reserved */
} gfsmIndexedAutomatonHeader;

/*======================================================================
 * Constants
 */

/** Magic header string for stored ::gfsmIndexedAutomaton files */
extern const gchar gfsm_indexed_header_magic[16];

/** Minimum libgfsm version required for loading files stored by this version of libgfsm */
extern const gfsmVersionInfo gfsm_indexed_version_bincompat_min_store;

/** Minimum libgfsm version whose binary files this version of libgfsm can read */
extern const gfsmVersionInfo gfsm_indexed_version_bincompat_min_check;

/*======================================================================
 * Methods: Binary I/O
 */
/// \name Indexed Automaton Methods: Binary I/O
//@{

/** Load an automaton header from a stored binary file.
 *  Returns TRUE iff the header looks valid. */
gboolean gfsm_indexed_automaton_load_header(gfsmIndexedAutomatonHeader *hdr, gfsmIOHandle *ioh, gfsmError **errp);

/** Load an automaton from a named binary file (implicitly clear()s \a fsm) */
gboolean gfsm_indexed_automaton_load_bin_handle(gfsmIndexedAutomaton *fsm, gfsmIOHandle *ioh, gfsmError **errp);

/** Load an automaton from a stored binary file (implicitly clear()s \a fsm) */
gboolean gfsm_indexed_automaton_load_bin_file(gfsmIndexedAutomaton *fsm, FILE *f, gfsmError **errp);

/** Load an automaton from a named binary file (implicitly clear()s \a fsm) */
gboolean gfsm_indexed_automaton_load_bin_filename(gfsmIndexedAutomaton *fsm, const gchar *filename, gfsmError **errp);

/** Load an automaton from an in-memory buffer */
gboolean gfsm_indexed_automaton_load_bin_gstring(gfsmIndexedAutomaton *fsm, GString *gs, gfsmError **errp);

/*--------------------------------------------------------------*/

/** Store an automaton in binary form to a gfsmIOHandle* */
gboolean gfsm_indexed_automaton_save_bin_handle(gfsmIndexedAutomaton *fsm, gfsmIOHandle *ioh, gfsmError **errp);

/** Store an automaton in binary form to a file */
gboolean gfsm_indexed_automaton_save_bin_file(gfsmIndexedAutomaton *fsm, FILE *f, gfsmError **errp);

/** Store an automaton to a named binary file (no compression) */
gboolean gfsm_indexed_automaton_save_bin_filename_nc(gfsmIndexedAutomaton *fsm, const gchar *filename, gfsmError **errp);

/** Store an automaton to a named binary file, possibly compressing.
 *  Set \a zlevel=-1 for default compression, and
 *  set \a zlevel=0  for no compression, otherwise should be as for zlib (1 <= zlevel <= 9)
 */
gboolean gfsm_indexed_automaton_save_bin_filename(gfsmIndexedAutomaton *fsm, const gchar *filename, int zlevel, gfsmError **errp);

/** Append an uncompressed binary automaton to an in-memory buffer */
gboolean gfsm_indexed_automaton_save_bin_gstring(gfsmIndexedAutomaton *fsm, GString *gs, gfsmError **errp);

//@}

/*======================================================================
 * Automaton Methods: Text I/O
 */
/// \name Automaton Methods: Text I/O (output only)
//@{

/** Print a ::gfsmIndexedAutomaton in Ma-Bell-compatible text-format to a ::gfsmIOHandle* */
gboolean gfsm_indexed_automaton_print_handle (gfsmIndexedAutomaton *fsm,
					      gfsmIOHandle  *ioh,
					      gfsmAlphabet  *lo_alphabet,
					      gfsmAlphabet  *hi_alphabet,
					      gfsmAlphabet  *state_alphabet,
					      gfsmError     **errp);


/** Print an automaton in Ma-Bell-compatible text-format to a FILE*  */
gboolean gfsm_indexed_automaton_print_file_full (gfsmIndexedAutomaton *fsm,
						 FILE          *f,
						 gfsmAlphabet  *lo_alphabet,
						 gfsmAlphabet  *hi_alphabet,
						 gfsmAlphabet  *state_alphabet,
						 int            zlevel,
						 gfsmError     **errp);

/** Convenience macro for printing to uncompresed all-numeric-id text streams  */
#define gfsm_indexed_automaton_print_file(fsm,f,errp) \
  gfsm_indexed_automaton_print_file_full(fsm,f,NULL,NULL,NULL,0,errp)

/** Print an automaton in Ma-Bell-compatible text-format to a named file */
gboolean gfsm_indexed_automaton_print_filename_full (gfsmIndexedAutomaton *fsm,
						     const gchar   *filename,
						     gfsmAlphabet  *lo_alphabet,
						     gfsmAlphabet  *hi_alphabet,
						     gfsmAlphabet  *state_alphabet,
						     int            zlevel,
						     gfsmError     **errp);

/** Convenience macro for printing to uncompressed all-numeric-id named text files */
#define gfsm_indexed_automaton_print_filename(fsm,f,errp) \
  gfsm_indexed_automaton_print_filename_full(fsm,f,NULL,NULL,NULL,0,errp)

/** Print an automaton in Ma-Bell-compatible text-format to an in-memory buffer */
gboolean gfsm_indexed_automaton_print_gstring_full (gfsmIndexedAutomaton *fsm,
						    GString       *gs,
						    gfsmAlphabet  *lo_alphabet,
						    gfsmAlphabet  *hi_alphabet,
						    gfsmAlphabet  *state_alphabet,
						    gfsmError     **errp);

//@}

#endif /* _GFSM_INDEXED_IO_H */
