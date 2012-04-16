#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "z_hookset.h"

/* additional hooks */
t_libpd_liststrhook libpd_liststrhook = NULL;
t_libpd_messagestrhook libpd_messagestrhook = NULL;

/* set hooks */

void libpd_set_printhook(const t_libpd_printhook hook){
	libpd_printhook = hook;
}

void libpd_set_banghook(const t_libpd_banghook hook){
	libpd_banghook = hook;
}

void libpd_set_floathook(const t_libpd_floathook hook){
	libpd_floathook = hook;
}

void libpd_set_symbolhook(const t_libpd_symbolhook hook){
	libpd_symbolhook = hook;
}

void libpd_set_listhook(const t_libpd_listhook hook){
	libpd_listhook = hook;
}

void libpd_set_messagehook(const t_libpd_messagehook hook){
	libpd_messagehook = hook;
}

//pointer to string memory
static char *arg_str;

//convert atom list to space seperated string
char* args_to_str(char *str, int argc, t_atom *args) {
  
  //free mem
  free(str);
  
  int i;
  int size = argc;
  char float_buff[64];
  
  //determine size
  for (i = 0; i < argc; i++) {
	t_atom a = args[i];
	if (libpd_is_float(a)) {
		sprintf(float_buff, "%f", libpd_get_float(a));
		size = size + strlen(float_buff);
    
	} else if (libpd_is_symbol(a)) {
		size = size + strlen(libpd_get_symbol(a));
    }
  }
  
  //get mem
  str = malloc(size);
  str[0] = '\0';
  
  //build string
  for (i = 0; i < argc; i++) {
	t_atom a = args[i];
	if (libpd_is_float(a)) {
		sprintf(float_buff, "%f ", libpd_get_float(a));
		strcat(str, float_buff);
    
	} else if (libpd_is_symbol(a)) {
		strcat(str, libpd_get_symbol(a));
		strcat(str, " ");
    }
  }
  
  return str;
}

/* string list */
void str_listhook(const char *recv, int argc, t_atom *argv) {
	if (libpd_liststrhook) (*libpd_liststrhook)(recv, argc, args_to_str(arg_str, argc, argv));
}

void libpd_set_liststrhook(const t_libpd_liststrhook hook){
	libpd_liststrhook = hook;
	libpd_listhook = (t_libpd_listhook)str_listhook;
}

/* string message */
void str_messagehook(const char *recv, const char *msg, int argc, t_atom *argv) {
	if (libpd_messagestrhook) (*libpd_messagestrhook)(recv, msg, argc, args_to_str(arg_str, argc, argv));
}

void libpd_set_messagestrhook(const t_libpd_messagestrhook hook) {
	libpd_messagestrhook = hook;
	libpd_messagehook = (t_libpd_messagehook)str_messagehook;
}

/* midi */
void libpd_set_noteonhook(const t_libpd_noteonhook hook) {
	libpd_noteonhook = hook;
}

void libpd_set_controlchangehook(const t_libpd_controlchangehook hook) {
	libpd_controlchangehook = hook;
}

void libpd_set_programchangehook(const t_libpd_programchangehook hook) {
	libpd_programchangehook = hook;
}

void libpd_set_pitchbendhook(const t_libpd_pitchbendhook hook) {
	libpd_pitchbendhook = hook;
}

void libpd_set_aftertouchhook(const t_libpd_aftertouchhook hook) {
	libpd_aftertouchhook = hook;
}

void libpd_set_polyaftertouchhook(const t_libpd_polyaftertouchhook hook) {
	libpd_polyaftertouchhook = hook;
}

void libpd_set_midibytehook(const t_libpd_midibytehook hook) {
	libpd_midibytehook = hook;
}


