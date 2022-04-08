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

t_libpdimp libpd_mainimp = {&libpd_mainhooks, NULL};

t_libpdimp* libpdimp_new(void) {
  t_libpdimp *imp = calloc(1, sizeof(t_libpdimp));
  imp->i_hooks = libpdhooks_new();
  return imp;
}

void libpdimp_free(t_libpdimp *imp) {
  if (imp == &libpd_mainimp) return;
  libpdhooks_free(imp->i_hooks);
  free(imp);
}
