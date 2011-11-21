/*
 * Basic Python bindings for libpd
 *
 * Copyright (c) 2010 Peter Brinkmann (peter.brinkmann@gmail.com)
 *
 * For information on usage and redistribution, and for a DISCLAIMER OF
 * ALL WARRANTIES, see the file, "LICENSE.txt," in this distribution.
 */

%module pylibpd

void libpd_clear_search_path();
void libpd_add_to_search_path(const char *dir);

int libpd_blocksize();
int libpd_init_audio(int inch, int outch, int srate);

#define TYPEMAPS(t) \
%typemap(in) t *inb { \
  Py_ssize_t dummy; \
  if (PyObject_AsReadBuffer($input, (const void **)&$1, &dummy)) return NULL; \
} \
%typemap(in) t *outb { \
  Py_ssize_t dummy; \
  if (PyObject_AsWriteBuffer($input, (void **)&$1, &dummy)) return NULL; \
}
TYPEMAPS(float)
TYPEMAPS(short)
TYPEMAPS(double)
int libpd_process_raw(float *inb, float *outb);
int libpd_process_float(int ticks, float *inb, float *outb);
int libpd_process_short(int ticks, short *inb, short *outb);
int libpd_process_double(int ticks, double *inb, double *outb);

int libpd_arraysize(const char *name);
int libpd_read_array(float *outb, const char *src, int offset, int n);
int libpd_write_array(const char *dest, int offset, float *inb, int n);

int libpd_bang(const char *dest);
int libpd_float(const char *dest, float val);
int libpd_symbol(const char *dest, const char *sym);

%rename(__libpd_start_message) libpd_start_message;
%rename(__libpd_add_float) libpd_add_float;
%rename(__libpd_add_symbol) libpd_add_symbol;
%rename(__libpd_finish_list) libpd_finish_list;
%rename(__libpd_finish_message) libpd_finish_message;
int libpd_start_message(int);
void libpd_add_float(float);
void libpd_add_symbol(const char *);
int libpd_finish_list(const char *);
int libpd_finish_message(const char *, const char *);

int libpd_exists(const char *sym);
%rename(__libpd_bind) libpd_bind;
%rename(__libpd_unbind) libpd_unbind;
void *libpd_bind(const char *sym);
void libpd_unbind(void *p);

%rename(__libpd_openfile) libpd_openfile;
%rename(__libpd_closefile) libpd_closefile;
%rename(__libpd_getdollarzero) libpd_getdollarzero;
void *libpd_openfile(const char *, const char *);
void libpd_closefile(void *);
int libpd_getdollarzero(void *);

int libpd_noteon(int ch, int n, int v);
int libpd_controlchange(int ch, int n, int v);
int libpd_programchange(int ch, int p);
int libpd_pitchbend(int ch, int b);
int libpd_aftertouch(int ch, int v);
int libpd_polyaftertouch(int ch, int n, int v);
int libpd_midibyte(int p, int b);
int libpd_sysex(int p, int b);
int libpd_sysrealtime(int p, int b);

#define SET_CALLBACK(s) \
  int libpd_set_##s##_callback(PyObject *callback);

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
SET_CALLBACK(midibyte)

%pythoncode %{
import array

def __process_args(args):
  if __libpd_start_message(len(args)): return -2
  for arg in args:
      if isinstance(arg, str):
        __libpd_add_symbol(arg)
      else:
        if isinstance(arg, int) or isinstance(arg, float):
          __libpd_add_float(arg)
        else:
          return -1
  return 0

def libpd_list(dest, *args):
  return __process_args(args) or __libpd_finish_list(dest)

def libpd_message(dest, sym, *args):
  return __process_args(args) or __libpd_finish_message(dest, sym)

__libpd_patches = {}

def libpd_open_patch(patch, dir = '.'):
  ptr = __libpd_openfile(patch, dir)
  if not ptr:
    raise IOError("unable to open patch: %s/%s" % (dir, patch))
  dz = __libpd_getdollarzero(ptr)
  __libpd_patches[dz] = ptr
  return dz

def libpd_close_patch(dz):
  __libpd_closefile(__libpd_patches[dz])
  del __libpd_patches[dz]

__libpd_subscriptions = {}

def libpd_subscribe(sym):
  if not __libpd_subscriptions.has_key(sym):
    __libpd_subscriptions[sym] = __libpd_bind(sym)

def libpd_unsubscribe(sym):
  __libpd_unbind(__libpd_subscriptions[sym])
  del __libpd_subscriptions[sym]

def libpd_compute_audio(flag):
  libpd_message('pd', 'dsp', flag)

def libpd_release():
  for p in __libpd_patches.values():
    __libpd_closefile(p)
  __libpd_patches.clear()
  for p in __libpd_subscriptions.values():
    __libpd_unbind(p)
  __libpd_subscriptions.clear()

class PdManager:
  def __init__(self, inch, outch, srate, ticks):
    self.__ticks = ticks
    self.__outbuf = array.array('h', '\x00\x00' * outch * libpd_blocksize())
    libpd_compute_audio(1)
    libpd_init_audio(inch, outch, srate)
  def process(self, inbuf):
    libpd_process_short(self.__ticks, inbuf, self.__outbuf)
    return self.__outbuf
%}

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
    if (libpd_is_float(a)) {
      x = PyFloat_FromDouble(libpd_get_float(a));
    } else if (libpd_is_symbol(a)) {
      x = PyString_FromString(libpd_get_symbol(a));
    }
    PyTuple_SetItem(result, i, x);
  }
  return result;
}

#define MAKE_CALLBACK(s, args1, cmd, args2) \
static PyObject *s##_callback = NULL; \
static int libpd_set_##s##_callback(PyObject *callback) { \
  Py_XDECREF(s##_callback); \
  if (PyCallable_Check(callback)) { \
    s##_callback = callback; \
    Py_INCREF(s##_callback); \
    return 0; \
  } else { \
    s##_callback = NULL; \
    return -1; \
  } \
} \
static void pylibpd_##s args1 { \
  if (s##_callback) { \
    PyObject *pyargs = cmd args2; \
    PyObject *result = PyObject_CallObject(s##_callback, pyargs); \
    Py_XDECREF(result); \
    Py_DECREF(pyargs); \
  } \
}

MAKE_CALLBACK(print, (const char *s), Py_BuildValue, ("(s)", s))
MAKE_CALLBACK(bang, (const char *dest), Py_BuildValue, ("(s)", dest))
MAKE_CALLBACK(float, (const char *dest, float val),
    Py_BuildValue, ("(sf)", dest, val))
MAKE_CALLBACK(symbol, (const char *dest, const char *sym),
    Py_BuildValue, ("(ss)", dest, sym))
MAKE_CALLBACK(list, (const char *dest, int n, t_atom *pd_args),
    convertArgs, (dest, NULL, n, pd_args))
MAKE_CALLBACK(message,
    (const char *dest, const char *sym, int n, t_atom *pd_args),
    convertArgs, (dest, sym, n, pd_args))
MAKE_CALLBACK(noteon, (int ch, int n, int v),
    Py_BuildValue, ("(iii)", ch, n, v))
MAKE_CALLBACK(controlchange, (int ch, int c, int v),
    Py_BuildValue, ("(iii)", ch, c, v))
MAKE_CALLBACK(programchange, (int ch, int pgm),
    Py_BuildValue, ("(ii)", ch, pgm))
MAKE_CALLBACK(pitchbend, (int ch, int bend),
    Py_BuildValue, ("(ii)", ch, bend))
MAKE_CALLBACK(aftertouch, (int ch, int v),
    Py_BuildValue, ("(ii)", ch, v))
MAKE_CALLBACK(polyaftertouch, (int ch, int n, int v),
    Py_BuildValue, ("(iii)", ch, n, v))
MAKE_CALLBACK(midibyte, (int p, int b),
    Py_BuildValue, ("(ii)", p, b))

%}

%init %{
#define ASSIGN_CALLBACK(s) libpd_##s##hook = pylibpd_##s;

ASSIGN_CALLBACK(print)
ASSIGN_CALLBACK(bang)
ASSIGN_CALLBACK(float)
ASSIGN_CALLBACK(symbol)
ASSIGN_CALLBACK(list)
ASSIGN_CALLBACK(message)

ASSIGN_CALLBACK(noteon)
ASSIGN_CALLBACK(controlchange)
ASSIGN_CALLBACK(programchange)
ASSIGN_CALLBACK(pitchbend)
ASSIGN_CALLBACK(aftertouch)
ASSIGN_CALLBACK(polyaftertouch)
ASSIGN_CALLBACK(midibyte)

libpd_init();
%}

