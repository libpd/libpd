/*
 * Copyright (c) 2012 Peter Brinkmann (peter.brinkmann@gmail.com)
 *
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.
 */

#include "z_queued.h"

#include <alloca.h>
#include <stdlib.h>
#include <string.h>

#include "ringbuffer.h"

t_libpd_printhook libpd_queued_printhook = NULL;
t_libpd_banghook libpd_queued_banghook = NULL;
t_libpd_floathook libpd_queued_floathook = NULL;
t_libpd_symbolhook libpd_queued_symbolhook = NULL;
t_libpd_listhook libpd_queued_listhook = NULL;
t_libpd_messagehook libpd_queued_messagehook = NULL;

t_libpd_noteonhook libpd_queued_noteonhook = NULL;
t_libpd_controlchangehook libpd_queued_controlchangehook = NULL;
t_libpd_programchangehook libpd_queued_programchangehook = NULL;
t_libpd_pitchbendhook libpd_queued_pitchbendhook = NULL;
t_libpd_aftertouchhook libpd_queued_aftertouchhook = NULL;
t_libpd_polyaftertouchhook libpd_queued_polyaftertouchhook = NULL;
t_libpd_midibytehook libpd_queued_midibytehook = NULL;

typedef struct _params {
  enum {
    LIBPD_PRINT, LIBPD_BANG, LIBPD_FLOAT,
    LIBPD_SYMBOL, LIBPD_LIST, LIBPD_MESSAGE,
    LIBPD_NOTEON, LIBPD_CONTROLCHANGE, LIBPD_PROGRAMCHANGE, LIBPD_PITCHBEND,
    LIBPD_AFTERTOUCH, LIBPD_POLYAFTERTOUCH, LIBPD_MIDIBYTE
  } type;
  const char *src;
  float x;
  const char *sym;
  int argc;
  int midi1;
  int midi2;
  int midi3;
} params;

#define BUFFER_SIZE 32768
#define S_PARAMS sizeof(params)
#define S_ATOM sizeof(t_atom)

static ring_buffer *receive_buffer = NULL;
static char temp_buffer[BUFFER_SIZE];

static void receive_print(params *p, char **buffer) {
  if (libpd_queued_printhook) {
    libpd_queued_printhook(*buffer);
  }
  *buffer += p->argc;
}

static void receive_bang(params *p, char **buffer) {
  if (libpd_queued_banghook) {
    libpd_queued_banghook(p->src);
  }
}

static void receive_float(params *p, char **buffer) {
  if (libpd_queued_floathook) {
    libpd_queued_floathook(p->src, p->x);
  }
}

static void receive_symbol(params *p, char **buffer) {
  if (libpd_queued_symbolhook) {
    libpd_queued_symbolhook(p->src, p->sym);
  }
}

static void receive_list(params *p, char **buffer) {
  int n = p->argc * S_ATOM;
  if (libpd_queued_listhook) {
    t_atom *args = alloca(n);
    memcpy(args, *buffer, n);
    libpd_queued_listhook(p->src, p->argc, args);
  }
  *buffer += n; 
}

static void receive_message(params *p, char **buffer) {
  int n = p->argc * S_ATOM;
  if (libpd_queued_messagehook) {
    t_atom *args = alloca(n);
    memcpy(args, *buffer, n);
    libpd_queued_messagehook(p->src, p->sym, p->argc, args);
  }
  *buffer += n; 
}

static void internal_printhook(const char *s) {
  int len = strlen(s) + 1; // remember terminating null char
  if (rb_available_to_write(receive_buffer) >= S_PARAMS + len) {
    params p = {LIBPD_PRINT, NULL, 0.0f, NULL, len, 0, 0, 0};
    rb_write_to_buffer(receive_buffer, (const char *)&p, S_PARAMS);
    rb_write_to_buffer(receive_buffer, s, len);
  }
}

static void internal_banghook(const char *src) {
  if (rb_available_to_write(receive_buffer) >= S_PARAMS) {
    params p = {LIBPD_BANG, src, 0.0f, NULL, 0, 0, 0, 0};
    rb_write_to_buffer(receive_buffer, (const char *)&p, S_PARAMS);
  }
}

static void internal_floathook(const char *src, float x) {
  if (rb_available_to_write(receive_buffer) >= S_PARAMS) {
    params p = {LIBPD_FLOAT, src, x, NULL, 0, 0, 0, 0};
    rb_write_to_buffer(receive_buffer, (const char *)&p, S_PARAMS);
  }
}

static void internal_symbolhook(const char *src, const char *sym) {
  if (rb_available_to_write(receive_buffer) >= S_PARAMS) {
    params p = {LIBPD_SYMBOL, src, 0.0f, sym, 0, 0, 0, 0};
    rb_write_to_buffer(receive_buffer, (const char *)&p, S_PARAMS);
  }
}

static void internal_listhook(const char *src, int argc, t_atom *argv) {
  int n = argc * S_ATOM;
  if (rb_available_to_write(receive_buffer) >= S_PARAMS + n) {
    params p = {LIBPD_LIST, src, 0.0f, NULL, argc, 0, 0, 0};
    rb_write_to_buffer(receive_buffer, (const char *)&p, S_PARAMS);
    rb_write_to_buffer(receive_buffer, (const char *)argv, n);
  }
}

static void internal_messagehook(const char *src, const char* sym,
    int argc, t_atom *argv) {
  int n = argc * S_ATOM;
  if (rb_available_to_write(receive_buffer) >= S_PARAMS + n) {
    params p = {LIBPD_MESSAGE, src, 0.0f, sym, argc, 0, 0, 0};
    rb_write_to_buffer(receive_buffer, (const char *)&p, S_PARAMS);
    rb_write_to_buffer(receive_buffer, (const char *)argv, n);
  }
}

static void receive_noteon(params *p, char **buffer) {
  if (libpd_queued_noteonhook) {
    libpd_queued_noteonhook(p->midi1, p->midi2, p->midi3);
  }
}

static void receive_controlchange(params *p, char **buffer) {
  if (libpd_queued_controlchangehook) {
    libpd_queued_controlchangehook(p->midi1, p->midi2, p->midi3);
  }
}

static void receive_programchange(params *p, char **buffer) {
  if (libpd_queued_programchangehook) {
    libpd_queued_programchangehook(p->midi1, p->midi2);
  }
}

static void receive_pitchbend(params *p, char **buffer) {
  if (libpd_queued_pitchbendhook) {
    libpd_queued_pitchbendhook(p->midi1, p->midi2);
  }
}

static void receive_aftertouch(params *p, char **buffer) {
  if (libpd_queued_aftertouchhook) {
    libpd_queued_aftertouchhook(p->midi1, p->midi2);
  }
}

static void receive_polyaftertouch(params *p, char **buffer) {
  if (libpd_queued_polyaftertouchhook) {
    libpd_queued_polyaftertouchhook(p->midi1, p->midi2, p->midi3);
  }
}

static void receive_midibyte(params *p, char **buffer) {
  if (libpd_queued_midibytehook) {
    libpd_queued_midibytehook(p->midi1, p->midi2);
  }
}

static void internal_noteonhook(int channel, int pitch, int velocity) {
  if (rb_available_to_write(receive_buffer) >= S_PARAMS) {
    params p = {LIBPD_NOTEON, NULL, 0.0f, NULL, 0, channel, pitch, velocity};
    rb_write_to_buffer(receive_buffer, (const char *)&p, S_PARAMS);
  }
}

static void internal_controlchangehook(int channel, int controller, int value) {
  if (rb_available_to_write(receive_buffer) >= S_PARAMS) {
    params p = {LIBPD_CONTROLCHANGE, NULL, 0.0f, NULL, 0,
        channel, controller, value};
    rb_write_to_buffer(receive_buffer, (const char *)&p, S_PARAMS);
  }
}

static void internal_programchangehook(int channel, int value) {
  if (rb_available_to_write(receive_buffer) >= S_PARAMS) {
    params p = {LIBPD_PROGRAMCHANGE, NULL, 0.0f, NULL, 0, channel, value, 0};
    rb_write_to_buffer(receive_buffer, (const char *)&p, S_PARAMS);
  }
}

static void internal_pitchbendhook(int channel, int value) {
  if (rb_available_to_write(receive_buffer) >= S_PARAMS) {
    params p = {LIBPD_PITCHBEND, NULL, 0.0f, NULL, 0, channel, value, 0};
    rb_write_to_buffer(receive_buffer, (const char *)&p, S_PARAMS);
  }
}

static void internal_aftertouchhook(int channel, int value) {
  if (rb_available_to_write(receive_buffer) >= S_PARAMS) {
    params p = {LIBPD_AFTERTOUCH, NULL, 0.0f, NULL, 0, channel, value, 0};
    rb_write_to_buffer(receive_buffer, (const char *)&p, S_PARAMS);
  }
}

static void internal_polyaftertouchhook(int channel, int pitch, int value) {
  if (rb_available_to_write(receive_buffer) >= S_PARAMS) {
    params p = {LIBPD_POLYAFTERTOUCH, NULL, 0.0f, NULL, 0,
        channel, pitch, value};
    rb_write_to_buffer(receive_buffer, (const char *)&p, S_PARAMS);
  }
}

static void internal_midibytehook(int port, int byte) {
  if (rb_available_to_write(receive_buffer) >= S_PARAMS) {
    params p = {LIBPD_MIDIBYTE, NULL, 0.0f, NULL, 0, port, byte, 0};
    rb_write_to_buffer(receive_buffer, (const char *)&p, S_PARAMS);
  }
}

int libpd_queued_init() {
  receive_buffer = rb_create(BUFFER_SIZE);
  if (!receive_buffer) return -1;

  libpd_printhook = (t_libpd_printhook) internal_printhook;
  libpd_banghook = (t_libpd_banghook) internal_banghook;
  libpd_floathook = (t_libpd_floathook) internal_floathook;
  libpd_symbolhook = (t_libpd_symbolhook) internal_symbolhook;
  libpd_listhook = (t_libpd_listhook) internal_listhook;
  libpd_messagehook = (t_libpd_messagehook) internal_messagehook;

  libpd_noteonhook = (t_libpd_noteonhook) internal_noteonhook;
  libpd_controlchangehook =
      (t_libpd_controlchangehook) internal_controlchangehook;
  libpd_programchangehook =
      (t_libpd_programchangehook) internal_programchangehook;
  libpd_pitchbendhook = (t_libpd_pitchbendhook) internal_pitchbendhook;
  libpd_aftertouchhook = (t_libpd_aftertouchhook) internal_aftertouchhook;
  libpd_polyaftertouchhook =
      (t_libpd_polyaftertouchhook) internal_polyaftertouchhook;
  libpd_midibytehook = (t_libpd_midibytehook) internal_midibytehook;

  libpd_init();
  return 0;
}

void libpd_queued_release() {
  rb_free(receive_buffer);
}

void libpd_queued_receive() {
  size_t available = rb_available_to_read(receive_buffer);
  if (!available) return;
  rb_read_from_buffer(receive_buffer, temp_buffer, available);
  char *end = temp_buffer + available;
  char *buffer = temp_buffer;
  while (buffer < end) {
    params p;
    memcpy(&p, buffer, S_PARAMS);
    buffer += S_PARAMS;
    switch (p.type) {
      case LIBPD_PRINT: {
        receive_print(&p, &buffer);
        break;
      }
      case LIBPD_BANG: {
        receive_bang(&p, &buffer);
        break;
      }
      case LIBPD_FLOAT: {
        receive_float(&p, &buffer);
        break;
      }
      case LIBPD_SYMBOL: {
        receive_symbol(&p, &buffer);
        break;
      }
      case LIBPD_LIST: {
        receive_list(&p, &buffer);
        break;
      }
      case LIBPD_MESSAGE: {
        receive_message(&p, &buffer);
        break;
      }
      case LIBPD_NOTEON: {
        receive_noteon(&p, &buffer);
        break;
      }
      case LIBPD_CONTROLCHANGE: {
        receive_controlchange(&p, &buffer);
        break;
      }
      case LIBPD_PROGRAMCHANGE: {
        receive_programchange(&p, &buffer);
        break;
      }
      case LIBPD_PITCHBEND: {
        receive_pitchbend(&p, &buffer);
        break;
      }
      case LIBPD_AFTERTOUCH: {
        receive_aftertouch(&p, &buffer);
        break;
      }
      case LIBPD_POLYAFTERTOUCH: {
        receive_polyaftertouch(&p, &buffer);
        break;
      }
      case LIBPD_MIDIBYTE: {
        receive_midibyte(&p, &buffer);
        break;
      }
      default:
        break;
    }
  }
}
