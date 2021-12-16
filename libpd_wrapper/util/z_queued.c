/*
 * Copyright (c) 2012 Peter Brinkmann (peter.brinkmann@gmail.com)
 *
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.
 *
 * See https://github.com/libpd/libpd/wiki for documentation
 *
 */

#include "z_queued.h"

#include <stdlib.h>
#include <string.h>
#include "z_hooks.h"
#include "ringbuffer.h"
#include "z_print_util.h"

#define BUFFER_SIZE 16384

typedef struct _queued_stuff {
  t_libpd_printhook q_printhook;
  t_libpd_banghook q_banghook;
  t_libpd_floathook q_floathook;
  t_libpd_symbolhook q_symbolhook;
  t_libpd_listhook q_listhook;
  t_libpd_messagehook q_messagehook;

  t_libpd_noteonhook q_noteonhook;
  t_libpd_controlchangehook q_controlchangehook;
  t_libpd_programchangehook q_programchangehook;
  t_libpd_pitchbendhook q_pitchbendhook;
  t_libpd_aftertouchhook q_aftertouchhook;
  t_libpd_polyaftertouchhook q_polyaftertouchhook;
  t_libpd_midibytehook q_midibytehook;

  ring_buffer *pd_receive_buffer;
  ring_buffer *midi_receive_buffer;
  char temp_buffer[BUFFER_SIZE];
} t_queued_stuff;

typedef struct _pd_params {
  enum {
    LIBPD_PRINT, LIBPD_BANG, LIBPD_FLOAT,
    LIBPD_SYMBOL, LIBPD_LIST, LIBPD_MESSAGE,
  } type;
  const char *src;
  float x;
  const char *sym;
  int argc;
} pd_params;

typedef struct _midi_params {
  enum {
    LIBPD_NOTEON, LIBPD_CONTROLCHANGE, LIBPD_PROGRAMCHANGE, LIBPD_PITCHBEND,
    LIBPD_AFTERTOUCH, LIBPD_POLYAFTERTOUCH, LIBPD_MIDIBYTE
  } type;
  int midi1;
  int midi2;
  int midi3;
} midi_params;

#define S_PD_PARAMS sizeof(pd_params)
#define S_MIDI_PARAMS sizeof(midi_params)
#define S_ATOM sizeof(t_atom)

#define QUEUED_STUFF ((t_queued_stuff *)(libpdimp_this()->i_queued_stuff))

static void receive_print(pd_params *p, char **buffer) {
  if (QUEUED_STUFF->q_printhook) {
    QUEUED_STUFF->q_printhook(*buffer);
  }
  *buffer += p->argc;
}

static void receive_bang(pd_params *p, char **buffer) {
  if (QUEUED_STUFF->q_banghook) {
    QUEUED_STUFF->q_banghook(p->src);
  }
}

static void receive_float(pd_params *p, char **buffer) {
  if (QUEUED_STUFF->q_floathook) {
    QUEUED_STUFF->q_floathook(p->src, p->x);
  }
}

static void receive_symbol(pd_params *p, char **buffer) {
  if (QUEUED_STUFF->q_symbolhook) {
    QUEUED_STUFF->q_symbolhook(p->src, p->sym);
  }
}

static void receive_list(pd_params *p, char **buffer) {
  if (QUEUED_STUFF->q_listhook) {
    QUEUED_STUFF->q_listhook(p->src, p->argc, (t_atom *) *buffer);
  }
  *buffer += p->argc * S_ATOM;
}

static void receive_message(pd_params *p, char **buffer) {
  if (QUEUED_STUFF->q_messagehook) {
    QUEUED_STUFF->q_messagehook(p->src, p->sym, p->argc, (t_atom *) *buffer);
  }
  *buffer += p->argc * S_ATOM;
}

#define LIBPD_WORD_ALIGN 8

static void internal_printhook(const char *s) {
  static char padding[LIBPD_WORD_ALIGN];
  int len = (int) strlen(s) + 1; // remember terminating null char
  int rest = len % LIBPD_WORD_ALIGN;
  if (rest) rest = LIBPD_WORD_ALIGN - rest;
  int total = len + rest;
  if (rb_available_to_write(QUEUED_STUFF->pd_receive_buffer) >= S_PD_PARAMS + total) {
    pd_params p = {LIBPD_PRINT, NULL, 0.0f, NULL, total};
    rb_write_to_buffer(QUEUED_STUFF->pd_receive_buffer, 3,
        (const char *)&p, S_PD_PARAMS, s, len, padding, rest);
  }
}

static void internal_banghook(const char *src) {
  if (rb_available_to_write(QUEUED_STUFF->pd_receive_buffer) >= S_PD_PARAMS) {
    pd_params p = {LIBPD_BANG, src, 0.0f, NULL, 0};
    rb_write_to_buffer(QUEUED_STUFF->pd_receive_buffer, 1, (const char *)&p, S_PD_PARAMS);
  }
}

static void internal_floathook(const char *src, float x) {
  if (rb_available_to_write(QUEUED_STUFF->pd_receive_buffer) >= S_PD_PARAMS) {
    pd_params p = {LIBPD_FLOAT, src, x, NULL, 0};
    rb_write_to_buffer(QUEUED_STUFF->pd_receive_buffer, 1, (const char *)&p, S_PD_PARAMS);
  }
}

static void internal_symbolhook(const char *src, const char *sym) {
  if (rb_available_to_write(QUEUED_STUFF->pd_receive_buffer) >= S_PD_PARAMS) {
    pd_params p = {LIBPD_SYMBOL, src, 0.0f, sym, 0};
    rb_write_to_buffer(QUEUED_STUFF->pd_receive_buffer, 1, (const char *)&p, S_PD_PARAMS);
  }
}

static void internal_listhook(const char *src, int argc, t_atom *argv) {
  int n = argc * S_ATOM;
  if (rb_available_to_write(QUEUED_STUFF->pd_receive_buffer) >= S_PD_PARAMS + n) {
    pd_params p = {LIBPD_LIST, src, 0.0f, NULL, argc};
    rb_write_to_buffer(QUEUED_STUFF->pd_receive_buffer, 2,
        (const char *)&p, S_PD_PARAMS, (const char *)argv, n);
  }
}

static void internal_messagehook(const char *src, const char* sym,
    int argc, t_atom *argv) {
  int n = argc * S_ATOM;
  if (rb_available_to_write(QUEUED_STUFF->pd_receive_buffer) >= S_PD_PARAMS + n) {
    pd_params p = {LIBPD_MESSAGE, src, 0.0f, sym, argc};
    rb_write_to_buffer(QUEUED_STUFF->pd_receive_buffer, 2,
        (const char *)&p, S_PD_PARAMS, (const char *)argv, n);
  }
}

static void receive_noteon(midi_params *p, char **buffer) {
  if (QUEUED_STUFF->q_noteonhook) {
    QUEUED_STUFF->q_noteonhook(p->midi1, p->midi2, p->midi3);
  }
}

static void receive_controlchange(midi_params *p, char **buffer) {
  if (QUEUED_STUFF->q_controlchangehook) {
    QUEUED_STUFF->q_controlchangehook(p->midi1, p->midi2, p->midi3);
  }
}

static void receive_programchange(midi_params *p, char **buffer) {
  if (QUEUED_STUFF->q_programchangehook) {
    QUEUED_STUFF->q_programchangehook(p->midi1, p->midi2);
  }
}

static void receive_pitchbend(midi_params *p, char **buffer) {
  if (QUEUED_STUFF->q_pitchbendhook) {
    QUEUED_STUFF->q_pitchbendhook(p->midi1, p->midi2);
  }
}

static void receive_aftertouch(midi_params *p, char **buffer) {
  if (QUEUED_STUFF->q_aftertouchhook) {
    QUEUED_STUFF->q_aftertouchhook(p->midi1, p->midi2);
  }
}

static void receive_polyaftertouch(midi_params *p, char **buffer) {
  if (QUEUED_STUFF->q_polyaftertouchhook) {
    QUEUED_STUFF->q_polyaftertouchhook(p->midi1, p->midi2, p->midi3);
  }
}

static void receive_midibyte(midi_params *p, char **buffer) {
  if (QUEUED_STUFF->q_midibytehook) {
    QUEUED_STUFF->q_midibytehook(p->midi1, p->midi2);
  }
}

static void internal_noteonhook(int channel, int pitch, int velocity) {
  if (rb_available_to_write(QUEUED_STUFF->midi_receive_buffer) >= S_MIDI_PARAMS) {
    midi_params p = {LIBPD_NOTEON, channel, pitch, velocity};
    rb_write_to_buffer(QUEUED_STUFF->midi_receive_buffer, 1, (const char *)&p, S_MIDI_PARAMS);
  }
}

static void internal_controlchangehook(int channel, int controller, int value) {
  if (rb_available_to_write(QUEUED_STUFF->midi_receive_buffer) >= S_MIDI_PARAMS) {
    midi_params p = {LIBPD_CONTROLCHANGE, channel, controller, value};
    rb_write_to_buffer(QUEUED_STUFF->midi_receive_buffer, 1, (const char *)&p, S_MIDI_PARAMS);
  }
}

static void internal_programchangehook(int channel, int value) {
  if (rb_available_to_write(QUEUED_STUFF->midi_receive_buffer) >= S_MIDI_PARAMS) {
    midi_params p = {LIBPD_PROGRAMCHANGE, channel, value, 0};
    rb_write_to_buffer(QUEUED_STUFF->midi_receive_buffer, 1, (const char *)&p, S_MIDI_PARAMS);
  }
}

static void internal_pitchbendhook(int channel, int value) {
  if (rb_available_to_write(QUEUED_STUFF->midi_receive_buffer) >= S_MIDI_PARAMS) {
    midi_params p = {LIBPD_PITCHBEND, channel, value, 0};
    rb_write_to_buffer(QUEUED_STUFF->midi_receive_buffer, 1, (const char *)&p, S_MIDI_PARAMS);
  }
}

static void internal_aftertouchhook(int channel, int value) {
  if (rb_available_to_write(QUEUED_STUFF->midi_receive_buffer) >= S_MIDI_PARAMS) {
    midi_params p = {LIBPD_AFTERTOUCH, channel, value, 0};
    rb_write_to_buffer(QUEUED_STUFF->midi_receive_buffer, 1, (const char *)&p, S_MIDI_PARAMS);
  }
}

static void internal_polyaftertouchhook(int channel, int pitch, int value) {
  if (rb_available_to_write(QUEUED_STUFF->midi_receive_buffer) >= S_MIDI_PARAMS) {
    midi_params p = {LIBPD_POLYAFTERTOUCH, channel, pitch, value};
    rb_write_to_buffer(QUEUED_STUFF->midi_receive_buffer, 1, (const char *)&p, S_MIDI_PARAMS);
  }
}

static void internal_midibytehook(int port, int byte) {
  if (rb_available_to_write(QUEUED_STUFF->midi_receive_buffer) >= S_MIDI_PARAMS) {
    midi_params p = {LIBPD_MIDIBYTE, port, byte, 0};
    rb_write_to_buffer(QUEUED_STUFF->midi_receive_buffer, 1, (const char *)&p, S_MIDI_PARAMS);
  }
}

void libpd_queued_stuff_new(void);

void libpd_set_queued_printhook(const t_libpd_printhook hook) {
  if (!QUEUED_STUFF) libpd_queued_stuff_new();
  QUEUED_STUFF->q_printhook = hook;
}

extern void libpd_print_concatenator(const char *s);
void libpd_set_concatenated_queued_printhook(const t_libpd_printhook hook) {
  if (!QUEUED_STUFF) libpd_queued_stuff_new();
  libpd_set_concatenated_printhook(hook);
  QUEUED_STUFF->q_printhook = libpd_print_concatenator;
}

void libpd_set_queued_banghook(const t_libpd_banghook hook) {
  if (!QUEUED_STUFF) libpd_queued_stuff_new();
  QUEUED_STUFF->q_banghook = hook;
}

void libpd_set_queued_floathook(const t_libpd_floathook hook) {
  if (!QUEUED_STUFF) libpd_queued_stuff_new();
  QUEUED_STUFF->q_floathook = hook;
}

void libpd_set_queued_symbolhook(const t_libpd_symbolhook hook) {
  if (!QUEUED_STUFF) libpd_queued_stuff_new();
  QUEUED_STUFF->q_symbolhook = hook;
}

void libpd_set_queued_listhook(const t_libpd_listhook hook) {
  if (!QUEUED_STUFF) libpd_queued_stuff_new();
  QUEUED_STUFF->q_listhook = hook;
}

void libpd_set_queued_messagehook(const t_libpd_messagehook hook) {
  if (!QUEUED_STUFF) libpd_queued_stuff_new();
  QUEUED_STUFF->q_messagehook = hook;
}

void libpd_set_queued_noteonhook(const t_libpd_noteonhook hook) {
  if (!QUEUED_STUFF) libpd_queued_stuff_new();
  QUEUED_STUFF->q_noteonhook = hook;
}

void libpd_set_queued_controlchangehook(const t_libpd_controlchangehook hook) {
  if (!QUEUED_STUFF) libpd_queued_stuff_new();
  QUEUED_STUFF->q_controlchangehook = hook;
}

void libpd_set_queued_programchangehook(const t_libpd_programchangehook hook) {
  if (!QUEUED_STUFF) libpd_queued_stuff_new();
  QUEUED_STUFF->q_programchangehook = hook;
}

void libpd_set_queued_pitchbendhook(const t_libpd_pitchbendhook hook) {
  if (!QUEUED_STUFF) libpd_queued_stuff_new();
  QUEUED_STUFF->q_pitchbendhook = hook;
}

void libpd_set_queued_aftertouchhook(const t_libpd_aftertouchhook hook) {
  if (!QUEUED_STUFF) libpd_queued_stuff_new();
  QUEUED_STUFF->q_aftertouchhook = hook;
}

void libpd_set_queued_polyaftertouchhook(const t_libpd_polyaftertouchhook hook) {
  if (!QUEUED_STUFF) libpd_queued_stuff_new();
  QUEUED_STUFF->q_polyaftertouchhook = hook;
}

void libpd_set_queued_midibytehook(const t_libpd_midibytehook hook) {
  if (!QUEUED_STUFF) libpd_queued_stuff_new();
  QUEUED_STUFF->q_midibytehook = hook;
}

void libpd_queued_stuff_new() {
  if (QUEUED_STUFF) return; /* already allocated */
  libpdimp_this()->i_queued_stuff = calloc(1, sizeof(t_queued_stuff));
  if (!QUEUED_STUFF) return;

  QUEUED_STUFF->pd_receive_buffer = rb_create(BUFFER_SIZE);
  if (!QUEUED_STUFF->pd_receive_buffer) {
    free(QUEUED_STUFF);
    libpdimp_this()->i_queued_stuff = NULL;
    return;
  }

  QUEUED_STUFF->midi_receive_buffer = rb_create(BUFFER_SIZE);
  if (!QUEUED_STUFF->midi_receive_buffer) {
    free(QUEUED_STUFF);
    libpdimp_this()->i_queued_stuff = NULL;
    return;
  }

  libpd_set_printhook(internal_printhook);
  libpd_set_banghook(internal_banghook);
  libpd_set_floathook(internal_floathook);
  libpd_set_symbolhook(internal_symbolhook);
  libpd_set_listhook(internal_listhook);
  libpd_set_messagehook(internal_messagehook);

  libpd_set_noteonhook(internal_noteonhook);
  libpd_set_controlchangehook(internal_controlchangehook);
  libpd_set_programchangehook(internal_programchangehook);
  libpd_set_pitchbendhook(internal_pitchbendhook);
  libpd_set_aftertouchhook(internal_aftertouchhook);
  libpd_set_polyaftertouchhook(internal_polyaftertouchhook);
  libpd_set_midibytehook(internal_midibytehook);
}

void libpd_queued_stuff_free() {
  if (!QUEUED_STUFF) return; /* was not allocated */
  rb_free(QUEUED_STUFF->pd_receive_buffer);
  rb_free(QUEUED_STUFF->midi_receive_buffer);
  free(QUEUED_STUFF);
  libpdimp_this()->i_queued_stuff = NULL;
}

int libpd_queued_init() {
  return libpd_init();
}

void libpd_queued_release() {}

void libpd_queued_receive_pd_messages() {
  size_t available = rb_available_to_read(QUEUED_STUFF->pd_receive_buffer);
  if (!available) return;
  rb_read_from_buffer(QUEUED_STUFF->pd_receive_buffer, QUEUED_STUFF->temp_buffer, (int) available);
  char *end = QUEUED_STUFF->temp_buffer + available;
  char *buffer = QUEUED_STUFF->temp_buffer;
  while (buffer < end) {
    pd_params *p = (pd_params *)buffer;
    buffer += S_PD_PARAMS;
    switch (p->type) {
      case LIBPD_PRINT: {
        receive_print(p, &buffer);
        break;
      }
      case LIBPD_BANG: {
        receive_bang(p, &buffer);
        break;
      }
      case LIBPD_FLOAT: {
        receive_float(p, &buffer);
        break;
      }
      case LIBPD_SYMBOL: {
        receive_symbol(p, &buffer);
        break;
      }
      case LIBPD_LIST: {
        receive_list(p, &buffer);
        break;
      }
      case LIBPD_MESSAGE: {
        receive_message(p, &buffer);
        break;
      }
      default:
        break;
    }
  }
}

void libpd_queued_receive_midi_messages() {
  size_t available = rb_available_to_read(QUEUED_STUFF->midi_receive_buffer);
  if (!available) return;
  rb_read_from_buffer(QUEUED_STUFF->midi_receive_buffer, QUEUED_STUFF->temp_buffer, (int) available);
  char *end = QUEUED_STUFF->temp_buffer + available;
  char *buffer = QUEUED_STUFF->temp_buffer;
  while (buffer < end) {
    midi_params *p = (midi_params *)buffer;
    buffer += S_MIDI_PARAMS;
    switch (p->type) {
      case LIBPD_NOTEON: {
        receive_noteon(p, &buffer);
        break;
      }
      case LIBPD_CONTROLCHANGE: {
        receive_controlchange(p, &buffer);
        break;
      }
      case LIBPD_PROGRAMCHANGE: {
        receive_programchange(p, &buffer);
        break;
      }
      case LIBPD_PITCHBEND: {
        receive_pitchbend(p, &buffer);
        break;
      }
      case LIBPD_AFTERTOUCH: {
        receive_aftertouch(p, &buffer);
        break;
      }
      case LIBPD_POLYAFTERTOUCH: {
        receive_polyaftertouch(p, &buffer);
        break;
      }
      case LIBPD_MIDIBYTE: {
        receive_midibyte(p, &buffer);
        break;
      }
      default:
        break;
    }
  }
}
