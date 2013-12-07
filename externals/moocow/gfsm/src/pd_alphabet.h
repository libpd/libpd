/*=============================================================================*\
 * File: pd_alphabet.h
 * Author: Bryan Jurish <moocow@ling.uni-potsdam.de>
 * Description: finite state automata for Pd
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

#ifndef _PD_GFSM_ALPHABET_H
#define _PD_GFSM_ALPHABET_H

#include <m_pd.h>
#include <gfsm.h>
#include <atom_alphabet.h>


/*--------------------------------------------------------------
 * pd_gfsm_alphabet
 */
typedef struct _pd_gfsm_alphabet_pd
{
  t_pd                 x_pd;
  t_symbol            *x_name;
  size_t               x_refcnt;
  gfsmAlphabet        *x_alphabet; //-- really a (gfsmPdAtomAlphabet*)
} t_pd_gfsm_alphabet_pd;


/*--------------------------------------------------------------
 * pd_gfsm_alphabet
 */
typedef struct _pd_gfsm_alphabet_obj
{
  t_object                x_obj;
  t_pd_gfsm_alphabet_pd  *x_alphabet_pd;
  t_outlet               *x_labout;
  t_outlet               *x_keyout;
} t_pd_gfsm_alphabet_obj;


/*--------------------------------------------------------------
 * pd_gfsm_alphabet: methods
 */
//-- finds pd_gfsm_alphabet named 'name', returns NULL if it doesn't exist
t_pd_gfsm_alphabet_pd *pd_gfsm_alphabet_pd_find(t_symbol *name);

//-- finds pd_gfsm_alphabet named 'name', creating it if it doesn't exist
t_pd_gfsm_alphabet_pd *pd_gfsm_alphabet_pd_get(t_symbol *name);

//-- releases one reference to pd_gfsm_alphabet named 'name', possibly freeing the pd_gfsm_alphabet
void pd_gfsm_alphabet_pd_release(t_pd_gfsm_alphabet_pd *x);

/*--------------------------------------------------------------
 * setup methods
 */
void pd_gfsm_alphabet_setup(void);


#endif /* _PD_GFSM_ALPHABET_H */
