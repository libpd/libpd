#include "z_libpd.h"

/*
t_libpd_printhook libpd_printhook = NULL;
t_libpd_banghook libpd_banghook = NULL;
t_libpd_floathook libpd_floathook = NULL;
t_libpd_symbolhook libpd_symbolhook = NULL;
t_libpd_listhook libpd_listhook = NULL;
t_libpd_messagehook libpd_messagehook = NULL;
*/

/*set hooks*/
void libpd_set_printhook(const t_libpd_printhook hook)
{
	libpd_printhook = hook;
}

void libpd_test_printhook(const char* msg)
{
	libpd_printhook(msg);
}

/*
t_libpd_noteonhook libpd_noteonhook = NULL;
t_libpd_controlchangehook libpd_controlchangehook = NULL;
t_libpd_programchangehook libpd_programchangehook = NULL;
t_libpd_pitchbendhook libpd_pitchbendhook = NULL;
t_libpd_aftertouchhook libpd_aftertouchhook = NULL;
t_libpd_polyaftertouchhook libpd_polyaftertouchhook = NULL;
t_libpd_midibytehook libpd_midibytehook = NULL;
*/


