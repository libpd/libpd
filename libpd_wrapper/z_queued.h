/*
 * Copyright (c) 2012 Peter Brinkmann (peter.brinkmann@gmail.com)
 *
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.
 */

#ifndef __Z_QUEUED_H__
#define __Z_QUEUED_H__

#include "z_libpd.h"

#ifdef __cplusplus
extern "C"
{
#endif

EXTERN t_libpd_printhook libpd_queued_printhook;
EXTERN t_libpd_banghook libpd_queued_banghook;
EXTERN t_libpd_floathook libpd_queued_floathook;
EXTERN t_libpd_symbolhook libpd_queued_symbolhook;
EXTERN t_libpd_listhook libpd_queued_listhook;
EXTERN t_libpd_messagehook libpd_queued_messagehook;

EXTERN t_libpd_noteonhook libpd_queued_noteonhook;
EXTERN t_libpd_controlchangehook libpd_queued_controlchangehook;
EXTERN t_libpd_programchangehook libpd_queued_programchangehook;
EXTERN t_libpd_pitchbendhook libpd_queued_pitchbendhook;
EXTERN t_libpd_aftertouchhook libpd_queued_aftertouchhook;
EXTERN t_libpd_polyaftertouchhook libpd_queued_polyaftertouchhook;
EXTERN t_libpd_midibytehook libpd_queued_midibytehook;

int libpd_queued_init();
void libpd_queued_release();
void libpd_queued_receive_pd_messages();
void libpd_queued_receive_midi_messages();

#ifdef __cplusplus
}
#endif

#endif
