/*
 * Copyright (c) 2010 Peter Brinkmann (peter.brinkmann@gmail.com)
 *
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.
 *
 * See http://gitorious.org/pdlib/pages/Libpd for documentation
 *
 */

#ifndef __Z_LIBPD_H__
#define __Z_LIBPD_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdbool.h>

#include "m_pd.h"

EXTERN bool libpd_init(bool use_gui, const char *libdir);
EXTERN void libpd_cleanup(void);
EXTERN void libpd_clear_search_path(void);
EXTERN void libpd_add_to_search_path(const char *sym);

EXTERN void *libpd_openfile(const char *basename, const char *dirname);
EXTERN void libpd_closefile(void *p);
EXTERN void *libpd_request_savefile(void *x);
EXTERN bool libpd_wait_until_file_is_saved(void *request, float max_seconds_to_wait); // returns true if file was saved. Can be called simultaneously with other libpd functions, except for libpd_request_savefile and libpd_wait_until_file_is_saved. It's optional whether to call this function after calling libpd_request_savefile.
EXTERN int libpd_getdollarzero(void *p);

EXTERN void libpd_show_gui(void);
EXTERN void libpd_hide_gui(void);

EXTERN int libpd_blocksize(void);
EXTERN int libpd_init_audio(int inChans, int outChans, int sampleRate);
EXTERN int libpd_process_raw(const float *inBuffer, float *outBuffer);
EXTERN int libpd_process_float_noninterleaved(int ticks, const float **inBuffer, float **outBuffer);
EXTERN int libpd_process_short(const int ticks,
    const short *inBuffer, short *outBuffer);
EXTERN int libpd_process_float(int ticks,
    const float *inBuffer, float *outBuffer);
EXTERN int libpd_process_double(int ticks,
    const double *inBuffer, double *outBuffer);

EXTERN int libpd_arraysize(const char *name);
// The parameters of the next two functions are inspired by memcpy.
EXTERN int libpd_read_array(float *dest, const char *src, int offset, int n);
EXTERN int libpd_write_array(const char *dest, int offset, float *src, int n);

EXTERN int libpd_bang(const char *recv);
EXTERN int libpd_float(const char *recv, float x);
EXTERN int libpd_symbol(const char *recv, const char *sym);

EXTERN void libpd_set_float(t_atom *v, float x);
EXTERN void libpd_set_symbol(t_atom *v, const char *sym);
EXTERN int libpd_list(const char *recv, int argc, t_atom *argv);
EXTERN int libpd_message(const char *recv, const char *msg, int argc, t_atom *argv);

EXTERN int libpd_start_message(int max_length);
EXTERN void libpd_add_float(float x);
EXTERN void libpd_add_symbol(const char *sym);
EXTERN int libpd_finish_list(const char *recv);
EXTERN int libpd_finish_message(const char *recv, const char *msg);

EXTERN int libpd_exists(const char *sym);
EXTERN void *libpd_bind(const char *sym, void *data);
EXTERN void libpd_unbind(void *p);

#define libpd_is_float(a) ((a).a_type == A_FLOAT)
#define libpd_is_symbol(a) ((a).a_type == A_SYMBOL)
#define libpd_get_float(a) ((a).a_w.w_float)
#define libpd_get_symbol(a) ((a).a_w.w_symbol->s_name)

typedef void (*t_libpd_printhook)(const char *recv);
typedef void (*t_libpd_banghook)(void *data, const char *recv);
typedef void (*t_libpd_floathook)(void *data, const char *recv, float x);
typedef void (*t_libpd_symbolhook)(void *data, const char *recv, const char *sym);
typedef void (*t_libpd_listhook)(void *data, const char *recv, int argc, t_atom *argv);
typedef void (*t_libpd_messagehook)(void *data, const char *recv, const char *msg,
    int argc, t_atom *argv);

EXTERN t_libpd_printhook libpd_printhook;
EXTERN t_libpd_banghook libpd_banghook;
EXTERN t_libpd_floathook libpd_floathook;
EXTERN t_libpd_symbolhook libpd_symbolhook;
EXTERN t_libpd_listhook libpd_listhook;
EXTERN t_libpd_messagehook libpd_messagehook;

EXTERN void libpd_set_printhook(t_libpd_printhook hook);
EXTERN void libpd_set_banghook(t_libpd_banghook hook);
EXTERN void libpd_set_floathook(t_libpd_floathook hook);
EXTERN void libpd_set_symbolhook(t_libpd_symbolhook hook);
EXTERN void libpd_set_listhook(t_libpd_listhook hook);
EXTERN void libpd_set_messagehook(t_libpd_messagehook hook);

EXTERN int libpd_noteon(int channel, int pitch, int velocity);
EXTERN int libpd_controlchange(int channel, int controller, int value);
EXTERN int libpd_programchange(int channel, int value);
EXTERN int libpd_pitchbend(int channel, int value);
EXTERN int libpd_aftertouch(int channel, int value);
EXTERN int libpd_polyaftertouch(int channel, int pitch, int value);
EXTERN int libpd_midibyte(int port, int byte);
EXTERN int libpd_sysex(int port, int byte);
EXTERN int libpd_sysrealtime(int port, int byte);

typedef void (*t_libpd_noteonhook)(void *data, int channel, int pitch, int velocity);
typedef void (*t_libpd_controlchangehook)(void *data, int channel, int controller, int value);
typedef void (*t_libpd_programchangehook)(void *data, int channel, int value);
typedef void (*t_libpd_pitchbendhook)(void *data, int channel, int value);
typedef void (*t_libpd_aftertouchhook)(void *data, int channel, int value);
typedef void (*t_libpd_polyaftertouchhook)(void *data, int channel, int pitch, int value);
typedef void (*t_libpd_midibytehook)(void *data, int port, int byte);

EXTERN t_libpd_noteonhook libpd_noteonhook;
EXTERN t_libpd_controlchangehook libpd_controlchangehook;
EXTERN t_libpd_programchangehook libpd_programchangehook;
EXTERN t_libpd_pitchbendhook libpd_pitchbendhook;
EXTERN t_libpd_aftertouchhook libpd_aftertouchhook;
EXTERN t_libpd_polyaftertouchhook libpd_polyaftertouchhook;
EXTERN t_libpd_midibytehook libpd_midibytehook;

EXTERN void libpd_set_noteonhook(t_libpd_noteonhook hook);
EXTERN void libpd_set_controlchangehook(t_libpd_controlchangehook hook);
EXTERN void libpd_set_programchangehook(t_libpd_programchangehook hook);
EXTERN void libpd_set_pitchbendhook(t_libpd_pitchbendhook hook);
EXTERN void libpd_set_aftertouchhook(t_libpd_aftertouchhook hook);
EXTERN void libpd_set_polyaftertouchhook(t_libpd_polyaftertouchhook hook);
EXTERN void libpd_set_midibytehook(t_libpd_midibytehook hook);

#ifdef __cplusplus
}
#endif

#endif
