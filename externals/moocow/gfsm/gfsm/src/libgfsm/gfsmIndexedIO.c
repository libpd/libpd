
/*=============================================================================*\
 * File: gfsmIndexedIO.c
 * Author: Bryan Jurish <moocow@ling.uni-potsdam.de>
 * Description: finite state machine library: indexed automata: I/O
 *
 * Copyright (c) 2007-2008 Bryan Jurish.
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

#include <gfsmIndexedIO.h>
#include <gfsmArcIter.h>
#include <gfsmUtils.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>



/*======================================================================
 * Constants: Binary I/O
 */
const gfsmVersionInfo gfsm_indexed_version_bincompat_min_store =
  {
    0, // major
    0, // minor
    10 // micro
  };

const gfsmVersionInfo gfsm_indexed_version_bincompat_min_check =
  {
    0,  // major
    0,  // minor
    10  // micro
  };

const gchar gfsm_indexed_header_magic[16] = "gfsm_indexed\0";

/*======================================================================
 * Methods: Binary I/O: load()
 */

/*--------------------------------------------------------------
 * load_bin_header()
 */
gboolean gfsm_indexed_automaton_load_bin_header(gfsmIndexedAutomatonHeader *hdr, gfsmIOHandle *ioh, gfsmError **errp)
{
  if (!gfsmio_read(ioh, hdr, sizeof(gfsmIndexedAutomatonHeader))) {
    g_set_error(errp,
		g_quark_from_static_string("gfsm"),
		g_quark_from_static_string("indexed_automaton_load_bin_header:size"),
		"could not read header");
    return FALSE;
  }
  else if (strcmp(hdr->magic, gfsm_indexed_header_magic) != 0) {
    g_set_error(errp,
		g_quark_from_static_string("gfsm"),
		g_quark_from_static_string("indexed_automaton_load_bin_header:magic"),
		"bad magic");
    return FALSE;
  }
  else if (gfsm_version_compare(hdr->version, gfsm_version_bincompat_min_check) < 0) {
    g_set_error(errp,
		g_quark_from_static_string("gfsm"),
		g_quark_from_static_string("indexed_automaton_load_bin_header:version"),
		"stored format v%u.%u.%u is obsolete - need at least v%u.%u.%u",
		hdr->version.major,
		hdr->version.minor,
		hdr->version.micro,
		gfsm_indexed_version_bincompat_min_check.major,
		gfsm_indexed_version_bincompat_min_check.minor,
		gfsm_indexed_version_bincompat_min_check.micro);
    return FALSE;
  }
  else if (gfsm_version_compare(gfsm_version, hdr->version_min) < 0) {
    g_set_error(errp,
		g_quark_from_static_string("gfsm"),
		g_quark_from_static_string("indexed_automaton_load_bin_header:version"),
		"libgfsm v%u.%u.%u is obsolete - stored automaton needs at least v%u.%u.%u",
		gfsm_version.major,
		gfsm_version.minor,
		gfsm_version.micro,
		hdr->version_min.major,
		hdr->version_min.minor,
		hdr->version_min.micro);
    return FALSE;
  }
  if (hdr->srtype == gfsmSRTUnknown || hdr->srtype >= gfsmSRTUser) {
    //-- compatibility hack
    hdr->srtype = gfsmAutomatonDefaultSRType;
  }
  return TRUE;
}

/*--------------------------------------------------------------
 * load_bin_handle()
 *   + supports stored file versions v0.0.9 -- CURRENT
 */
gboolean gfsm_indexed_automaton_load_bin_handle_0_0_9(gfsmIndexedAutomatonHeader *hdr,
						      gfsmIndexedAutomaton *xfsm,
						      gfsmIOHandle *ioh,
						      gfsmError **errp)
{
  //-- reserve states & ars
  gfsm_indexed_automaton_reserve_states(xfsm, hdr->n_states);
  gfsm_indexed_automaton_reserve_arcs(xfsm, hdr->n_arcs);

  //-- set automaton-global properties
  xfsm->flags      = hdr->flags;
  gfsm_indexed_automaton_set_semiring_type(xfsm, hdr->srtype);
  xfsm->root_id    = hdr->root_id;

  //------ load: state_final_weight
  if (!gfsm_weight_vector_read_bin_handle(xfsm->state_final_weight, ioh, errp)) { return FALSE; }

  //------ load: arcs
  if (!gfsm_arc_table_index_read_bin_handle(xfsm->arcs, ioh, errp)) { return FALSE; }

  return TRUE;
}


/*--------------------------------------------------------------
 * load_bin_handle()
 *   + dispatch
 */
gboolean gfsm_indexed_automaton_load_bin_handle(gfsmIndexedAutomaton *fsm, gfsmIOHandle *ioh, gfsmError **errp)
{
  gfsmIndexedAutomatonHeader hdr;
  gfsm_indexed_automaton_clear(fsm);

  //-- load header
  if (!gfsm_indexed_automaton_load_bin_header(&hdr,ioh,errp)) return FALSE;

  //-- guts
  return gfsm_indexed_automaton_load_bin_handle_0_0_9(&hdr,fsm,ioh,errp);
}

/*--------------------------------------------------------------
 * load_bin_file()
 */
gboolean gfsm_indexed_automaton_load_bin_file(gfsmIndexedAutomaton *fsm, FILE *f, gfsmError **errp)
{
  gfsmIOHandle *ioh = gfsmio_new_zfile(f,"rb",-1);
  gboolean rc = gfsm_indexed_automaton_load_bin_handle(fsm, ioh, errp);
  if (ioh) {
    gfsmio_close(ioh);
    gfsmio_handle_free(ioh);
  }
  return rc;
}

/*--------------------------------------------------------------
 * load_bin_filename()
 */
gboolean gfsm_indexed_automaton_load_bin_filename(gfsmIndexedAutomaton *fsm, const gchar *filename, gfsmError **errp)
{
  gfsmIOHandle *ioh = gfsmio_new_filename(filename, "rb", -1, errp);
  gboolean rc = ioh && !(*errp) && gfsm_indexed_automaton_load_bin_handle(fsm, ioh, errp);
  if (ioh) {
    gfsmio_close(ioh);
    gfsmio_handle_free(ioh);
  }
  return rc;
}

/*--------------------------------------------------------------
 * load_bin_gstring()
 */
gboolean gfsm_indexed_automaton_load_bin_gstring(gfsmIndexedAutomaton *fsm, GString *gs, gfsmError **errp)
{
  gfsmPosGString pgs = { gs, 0 };
  gfsmIOHandle *ioh = gfsmio_new_gstring(&pgs);
  gboolean rc = ioh && !(*errp) && gfsm_indexed_automaton_load_bin_handle(fsm, ioh, errp);
  if (ioh) {
    gfsmio_close(ioh);
    gfsmio_handle_free(ioh);
  }
  return rc;
}


/*======================================================================
 * Methods: Binary I/O: save()
 */

/*--------------------------------------------------------------
 * save_bin_handle()
 */
gboolean gfsm_indexed_automaton_save_bin_handle(gfsmIndexedAutomaton *xfsm, gfsmIOHandle *ioh, gfsmError **errp)
{
  gfsmIndexedAutomatonHeader hdr;

  //-- create header
  memset(&hdr, 0, sizeof(gfsmIndexedAutomatonHeader));
  strcpy(hdr.magic, gfsm_indexed_header_magic);
  hdr.version     = gfsm_version;
  hdr.version_min = gfsm_indexed_version_bincompat_min_store;
  hdr.flags       = xfsm->flags;
  hdr.root_id     = xfsm->root_id;
  hdr.n_states    = gfsm_indexed_automaton_n_states(xfsm);
  hdr.n_arcs      = gfsm_indexed_automaton_n_arcs(xfsm);
  hdr.srtype      = gfsm_indexed_automaton_get_semiring(xfsm)->type;

  //-- write header
  if (!gfsmio_write(ioh, &hdr, sizeof(gfsmIndexedAutomatonHeader))) {
    g_set_error(errp, g_quark_from_static_string("gfsm"),
		      g_quark_from_static_string("indexed_automaton_save_bin:header"),
		      "could not store header");
    return FALSE;
  }

  //------ save: state_final_weight
  if (!gfsm_weight_vector_write_bin_handle(xfsm->state_final_weight, ioh, errp)) { return FALSE; }

  //------ save: arcs
  if (!gfsm_arc_table_index_write_bin_handle(xfsm->arcs, ioh, errp)) { return FALSE; }

  return TRUE;
}

/*--------------------------------------------------------------
 * save_bin_file()
 */
gboolean gfsm_indexed_automaton_save_bin_file(gfsmIndexedAutomaton *fsm, FILE *f, gfsmError **errp)
{
  gfsmIOHandle *ioh = gfsmio_new_file(f);
  gboolean rc = ioh && !(*errp) && gfsm_indexed_automaton_save_bin_handle(fsm, ioh, errp);
  if (ioh) {
    //gfsmio_close(ioh);
    gfsmio_handle_free(ioh);
  }
  return rc;
}

/*--------------------------------------------------------------
 * save_bin_filename_nc()
 */
gboolean gfsm_indexed_automaton_save_bin_filename_nc(gfsmIndexedAutomaton *fsm, const gchar *filename, gfsmError **errp)
{
  FILE *f;
  gboolean rc;
  if (!(f=gfsm_open_filename(filename,"wb",errp))) return FALSE;
  rc = gfsm_indexed_automaton_save_bin_file(fsm, f, errp);
  if (f != stdout) fclose(f);
  return rc;
}

/*--------------------------------------------------------------
 * save_bin_filename()
 */
gboolean gfsm_indexed_automaton_save_bin_filename(gfsmIndexedAutomaton *fsm, const gchar *filename, int zlevel, gfsmError **errp)
{
  gfsmIOHandle *ioh = gfsmio_new_filename(filename, "wb", zlevel, errp);
  gboolean rc = ioh && !(*errp) && gfsm_indexed_automaton_save_bin_handle(fsm, ioh, errp);
  if (ioh) {
    gfsmio_close(ioh);
    gfsmio_handle_free(ioh);
  }
  return rc;
}

/*--------------------------------------------------------------
 * save_bin_gstring()
 */
gboolean gfsm_indexed_automaton_save_bin_gstring(gfsmIndexedAutomaton *fsm, GString *gs, gfsmError **errp)
{
  gfsmPosGString pgs = { gs, gs->len };
  gfsmIOHandle *ioh = gfsmio_new_gstring(&pgs);
  gboolean rc = ioh && !(*errp) && gfsm_indexed_automaton_save_bin_handle(fsm, ioh, errp);
  if (ioh) {
    gfsmio_close(ioh);
    gfsmio_handle_free(ioh);
  }
  return rc;
}


/*======================================================================
 * Methods: Text I/O: compile() : NOT IMPLEMNENTED
 */


/*======================================================================
 * Methods: Text I/O: print()
 */

/*--------------------------------------------------------------
 * print_handle()
 */
gboolean gfsm_indexed_automaton_print_handle (gfsmIndexedAutomaton *xfsm,
					      gfsmIOHandle  *ioh,
					      gfsmAlphabet  *lo_alphabet,
					      gfsmAlphabet  *hi_alphabet,
					      gfsmAlphabet  *state_alphabet,
					      GFSM_UNUSED gfsmError **errp)
{
  gfsmStateId qid;
  gfsmArcRange range;
  GString *gs = g_string_new("");
  gboolean rc = TRUE;
  gpointer  key;

  if (xfsm->root_id == gfsmNoState) rc = FALSE; //-- sanity check

  for (qid=0; rc && qid < gfsm_indexed_automaton_n_states(xfsm); qid++) {
    if (!gfsm_indexed_automaton_has_state(xfsm,qid)) continue;

    for (gfsm_arcrange_open_indexed(&range,xfsm,qid); gfsm_arcrange_ok(&range); gfsm_arcrange_next(&range))
      {
	gfsmArc *a = gfsm_arcrange_arc(&range);

	//-- source state
	if (state_alphabet && (key=gfsm_alphabet_find_key(state_alphabet,qid)) != gfsmNoKey) {
	  gfsm_alphabet_key2string(state_alphabet,key,gs);
	  gfsmio_puts(ioh, gs->str);
	} else {
	  if (state_alphabet) g_printerr("Warning: no label defined for state '%u'!\n", qid);
	  gfsmio_printf(ioh, "%u", qid);
	}
	gfsmio_putc(ioh, '\t');

	//-- sink state
	if (state_alphabet && (key=gfsm_alphabet_find_key(state_alphabet,a->target)) != gfsmNoKey) {
	  gfsm_alphabet_key2string(state_alphabet,key,gs);
	  gfsmio_puts(ioh,gs->str);
	} else {
	  if (state_alphabet) g_printerr("Warning: no label defined for state '%u'!\n", a->target);
	  gfsmio_printf(ioh, "%u", a->target);
	}
	gfsmio_putc(ioh,'\t');

	//-- lower label
	if (lo_alphabet && (key=gfsm_alphabet_find_key(lo_alphabet,a->lower)) != gfsmNoKey) {
	  gfsm_alphabet_key2string(lo_alphabet,key,gs);
	  gfsmio_puts(ioh, gs->str);
	} else {
	  if (lo_alphabet) g_printerr("Warning: no lower label defined for Id '%u'!\n", a->lower);
	  gfsmio_printf(ioh, "%u", a->lower);
	}

	//-- upper label
	if (xfsm->flags.is_transducer) {
	  gfsmio_putc(ioh, '\t');
	  if (hi_alphabet && (key=gfsm_alphabet_find_key(hi_alphabet,a->upper)) != gfsmNoKey) {
	    gfsm_alphabet_key2string(hi_alphabet,key,gs);
	    gfsmio_puts(ioh, gs->str);
	  } else {
	    if (hi_alphabet) g_printerr("Warning: no upper label defined for Id '%u'!\n", a->upper);
	    gfsmio_printf(ioh, "%u", a->upper);
	  }
	}

	//-- weight
	if (xfsm->flags.is_weighted) { // && a->weight != fsm->sr->one
	  gfsmio_printf(ioh, "\t%g", a->weight);
	}

	gfsmio_putc(ioh, '\n');
      }
    gfsm_arcrange_close(&range);

    //-- final?
    if (gfsm_indexed_automaton_state_is_final(xfsm,qid)) {
      if (state_alphabet && (key=gfsm_alphabet_find_key(state_alphabet,qid)) != NULL) {
	gfsm_alphabet_key2string(state_alphabet,key,gs);
	gfsmio_puts(ioh, gs->str);
      } else {
	gfsmio_printf(ioh, "%u", qid);
      }
      if (xfsm->flags.is_weighted) {
	gfsmio_printf(ioh, "\t%g", gfsm_indexed_automaton_get_final_weight(xfsm,qid));
      }
      gfsmio_putc(ioh, '\n');
    }
  }

  //-- cleanup
  g_string_free(gs,TRUE);
  
  return rc;
}

/*--------------------------------------------------------------
 * print_file_full()
 */
gboolean gfsm_indexed_automaton_print_file_full (gfsmIndexedAutomaton *fsm,
						 FILE          *f,
						 gfsmAlphabet  *lo_alphabet,
						 gfsmAlphabet  *hi_alphabet,
						 gfsmAlphabet  *state_alphabet,
						 int            zlevel,
						 gfsmError     **errp)
{
  gfsmIOHandle *ioh = gfsmio_new_zfile(f,"wb",zlevel);
  gboolean rc = (ioh && !(*errp) &&
		 gfsm_indexed_automaton_print_handle(fsm,ioh,lo_alphabet,hi_alphabet,state_alphabet,errp));
  if (ioh) {
    gfsmio_close(ioh);
    gfsmio_handle_free(ioh);
  }
  return rc;
}

/*--------------------------------------------------------------
 * print_filename()
 */
gboolean gfsm_indexed_automaton_print_filename_full (gfsmIndexedAutomaton *fsm,
						     const gchar   *filename,
						     gfsmAlphabet  *lo_alphabet,
						     gfsmAlphabet  *hi_alphabet,
						     gfsmAlphabet  *state_alphabet,
						     int            zlevel,
						     gfsmError     **errp)
{
  gfsmIOHandle *ioh = gfsmio_new_filename(filename,"wb",zlevel,errp);
  gboolean rc = (ioh && !(*errp) &&
		 gfsm_indexed_automaton_print_handle(fsm,ioh,lo_alphabet,hi_alphabet,state_alphabet,errp));
  if (ioh) {
    gfsmio_close(ioh);
    gfsmio_handle_free(ioh);
  }
  return rc;
}
/*--------------------------------------------------------------
 * print_gstring_full()
 */
gboolean gfsm_indexed_automaton_print_gstring_full (gfsmIndexedAutomaton *fsm,
						    GString      *gs,
						    gfsmAlphabet  *lo_alphabet,
						    gfsmAlphabet  *hi_alphabet,
						    gfsmAlphabet  *state_alphabet,
						    gfsmError     **errp)
{
  gfsmPosGString pgs = { gs, gs->len };
  gfsmIOHandle *ioh = gfsmio_new_gstring(&pgs);
  gboolean rc = (ioh && !(*errp) &&
		 gfsm_indexed_automaton_print_handle(fsm,ioh,lo_alphabet,hi_alphabet,state_alphabet,errp));
  if (ioh) {
    gfsmio_close(ioh);
    gfsmio_handle_free(ioh);
  }
  return rc;
}

