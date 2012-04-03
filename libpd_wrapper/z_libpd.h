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

#include "m_pd.h"

EXTERN void libpd_init(void);
EXTERN void libpd_clear_search_path(void);
EXTERN void libpd_add_to_search_path(const char *sym);

EXTERN void *libpd_openfile(const char *basename, const char *dirname);
EXTERN void libpd_closefile(void *p);
EXTERN int libpd_getdollarzero(void *p);

EXTERN int libpd_blocksize(void);
EXTERN int libpd_init_audio(int inChans, int outChans, int sampleRate);
EXTERN int libpd_process_raw(float *inBuffer, float *outBuffer);
EXTERN int libpd_process_short(int ticks, short *inBuffer, short *outBuffer);
EXTERN int libpd_process_float(int ticks, float *inBuffer, float *outBuffer);
EXTERN int libpd_process_double(int ticks, double *inBuffer, double *outBuffer);

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
EXTERN void *libpd_bind(const char *sym);
EXTERN void libpd_unbind(void *p);

#define libpd_is_float(a) ((a).a_type == A_FLOAT)
#define libpd_is_symbol(a) ((a).a_type == A_SYMBOL)
#define libpd_get_float(a) ((a).a_w.w_float)
#define libpd_get_symbol(a) ((a).a_w.w_symbol->s_name)

typedef void (*t_libpd_printhook)(const char *recv);
typedef void (*t_libpd_banghook)(const char *recv);
typedef void (*t_libpd_floathook)(const char *recv, float x);
typedef void (*t_libpd_symbolhook)(const char *recv, const char *sym);
typedef void (*t_libpd_listhook)(const char *recv, int argc, t_atom *argv);
typedef void (*t_libpd_messagehook)(const char *recv, const char *msg,
    int argc, t_atom *argv);

EXTERN t_libpd_printhook libpd_printhook;
EXTERN t_libpd_banghook libpd_banghook;
EXTERN t_libpd_floathook libpd_floathook;
EXTERN t_libpd_symbolhook libpd_symbolhook;
EXTERN t_libpd_listhook libpd_listhook;
EXTERN t_libpd_messagehook libpd_messagehook;

EXTERN int libpd_noteon(int channel, int pitch, int velocity);
EXTERN int libpd_controlchange(int channel, int controller, int value);
EXTERN int libpd_programchange(int channel, int value);
EXTERN int libpd_pitchbend(int channel, int value);
EXTERN int libpd_aftertouch(int channel, int value);
EXTERN int libpd_polyaftertouch(int channel, int pitch, int value);
EXTERN int libpd_midibyte(int port, int byte);
EXTERN int libpd_sysex(int port, int byte);
EXTERN int libpd_sysrealtime(int port, int byte);

typedef void (*t_libpd_noteonhook)(int channel, int pitch, int velocity);
typedef void (*t_libpd_controlchangehook)(int channel,
    int controller, int value);
typedef void (*t_libpd_programchangehook)(int channel, int value);
typedef void (*t_libpd_pitchbendhook)(int channel, int value);
typedef void (*t_libpd_aftertouchhook)(int channel, int value);
typedef void (*t_libpd_polyaftertouchhook)(int channel, int pitch, int value);
typedef void (*t_libpd_midibytehook)(int port, int byte);
EXTERN t_libpd_noteonhook libpd_noteonhook;
EXTERN t_libpd_controlchangehook libpd_controlchangehook;
EXTERN t_libpd_programchangehook libpd_programchangehook;
EXTERN t_libpd_pitchbendhook libpd_pitchbendhook;
EXTERN t_libpd_aftertouchhook libpd_aftertouchhook;
EXTERN t_libpd_polyaftertouchhook libpd_polyaftertouchhook;
EXTERN t_libpd_midibytehook libpd_midibytehook;

#ifdef __cplusplus
}
#endif

#endif
