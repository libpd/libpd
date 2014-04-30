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

extern t_libpd_printhook libpd_printhook;
extern t_libpd_banghook libpd_banghook;
extern t_libpd_floathook libpd_floathook;
extern t_libpd_symbolhook libpd_symbolhook;
extern t_libpd_listhook libpd_listhook;
extern t_libpd_messagehook libpd_messagehook;

extern t_libpd_noteonhook libpd_noteonhook;
extern t_libpd_controlchangehook libpd_controlchangehook;
extern t_libpd_programchangehook libpd_programchangehook;
extern t_libpd_pitchbendhook libpd_pitchbendhook;
extern t_libpd_aftertouchhook libpd_aftertouchhook;
extern t_libpd_polyaftertouchhook libpd_polyaftertouchhook;
extern t_libpd_midibytehook libpd_midibytehook;

#endif
