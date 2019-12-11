/*
 * Copyright (c) 2013 Dan Wilcox (danomatika@gmail.com)
 * Copyright (c) 2013-2019 libpd team
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

// internal hooks, etc
// do *not* include this file in a user-facing header

/* hooks */

typedef struct _libpdhooks {

  // messages
  // no libpd_printhook as libpd_set_printhook() sets internal sys_printhook
  t_libpd_banghook banghook;
  t_libpd_floathook floathook;
  t_libpd_symbolhook symbolhook;
  t_libpd_listhook listhook;
  t_libpd_messagehook messagehook;

  // MIDI
  t_libpd_noteonhook noteonhook;
  t_libpd_controlchangehook controlchangehook;
  t_libpd_programchangehook programchangehook;
  t_libpd_pitchbendhook pitchbendhook;
  t_libpd_aftertouchhook aftertouchhook;
  t_libpd_polyaftertouchhook polyaftertouchhook;
  t_libpd_midibytehook midibytehook;
} t_libpdhooks;

/// main instance hooks
extern t_libpdhooks libpd_mainhooks;

/// alloc new hooks struct and set all to NULL
t_libpdhooks *libpdhooks_new(void);

/// free a hooks struct
/// does nothing if hooks are from the main instance
void libpdhooks_free(t_libpdhooks *hooks);

/* instance */

typedef struct t_libpdinstance {
  t_pdinstance *pd;
  t_libpdhooks *hooks;
} t_libpdinstance;

/// main instance, always valid
extern t_libpdinstance libpd_maininstance;

#ifdef PDINSTANCE

/// current instance
extern PERTHREAD t_libpdinstance *libpd_this;

/// available instances
extern t_libpdinstance **libpd_instances;

/// set the current instance
void libpd_setinstance(t_libpdinstance *x);

/// create a new instance, sets x->pd = pd_this
/// this will be in addition to the main instance
t_libpdinstance *libpdinstance_new(void);

/// frees an instance, does not free x->pd
/// does nothing if x is the main instance
void libpdinstance_free(t_libpdinstance *x);

#else

/// current instance
#define libpd_this (&libpd_maininstance)

#endif /* PDINSTANCE */

#endif
