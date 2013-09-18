
/*
 *   Pure Data Packet header file. Scalar type definitions.
 *   Copyright (c) by Tom Schouten <tom@zwizwa.be>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

/* some typedefs and utility classes */

#ifndef PDP_TYPES_H
#define PDP_TYPES_H

/* C99 standard header for bool, true, and false */
#include <stdbool.h>

/* check
   http://www.unix.org/whitepapers/64bit.html 
   on unix (LP64) int = 32bit

   it was like this:

   typedef signed long      s32;
   typedef unsigned long    u32;
*/


typedef signed char        s8;
typedef signed short       s16;
typedef signed int         s32;
typedef signed long long   s64;

typedef unsigned char      u8;
typedef unsigned short     u16;
typedef unsigned int       u32;
typedef unsigned long long u64;

typedef unsigned long      uptr;  /* An unsigned int the size of a pointer. */
typedef signed   long      sptr;

typedef struct _pdp t_pdp;
typedef void (*t_pdp_packet_method1)(t_pdp *);              /* dst */
typedef void (*t_pdp_packet_method2)(t_pdp *, t_pdp *);     /* dst, src */




/* generic packet subheader */
//typedef unsigned char t_raw[PDP_SUBHEADER_SIZE];
typedef unsigned int t_raw;


#endif
