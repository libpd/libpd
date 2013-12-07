/*=============================================================================*\
 * File: pd_state.h
 * Author: Bryan Jurish <moocow@ling.uni-potsdam.de>
 * Description: finite state automata for Pd
 *
 * Copyright (c) 2004-2006 Bryan Jurish.
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

/*=====================================================================
 * Protos
 *=====================================================================*/
#ifndef PD_GFSM_STATE_H
#define PD_GFSM_STATE_H

/*----------------------------------------------------------------------
 * includes
 */
#include <pd_automaton.h>

/*--------------------------------------------------------------
 * pd_gfsm_state
 */
typedef struct _pd_gfsm_state
{
  t_object                 x_obj;
  t_pd_gfsm_automaton_pd  *x_automaton_pd;
  gfsmStateId              x_id;
  gfsmArcIter              x_arci;
  gboolean                 x_open;
  t_atom                   x_argv[4];
  t_outlet                *x_valout;
} t_pd_gfsm_state;

/*----------------------------------------------------------------------
 * setup routines
 */
void pd_gfsm_state_setup(void);

#endif /* PD_GFSM_STATE_H */
