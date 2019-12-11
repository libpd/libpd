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
#include <stdlib.h>

/* hooks */

// main instance hooks
static t_libpdhooks libpd_mainhooks = {0};

t_libpdhooks* libpdhooks_new(void) {
  t_libpdhooks *hooks = calloc(1, sizeof(t_libpdhooks));
  return hooks;
}

void libpdhooks_free(t_libpdhooks *hooks) {
  if (hooks != libpd_mainhooks)
    free(hooks);
}

/* instance */

t_libpdinstance libpd_maininstance = {&pd_maininstance, &libpd_mainhooks};

#ifdef PDINSTANCE

PERTHREAD t_libpdinstance *libpd_this = &libpd_maininstance;
t_libpdinstance **libpd_instances = NULL;

void libpd_setinstance(t_libpdinstance *x) {
  sys_lock();
  libpd_this = x;
  sys_unlock();
}

t_libpdinstance *libpdinstance_new(void) {
  t_libpdinstance *x = calloc(1, sizeof(t_libpdinstance));
  x->pd = pd_this;
  x->hooks = libpdhooks_new();
  libpd_this = x;
  sys_lock();
  libpd_instances = (t_libpdinstance **)resizebytes(libpd_instances,
        pd_ninstances * sizeof(*libpd_instances),
        (pd_ninstances+1) * sizeof(*libpd_instances));
  libpd_instances[pd_ninstances] = x;
  sys_unlock();
  return x;
}

void libpdinstance_free(t_libpdinstance *x) {
  int i;
  sys_lock();
  for (i = x->pd->pd_instanceno; i < pd_ninstances-1; i++)
      libpd_instances[i] = libpd_instances[i+1];
  libpd_instances = (t_libpdinstance **)resizebytes(libpd_instances,
      pd_ninstances * sizeof(*libpd_instances),
      (pd_ninstances-1) * sizeof(*libpd_instances));
  sys_unlock();
  libpd_setinstance(&libpd_maininstance);
  libpdhooks_free(x->hooks);
  free(x);
}

#endif /* PDINSTANCE */
