/*=============================================================================*\
 * File: pd_paths.h
 * Author: Bryan Jurish <moocow@ling.uni-potsdam.de>
 * Description: finite state automata for Pd: paths()
 *
 * Copyright (c) 2006 Bryan Jurish.
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
#ifndef PD_GFSM_PATHS_H
#define PD_GFSM_PATHS_H

/*----------------------------------------------------------------------
 * includes
 */
#include <pd_automaton.h>

/*--------------------------------------------------------------
 * pd_gfsm_lookup
 */
typedef struct _pd_gfsm_paths
{
  t_object                 x_obj;
  t_pd_gfsm_automaton_pd  *x_automaton_pd;
  GPtrArray               *x_paths;
  t_outlet                *x_out_lo;
  t_outlet                *x_out_hi;
  t_outlet                *x_out_w;
  t_outlet                *x_out_done;
} t_pd_gfsm_lookup;

/*----------------------------------------------------------------------
 * setup routines
 */
void pd_gfsm_paths_setup(void);

#endif /* PD_GFSM_PATHS_H */
