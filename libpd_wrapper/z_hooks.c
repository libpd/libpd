/*
 * Copyright (c) 2013 Dan Wilcox (danomatika@gmail.com)
 *
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.
 *
 * See https://github.com/libpd/libpd/wiki for documentation
 *
 */
 
#include "z_hooks.h"

void libpd_clear_hooks(void) {
	libpd_printhook = NULL;
	libpd_banghook = NULL;
	libpd_floathook = NULL;
	libpd_symbolhook = NULL;
	libpd_listhook = NULL;
	libpd_messagehook = NULL;

	libpd_noteonhook = NULL;
	libpd_controlchangehook = NULL;
	libpd_programchangehook = NULL;
	libpd_pitchbendhook = NULL;
	libpd_aftertouchhook = NULL;
	libpd_polyaftertouchhook = NULL;
	libpd_midibytehook = NULL;
}
