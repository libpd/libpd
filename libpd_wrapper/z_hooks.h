/*
 * Copyright (c) 2013 Dan Wilcox (danomatika@gmail.com)
 * Copyright (c) 2013-2021 libpd team
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
#include "util/ringbuffer.h"

// internal hooks, etc
// do *not* include this file in a user-facing header

/* hooks */

#define PRINT_LINE_SIZE 2048

typedef struct _libpdhooks {

  // messages
  t_libpd_printhook h_printhook;
  t_libpd_banghook h_banghook;
  t_libpd_floathook h_floathook;
  t_libpd_symbolhook h_symbolhook;
  t_libpd_listhook h_listhook;
  t_libpd_messagehook h_messagehook;

  // MIDI
  t_libpd_noteonhook h_noteonhook;
  t_libpd_controlchangehook h_controlchangehook;
  t_libpd_programchangehook h_programchangehook;
  t_libpd_pitchbendhook h_pitchbendhook;
  t_libpd_aftertouchhook h_aftertouchhook;
  t_libpd_polyaftertouchhook h_polyaftertouchhook;
  t_libpd_midibytehook h_midibytehook;

  char h_print_linebuffer[PRINT_LINE_SIZE];
  int h_print_linebuflen;
  
  // QUEUED HOOKS
  t_libpd_printhook h_queued_printhook;
  t_libpd_banghook h_queued_banghook;
  t_libpd_floathook h_queued_floathook;
  t_libpd_symbolhook h_queued_symbolhook;
  t_libpd_listhook h_queued_listhook;
  t_libpd_messagehook h_queued_messagehook;

  t_libpd_noteonhook h_queued_noteonhook;
  t_libpd_controlchangehook h_queued_controlchangehook;
  t_libpd_programchangehook h_queued_programchangehook;
  t_libpd_pitchbendhook h_queued_pitchbendhook;
  t_libpd_aftertouchhook h_queued_aftertouchhook;
  t_libpd_polyaftertouchhook h_queued_polyaftertouchhook;
  t_libpd_midibytehook h_queued_midibytehook;

  ring_buffer *h_queued_pdbuffer;
  ring_buffer *h_queued_midibuffer;
  char *h_queued_tmpbuffer;
} t_libpdhooks;

/// alloc new hooks struct and set all to NULL
t_libpdhooks *libpdhooks_new(void);

/// free a hooks struct
/// does nothing if hooks are from the main instance
void libpdhooks_free(t_libpdhooks *hooks);

/* instance */

/// libpd per-instance implementation data
typedef struct _libpdimp {
  t_libpdhooks *i_hooks; /* event hooks */
  void *i_data;          /* user data, default NULL */
} t_libpdimp;

/// main instance implementation data, always valid
extern t_libpdimp libpd_mainimp;

/// alloc new instance implementation data
t_libpdimp* libpdimp_new(void);

/// free instance implementation data
/// does nothing if imp is libpd_mainimp
void libpdimp_free(t_libpdimp *imp);

/// get current instance implementation data
t_libpdimp* libpdimp_this(void);

#endif
