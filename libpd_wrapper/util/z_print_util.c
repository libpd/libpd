/*
 * Copyright (c) 2013 Dan Wilcox (danomatika@gmail.com) &
 *                    Peter Brinkmann (peter.brinkmann@gmail.com)
 *
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.
 *
 * See https://github.com/libpd/libpd/wiki for documentation
 *
 */

#include "z_print_util.h"

#include <stdlib.h>
#include <string.h>
#include "z_hooks.h"

#define PRINT_LINE_SIZE 2048

typedef struct _libpd_concatenated_print_stuff {
  char c_concatenated_printbuffer[PRINT_LINE_SIZE];
  int c_concatenated_printbuflen;
  t_libpd_printhook c_concatenated_printhook;
} t_libpd_concatenated_print_stuff;


#define CONCAT_STUFF ((t_libpd_concatenated_print_stuff *)(libpdimp_this()->i_concat_stuff))
#define CONCAT_BUFFER (CONCAT_STUFF->c_concatenated_printbuffer)
#define CONCAT_BUFLEN (CONCAT_STUFF->c_concatenated_printbuflen)
#define CONCAT_HOOK (CONCAT_STUFF->c_concatenated_printhook)

void libpd_concatenated_stuff_new(void) {
  if(CONCAT_STUFF) return; /* already allocated */
  libpdimp_this()->i_concat_stuff = calloc(1, sizeof(t_libpd_concatenated_print_stuff));
}

void libpd_concatenated_stuff_free(void) {
  if(CONCAT_STUFF) free(CONCAT_STUFF);
}

void libpd_set_concatenated_printhook(const t_libpd_printhook hook) {
  libpd_concatenated_stuff_new();
  if (CONCAT_STUFF) {
    CONCAT_HOOK = hook;
    CONCAT_BUFLEN = 0;
    libpd_set_printhook(libpd_print_concatenator);
  }
}

void libpd_print_concatenator(const char *s) {
  if (!CONCAT_STUFF || !CONCAT_HOOK) return;

  CONCAT_BUFFER[CONCAT_BUFLEN] = '\0';
  int len = (int) strlen(s);
  while (CONCAT_BUFLEN + len >= PRINT_LINE_SIZE) {
    int d = PRINT_LINE_SIZE - 1 - CONCAT_BUFLEN;
    strncat(CONCAT_BUFFER, s, d);
    CONCAT_HOOK(CONCAT_BUFFER);
    s += d;
    len -= d;
    CONCAT_BUFLEN = 0;
    CONCAT_BUFFER[0] = '\0';
  }

  strncat(CONCAT_BUFFER, s, len);
  CONCAT_BUFLEN += len;

  if (CONCAT_BUFLEN > 0 && CONCAT_BUFFER[CONCAT_BUFLEN - 1] == '\n') {
    CONCAT_BUFFER[CONCAT_BUFLEN - 1] = '\0';
    CONCAT_HOOK(CONCAT_BUFFER);
    CONCAT_BUFLEN = 0;
  }
}
