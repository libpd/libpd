/*
 * Basic Python bindings for libpd
 *
 * Copyright (c) 2010 Peter Brinkmann (peter.brinkmann@gmail.com)
 *
 * For information on usage and redistribution, and for a DISCLAIMER OF
 * ALL WARRANTIES, see the file, "LICENSE.txt," in this distribution.
 */

%module pylibpd

%include "carrays.i"
%array_class(float, float_array);

%{
#include "z_libpd.h"

static PyObject *convertArgs(const char *dest, const char* sym,
                              int n, t_atom *args) {
  int i = (sym) ? 2 : 1;
  n += i;
  PyObject *result = PyTuple_New(n);
  PyTuple_SetItem(result, 0, PyString_FromString(dest));
  if (sym) {
    PyTuple_SetItem(result, 1, PyString_FromString(sym));
  }
  int j;
  for (j = 0; i < n; i++, j++) {
    t_atom a = args[j];
    PyObject *x;
    if (a.a_type == A_FLOAT) {  
      x = PyFloat_FromDouble(a.a_w.w_float);
    } else if (a.a_type == A_SYMBOL) {  
      x = PyString_FromString(a.a_w.w_symbol->s_name);
    }
    PyTuple_SetItem(result, i, x);
  }
  return result;
}

static PyObject *print_callback = NULL;
static PyObject *bang_callback = NULL;
static PyObject *float_callback = NULL;
static PyObject *symbol_callback = NULL;
static PyObject *list_callback = NULL;
static PyObject *message_callback = NULL;

static PyObject *noteon_callback = NULL;
static PyObject *controlchange_callback = NULL;
static PyObject *programchange_callback = NULL;
static PyObject *pitchbend_callback = NULL;
static PyObject *aftertouch_callback = NULL;
static PyObject *polyaftertouch_callback = NULL;

#define SET_CALLBACK(s) \
static int libpd_set_##s##_callback(PyObject *callback) { \
  if (PyCallable_Check(callback)) { \
    Py_XDECREF(s##_callback); \
    s##_callback = callback; \
    Py_INCREF(s##_callback); \
    return 0; \
  } else { \
    return -1; \
  } \
}

SET_CALLBACK(print)
SET_CALLBACK(bang)
SET_CALLBACK(float)
SET_CALLBACK(symbol)
SET_CALLBACK(list)
SET_CALLBACK(message)

SET_CALLBACK(noteon)
SET_CALLBACK(controlchange)
SET_CALLBACK(programchange)
SET_CALLBACK(pitchbend)
SET_CALLBACK(aftertouch)
SET_CALLBACK(polyaftertouch)

#define FINISH_CALLBACK(callback) \
  PyObject *result = PyObject_CallObject(callback, args); \
  Py_XDECREF(result); \
  Py_XDECREF(args);

static void pylibpd_print(const char *s) {
  if (print_callback) {
    PyObject *args = Py_BuildValue("(s)", s);
    FINISH_CALLBACK(print_callback)
  }
}

static void pylibpd_bang(const char *dest) {
  if (bang_callback) {
    PyObject *args = Py_BuildValue("(s)", dest);
    FINISH_CALLBACK(bang_callback)
  }
}

static void pylibpd_float(const char *dest, float val) {
  if (float_callback) {
    PyObject *args = Py_BuildValue("(sf)", dest, val);
    FINISH_CALLBACK(float_callback)
  }
}

static void pylibpd_symbol(const char *dest, const char *sym) {
  if (symbol_callback) {
    PyObject *args = Py_BuildValue("(ss)", dest, sym);
    FINISH_CALLBACK(symbol_callback)
  }
}

static void pylibpd_list(const char *dest, int n, t_atom *pd_args) {
  if (list_callback) {
    PyObject *args = convertArgs(dest, NULL, n, pd_args);
    FINISH_CALLBACK(list_callback)
  }
}

static void pylibpd_message(const char *dest, const char *sym,
                 int n, t_atom *pd_args) {
  if (message_callback) {
    PyObject *args = convertArgs(dest, sym, n, pd_args);
    FINISH_CALLBACK(message_callback)
  }
}

static void pylibpd_noteon(int ch, int n, int v) {
  if (noteon_callback) {
    PyObject *args = Py_BuildValue("(iii)", ch, n, v);
    FINISH_CALLBACK(noteon_callback)
  }
}

static void pylibpd_controlchange(int ch, int c, int v) {
  if (controlchange_callback) {
    PyObject *args = Py_BuildValue("(iii)", ch, c, v);
    FINISH_CALLBACK(controlchange_callback)
  }
}

static void pylibpd_programchange(int ch, int pgm) {
  if (programchange_callback) {
    PyObject *args = Py_BuildValue("(ii)", ch, pgm);
    FINISH_CALLBACK(programchange_callback)
  }
}

static void pylibpd_pitchbend(int ch, int bend) {
  if (pitchbend_callback) {
    PyObject *args = Py_BuildValue("(ii)", ch, bend);
    FINISH_CALLBACK(pitchbend_callback)
  }
}

static void pylibpd_aftertouch(int ch, int v) {
  if (aftertouch_callback) {
    PyObject *args = Py_BuildValue("(ii)", ch, v);
    FINISH_CALLBACK(aftertouch_callback)
  }
}

static void pylibpd_polyaftertouch(int ch, int n, int v) {
  if (polyaftertouch_callback) {
    PyObject *args = Py_BuildValue("(iii)", ch, n, v);
    FINISH_CALLBACK(polyaftertouch_callback)
  }
}

%}

#define REGISTER_CALLBACK_SETTER(s) \
  int libpd_set_##s##_callback(PyObject *callback);

REGISTER_CALLBACK_SETTER(print)
REGISTER_CALLBACK_SETTER(bang)
REGISTER_CALLBACK_SETTER(float)
REGISTER_CALLBACK_SETTER(symbol)
REGISTER_CALLBACK_SETTER(list)
REGISTER_CALLBACK_SETTER(message)

REGISTER_CALLBACK_SETTER(noteon)
REGISTER_CALLBACK_SETTER(controlchange)
REGISTER_CALLBACK_SETTER(programchange)
REGISTER_CALLBACK_SETTER(pitchbend)
REGISTER_CALLBACK_SETTER(aftertouch)
REGISTER_CALLBACK_SETTER(polyaftertouch)

void libpd_clear_search_path();
void libpd_add_to_search_path(const char *s);

int libpd_blocksize();
int libpd_init_audio(int, int, int, int);
int libpd_process_raw(float *, float *);
int libpd_process_float(float *, float *);

int libpd_bang(const char *);
int libpd_float(const char *, float);
int libpd_symbol(const char *, const char *);
int libpd_start_message();
void libpd_add_float(float);
void libpd_add_symbol(const char *);
int libpd_finish_list(const char *);
int libpd_finish_message(const char *, const char *);

int libpd_exists(const char *);
void *libpd_bind(const char *);
void libpd_unbind(void *p);

int libpd_noteon(int, int, int);
int libpd_controlchange(int, int, int);
int libpd_programchange(int, int);
int libpd_pitchbend(int, int);
int libpd_aftertouch(int, int);
int libpd_polyaftertouch(int, int, int);

%pythoncode %{
def __process_args(args):
  n = libpd_start_message();
  if (len(args) > n): return -1
  for arg in args:
      if isinstance(arg, str):
        libpd_add_symbol(arg)
      else:
        if isinstance(arg, int) or isinstance(arg, float):
          libpd_add_float(arg)
        else:
          return -1
  return 0

def libpd_list(dest, *args):
  return __process_args(args) or libpd_finish_list(dest)

def libpd_message(dest, sym, *args):
  return __process_args(args) or libpd_finish_message(dest, sym)

%}

%init %{
libpd_printhook = pylibpd_print;
libpd_banghook = pylibpd_bang;
libpd_floathook = pylibpd_float;
libpd_symbolhook = pylibpd_symbol;
libpd_listhook = pylibpd_list;
libpd_messagehook = pylibpd_message;

libpd_noteonhook = pylibpd_noteon;
libpd_controlchangehook = pylibpd_controlchange;
libpd_programchangehook = pylibpd_programchange;
libpd_pitchbendhook= pylibpd_pitchbend;
libpd_aftertouchhook = pylibpd_aftertouch;
libpd_polyaftertouchhook = pylibpd_polyaftertouch;

libpd_init();
%}

