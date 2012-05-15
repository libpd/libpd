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

EXTERN int libpd_safe_init();

EXTERN void libpd_set_printhook(const t_libpd_printhook hook);
EXTERN void libpd_set_banghook(const t_libpd_banghook hook);
EXTERN void libpd_set_floathook(const t_libpd_floathook hook);
EXTERN void libpd_set_symbolhook(const t_libpd_symbolhook hook);
EXTERN void libpd_set_listhook(const t_libpd_listhook hook);
EXTERN void libpd_set_messagehook(const t_libpd_messagehook hook);

EXTERN int libpd_atom_is_float(t_atom *a);
EXTERN int libpd_atom_is_symbol(t_atom *a);
EXTERN float libpd_atom_get_float(t_atom *a);
EXTERN char *libpd_atom_get_symbol(t_atom *a);
EXTERN t_atom *libpd_next_atom(t_atom *a);

#ifdef __cplusplus
}
#endif

#endif
