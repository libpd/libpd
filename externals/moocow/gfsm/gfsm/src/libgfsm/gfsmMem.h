
/*=============================================================================*\
 * File: gfsmMem.h
 * Author: Bryan Jurish <moocow@ling.uni-potsdam.de>
 * Description: finite state machine library: memory utilities (currently unused)
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

/** \file gfsmMem.h
 *  \brief Memory utilities
 */

#ifndef _GFSM_MEM_H
#define _GFSM_MEM_H

#include <glib.h>
#include <gfsmConfig.h>

/*----------------------------------------------------------------------
 * Allocators: variables
 */

/** Default GNode allocator */
extern GAllocator *gfsm_node_allocator;

/** Default GSList allocator */
extern GAllocator *gfsm_slist_allocator;

/** Default GList allocator */
extern GAllocator *gfsm_list_allocator;

/** Whether gfsm allocators are currently enabled */
extern gboolean gfsm_allocators_enabled;


/*----------------------------------------------------------------------
 * Allocators
 *  - these aren't used by default!
 */

/** Ensure that gfsm allocators are defined and non-NULL */
GFSM_INLINE
void gfsm_allocators_init(void);

/** Push gfsm allocators to the stack */
GFSM_INLINE
void gfsm_allocators_enable(void);

/** Pop gfsm allocators from the stack */
GFSM_INLINE
void gfsm_allocators_disable(void);

/** Free all memory allocated by the gfsm allocators */
GFSM_INLINE
void gfsm_allocators_free(void);


/*----------------------------------------------------------------------
 * Copying
 */
/** Abstract copy function */
typedef gpointer    (*gfsmDupNFunc)    (gconstpointer src, gsize size);

/** Abstract duplication function */
typedef gpointer    (*gfsmDupFunc)     (gconstpointer src);

/** String copy function for NUL-terminated strings */
GFSM_INLINE
gpointer gfsm_string_dup_n (gconstpointer src, gsize size);

/** size-based copy function */
GFSM_INLINE
gpointer gfsm_mem_dup_n (gconstpointer src, gsize size);

/** String duplication function for NUL-terminated strings */
#define gfsm_string_dup g_strdup

/** String duplication function for GString*s */
GFSM_INLINE
GString *gfsm_gstring_dup (GString *gstr);

/** Byte-assignment for GString*s */
GFSM_INLINE
void gfsm_gstring_assign_bytes (GString *gstr, const gchar *src, gsize len);

/** Byte-vector creation for GString*s */
GFSM_INLINE
GString *gfsm_gstring_new_bytes (const gchar *src, gsize len);

//-- inline definitions
#ifdef GFSM_INLINE_ENABLED
# include <gfsmMem.hi>
#endif

#endif /* _GFSM_MEM_H */
