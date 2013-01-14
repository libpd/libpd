/*
 * Copyright (c) 2012 Dan Wilcox (danomatika@gmail.com)
 *
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.
 */

#ifndef __Z_UTIL_H__
#define __Z_UTIL_H__

#include "z_libpd.h"
#include "z_queued.h"

#ifdef __cplusplus
extern "C"
{
#endif

// set one of these callbacks instead of the normal or queued printhook
// before calling libpd_concatenate_print_messages()
EXTERN t_libpd_printhook libpd_concatenated_printhook;
EXTERN t_libpd_printhook libpd_queued_concatenated_printhook;

// return print messages as single lines
// ie "hello 123" is sent in 1 part -> "hello 123"
//
// call this before libpd_init()
//
// note: one of the concateneated print hooks must be set,
//       or this function has no effect
void libpd_concatenate_print_messages();

// return print messages as individual words and spaces, on by default
// ie "hello 123" is sent in 3 parts -> "hello", " ", "123"
//
// the libpd or queued print hook must be manually reconnected
// after calling this function
//
// note: has no effect if libpd_concatenate_print_messages()
//       was not called previously
void libpd_segment_print_messages();

#ifdef __cplusplus
}
#endif

#endif
