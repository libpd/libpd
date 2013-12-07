
/*=============================================================================*\
 * File: gfsmDraw.c
 * Author: Bryan Jurish <moocow@ling.uni-potsdam.de>
 * Description: finite state machine library: automata: visualization
 *
 * Copyright (c) 2004-2008 Bryan Jurish.
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

#include <gfsmDraw.h>
#include <gfsmArcIter.h>
#include <gfsmUtils.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*======================================================================
 * Methods: Text I/O: vcg
 */

/*--------------------------------------------------------------
 * draw_vcg_file()
 */
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
					    GFSM_UNUSED gfsmError **errp)
{
  gfsmStateId id;
  GString     *gstr = g_string_new("");

  fprintf(f, "graph: {\n");
  fprintf(f, " title: \"%s\"\n", (title ? title : "(gfsm)"));
  fprintf(f, " display_edge_labels:yes\n");
  fprintf(f, " splines:yes\n");
  fprintf(f, " color:white\n");
  fprintf(f, " xspace:%d\n", (xspace ? xspace : 40));
  fprintf(f, " yspace:%d\n", (yspace ? yspace : 20));
  fprintf(f, " orientation:%s\n", (orientation ? orientation : "left_to_right"));
  fprintf(f, " node.shape:%s\n", (state_shape ? state_shape : "ellipse"));
  fprintf(f, " node.color:%s\n", (state_color ? state_color : "white"));
  fprintf(f, " node.borderwidth:1\n");

  //-- ye olde iterationne
  for (id = 0; id < fsm->states->len; id++) {
    gfsmState   *s = gfsm_automaton_find_state(fsm,id);
    gfsmArcIter ai;
    gchar       *sym;
    if (!s || !s->is_valid) continue;

    //-- source state
    fprintf(f, " node: {title:\"%u\" label:\"", id);
    if (state_alphabet && (sym=gfsm_alphabet_find_key(state_alphabet,id)) != NULL) {
      gfsm_alphabet_key2string(state_alphabet, sym, gstr);
      fprintf(f, "%s", gstr->str);
    } else {
      if (state_alphabet) g_printerr("Warning: no label defined for state '%u'!\n", id);
      fprintf(f, "%u", id);
    }
    if (fsm->flags.is_weighted) {
      fprintf(f, "/%g", gfsm_automaton_get_final_weight(fsm,id));
    }
    fprintf(f, "\"");

    if (s->is_final) {
      fprintf(f, " color:%s", (final_color ? final_color : "lightgrey"));
    }
    if (id == fsm->root_id) fputs(" borderwidth:3", f);
    fputs("}\n", f);

    for (gfsm_arciter_open_ptr(&ai,fsm,s); gfsm_arciter_ok(&ai); gfsm_arciter_next(&ai)) {
      gfsmArc *a = gfsm_arciter_arc(&ai);
      fprintf(f, "  edge: {sourcename:\"%u\" targetname:\"%u\" label:\"", id, a->target);

      if (lo_alphabet && (sym=gfsm_alphabet_find_key(lo_alphabet,a->lower)) != NULL) {
	gfsm_alphabet_key2string(lo_alphabet, sym, gstr);
	fputs(gstr->str, f);
      } else {
	if (lo_alphabet)
	  g_printerr("Warning: no label defined for lower label '%u'!\n", a->lower);
	fprintf(f, "%u", a->lower);
      }

      if (fsm->flags.is_transducer) {
	fputc(':', f);
	if (hi_alphabet && (sym=gfsm_alphabet_find_key(hi_alphabet,a->upper)) != NULL) {
	  gfsm_alphabet_key2string(hi_alphabet, sym, gstr);
	  fputs(gstr->str, f);
	} else {
	  if (hi_alphabet)
	    g_printerr("Warning: no label defined for upper label '%u'!\n", a->upper);
	  fprintf(f, "%u", a->upper);
	}
      }

      if (fsm->flags.is_weighted) fprintf(f, "/%g", a->weight);
      fprintf(f, "\"}\n");
    }
  }
  fputs("}\n", f);

  return TRUE;
}


/*--------------------------------------------------------------
 * draw_vcg_filename()
 */
gboolean gfsm_automaton_draw_vcg_filename_full (gfsmAutomaton *fsm,
						const gchar   *filename,
						gfsmAlphabet  *lo_alphabet,
						gfsmAlphabet  *hi_alphabet,
						gfsmAlphabet  *state_alphabet,
						const gchar   *title,
						int            xspace,
						int            yspace,
						const gchar   *orientation,
						const gchar   *state_shape,
						const gchar   *state_color,
						const gchar   *final_color,
						gfsmError    **errp)
{
  FILE *f;
  gboolean rc;
  if (!(f=gfsm_open_filename(filename, "w", errp))) return FALSE;
  rc = gfsm_automaton_draw_vcg_file_full(fsm, f, lo_alphabet, hi_alphabet, state_alphabet,
					 title, xspace, yspace, orientation,
					 state_shape, state_color, final_color,
					 errp);
  if (f != stdout) fclose(f);
  return rc;
}


/*======================================================================
 * Methods: Draw: dot
 */

/*--------------------------------------------------------------
 * draw_dot_file()
 */
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
					    GFSM_UNUSED gfsmError **errp)
{
  gfsmStateId id;
  GString     *gstr = g_string_new("");

  fprintf(f, "digraph GFSM {\n");
  fprintf(f, " rankdir = %s;\n", vertical ? "TB" : "LR");
  if (width>0 && height>0) {
    fprintf(f, " size = \"%g,\%g\";\n", (width ? width : 8.5), (height ? height : 11));
  }
  fprintf(f, " label = \"%s\";\n", (title ? title : "(gfsm)"));
  fprintf(f, " center = 1;\n");
  fprintf(f, " nodesep = \"%f\";\n", (nodesep ? nodesep : 0.25));
  fprintf(f, " ranksep = \"%f\";\n", (ranksep ? ranksep : 0.4));
  if (!portrait) {
    //fprintf(f, " orientation = \"Landscape\";\n");
    fprintf(f, " rotate = 90;\n");
  }

  //-- ye olde iterationne
  for (id = 0; id < fsm->states->len; id++) {
    gfsmState   *s = gfsm_automaton_find_state(fsm,id);
    gfsmArcIter ai;
    gchar       *sym;
    if (!s || !s->is_valid) continue;

    //-- source state
    fprintf(f, "%u [label=\"", id);
    if (state_alphabet && (sym=gfsm_alphabet_find_key(state_alphabet,id)) != NULL) {
      gfsm_alphabet_key2string(state_alphabet, sym, gstr);
      fputs(gstr->str, f);
    } else {
      if (state_alphabet) g_printerr("Warning: no label defined for state '%u'!\n", id);
      fprintf(f, "%u", id);
    }
    if (fsm->flags.is_weighted && s->is_final) {
      fprintf(f, "/%g", gfsm_automaton_get_final_weight(fsm,id));
    }
    fprintf(f, "\", shape=%s, style=%s, fontsize=%d",
	    (s->is_final ? "doublecircle" : "circle"),
	    (id == fsm->root_id ? "bold" : "solid"),
	    (fontsize ? fontsize : 14));
    if (fontname && *fontname) fprintf(f, ", fontname=\"%s\"", fontname);
    fprintf(f, "]\n");

    for (gfsm_arciter_open_ptr(&ai,fsm,s); gfsm_arciter_ok(&ai); gfsm_arciter_next(&ai)) {
      gfsmArc *a = gfsm_arciter_arc(&ai);
      fprintf(f, "   %u -> %u \[label=\"", id, a->target);

      if (lo_alphabet && (sym=gfsm_alphabet_find_key(lo_alphabet,a->lower)) != NULL) {
	gfsm_alphabet_key2string(lo_alphabet, sym, gstr);
	fputs(gstr->str, f);
      } else {
	if (lo_alphabet)
	  g_printerr("Warning: no label defined for lower label '%u'!\n", a->lower);
	fprintf(f, "%u", a->lower);
      }

      if (fsm->flags.is_transducer) {
	fputc(':', f);
	if (hi_alphabet && (sym=gfsm_alphabet_find_key(hi_alphabet,a->upper)) != NULL) {
	  gfsm_alphabet_key2string(hi_alphabet, sym, gstr);
	  fputs(gstr->str, f);
	} else {
	  if (hi_alphabet)
	    g_printerr("Warning: no label defined for upper label '%u'!\n", a->upper);
	  fprintf(f, "%u", a->upper);
	}
      }

      if (fsm->flags.is_weighted) fprintf(f, "/%g", a->weight);
      
      fprintf(f, "\", fontsize=%d", (fontsize ? fontsize : 14));
      if (fontname && *fontname) fprintf(f, ", fontname=\"%s\"", fontname);
      fprintf(f, "];\n");
    }
  }
  fputs("}\n", f);

  //--cleanup
  g_string_free(gstr,TRUE);

  return TRUE;
}


/*--------------------------------------------------------------
 * draw_dot_filename()
 */
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
						gfsmError    **errp)
{
  FILE *f;
  gboolean rc;
  if (!(f=gfsm_open_filename(filename, "w", errp))) return FALSE;
  rc = gfsm_automaton_draw_dot_file_full(fsm, f, lo_alphabet, hi_alphabet, state_alphabet,
					 title, width, height, fontsize, fontname,
					 portrait, vertical, nodesep, ranksep,
					 errp);
  if (f != stdout) fclose(f);
  return rc;
}
