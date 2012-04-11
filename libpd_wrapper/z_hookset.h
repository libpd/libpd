

#ifndef __Z_HOOKSET_H__
#define __Z_HOOKSET_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include "z_libpd.h"

/*
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
EXTERN t_libpd_messagehook libpd_messagehook;*/

EXTERN void libpd_set_printhook(const t_libpd_printhook hook);
EXTERN void libpd_test_printhook(const char* msg);

/*EXTERN void libpd_set_banghook(const t_libpd_banghook hook);
EXTERN void libpd_set_floathook(const t_libpd_floathook hook);
EXTERN void libpd_set_symbolhook(const t_libpd_symbolhook hook);
EXTERN void libpd_set_listhook(const t_libpd_listhook hook);
EXTERN void libpd_set_messagehook(const t_libpd_messagehook hook);*/


/*
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
*/

#ifdef __cplusplus
}
#endif

#endif
