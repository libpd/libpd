
/*=============================================================================*\
 * File: gfsmCommon.h
 * Author: Bryan Jurish <moocow@ling.uni-potsdam.de>
 * Description: finite state machine library: common definitions
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

/** \file gfsmCommon.h
 *  \brief Commonly use typedefs and constants
 */

#ifndef _GFSM_COMMON_H
#define _GFSM_COMMON_H

#include <gfsmConfig.h>
#include <glib.h>

/*======================================================================
 * Defines
 */
#ifdef __GNUC__
# define GFSM_UNUSED __attribute__((unused))
#else
# define GFSM_UNUSED
#endif


/*======================================================================
 * Basic Types
 */
/** Type for elementary arc-labels */
typedef guint16 gfsmLabelId;

/** Type for extended arc-labels (parameters and return values) */
typedef guint32 gfsmLabelVal;

/** Type for elementary state identifiers */
typedef guint32 gfsmStateId;

/** Alias for state identifiers */
typedef gfsmStateId gfsmNodeId;

/** Type for arc weights */
typedef gfloat gfsmWeight;

/*======================================================================
 * Vector types
 */
/** Type for sequence of (gfsmLabelVal)s */
typedef GPtrArray gfsmLabelVector;

/** Type for a sequence of (gfsmStateId)s */
typedef GPtrArray gfsmStateIdVector;


/*======================================================================
 * Constants
 */
/** Constant epsilon label */
extern const gfsmLabelId gfsmEpsilon;

/** Constant label for pseudo-epsilon moves in 1st argument to compose() */
extern const gfsmLabelId gfsmEpsilon1;

/** Constant label for pseudo-epsilon moves in 2nd argument to compose() */
extern const gfsmLabelId gfsmEpsilon2;

/** Constant indicating missing alphabet key */
extern const gpointer    gfsmNoKey;

/** Constant indicating missing label */
extern const gfsmLabelId gfsmNoLabel;

/** Constant indicating missing state */
extern const gfsmStateId gfsmNoState;

/** Constant indicating missing weight
 *  \warning Deprecated: prefer gfsm_sr_one()
 */
extern const gfsmWeight gfsmNoWeight;

#endif /* _GFSM_COMMON_H */
