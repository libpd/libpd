

#ifndef __Z_HOOKSET_H__
#define __Z_HOOKSET_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include "z_libpd.h"

typedef void (*t_libpd_liststrhook)(const char *recv, int argc, const char *argv);
typedef void (*t_libpd_messagestrhook)(const char *recv, const char *msg, int argc, const char *argv);

EXTERN t_libpd_liststrhook libpd_liststrhook;
EXTERN t_libpd_messagestrhook libpd_messagestrhook;

EXTERN void libpd_set_printhook(const t_libpd_printhook hook);
EXTERN void libpd_set_banghook(const t_libpd_banghook hook);
EXTERN void libpd_set_floathook(const t_libpd_floathook hook);
EXTERN void libpd_set_symbolhook(const t_libpd_symbolhook hook);
EXTERN void libpd_set_listhook(const t_libpd_listhook hook);
EXTERN void libpd_set_messagehook(const t_libpd_messagehook hook);
EXTERN void libpd_set_liststrhook(const t_libpd_liststrhook hook);
EXTERN void libpd_set_messagestrhook(const t_libpd_messagestrhook hook);

EXTERN void libpd_set_noteonhook(const t_libpd_noteonhook hook);
EXTERN void libpd_set_controlchangehook(const t_libpd_controlchangehook hook);
EXTERN void libpd_set_programchangehook(const t_libpd_programchangehook hook);
EXTERN void libpd_set_pitchbendhook(const t_libpd_pitchbendhook hook);
EXTERN void libpd_set_aftertouchhook(const t_libpd_aftertouchhook hook);
EXTERN void libpd_set_polyaftertouchhook(const t_libpd_polyaftertouchhook hook);
EXTERN void libpd_set_midibytehook(const t_libpd_midibytehook hook);

#ifdef __cplusplus
}
#endif

#endif
