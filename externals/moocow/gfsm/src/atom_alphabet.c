/*=============================================================================*\
 * File: atom_alphabet.c
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

#include <atom_alphabet.h>
#include <string.h>

/*======================================================================
 * constants
 */
gfsmUserAlphabetMethods pd_atom_alphabet_methods =
  {
    NULL, //-- key->label lookup function
    NULL, //-- label->key lookup function
    NULL, //-- insertion function: receives a newly copied key!
    NULL, //-- label removal function
    (gfsmAlphabetKeyReadFunc)gfsm_pd_atom_read,   //-- key input function
    (gfsmAlphabetKeyWriteFunc)gfsm_pd_atom_write  //-- key output function
  };

/*======================================================================
 * gfsmPdAtomAlphabet: user methods
 */


/*--------------------------------------------------------------
 * hash()
 */
guint gfsm_pd_atom_hash(t_atom *a)
{
  if (!a) return 0;
  switch (a->a_type) {
  case A_FLOAT:  return (guint)(a->a_w.w_float);
  case A_SYMBOL: return (guint)(a->a_w.w_symbol);
  case A_SEMI:   return (guint)';';
  case A_COMMA:  return (guint)',';
  default:       return 0;
  }
  return 0; //-- never reached
}

/*--------------------------------------------------------------
 * equal()
 */
gboolean gfsm_pd_atom_equal(t_atom *a1, t_atom *a2)
{
  if (a1->a_type != a2->a_type) return FALSE;
  switch (a1->a_type) {
  case A_FLOAT:  return a1->a_w.w_float == a2->a_w.w_float;
  case A_SYMBOL: return a1->a_w.w_symbol == a2->a_w.w_symbol;
  default:       return memcmp(a1,a2,sizeof(t_atom))==0;
  }
}

/*--------------------------------------------------------------
 * dup()
 */
t_atom *gfsm_pd_atom_dup(GFSM_UNUSED gfsmPdAtomAlphabet *alph, t_atom *a)
{
  //return (a ? copybytes(a, sizeof(t_atom)) : NULL);
  return (a ? gfsm_mem_dup_n(a,sizeof(t_atom)) : NULL);
}

/*--------------------------------------------------------------
 * free()
 */
void gfsm_pd_atom_free(t_atom *a)
{ g_free(a); }

/*--------------------------------------------------------------
 * read()
 */
t_atom *gfsm_pd_atom_read(gfsmPdAtomAlphabet *alph, GString *gstr)
{
  binbuf_clear(alph->aa_binbuf);
  binbuf_text(alph->aa_binbuf, gstr->str, gstr->len);
  return binbuf_getvec(alph->aa_binbuf);
}

/*--------------------------------------------------------------
 * write()
 */
void gfsm_pd_atom_write(gfsmPdAtomAlphabet *alph, t_atom *a, GString *gstr)
{
  atom_string(a, alph->aa_strbuf, PdGfsmStrBufSize);
  g_string_assign(gstr, alph->aa_strbuf);
}

/*======================================================================
 * gfsmPdAtomAlphabet: shortcuts
 */

/*--------------------------------------------------------------
 * new
 */
gfsmPdAtomAlphabet *gfsm_pd_atom_alphabet_new(void)
{
  gfsmPdAtomAlphabet *alph = g_new0(gfsmPdAtomAlphabet,1);
  ((gfsmAlphabet*)alph)->type = gfsmATUser;
  gfsm_user_alphabet_init((gfsmUserAlphabet*)alph,
			  (gfsmAlphabetKeyDupFunc)gfsm_pd_atom_dup,
			  (GHashFunc)gfsm_pd_atom_hash,
			  (GEqualFunc)gfsm_pd_atom_equal,
			  (GDestroyNotify)gfsm_pd_atom_free,
			  NULL,
			  &pd_atom_alphabet_methods);
  alph->aa_binbuf = binbuf_new();
  return alph;
}

//-- destructor
void gfsm_pd_atom_alphabet_free(gfsmPdAtomAlphabet *alph)
{
  if (alph->aa_binbuf) binbuf_free(alph->aa_binbuf);
  gfsm_alphabet_free((gfsmAlphabet*)alph);
}
