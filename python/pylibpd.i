/*
 * Basic Python bindings for libpd
 *
 * Copyright (c) 2010 Peter Brinkmann (peter.brinkmann@gmail.com)
 * Updated 2013,2020 Dan Wilcox (danomatika@gmail.com)
 *
 * For information on usage and redistribution, and for a DISCLAIMER OF
 * ALL WARRANTIES, see the file, "LICENSE.txt," in this distribution.
 */

%module pylibpd

/* initializing pd */

// libpd_init() in %init
void libpd_clear_search_path(void);
void libpd_add_to_search_path(const char *path);

/* opening patches */

%rename(__libpd_openfile) libpd_openfile;
%rename(__libpd_closefile) libpd_closefile;
%rename(__libpd_getdollarzero) libpd_getdollarzero;
void *libpd_openfile(const char *name, const char *dir);
void libpd_closefile(void *p);
int libpd_getdollarzero(void *p);

/* audio processing */

int libpd_blocksize(void);
int libpd_init_audio(int inChannels, int outChannels, int sampleRate);

#define TYPEMAPS(t) \
%typemap(in) t *inBuffer { \
  Py_ssize_t dummy; \
  if (PyObject_AsReadBuffer($input, (const void **)&$1, &dummy)) return NULL; \
} \
%typemap(in) t *outBuffer { \
  Py_ssize_t dummy; \
  if (PyObject_AsWriteBuffer($input, (void **)&$1, &dummy)) return NULL; \
}
TYPEMAPS(float)
TYPEMAPS(short)
TYPEMAPS(double)

int libpd_process_float(const int ticks,
    const float *inBuffer, float *outBuffer);
int libpd_process_short(const int ticks,
    const short *inBuffer, short *outBuffer);
int libpd_process_double(const int ticks,
    const double *inBuffer, double *outBuffer);

int libpd_process_raw(const float *inBuffer, float *outBuffer);
int libpd_process_raw_short(const short *inBuffer, short *outBuffer);
int libpd_process_raw_double(const double *inBuffer, double *outBuffer);

/* array access */

int libpd_arraysize(const char *name);
int libpd_resize_array(const char *name, long size);
int libpd_read_array(float *outBuffer, const char *name, int offset, int n);
int libpd_write_array(const char *name, int offset, const float *inBuffer, int n);

/* sending messages to pd */

int libpd_bang(const char *recv);
int libpd_float(const char *recv, float x);
int libpd_symbol(const char *recv, const char *symbol);

%rename(__libpd_start_message) libpd_start_message;
%rename(__libpd_add_float) libpd_add_float;
%rename(__libpd_add_symbol) libpd_add_symbol;
%rename(__libpd_finish_list) libpd_finish_list;
%rename(__libpd_finish_message) libpd_finish_message;
int libpd_start_message(int maxlen);
void libpd_add_float(float x);
void libpd_add_symbol(const char *symbol);
int libpd_finish_list(const char *recv);
int libpd_finish_message(const char *recv, const char *msg);

/* sending compound messages: atom array */

// probably need to be wrapped manually due to t_atom
//void libpd_set_float(t_atom *a, float x);
//int libpd_list(const char *recv, int argc, t_atom *argv);
//int libpd_message(const char *recv, const char *msg, int argc, t_atom *argv);

/* receiving messages from pd */

%rename(__libpd_bind) libpd_bind;
%rename(__libpd_unbind) libpd_unbind;
void *libpd_bind(const char *recv);
void libpd_unbind(void *p);
int libpd_exists(const char *recv);

#define SET_CALLBACK(s) \
  int libpd_set_##s##_callback(PyObject *callback);

SET_CALLBACK(print)
SET_CALLBACK(bang)
SET_CALLBACK(float)
SET_CALLBACK(symbol)
SET_CALLBACK(list)
SET_CALLBACK(message)

// probably need to be wrapped manually due to t_atom
//int libpd_is_float(t_atom *a);
//int libpd_is_symbol(t_atom *a);
//float libpd_get_float(t_atom *a);
//const char *libpd_get_symbol(t_atom *a);
//t_atom *libpd_next_atom(t_atom *a);

/* sending MIDI messages to pd */

int libpd_noteon(int channel, int pitch, int velocity);
int libpd_controlchange(int channel, int controller, int value);
int libpd_programchange(int channel, int value);
int libpd_pitchbend(int channel, int value);
int libpd_aftertouch(int channel, int value);
int libpd_polyaftertouch(int channel, int pitch, int value);
int libpd_midibyte(int port, int byte);
int libpd_sysex(int port, int byte);
int libpd_sysrealtime(int port, int byte);

/* receiving MIDI messages from pd */

SET_CALLBACK(noteon)
SET_CALLBACK(controlchange)
SET_CALLBACK(programchange)
SET_CALLBACK(pitchbend)
SET_CALLBACK(aftertouch)
SET_CALLBACK(polyaftertouch)
SET_CALLBACK(midibyte)

/* GUI */

int libpd_start_gui(char *path);
void libpd_stop_gui(void);
int libpd_poll_gui(void);

/* multiple instances */

// probably need to be handled manually due to t_pdinstance?
//t_pdinstance *libpd_new_instance(void);
//void libpd_set_instance(t_pdinstance *p);
//void libpd_free_instance(t_pdinstance *p);
//t_pdinstance *libpd_this_instance(void);
//t_pdinstance *libpd_main_instance(void);
//int libpd_num_instances(void);
//void libpd_set_instancedata(void *data);
//void* libpd_get_instancedata(void);

/* log level */

void libpd_set_verbose(int verbose);
int libpd_get_verbose(void);

/* manual bindings */

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

def libpd_list(recv, *args):
  return __process_args(args) or __libpd_finish_list(recv)

def libpd_message(recv, symbol, *args):
  return __process_args(args) or __libpd_finish_message(recv, symbol)

__libpd_patches = {}

def libpd_open_patch(patchannel, dir = '.'):
  ptr = __libpd_openfile(patchannel, dir)
  if not ptr:
    raise IOError("unable to open patch: %s/%s" % (dir, patch))
  dz = __libpd_getdollarzero(ptr)
  __libpd_patches[dz] = ptr
  return dz

def libpd_close_patch(dz):
  __libpd_closefile(__libpd_patches[dz])
  del __libpd_patches[dz]

__libpd_subscriptions = {}

def libpd_subscribe(recv):
  if recv not in __libpd_subscriptions:
    __libpd_subscriptions[recv] = __libpd_bind(recv)

def libpd_unsubscribe(recv):
  __libpd_unbind(__libpd_subscriptions[recv])
  del __libpd_subscriptions[recv]

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
  def __init__(self, inChannels, outChannels, sampleRate, ticks):
    self.__ticks = ticks
    self.__outbuf = array.array('b', '\x00\x00'.encode() * outChannels * libpd_blocksize())
    libpd_compute_audio(1)
    libpd_init_audio(inChannels, outChannels, sampleRate)
  def process(self, inBuffer):
    libpd_process_short(self.__ticks, inBuffer, self.__outbuf)
    return self.__outbuf
%}

%{
#include "z_libpd.h"
#include "util/z_print_util.h"

static PyObject *convertArgs(const char *recv, const char *symbol,
                              int n, t_atom *args) {
  int i = (symbol) ? 2 : 1;
  n += i;
  PyObject *result = PyTuple_New(n);
  PyTuple_SetItem(result, 0, PyString_FromString(recv));
  if (symbol) {
    PyTuple_SetItem(result, 1, PyString_FromString(symbol));
  }
  int j;
  for (j = 0; i < n; i++, j++) {
    t_atom *a = &args[j];
    PyObject *x = NULL;
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
MAKE_CALLBACK(bang, (const char *recv), Py_BuildValue, ("(s)", recv))
MAKE_CALLBACK(float, (const char *recv, float x),
    Py_BuildValue, ("(sf)", recv, x))
MAKE_CALLBACK(symbol, (const char *recv, const char *symbol),
    Py_BuildValue, ("(ss)", recv, symbol))
MAKE_CALLBACK(list, (const char *recv, int n, t_atom *pd_args),
    convertArgs, (recv, NULL, n, pd_args))
MAKE_CALLBACK(message,
    (const char *recv, const char *symbol, int n, t_atom *pd_args),
    convertArgs, (recv, symbol, n, pd_args))

MAKE_CALLBACK(noteon, (int channel, int pitch, int velocity),
    Py_BuildValue, ("(iii)", channel, pitch, velocity))
MAKE_CALLBACK(controlchange, (int channel, int controller, int velocity),
    Py_BuildValue, ("(iii)", channel, controller, velocity))
MAKE_CALLBACK(programchange, (int channel, int value),
    Py_BuildValue, ("(ii)", channel, value))
MAKE_CALLBACK(pitchbend, (int channel, int value),
    Py_BuildValue, ("(ii)", channel, value))
MAKE_CALLBACK(aftertouch, (int channel, int velocity),
    Py_BuildValue, ("(ii)", channel, velocity))
MAKE_CALLBACK(polyaftertouch, (int channel, int pitch, int velocity),
    Py_BuildValue, ("(iii)", channel, pitch, velocity))
MAKE_CALLBACK(midibyte, (int port, int byte),
    Py_BuildValue, ("(ii)", port, byte))

%}

%init %{
#define ASSIGN_CALLBACK(s) libpd_set_##s##hook(pylibpd_##s);

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
