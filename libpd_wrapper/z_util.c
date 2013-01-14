/*
 * Copyright (c) 2012 Dan Wilcox (danomatika@gmail.com)
 *
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.
 */

#include "z_queued.h"

#include <stdlib.h>
#include <string.h>

t_libpd_printhook libpd_concatenated_printhook = NULL;
t_libpd_printhook libpd_queued_concatenated_printhook = NULL;

#define PRINT_LINE_SIZE 2048

static char *concatenated_print_line = NULL;

static void internal_printhook(const char *s) {
  int len = strlen(s);
  int len_line = strlen(concatenated_print_line);
  
  if (len > 0 && s[len-1] == '\n') { // send message when an endline is detected
	if (len + len_line >= PRINT_LINE_SIZE-1) { // flush if we're at the limit
	  if (libpd_queued_concatenated_printhook) {
	    libpd_queued_concatenated_printhook(concatenated_print_line);
	  }
	  else if (libpd_concatenated_printhook) {
	    libpd_concatenated_printhook(concatenated_print_line);
	  }
	  concatenated_print_line[0] = '\0';
	}
	strncat(concatenated_print_line, s, len-1); // don't copy '\n'
    
    if (libpd_queued_concatenated_printhook) { // send the message
	  libpd_queued_concatenated_printhook(concatenated_print_line);
	}
    else if (libpd_concatenated_printhook) {
      libpd_concatenated_printhook(concatenated_print_line);
    }
	concatenated_print_line[0] = '\0';
    return;
  }
  
  strncat(concatenated_print_line, s, len); // build the message
}

int libpd_concatenate_print_messages(void) {
  concatenated_print_line = malloc(PRINT_LINE_SIZE);
  concatenated_print_line[0] = '\0';
  if (!concatenated_print_line) return -1;
  if (libpd_queued_concatenated_printhook) {
	libpd_queued_printhook = (t_libpd_printhook) internal_printhook;
  }
  else if (libpd_concatenated_printhook) {
	libpd_printhook = (t_libpd_printhook) internal_printhook;
  }
  return 0;
}

void libpd_segment_print_messages(void) {
  if (libpd_queued_printhook == internal_printhook) {
	libpd_queued_printhook = NULL;
  }
  if (libpd_printhook == internal_printhook) {
	libpd_printhook = NULL;
  }
  if (concatenated_print_line) {
    free(concatenated_print_line);
    concatenated_print_line = NULL;
  }
}
