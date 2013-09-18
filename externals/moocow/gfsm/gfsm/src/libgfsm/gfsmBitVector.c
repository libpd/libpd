
/*=============================================================================*\
 * File: gfsmBitVector.c
 * Author: Bryan Jurish <moocow@ling.uni-potsdam.de>
 * Description: finite state machine library: bit vectors: extern functions
 *
 * Copyright (c) 2004-2007 Bryan Jurish.
 *
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.
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

#include <gfsmConfig.h>
#include <gfsmBitVector.h>

//-- no-inline definitions
#ifndef GFSM_INLINE_ENABLED
# include <gfsmBitVector.hi>
#endif

/*======================================================================
 * I/O
 */

//--------------------------------------------------------------
// bitvector_write_bin_handle()
gboolean gfsm_bitvector_write_bin_handle(gfsmBitVector *bv, gfsmIOHandle *ioh, gfsmError **errp)
{
  guint32 len = bv->len;
  if (!gfsmio_write(ioh,&len,sizeof(guint32))) {
    g_set_error(errp, g_quark_from_static_string("gfsm"),
		g_quark_from_static_string("bitvector_write_bin_handle:len"),
		"could not store bit vector length ");
    return FALSE;
  }
  if (!gfsmio_write(ioh, bv->data, bv->len)) {
    g_set_error(errp, g_quark_from_static_string("gfsm"),
		g_quark_from_static_string("bitvector_write_bin_handle:weights"),
		"could not store bit vector data");
    return FALSE;
  }
  return TRUE;
}

//--------------------------------------------------------------
// bitvector_read_bin_handle()
gboolean gfsm_bitvector_read_bin_handle(gfsmBitVector *bv, gfsmIOHandle *ioh, gfsmError **errp)
{
  guint32 len;
  if (!gfsmio_read(ioh, &len, sizeof(guint32))) {
    g_set_error(errp,
		g_quark_from_static_string("gfsm"),
		g_quark_from_static_string("bitvector_read_bin_handle:len"),
		"could not read bit vector length");
    return FALSE;
  }
  gfsm_bitvector_resize(bv,len);
  if (!gfsmio_read(ioh, bv->data, len)) {
    g_set_error(errp,
		g_quark_from_static_string("gfsm"),
		g_quark_from_static_string("bitvector_read_bin_handle:data"),
		"could not read bit vector data");
    return FALSE;
  }
  return TRUE;
}
