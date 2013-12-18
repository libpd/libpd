/*
 * Copyright (c) 2013 Dan Wilcox (danomatika@gmail.com)
 *
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.
 *
 * See https://github.com/libpd/libpd/wiki for documentation
 *
 */

#ifndef __Z_HOOKS_H__
#define __Z_HOOKS_H__

#include "z_libpd.h"

// the internal hooks
// in a separate file so they can be used throughout the libpd_wrapper sources,
// do *not* include this file in a user-facing header

t_libpd_printhook libpd_printhook;
t_libpd_banghook libpd_banghook;
t_libpd_floathook libpd_floathook;
t_libpd_symbolhook libpd_symbolhook;
t_libpd_listhook libpd_listhook;
t_libpd_messagehook libpd_messagehook;

t_libpd_noteonhook libpd_noteonhook;
t_libpd_controlchangehook libpd_controlchangehook;
t_libpd_programchangehook libpd_programchangehook;
t_libpd_pitchbendhook libpd_pitchbendhook;
t_libpd_aftertouchhook libpd_aftertouchhook;
t_libpd_polyaftertouchhook libpd_polyaftertouchhook;
t_libpd_midibytehook libpd_midibytehook;

// sets all hook function pointers to NULL
void libpd_clear_hooks(void);

#endif
