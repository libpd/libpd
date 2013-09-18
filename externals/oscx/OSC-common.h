/*
Copyright © 1998. The Regents of the University of California (Regents). 
All Rights Reserved.

Written by Matt Wright, The Center for New Music and Audio Technologies,
University of California, Berkeley.

Permission to use, copy, modify, distribute, and distribute modified versions
of this software and its documentation without fee and without a signed
licensing agreement, is hereby granted, provided that the above copyright
notice, this paragraph and the following two paragraphs appear in all copies,
modifications, and distributions.

IN NO EVENT SHALL REGENTS BE LIABLE TO ANY PARTY FOR DIRECT, INDIRECT,
SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES, INCLUDING LOST PROFITS, ARISING
OUT OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION, EVEN IF REGENTS HAS
BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

REGENTS SPECIFICALLY DISCLAIMS ANY WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
PURPOSE. THE SOFTWARE AND ACCOMPANYING DOCUMENTATION, IF ANY, PROVIDED
HEREUNDER IS PROVIDED "AS IS". REGENTS HAS NO OBLIGATION TO PROVIDE
MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.

The OpenSound Control WWW page is 
    http://www.cnmat.berkeley.edu/OpenSoundControl
*/



/* OSC-common.h
   Simple stuff to #include everywhere in the OSC package

   by Matt Wright, 3/13/98
*/

#ifndef _OSC_COMMON_H
#define _OSC_COMMON_H

/* Boolean type */

#ifndef TRUE
typedef int Boolean;
#define TRUE 1
#define FALSE 0
#endif


#ifdef _WIN32
	#include <stdio.h>
	#ifdef _DEBUG
		#define DEBUG
	#endif
#endif /* _WIN32 */

/* only needed on Microsoft compilers */
#ifdef _MSC_VER
	#ifdef OSC_EXPORTS
		#define OSC_API __declspec(dllexport)
	#else
		#define OSC_API __declspec(dllimport)
	#endif
#endif /* _MSC_VER */

//#define int32_t t_int

/* Fixed byte width types */
typedef int int4;   /* 4 byte int */

/* Printing type procedures.  All take printf-style format string */

/* Catastrophic failure: print message and halt system */
void fatal_error(char *s, ...);

/* Error message for user */
void OSCProblem(char *s, ...);

/* Warning for user */
void OSCWarning(char *s, ...);



#endif /* _OSC_COMMON_H */

