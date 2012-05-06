/*
 * Copyright (c) 2012 Tebjan Halm (tebjan@vvvv.org)
 *
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.
 *
 * See https://github.com/libpd/libpd/wiki for documentation
 *
 */

#ifndef __Z_HOOKSET_H__
#define __Z_HOOKSET_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include "z_libpd.h"

EXTERN void libpd_set_printhook(const t_libpd_printhook hook);

#ifdef __cplusplus
}
#endif

#endif