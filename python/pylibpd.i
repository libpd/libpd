%module pylibpd

%{
#include "z_libpd.h"
%}

%init %{
libpd_init();
%}

void libpd_clear_search_path();
void libpd_add_to_search_path(const char *s);

int libpd_blocksize();
int libpd_init_audio(int, int, int, int);
int libpd_process_raw(float *, float *);
int libpd_process_short(short *, short *);
int libpd_process_float(float *, float *);
int libpd_process_double(double *, double *);

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

