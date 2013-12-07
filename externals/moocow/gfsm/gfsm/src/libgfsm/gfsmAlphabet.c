/*=============================================================================*\
 * File: gfsmAlphabet.c
 * Author: Bryan Jurish <moocow@ling.uni-potsdam.de>
 * Description: finite state machine library: alphabet
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

#include <gfsmAlphabet.h>
#include <gfsmSet.h>
#include <gfsmUtils.h>
#include <gfsmError.h>

#include <string.h>
#include <errno.h>
#include <ctype.h>
#include <stdlib.h>

/*======================================================================
 * Constants
 */
gfsmUserAlphabetMethods gfsmUserAlphabetDefaultMethods =
  {
    NULL, //-- key_lookup
    NULL, //-- lab_lookup
    NULL, //-- insert
    //NULL, //-- key_remove
    NULL, //-- lab_remove
    NULL, //-- key_read
    NULL  //-- key_write
  };

/*======================================================================
 * Methods: Constructors
 */

/*--------------------------------------------------------------
 * new()
 */
gfsmAlphabet *gfsm_alphabet_new(gfsmAType type)
{
  gfsmAlphabet *a=NULL;
  switch (type) {
  case gfsmATIdentity:
    a = (gfsmAlphabet*)g_new0(gfsmIdentityAlphabet,1);
    break;
  case gfsmATPointer:
    a = (gfsmAlphabet*)g_new0(gfsmPointerAlphabet,1);
    break;
  case gfsmATUser:
    a = (gfsmAlphabet*)g_new0(gfsmUserAlphabet,1);
    break;
  case gfsmATString:
    a = (gfsmAlphabet*)g_new0(gfsmStringAlphabet,1);
    break;
  case gfsmATUnknown:
  case gfsmATRange:
  default:
    a = (gfsmAlphabet*)g_new0(gfsmRangeAlphabet,1);
    break;
  }
  a->type = type;
  a->lab_min = gfsmNoLabel;
  a->lab_max = gfsmNoLabel;
  return a;
}

/*--------------------------------------------------------------
 * string_new()
 */
/*gfsmAlphabet *gfsm_string_alphabet_new(void)
{
  return gfsm_string_alphabet_init(g_new(gfsmStringAlphabet,1));
  }
*/

/*--------------------------------------------------------------
 * init()
 */
gfsmAlphabet *gfsm_alphabet_init(gfsmAlphabet *a)
{
  if (!a) return NULL;

  a->lab_min = gfsmNoLabel;
  a->lab_max = gfsmNoLabel;

  switch (a->type) {
  case gfsmATIdentity:
    return gfsm_identity_alphabet_init((gfsmIdentityAlphabet*)a);
  case gfsmATPointer:
    return gfsm_pointer_alphabet_init((gfsmPointerAlphabet*)a,NULL,NULL,NULL,NULL);
  case gfsmATUser:
    return gfsm_user_alphabet_init((gfsmUserAlphabet*)a,NULL,NULL,NULL,NULL,
				   NULL,&gfsmUserAlphabetDefaultMethods);
  case gfsmATString:
    return gfsm_string_alphabet_init((gfsmStringAlphabet*)a,FALSE);
  case gfsmATUnknown:
  case gfsmATRange:
  default:
    break;
  }
  return a;
}

/*--------------------------------------------------------------
 * range_init()
 */
gfsmAlphabet *gfsm_range_alphabet_init (gfsmRangeAlphabet *a, gfsmLabelVal min, gfsmLabelVal max)
{
  a->lab_min = min;
  a->lab_max = max;
  return a;
}

/*--------------------------------------------------------------
 * identity_init()
 */
gfsmAlphabet *gfsm_identity_alphabet_init (gfsmIdentityAlphabet *a)
{
  gfsm_range_alphabet_init((gfsmRangeAlphabet*)a, gfsmNoLabel, gfsmNoLabel);
  if (!a->labels) a->labels = gfsm_set_new(gfsm_uint_compare);
  gfsm_set_clear(a->labels);
  return (gfsmAlphabet*)a;
}

/*--------------------------------------------------------------
 * pointer_init()
 */
gfsmAlphabet *gfsm_pointer_alphabet_init(gfsmPointerAlphabet *a,
					 gfsmAlphabetKeyDupFunc   key_dup_func,
					 GHashFunc      key_hash_func,
					 GEqualFunc     key_equal_func,
					 GDestroyNotify key_free_func)
{
  gfsm_range_alphabet_init((gfsmRangeAlphabet*)a,gfsmNoLabel,gfsmNoLabel);

  if (a->keys2labels) g_hash_table_destroy(a->keys2labels);
  if (a->labels2keys) g_ptr_array_free(a->labels2keys,TRUE);

  a->keys2labels  = g_hash_table_new_full(key_hash_func, key_equal_func, key_free_func, NULL);
  a->labels2keys  = g_ptr_array_new();
  a->key_dup_func = key_dup_func;

  return (gfsmAlphabet*)a;
}

/*--------------------------------------------------------------
 * string_init()
 */
gfsmAlphabet *gfsm_string_alphabet_init(gfsmStringAlphabet *a, gboolean do_copy)
{
  if (do_copy)
    return gfsm_pointer_alphabet_init(a,
				      (gfsmAlphabetKeyDupFunc)gfsm_alphabet_strdup,
				      g_str_hash, g_str_equal, g_free);
  return gfsm_pointer_alphabet_init(a, NULL, g_str_hash, g_str_equal, NULL);
}

/*--------------------------------------------------------------
 * user_init()
 */
gfsmAlphabet *gfsm_user_alphabet_init(gfsmUserAlphabet        *a,
				      gfsmAlphabetKeyDupFunc   key_dup_func,
				      GHashFunc                key_hash_func,
				      GEqualFunc               key_equal_func,
				      GDestroyNotify           key_destroy_func,
				      gpointer                 user_data,
				      gfsmUserAlphabetMethods *methods)
{
  gfsm_pointer_alphabet_init((gfsmPointerAlphabet*)a,
			     key_dup_func,
			     key_hash_func,
			     key_equal_func,
			     key_destroy_func);
  a->data    = user_data;
  a->methods = methods ? (*methods) : gfsmUserAlphabetDefaultMethods;
  return (gfsmAlphabet*)a;
}

/*--------------------------------------------------------------
 * clear()
 */
void gfsm_alphabet_clear(gfsmAlphabet *a)
{
  switch (a->type) {
  case gfsmATUnknown:
  case gfsmATRange:
    break;
  case gfsmATIdentity:
    gfsm_set_clear(((gfsmIdentityAlphabet*)a)->labels);
    break;
  case gfsmATPointer:
  case gfsmATString:
    g_ptr_array_set_size(((gfsmPointerAlphabet*)a)->labels2keys,0);
    g_hash_table_foreach_remove(((gfsmPointerAlphabet*)a)->keys2labels, gfsm_hash_clear_func, NULL);
    break;
  case gfsmATUser:
  default:
    gfsm_alphabet_foreach(a, gfsm_alphabet_foreach_remove_func, NULL);
    g_ptr_array_set_size(((gfsmPointerAlphabet*)a)->labels2keys,0);
    break;
  }

  a->lab_min = gfsmNoLabel;
  a->lab_max = gfsmNoLabel;
}

/*--------------------------------------------------------------
 * gfsm_alphabet_foreach_remove_func()
 */
gboolean gfsm_alphabet_foreach_remove_func(gfsmAlphabet *a,
					   GFSM_UNUSED gpointer key,
					   gfsmLabelVal   lab,
					   GFSM_UNUSED gpointer data)
{
  gfsm_alphabet_remove_label(a,lab);
  return FALSE;
}

/*--------------------------------------------------------------
 * free()
 */
void gfsm_alphabet_free(gfsmAlphabet *a)
{
  switch (a->type) {
  case gfsmATIdentity:
    gfsm_set_free(((gfsmIdentityAlphabet*)a)->labels);
    g_free((gfsmIdentityAlphabet*)a);
    return;
  case gfsmATUser:
  case gfsmATPointer:
  case gfsmATString:
    g_ptr_array_free(((gfsmPointerAlphabet*)a)->labels2keys,TRUE);
    g_hash_table_destroy(((gfsmPointerAlphabet*)a)->keys2labels);
    g_free((gfsmPointerAlphabet*)a);
    return;
  case gfsmATUnknown:
  case gfsmATRange:
  default:
    break;
  }
  g_free(a);
};

/*======================================================================
 * Methods: Utilties
 */

/*--------------------------------------------------------------
 * gfsm_alphabet_foreach()
 */
gboolean gfsm_alphabet_foreach (gfsmAlphabet            *a,
				gfsmAlphabetForeachFunc  func,
				gpointer                 data)
{
  gfsmLabelVal lab;
  gpointer     key;
  gboolean     rc=FALSE;
  for (lab = a->lab_min; !rc && lab <= a->lab_max && lab < gfsmNoLabel; lab++) {
    if ((key=gfsm_alphabet_find_key(a,lab))==gfsmNoKey) continue;
    rc  = (*func)(a,key,lab,data);
  }
  return rc;
}

/*--------------------------------------------------------------
 * strdup()
 */
gpointer gfsm_alphabet_strdup(GFSM_UNUSED gfsmAlphabet *a, const gchar *str)
{ return g_strdup(str); }

/*======================================================================
 * Methods: Accessors
 */

/*--------------------------------------------------------------
 * size()
 */
gfsmLabelVal gfsm_alphabet_size(gfsmAlphabet *a)
{
  guint n=0;
  switch (a->type) {
  case gfsmATIdentity:
    return gfsm_set_size(((gfsmIdentityAlphabet*)a)->labels);
  case gfsmATUser:
    gfsm_alphabet_foreach(a, (gfsmAlphabetForeachFunc)gfsm_alphabet_foreach_size_func, &n);
    return (gfsmLabelVal)n;
  case gfsmATPointer:
  case gfsmATString:
    return ((gfsmPointerAlphabet*)a)->labels2keys->len;
  case gfsmATUnknown:
  case gfsmATRange:
  default:
    return (a->lab_min != gfsmNoLabel && a->lab_max != gfsmNoLabel 
	    ? (a->lab_max - a->lab_min)
	    : 0);
  }
  return n;
}

/*--------------------------------------------------------------
 * foreach_size_func()
 */
gboolean gfsm_alphabet_foreach_size_func(GFSM_UNUSED gfsmAlphabet *a,
					 gpointer      key,
					 gfsmLabelVal  lab,
					 guint        *np)
{
  if (key != gfsmNoKey && lab != gfsmNoLabel) ++(*np);
  return FALSE;
}

/*--------------------------------------------------------------
 * insert()
 */
gfsmLabelVal gfsm_alphabet_insert(gfsmAlphabet *a, gpointer key, gfsmLabelVal label)
{
  switch (a->type) {

  case gfsmATIdentity:
    gfsm_set_insert(((gfsmIdentityAlphabet*)a)->labels, key);
    label = (gfsmLabelVal) GPOINTER_TO_INT(key);
    break;

  case gfsmATUser:
    if (((gfsmUserAlphabet*)a)->methods.insert) {
      if (((gfsmPointerAlphabet*)a)->key_dup_func) {
	key = (*(((gfsmPointerAlphabet*)a)->key_dup_func))(((gfsmPointerAlphabet*)a),key);
      }
      label = (*(((gfsmUserAlphabet*)a)->methods.insert))((gfsmUserAlphabet*)a, key, label);
      break;
    }

  case gfsmATPointer:
  case gfsmATString:
    if (label==gfsmNoLabel)
      label = ((gfsmPointerAlphabet*)a)->labels2keys->len;

    if (label >= ((gfsmPointerAlphabet*)a)->labels2keys->len)
      g_ptr_array_set_size(((gfsmPointerAlphabet*)a)->labels2keys, label+1);

    if (((gfsmPointerAlphabet*)a)->key_dup_func)
      key = (*(((gfsmPointerAlphabet*)a)->key_dup_func))(((gfsmPointerAlphabet*)a),key);

    g_ptr_array_index(((gfsmPointerAlphabet*)a)->labels2keys, label) = key;
    g_hash_table_insert(((gfsmPointerAlphabet*)a)->keys2labels, key, GUINT_TO_POINTER(label));
    break;

  case gfsmATUnknown:
  case gfsmATRange:
  default:
    break;
  }

  //-- range
  if (label != gfsmNoLabel) {
    if (a->lab_min==gfsmNoLabel || label < a->lab_min) a->lab_min = label;
    if (a->lab_max==gfsmNoLabel || label > a->lab_max) a->lab_max = label;
  }

  return label;
}

/*--------------------------------------------------------------
 * get_full()
 */
gfsmLabelVal gfsm_alphabet_get_full(gfsmAlphabet *a, gpointer key, gfsmLabelVal label)
{
  gfsmLabelVal l = gfsm_alphabet_find_label(a,key);
  if (l != gfsmNoLabel) {
    //-- old mapping exists
    if (label == gfsmNoLabel) return l;  //-- ... but no new mapping was requested
    gfsm_alphabet_remove_label(a,l);
  }
  return gfsm_alphabet_insert(a,key,label);
}

/*--------------------------------------------------------------
 * find_label()
 */
gfsmLabelVal gfsm_alphabet_find_label (gfsmAlphabet *a, gconstpointer key)
{
  gpointer k, l;

  switch (a->type) {

  case gfsmATIdentity:
    if (gfsm_set_contains(((gfsmIdentityAlphabet*)a)->labels, key))
      return (gfsmLabelVal)GPOINTER_TO_UINT(key);
    break;

  case gfsmATUser:
    if (((gfsmUserAlphabet*)a)->methods.key_lookup)
      return (*(((gfsmUserAlphabet*)a)->methods.key_lookup))((gfsmUserAlphabet*)a, key);

  case gfsmATPointer:
  case gfsmATString:
    if ( g_hash_table_lookup_extended(((gfsmPointerAlphabet*)a)->keys2labels, key, &k, &l) )
      return (gfsmLabelVal)GPOINTER_TO_UINT(l);
    break;

  case gfsmATUnknown:
  case gfsmATRange:
  default:
    return ( ((gfsmLabelVal)GPOINTER_TO_UINT(key)) >= a->lab_min
	       &&
	     ((gfsmLabelVal)GPOINTER_TO_UINT(key)) <= a->lab_max
	     ? ((gfsmLabelVal)GPOINTER_TO_UINT(key))
	     : gfsmNoLabel );
  }

  return gfsmNoLabel;
}

/*--------------------------------------------------------------
 * find_key
 */
gpointer gfsm_alphabet_find_key(gfsmAlphabet *a, gfsmLabelVal label)
{
  switch (a->type) {

  case gfsmATIdentity:
    if ( gfsm_set_contains(((gfsmIdentityAlphabet*)a)->labels, GUINT_TO_POINTER(label)) )
      return GUINT_TO_POINTER(label);
    break;

  case gfsmATUser:
    if (((gfsmUserAlphabet*)a)->methods.lab_lookup)
      return (*(((gfsmUserAlphabet*)a)->methods.lab_lookup))((gfsmUserAlphabet*)a, label);

  case gfsmATPointer:
  case gfsmATString:
    if (label < ((gfsmPointerAlphabet*)a)->labels2keys->len)
      return g_ptr_array_index(((gfsmPointerAlphabet*)a)->labels2keys,label);
    break;

  case gfsmATUnknown:
  case gfsmATRange:
  default:
    if (label >= a->lab_min && label <= a->lab_max)
      return GUINT_TO_POINTER(label);
  }

  return gfsmNoKey;
}

/*--------------------------------------------------------------
 * get_key()
 */
gpointer gfsm_alphabet_get_key(gfsmAlphabet *a, gfsmLabelVal label)
{
  gpointer key;
  if (label == gfsmNoLabel) return gfsmNoKey;

  key = gfsm_alphabet_find_key(a,label);
  if (key != gfsmNoKey) return key;
  gfsm_alphabet_get_full(a, gfsmNoKey, label);

  return gfsmNoKey;
}

/*--------------------------------------------------------------
 * remove_key()
 */
void gfsm_alphabet_remove_key(gfsmAlphabet *a, gconstpointer key)
{
  gfsmLabelVal label;

  switch (a->type) {

  case gfsmATIdentity:
    gfsm_set_remove(((gfsmIdentityAlphabet*)a)->labels, key);
    break;
  
  case gfsmATUser:
    if (((gfsmUserAlphabet*)a)->methods.lab_remove) {
      label = gfsm_alphabet_find_label(a,key);
      (*(((gfsmUserAlphabet*)a)->methods.lab_remove))((gfsmUserAlphabet*)a, label);
      break;
    }

  case gfsmATPointer:
  case gfsmATString:
    label = gfsm_alphabet_find_label(a,key);
    g_hash_table_remove(((gfsmPointerAlphabet*)a)->keys2labels,key);
    if (label != gfsmNoLabel && label < ((gfsmPointerAlphabet*)a)->labels2keys->len) {
      g_ptr_array_index(((gfsmPointerAlphabet*)a)->labels2keys, label) = NULL;
    }
    break;
  
  case gfsmATUnknown:
  case gfsmATRange:
  default:
    break;
  }

  //-- ranges
  //(missing)
}

/*--------------------------------------------------------------
 * remove_label()
 */
void gfsm_alphabet_remove_label(gfsmAlphabet *a, gfsmLabelVal label)
{
  gpointer key;

  switch (a->type) {
  case gfsmATIdentity:
    gfsm_set_remove(((gfsmIdentityAlphabet*)a)->labels, GUINT_TO_POINTER(label));
    break;
  
  case gfsmATUser:
    if (((gfsmUserAlphabet*)a)->methods.lab_remove) {
      (*(((gfsmUserAlphabet*)a)->methods.lab_remove))((gfsmUserAlphabet*)a, label);
      break;
    }

  case gfsmATPointer:
  case gfsmATString:
    if (label < ((gfsmPointerAlphabet*)a)->labels2keys->len) {
      key = g_ptr_array_index(((gfsmPointerAlphabet*)a)->labels2keys,label);
      g_ptr_array_index(((gfsmPointerAlphabet*)a)->labels2keys,label) = NULL;
      g_hash_table_remove(((gfsmPointerAlphabet*)a)->keys2labels,key);
    }
    break;
  
  case gfsmATUnknown:
  case gfsmATRange:
  default:
    break;
  }
}

/*--------------------------------------------------------------
 * union()
 */
gfsmAlphabet *gfsm_alphabet_union(gfsmAlphabet *a1, gfsmAlphabet *a2)
{
  gfsm_alphabet_foreach(a2, (gfsmAlphabetForeachFunc)gfsm_alphabet_foreach_union_func, a1);
  return a1;
}

/*--------------------------------------------------------------
 * union_func()
 */
gboolean gfsm_alphabet_foreach_union_func(GFSM_UNUSED gfsmAlphabet *src,
					  gpointer      src_key,
					  GFSM_UNUSED gfsmLabelVal  src_id,
					  gfsmAlphabet *dst)
{
  gfsm_alphabet_get_label(dst,src_key);      
  return FALSE;
}

/*--------------------------------------------------------------
 * gfsm_alphabet_labels_to_array_func()
 */
gboolean gfsm_alphabet_labels_to_array_func(GFSM_UNUSED gfsmAlphabet *alph,
					    GFSM_UNUSED gpointer      key,
					    gfsmLabelVal  lab,
					    GPtrArray    *ary)
{
  //g_array_append_val(ary, lab);
  g_ptr_array_add(ary, GUINT_TO_POINTER(lab));
  return FALSE;
}

/*--------------------------------------------------------------
 * gfsm_alphabet_labels_to_array()
 */
void gfsm_alphabet_labels_to_array(gfsmAlphabet *alph, GPtrArray *ary)
{
  gfsm_alphabet_foreach(alph,
			(gfsmAlphabetForeachFunc)gfsm_alphabet_labels_to_array_func,
			ary);
  //return ary;
}

/*======================================================================
 * Methods: I/O
 */

/*--------------------------------------------------------------
 * string2key()
 */
gpointer gfsm_alphabet_string2key(gfsmAlphabet *a, GString *gstr)
{
  gpointer key=NULL;

  switch (a->type) {
  
  case gfsmATUser:
    if (((gfsmUserAlphabet*)a)->methods.key_read) {
      key = (*(((gfsmUserAlphabet*)a)->methods.key_read))((gfsmUserAlphabet*)a, gstr);
      break;
    }

  case gfsmATPointer:
  case gfsmATString:
    key = gstr->str;
    break;
  
  case gfsmATUnknown:
  case gfsmATRange:
  case gfsmATIdentity:
  default:
    key = (gpointer)strtol(gstr->str,NULL,10);
    break;
  }
  return key;
}

/*--------------------------------------------------------------
 * key2string()
 */
void gfsm_alphabet_key2string(gfsmAlphabet *a, gpointer key, GString *gstr)
{
  switch (a->type) {
  
  case gfsmATUser:
    if (((gfsmUserAlphabet*)a)->methods.key_write) {
      (*(((gfsmUserAlphabet*)a)->methods.key_write))((gfsmUserAlphabet*)a, key, gstr);
      break;
    }

  case gfsmATPointer:
    //-- ?
  case gfsmATString:
    g_string_assign(gstr,key);
    break;

  case gfsmATUnknown:
  case gfsmATRange:
  case gfsmATIdentity:
  default:
    g_string_printf(gstr,"%u", GPOINTER_TO_UINT(key));
    break;
  }
}

/*--------------------------------------------------------------
 * load_handle()
 */
gboolean gfsm_alphabet_load_handle (gfsmAlphabet *a, gfsmIOHandle *ioh, GFSM_UNUSED gfsmError **errp)
{
  int c;
  gpointer    key;
  gfsmLabelVal label;
  GString *s_key = g_string_new("");
  GString *s_lab = g_string_new("");

  //if (!myname) myname = "gfsm_string_alphabet_load_file()";

  do {
    g_string_truncate(s_key,0);
    g_string_truncate(s_lab,0);

    //-- read data fields into temp strings
    for (c=gfsmio_getc(ioh); !gfsmio_eof(ioh) && isspace((char)c); c=gfsmio_getc(ioh)) ;
    if (gfsmio_eof(ioh)) break;

    for (g_string_append_c(s_key,c), c=gfsmio_getc(ioh);
	 !gfsmio_eof(ioh) && !isspace((char)c);
	 c=gfsmio_getc(ioh))
      {
	g_string_append_c(s_key,c);
      }

    for ( ; !gfsmio_eof(ioh) && isspace((char)c); c=gfsmio_getc(ioh)) ;
    if (gfsmio_eof(ioh)) break;

    for (g_string_append_c(s_lab,c), c=gfsmio_getc(ioh);
	 !gfsmio_eof(ioh) && !isspace((char)c);
	 c=gfsmio_getc(ioh))
      {
	g_string_append_c(s_lab,c);
      }

    for ( ; (char)c != '\n' && !gfsmio_eof(ioh); c=gfsmio_getc(ioh) ) ;

    //-- get actual key and label
    key   = gfsm_alphabet_string2key(a,s_key);
    label = strtol(s_lab->str, NULL, 10);
    if (gfsm_alphabet_find_label(a,key) != label) {
      gfsm_alphabet_remove_key(a, key);
      gfsm_alphabet_insert(a, key, label);
    }
  } while (!gfsmio_eof(ioh));

  //-- cleanup
  g_string_free(s_key,TRUE);
  g_string_free(s_lab,TRUE);
  return TRUE;
}


/*--------------------------------------------------------------
 * load_file()
 */
gboolean gfsm_alphabet_load_file (gfsmAlphabet *a, FILE *f, gfsmError **errp)
{
  gfsmIOHandle *ioh = gfsmio_new_zfile(f,"rb",-1);
  gboolean rc = gfsm_alphabet_load_handle(a, ioh, errp);
  if (ioh) {
    gfsmio_close(ioh);
    gfsmio_handle_free(ioh);
  }
  return rc;
}

/*--------------------------------------------------------------
 * load_filename()
 */
gboolean gfsm_alphabet_load_filename (gfsmAlphabet *a, const gchar *filename, gfsmError **errp)
{
  gfsmIOHandle *ioh = gfsmio_new_filename(filename, "rb", -1, errp);
  gboolean rc = ioh && !(*errp) && gfsm_alphabet_load_handle(a, ioh, errp);
  if (ioh) {
    gfsmio_close(ioh);
    gfsmio_handle_free(ioh);
  }
  return rc;
}



/*--------------------------------------------------------------
 * save_handle()
 */
gboolean gfsm_alphabet_save_handle(gfsmAlphabet *a, gfsmIOHandle *ioh, gfsmError **errp)
{
  gfsmAlphabetSaveFileData sfdata;
  gboolean rc;
  sfdata.ioh = ioh;
  sfdata.errp = errp;
  sfdata.gstr = g_string_new("");
  sfdata.field_sep = "\t";
  sfdata.record_sep = "\n";

  //-- guts
  rc = gfsm_alphabet_foreach(a, (gfsmAlphabetForeachFunc)gfsm_alphabet_save_file_func, &sfdata);

  //-- cleanup
  g_string_free(sfdata.gstr,TRUE);

  return !rc;
}


/*--------------------------------------------------------------
 * save_file_full()
 */
gboolean gfsm_alphabet_save_file_full(gfsmAlphabet *a, FILE *f, int zlevel, gfsmError **errp)
{
  gfsmIOHandle *ioh = gfsmio_new_zfile(f,"wb",zlevel);
  gboolean rc = ioh && !(*errp) && gfsm_alphabet_save_handle(a, ioh, errp);
  if (ioh) {
    gfsmio_close(ioh);
    gfsmio_handle_free(ioh);
  }
  return rc;
}

/*--------------------------------------------------------------
 * save_file()
 */
gboolean gfsm_alphabet_save_file(gfsmAlphabet *a, FILE *f, gfsmError **errp)
{
  return gfsm_alphabet_save_file_full(a,f,0,errp);
}


/*--------------------------------------------------------------
 * save_filename_full()
 */
gboolean gfsm_alphabet_save_filename_full (gfsmAlphabet *a, const gchar *filename, int zlevel, gfsmError **errp)
{
  gfsmIOHandle *ioh = gfsmio_new_filename(filename,"wb",zlevel,errp);
  gboolean rc = ioh && !(*errp) && gfsm_alphabet_save_handle(a, ioh, errp);
  if (ioh) {
    gfsmio_close(ioh);
    gfsmio_handle_free(ioh);
  }
  return rc;
}

/*--------------------------------------------------------------
 * save_filename()
 */
gboolean gfsm_alphabet_save_filename (gfsmAlphabet *a, const gchar *filename, gfsmError **errp)
{
  return gfsm_alphabet_save_filename_full(a,filename,0,errp);
}


/*--------------------------------------------------------------
 * save_file_func()
 */
gboolean gfsm_alphabet_save_file_func(gfsmAlphabet     *a,
				      gpointer          key,
				      gfsmLabelVal      lab,
				      gfsmAlphabetSaveFileData *sfdata)
{
  gfsm_alphabet_key2string(a,key,sfdata->gstr);
  gfsmio_printf(sfdata->ioh,
		"%s%s%u%s",
		sfdata->gstr->str, sfdata->field_sep, lab, sfdata->record_sep);
  return (sfdata->errp && *(sfdata->errp));
}



/*======================================================================
 * Methods: String Alphabet Utilities
 */

/*--------------------------------------------------------------
 * gfsm_alphabet_string_to_labels()
 */
gfsmLabelVector *gfsm_alphabet_string_to_labels(gfsmAlphabet *abet,
						const gchar *str,
						gfsmLabelVector *vec,
						gboolean warn_on_undefined)
{
  gfsmLabelVal lab;
  const gchar *s = str;
  gchar        cs[2] = {0,0};

  //-- setup vector
  if (vec==NULL) {
    vec = g_ptr_array_sized_new(strlen(str));
  } else {
    g_ptr_array_set_size(vec, 0);
  }

  for (; *s; s++) {
    cs[0] = *s;
    lab   = gfsm_alphabet_find_label(abet, cs);

    //-- check for non-existant labels
    if (lab==gfsmNoLabel) {
      if (warn_on_undefined) {
	gfsm_carp(g_error_new(g_quark_from_static_string("gfsm"), //--domain
			      g_quark_from_static_string("gfsm_alphabet_string_to_labels"), //-- code
			      "Warning: unknown character '%c' in string '%s' -- skipping.",
			      *s, str));
      }
      continue;
    }

    g_ptr_array_add(vec, GUINT_TO_POINTER(lab));
  }

  return vec;
}

/*--------------------------------------------------------------
 * gfsm_alphabet_att_string_to_labels()
 */
gfsmLabelVector *gfsm_alphabet_att_string_to_labels(gfsmAlphabet *abet,
						    const gchar *str,
						    gfsmLabelVector *vec,
						    gboolean warn_on_undefined)
{
  gfsmLabelVal lab;
  const gchar *s = str;
  GString     *gs = g_string_sized_new(4);
  gchar        mode = 0;

  //-- setup vector
  if (vec==NULL) {
    vec = g_ptr_array_sized_new(strlen(str));
  } else {
    g_ptr_array_set_size(vec, 0);
  }

  //-- loop(str): beginning of next symbol
  for (; *s; s++) {
    switch (mode) {
    case '[':
      //-- bracket-escape mode
      if (*s==']') { mode=0; }
      else { g_string_append_c(gs,*s); continue; }
      break;

    case '\\':
      //-- backslash-escape mode
      g_string_append_c(gs,*s);
      mode = 0;
      break;

    default:
    case 0:
      //-- outer (unescaped) mode
      if      (*s == '[')   { mode='['; continue; }
      else if (*s == '\\')  { mode='\\'; continue; }
      else if (isspace(*s)) { continue; } //-- ignore spaces
      //-- plain single-character symbol: set key-string
      g_string_append_c(gs,*s);
      break;
    }

    //-- lookup key
    lab = gfsm_alphabet_find_label(abet, gs->str);

    //-- check for non-existant labels
    if (lab==gfsmNoLabel) {
      if (warn_on_undefined) {
	gfsm_carp(g_error_new(g_quark_from_static_string("gfsm"), //--domain
			      g_quark_from_static_string("gfsm_alphabet_att_string_to_labels"), //-- code
			      "Warning: unknown symbol [%s] in string '%s' -- skipping.",
			      gs->str, str));
      }
      g_string_truncate(gs,0);
      continue;
    }

    //-- add to vector
    g_ptr_array_add(vec, GUINT_TO_POINTER(lab));
    g_string_truncate(gs,0);
  }

  //-- cleanup
  g_string_free(gs,TRUE);

  return vec;
}

/*--------------------------------------------------------------
 * gfsm_alphabet_generic_string_to_labels()
 */
gfsmLabelVector *gfsm_alphabet_generic_string_to_labels(gfsmAlphabet *abet,
							const gchar *str,
							gfsmLabelVector *vec,
							gboolean warn_on_undefined,
							gboolean att_mode)
{
  return (att_mode
	  ? gfsm_alphabet_att_string_to_labels(abet,str,vec,warn_on_undefined)	
	  : gfsm_alphabet_string_to_labels(abet,str,vec,warn_on_undefined));
}



/*--------------------------------------------------------------
 * gfsm_alphabet_labels_to_gstring()
 */
GString *gfsm_alphabet_labels_to_gstring(gfsmAlphabet *abet,
					 gfsmLabelVector *vec,
					 GString *gstr,
					 gboolean warn_on_undefined,
					 gboolean att_style)
{
  gfsmLabelVal lab;
  const gchar  *sym;
  guint i;

  //-- setup GString
  if (gstr==NULL) {
    gstr = g_string_new_len("",vec->len);
  }

  //-- lookup & append symbols
  for (i=0; i < vec->len; i++) {
    lab = (gfsmLabelVal)GPOINTER_TO_UINT(g_ptr_array_index(vec,i));
    sym = (const gchar*)gfsm_alphabet_find_key(abet,lab);

    //-- check for unknown labels
    if (sym==NULL) {
      if (warn_on_undefined) {
	gfsm_carp(g_error_new(g_quark_from_static_string("gfsm"), //--domain
			      g_quark_from_static_string("gfsm_alphabet_labels_to_gstring"), //-- code
			      "Warning: unknown label '%d' -- skipping.",
			      lab));
      }
      continue;
    }

    //-- append the symbol to the output string
    if (att_style) {
      if (strlen(sym)==1) {
	g_string_append_c(gstr,sym[0]);
      }
      else {
	g_string_append_c(gstr,'[');
	g_string_append(gstr,sym);
	g_string_append_c(gstr,']');
      } 
    } else { //-- !att_style
      if (i != 0) g_string_append_c(gstr,' ');
      g_string_append(gstr, sym);
    }
  }

  return gstr;
}

/*--------------------------------------------------------------
 * gfsm_alphabet_labels_to_string()
 */
char *gfsm_alphabet_labels_to_string(gfsmAlphabet *abet,
				     gfsmLabelVector *vec,
				     gboolean warn_on_undefined,
				     gboolean att_style)
{
  GString *gstr = g_string_new("");
  gfsm_alphabet_labels_to_gstring(abet,vec,gstr,warn_on_undefined,att_style);
  char *str = gstr->str;
  g_string_free(gstr,FALSE);
  return str;
}
