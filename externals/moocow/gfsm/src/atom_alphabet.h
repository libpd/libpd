/*=============================================================================*\
 * File: atom_alphabet.h
 * Author: Bryan Jurish <moocow@ling.uni-potsdam.de>
 * Description: finite state automata for Pd: atom alphabet
 *
 * Copyright (c) 2004 Bryan Jurish.
 *
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * See file LICENSE for further informations on licensing terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *=============================================================================*/

#ifndef _PD_GFSM_ATOM_ALPHABET_H
#define _PD_GFSM_ATOM_ALPHABET_H

#include <m_pd.h>
#include <gfsm.h>

/*--------------------------------------------------------------
 * Constants
 */
#define PdGfsmStrBufSize 256

extern gfsmUserAlphabetMethods pd_atom_alphabet_methods;

/*--------------------------------------------------------------
 * Types: gfsmPdAtomAlphabet
 */
typedef struct {
  gfsmUserAlphabet  aa_alph;
  t_binbuf         *aa_binbuf;
  char              aa_strbuf[PdGfsmStrBufSize];
} gfsmPdAtomAlphabet;

/*--------------------------------------------------------------
 * gfsmPdAtomAlphabet: user methods
 */
//-- hash function for pd atoms
guint gfsm_pd_atom_hash(t_atom *a);

//-- equality check for pd atoms
gboolean gfsm_pd_atom_equal(t_atom *a1, t_atom *a2);

//-- dup function for pd atoms
t_atom *gfsm_pd_atom_dup(gfsmPdAtomAlphabet *alph, t_atom *a);

//-- free function for pd atoms
void gfsm_pd_atom_free(t_atom *a);

//-- string read function for pd atoms
t_atom *gfsm_pd_atom_read(gfsmPdAtomAlphabet *alph, GString *gstr);

//-- string write function for pd atoms
void gfsm_pd_atom_write(gfsmPdAtomAlphabet *alph, t_atom *a, GString *gstr);

/*--------------------------------------------------------------
 * gfsmPdAtomAlphabet: utilities
 */
//-- constructor
gfsmPdAtomAlphabet *gfsm_pd_atom_alphabet_new(void);

//-- destructor
void gfsm_pd_atom_alphabet_free(gfsmPdAtomAlphabet *alph);

#endif /* _PD_GFSM_ATOM_ALPHABET_H */
