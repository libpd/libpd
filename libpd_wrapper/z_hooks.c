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

#include "z_hooks.h"
#include <stdlib.h>

/* hooks */

static t_libpdhooks libpd_mainhooks = {0};

t_libpdhooks *libpdhooks_new(void) {
  t_libpdhooks *hooks = (t_libpdhooks *)calloc(1, sizeof(t_libpdhooks));
  return hooks;
}

void libpdhooks_free(t_libpdhooks *hooks) {
  if (hooks != &libpd_mainhooks)
    free(hooks);
}

/* instance */

t_libpdinstance libpd_maininstance = {&pd_maininstance, &libpd_mainhooks};

#ifdef PDINSTANCE

PERTHREAD t_libpdinstance *libpd_this = &libpd_maininstance;
t_libpdinstance **libpd_instances = NULL;

void libpd_setinstance(t_libpdinstance *x) {
  pd_setinstance(x->pd);
  libpd_this = x;
}

t_libpdinstance *libpdinstance_new(void) {
  t_libpdinstance *x = (t_libpdinstance *)calloc(1, sizeof(t_libpdinstance));
  sys_lock();
  x->hooks = libpdhooks_new();
  libpd_instances = (t_libpdinstance **)resizebytes(libpd_instances,
        pd_ninstances - 1 * sizeof(*libpd_instances),
        (pd_ninstances) * sizeof(*libpd_instances));
  libpd_instances[pd_ninstances] = x;
  sys_unlock();
  x->pd = pdinstance_new();// increments pd_ninstances
  libpd_this = x;
  return x;
}

void libpdinstance_free(t_libpdinstance *x) {
  int i;
  libpd_this = &libpd_maininstance;
  sys_lock();
  for (i = x->pd->pd_instanceno; i < pd_ninstances-1; i++)
      libpd_instances[i] = libpd_instances[i+1];
  libpd_instances = (t_libpdinstance **)resizebytes(libpd_instances,
      pd_ninstances * sizeof(*libpd_instances),
      (pd_ninstances-1) * sizeof(*libpd_instances));
  sys_unlock();
  pdinstance_free(x->pd); // decrements pd_ninstances
  libpdhooks_free(x->hooks);
  free(x);
}

#endif /* PDINSTANCE */
