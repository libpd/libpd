/*
 * Copyright (c) 2012 Tebjan Halm (tebjan@vvvv.org)
 *
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.
 *
 * See https://github.com/libpd/libpd/wiki for documentation
 *
 */

#include "z_hookset.h"

/* set hooks */

void libpd_set_printhook(const t_libpd_printhook hook){
	libpd_printhook = hook;
}

void libpd_set_banghook(const t_libpd_banghook hook){
	libpd_banghook = hook;
}

void libpd_set_floathook(const t_libpd_floathook hook){
	libpd_floathook = hook;
}

void libpd_set_symbolhook(const t_libpd_symbolhook hook){
	libpd_symbolhook = hook;
}

int libpd_atom_is_float(t_atom *a) {
  return libpd_is_float(*a);
}

int libpd_atom_is_symbol(t_atom *a) {
  return libpd_is_symbol(*a);
}

float libpd_atom_get_float(t_atom *a) {
  return libpd_get_float(*a);
}

char *libpd_atom_get_symbol(t_atom *a) {
  return libpd_get_symbol(*a);
}

