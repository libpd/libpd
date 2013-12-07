/*=============================================================================*\
 * File: pd_io.c
 * Author: Bryan Jurish <moocow@ling.uni-potsdam.de>
 * Description: finite state automata for Pd: I/O
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

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <pd_io.h>

/*=====================================================================
 * I/O utilities
 *=====================================================================*/

/*--------------------------------------------------------------------
 * pd_gfsm_console_handle_flush()
 */
static void pd_gfsm_console_handle_flush(gfsmPosGString *pgs)
{
  if (pgs->gs->len) post(pgs->gs->str);
  g_string_truncate(pgs->gs,0);
  pgs->pos = 0;
}

/*--------------------------------------------------------------------
 * pd_gfsm_console_handle_new()
 */
gfsmIOHandle *pd_gfsm_console_handle_new(void)
{
  GString *gs = g_string_new("");
  gfsmPosGString *pgs = g_new(gfsmPosGString,1);
  gfsmIOHandle *ioh;
  pgs->gs  = gs;
  pgs->pos = 0;

  ioh = gfsmio_new_gstring(pgs);
  ioh->flush_func = (gfsmIOFlushFunc)pd_gfsm_console_handle_flush;

  return ioh;
}

/*--------------------------------------------------------------------
 * pd_gfsm_console_handle_free()
 */
void pd_gfsm_console_handle_free(gfsmIOHandle *ioh)
{
  gfsmPosGString *pgs = (gfsmPosGString*)ioh->handle;
  gfsmio_flush(ioh);
  g_string_free(pgs->gs,TRUE);
  g_free(pgs);
  g_free(ioh);
}
