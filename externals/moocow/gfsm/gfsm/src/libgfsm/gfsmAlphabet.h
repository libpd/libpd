
/*=============================================================================*\
 * File: gfsmAlphabet.h
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

/** \file gfsmAlphabet.h
 *  \brief Map between gfsmLabelIds and external objects
 */

#ifndef _GFSM_ALPHABET_H
#define _GFSM_ALPHABET_H

#include <gfsmCommon.h>
#include <gfsmSet.h>
#include <gfsmIO.h>

/*======================================================================
 * Alphabet: Flags
 */
/** Enumeration of builtin alphabet types */
typedef enum {
  gfsmATUnknown  = 0,   ///< unknown alphabet type
  gfsmATRange    = 1,   ///< alphabet type for label ranges
  gfsmATIdentity = 2,   ///< alphabet type for sparse identity alphabets
  gfsmATPointer  = 3,   ///< pointer-hashing alphabet
  gfsmATString   = 4,   ///< string alphabet
  gfsmATUser     = 256  ///< user-defined alphabet
} gfsmAType;

/*======================================================================
 * Alphabet: Basic Types
 */
/// Generic alphabet structure
typedef struct {
  gfsmAType     type;          /**< alphabet type */
  gfsmLabelVal  lab_min;       /**< minimum label */
  gfsmLabelVal  lab_max;       /**< maximum label */
} gfsmAlphabet;

/// Ranged alphabet structure
typedef gfsmAlphabet gfsmRangeAlphabet;

/// Sparse identity alphabet structure
typedef struct {
  gfsmAlphabet  a;             /**< inheritance magic */
  gfsmSet      *labels;        /**< known labels */
} gfsmIdentityAlphabet;

// Pointer-hashing alphabet structure (forward decl)
struct gfsmPointerAlphabet_;

// User-extendable alphabet structure (forward decl)
struct gfsmUserAlphabet_;

//@}

/*======================================================================
 * Alphabet: Function types
 */
///\name Alphabet: Function Types
//@{
/** Type for key-duplication functions */
typedef gpointer (*gfsmAlphabetKeyDupFunc) (struct gfsmPointerAlphabet_ *a, gpointer key);

/** Type for alphabet (key->label) lookup functions */
typedef gfsmLabelVal (*gfsmAlphabetKeyLookupFunc) (struct gfsmUserAlphabet_ *a, gconstpointer key);

/** Type for alphabet (label->key) lookup functions */
typedef gpointer (*gfsmAlphabetLabLookupFunc) (struct gfsmUserAlphabet_ *a, gfsmLabelVal lab);

/** Type for alphabet insertion functions */
typedef gfsmLabelVal (*gfsmAlphabetInsertFunc) (struct gfsmUserAlphabet_ *a, gpointer key, gfsmLabelVal lab);

/** Type for alphabet key removal functions (unused) */
typedef void (*gfsmAlphabetKeyRemoveFunc) (struct gfsmUserAlphabet_ *a, gpointer key);

/** Type for alphabet label removal functions */
typedef void (*gfsmAlphabetLabRemoveFunc) (struct gfsmUserAlphabet_ *a, gfsmLabelVal lab);

/** Type for alphabet string input functions (should return a static key) */
typedef gpointer (*gfsmAlphabetKeyReadFunc) (struct gfsmUserAlphabet_ *a, GString *gstr);

/** Type for alphabet string output functions (should write to @str) */
typedef void (*gfsmAlphabetKeyWriteFunc) (struct gfsmUserAlphabet_ *a, gconstpointer key, GString *str);

/// method table for user-defined alphabets
typedef struct {
  gfsmAlphabetKeyLookupFunc key_lookup;  /**< key->label lookup function */
  gfsmAlphabetLabLookupFunc lab_lookup;  /**< label->key lookup function */
  gfsmAlphabetInsertFunc    insert;      /**< insertion function: receives a newly copied key! */
  gfsmAlphabetLabRemoveFunc lab_remove;  /**< label removal function */
  gfsmAlphabetKeyReadFunc   key_read;    /**< key input function */
  gfsmAlphabetKeyWriteFunc  key_write;   /**< key output function */
} gfsmUserAlphabetMethods;

/// default methods for user-defined alphabets (dummy)
extern gfsmUserAlphabetMethods gfsmUserAlphabetDefaultMethods;
//@}

/*======================================================================
 * Alphabet: Extendible types
 */
///\name Alphabet: Extendible Types
//@{
/// Pointer-hashing alphabet structure
typedef struct gfsmPointerAlphabet_ {
  gfsmAlphabet           a;             /**< inheritance magic */
  GPtrArray             *labels2keys;   /**< label->key lookup table */
  GHashTable            *keys2labels;   /**< key->label lookup table */
  gfsmAlphabetKeyDupFunc key_dup_func;  /**< key duplication function */
} gfsmPointerAlphabet;

/// type for string alphabets
typedef gfsmPointerAlphabet gfsmStringAlphabet;

/// User-extendable alphabet structure
typedef struct gfsmUserAlphabet_
{
  gfsmPointerAlphabet      aa;          /**< inheritance magic */
  gpointer                 data;        /**< user data */
  gfsmUserAlphabetMethods  methods;     /**< method table */
} gfsmUserAlphabet;
//@}

/*======================================================================
 * Methods: Constructors etc.
 */
/// \name Constructors etc.
//@{

/** Create a new alphabet. The alphabet will be uninitialized until you call
 *  one of the gfsm_*_alphabet_init() functions.
 *
 *  \param type Type of alphabet to create.
 */
gfsmAlphabet *gfsm_alphabet_new(gfsmAType type);


/** Create and initialize a new identity alphabet.
 *  You do not need to call an init() function for the returned alphabet.
 */
#define gfsm_identity_alphabet_new() \
  gfsm_identity_alphabet_init((gfsmIdentityAlphabet*)gfsm_alphabet_new(gfsmATIdentity))

/** Create and initialize a new string alphabet.
 *  You do not need to call an init() function for the returned alphabet.
 */
#define gfsm_string_alphabet_new_full(docopy) \
  gfsm_string_alphabet_init((gfsmStringAlphabet*)gfsm_alphabet_new(gfsmATString),(docopy))

/** Create and initialize a new string alphabet which copies keys.
 *  You do not need to call an init() function for the returned alphabet.
 */
#define gfsm_string_alphabet_new() gfsm_string_alphabet_new_full(TRUE)

/** Create and initialize a new range alphabet.
 *  You do not need to call an init() function for the returned alphabet. */
#define gfsm_range_alphabet_new() \
  gfsm_range_alphabet_init((gfsmRangeAlphabet*)gfsm_alphabet_new(gfsmATRange), \
                           gfsmNoLabel, gfsmNoLabel)

/** Create and initialize a new pointer alphabet.
 *  You do not need to call an init() function for the returned alphabet. */
#define gfsm_pointer_alphabet_new(key_dup_f, key_hash_f, key_eq_f, key_free_f) \
  gfsm_pointer_alphabet_init((gfsmPointerAlphabet*)gfsm_alphabet_new(gfsmATPointer),\
                             key_dup_f, key_hash_f, key_eq_f, key_free_f)


/** Initialize a builtin alphabet (depending on \a a->type)
 *  This really only works well identity, range, and string alphabets,
 *  as well as for literal pointer alphabets (without copy and/or free)
 *  and for user alphabets using literal pointers.
 */
gfsmAlphabet *gfsm_alphabet_init(gfsmAlphabet *a);


/** Initialize a range alphabet */
gfsmAlphabet *gfsm_range_alphabet_init (gfsmRangeAlphabet *a, gfsmLabelVal min, gfsmLabelVal max);

/** Initialize a sparse identity alphabet */
gfsmAlphabet *gfsm_identity_alphabet_init (gfsmIdentityAlphabet *a);

/** Initialize a pointer alphabet */
gfsmAlphabet *gfsm_pointer_alphabet_init (gfsmPointerAlphabet *a,
					  gfsmAlphabetKeyDupFunc key_dup_func,
					  GHashFunc      key_hash_func,
					  GEqualFunc     key_equal_func,
					  GDestroyNotify key_destroy_func);

/** Initialize a string alphabet */
gfsmAlphabet *gfsm_string_alphabet_init (gfsmStringAlphabet *a, gboolean do_copy);


/** Initialize a user alphabet */
gfsmAlphabet *gfsm_user_alphabet_init(gfsmUserAlphabet        *a,
				      gfsmAlphabetKeyDupFunc   key_dup_func,
				      GHashFunc                key_hash_func,
				      GEqualFunc               key_equal_func,
				      GDestroyNotify           key_destroy_func,
				      gpointer                 user_data,
				      gfsmUserAlphabetMethods *methods);

/** Clear all labels and keys from a gfsmAlphabet */
void gfsm_alphabet_clear(gfsmAlphabet *a);

/** foreach utility function to clear user alphabets */
gboolean gfsm_alphabet_foreach_remove_func (gfsmAlphabet *a,
					    gpointer      key,
					    gfsmLabelVal   lab,
					    gpointer      data);

/** Free all memory allocated by a gfsmAlphabet */
void gfsm_alphabet_free(gfsmAlphabet *a);
//@}

/*======================================================================
 * Methods: Utilties
 */
///\name Utilities
//@{
/** Type for alphabet iterator functions.
 *  Functions should return \a TRUE to stop the traversal
 */
typedef gboolean (*gfsmAlphabetForeachFunc) (gfsmAlphabet *a,
					     gpointer      key,
					     gfsmLabelVal   lab,
					     gpointer      data);

/** General iteration utility */
gboolean gfsm_alphabet_foreach (gfsmAlphabet            *a,
				gfsmAlphabetForeachFunc  func,
				gpointer                 data);

/** dup function for string alphabets */
gpointer gfsm_alphabet_strdup(gfsmAlphabet *a, const gchar *str);
//@}

/*======================================================================
 * Methods: Accessors
 */
/// \name Accessors
///@{
/** Get number of elements in the alphabet */
gfsmLabelVal gfsm_alphabet_size(gfsmAlphabet *a);

/** Utility for counting size of user alphabets (linear time) */
gboolean gfsm_alphabet_foreach_size_func(gfsmAlphabet *a,
					 gpointer      key,
					 gfsmLabelVal  lab,
					 guint        *np);

/**
 * Insert a \a (key,label) pair into the alphabet.
 * If \a label is \a gfsmNoLabel, a new label will be assigned.
 * \note No sanity checks are performed.
 *
 * \returns the new label for \a key
 */
gfsmLabelVal gfsm_alphabet_insert(gfsmAlphabet *a, gpointer key, gfsmLabelVal label);

/** Get or assign a label for \a key.
 *  If \a label is gfsmNoLabel, a new label will be assigned for \a key if none exists.
 * \returns label for \a key
 */
gfsmLabelVal gfsm_alphabet_get_full(gfsmAlphabet *a, gpointer key, gfsmLabelVal label);

/** Get label for \a key or assign a new one if none exists.
 * \returns label for \a key
 */
#define gfsm_alphabet_get_label(a,key) gfsm_alphabet_get_full(a,key,gfsmNoLabel)

/** Lookup label for \a key.
 * \returns label for \a key, or gfsmNoLabel if none is defined.
 */
gfsmLabelVal gfsm_alphabet_find_label(gfsmAlphabet *a, gconstpointer key);

/** Lookup key for \a label
 * \returns pointer to key for \a label, or \a NULL if no key is defined.
 */
gpointer gfsm_alphabet_find_key(gfsmAlphabet *a, gfsmLabelVal label);

/** Get key for \a label or assign gfsmNoKey if none exists.
 * \returns key for \a label
 */
gpointer gfsm_alphabet_get_key(gfsmAlphabet *a, gfsmLabelVal label);

/** Remove mapping for \a key (and associated label, if any) */
void gfsm_alphabet_remove_key(gfsmAlphabet *a, gconstpointer key);

/** Remove mapping for \a label (and associated key, if any) */
void gfsm_alphabet_remove_label(gfsmAlphabet *a, gfsmLabelVal label);

/** Add all keys from alphabet \a a2 to \a a1. \returns \a a1 */
gfsmAlphabet *gfsm_alphabet_union(gfsmAlphabet *a1, gfsmAlphabet *a2);

/** foreach utility func for union() */
gboolean gfsm_alphabet_foreach_union_func(gfsmAlphabet *src,
					  gpointer      src_key,
					  gfsmLabelVal   src_id,
					  gfsmAlphabet *dst);

/** Append all defined labels to a GPtrArray of (gfsmLabelVal)s */
void gfsm_alphabet_labels_to_array(gfsmAlphabet *alph, GPtrArray *ary);

//@}

/*======================================================================
 * Methods: I/O
 */
///\name I/O
//@{

/** Convert a string to a temporary key, used by load().
 *  If you allocate anything here, you need to free it yourself.
 */
gpointer gfsm_alphabet_string2key(gfsmAlphabet *a, GString *gstr);

/** Convert a key to a constant string, used by save() */
void gfsm_alphabet_key2string(gfsmAlphabet *a, gpointer key, GString *gstr);


/** Load a string alphabet from a stream.  Returns true on success */
gboolean gfsm_alphabet_load_handle (gfsmAlphabet *a, gfsmIOHandle *ioh, gfsmError **errp);

/** Load a string alphabet from a stream.  Returns true on success */
gboolean gfsm_alphabet_load_file (gfsmAlphabet *a, FILE *f, gfsmError **errp);

/** Load a string alphabet from a named file */
gboolean gfsm_alphabet_load_filename (gfsmAlphabet *a, const gchar *filename, gfsmError **errp);


/** Save a string alphabet to a gfsmIOHandle* */
gboolean gfsm_alphabet_save_handle(gfsmAlphabet *a, gfsmIOHandle *ioh, gfsmError **errp);

/** Save a string alphabet to a stream (uncompressed) */
gboolean gfsm_alphabet_save_file (gfsmAlphabet *a, FILE *f, gfsmError **errp);

/** Save a string alphabet to a (compressed) stream */
gboolean gfsm_alphabet_save_file_full (gfsmAlphabet *a, FILE *f, int zlevel, gfsmError **errp);

/** Save a string alphabet to a named file (uncompressed) */
gboolean gfsm_alphabet_save_filename (gfsmAlphabet *a, const gchar *filename, gfsmError **errp);

/** Save a string alphabet to a (compressed) named file */
gboolean gfsm_alphabet_save_filename_full (gfsmAlphabet *a, const gchar *filename, int zlevel, gfsmError **errp);

/// datatype used for save_file() iteration
typedef struct {
  gfsmIOHandle  *ioh;
  gfsmError    **errp;
  GString       *gstr;
  gchar         *field_sep;
  gchar         *record_sep;
} gfsmAlphabetSaveFileData;

/** save_file iterator func */
gboolean gfsm_alphabet_save_file_func(gfsmAlphabet     *a,
				      gpointer          key,
				      gfsmLabelVal      lab,
				      gfsmAlphabetSaveFileData *sfdata);
//@}

/*======================================================================
 * String Alphabet Utilties
 */
///\name String Alphabet Utilities
//@{

/** Convert an ASCII string character-wise to a vector of (gfsmLabel)s.
 *  \a vec is not cleared -- use g_ptr_array_set_size() for that.
 *  \returns \a vec if non-\a NULL, otherwise a new gfsmLabelVector.
 *  \a abet should be a gfsmStringAlphabet.
 */
gfsmLabelVector *gfsm_alphabet_string_to_labels(gfsmAlphabet *abet,
						const gchar *str,
						gfsmLabelVector *vec,
						gboolean warn_on_undefined);

/** Convert an ASCII GString character-wise to a vector of (gfsmLabel)s.
 *  \a vec is not cleared -- use g_ptr_array_set_size() for that.
 *  \returns \a vec if non-\a NULL, otherwise a new gfsmLabelVector.
 *  \a abet should be a gfsmStringAlphabet.
 */
#define gfsm_alphabet_gstring_to_labels(abet,gstr,vec,warn) \
        gfsm_alphabet_string_to_labels((abet),(gstr)->str,(vec),(warn))


/** Convert an ASCII string in AT&T syntax to a vector of (gfsmLabel)s.
 *  \a vec is not cleared -- use g_ptr_array_set_size() for that.
 *  \returns \a vec if non-\a NULL, otherwise a new gfsmLabelVector.
 *  \a abet should be a gfsmStringAlphabet.
 */
gfsmLabelVector *gfsm_alphabet_att_string_to_labels(gfsmAlphabet *abet,
						    const gchar *str,
						    gfsmLabelVector *vec,
						    gboolean warn_on_undefined);

/** Convert an ASCII string to a vector of (gfsmLabel)s,
 *  using either ::gfsm_alphabet_string_to_labels() or ::gfsm_alphabet_att_string_to_labels().
 *  \param abet,str,vec,warn_undef as for ::gfsm_alphabet_string_to_labels().
 *  \param att_mode if true, \c str is parsed as att-syntax, otherwise character-wise
 *  \returns as for ::gfsm_alphabet_string_to_labels()
 */
gfsmLabelVector *gfsm_alphabet_generic_string_to_labels(gfsmAlphabet *abet,
							const gchar *str,
							gfsmLabelVector *vec,
							gboolean warn_on_undefined,
							gboolean att_mode);

/** Convert a gfsmLabelVector to a GString.
 *  \a gstr is not cleared.
 *  \returns \a gstr if non-\a NULL, otherwise a new GString*.
 *  \a abet should be a gfsmStringAlphabet.
 */
GString *gfsm_alphabet_labels_to_gstring(gfsmAlphabet *abet,
					 gfsmLabelVector *vec,
					 GString *gstr,
					 gboolean warn_on_undefined,
					 gboolean att_style);

/** Convert a gfsmLabelVector to a new string.
 *  \a gstr is not cleared.
 *  \returns \a gstr if non-\a NULL, otherwise a new GString*.
 *  \a abet should be a gfsmStringAlphabet.
 */
char *gfsm_alphabet_labels_to_string(gfsmAlphabet *abet,
				     gfsmLabelVector *vec,
				     gboolean warn_on_undefined,
				     gboolean att_style);
//@}

#endif /*_GFSM_ALPHABET_H */
