/*
 * Copyright (c) 2012 Dan Wilcox (danomatika@gmail.com) &
 *                    Peter Brinkmann (peter.brinkmann@gmail.com)
 *
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.
 */

#ifndef __Z_UTIL_H__
#define __Z_UTIL_H__

#include "z_libpd.h"

#ifdef __cplusplus
extern "C"
{
#endif

// Concatenate print messages into single lines before returning them to a print hook.
// ie "hello 123" is sent in 1 part -> "hello 123"
//
// For comparison, the default behavior returns individual words and spaces.
// ie "hello 123" is sent in 3 parts -> "hello", " ", "123"

// Assign the pointer to your print handler to this variable.
EXTERN t_libpd_printhook libpd_concatenated_printhook;

// Assign this function pointer to libpd_printhook or libpd_queued_printhook,
// depending on whether you're using queued messages, to intercept and
// concatenate print messages:
//
// libpd_printhook = (t_libpd_printhook) libpd_internal_concatenated_printhook;
// libpd_concatenated_printhook = (t_libpd_printhook) yourPrintHandler;
//
void libpd_internal_concatenated_printhook(const char *s);

#ifdef __cplusplus
}
#endif

#endif
