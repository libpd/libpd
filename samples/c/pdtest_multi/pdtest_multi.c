/*
  This tests libpd's multi instance support by creating two Pd instances and
  running them simultaneously.  The test patch has a phasor~ object - in one
  instance it runs forward, and in the other, backward.
*/
#include <stdio.h>
#include "z_libpd.h"
#include "z_print_util.h"

void pdprint1(const char *s) {
  printf("pd1: %s\n", s);
}

void pdprint2(const char *s) {
  printf("pd2: %s\n", s);
}

void pdnoteon1(int ch, int pitch, int vel) {
  printf("pd1 noteon: %d %d %d\n", ch, pitch, vel);
}

void pdnoteon2(int ch, int pitch, int vel) {
  printf("pd2 noteon: %d %d %d\n", ch, pitch, vel);
}

int main(int argc, char **argv) {
  t_pdinstance *pd1, *pd2;
  int srate = 48000;
  float inbuf[64], outbuf[128];  // one input channel, two output channels
                                 // block size 64, one tick per buffer
  char *filename = "test.pd", *dirname = ".";

  // accept overrides from the commandline:
  // $ pdtest_multi file.pd ../dir
  if (argc > 1) {
    filename = argv[1];
  }
  if (argc > 2) {
    dirname = argv[2];
  }

  libpd_init();

  pd1 = libpd_new_instance(); // create an instance
  pd2 = libpd_new_instance(); // create a second instance
  printf("%d instances\n", libpd_num_instances());

  libpd_set_instance(pd1); // talk to first pd instance

  // set hooks for this instance
  libpd_set_printhook(libpd_print_concatenator);
  libpd_set_concatenated_printhook(pdprint1);
  libpd_set_noteonhook(pdnoteon1);

  libpd_init_audio(1, 2, srate);
  // compute audio    [; pd dsp 1(
  libpd_start_message(1); // one entry in list
  libpd_add_float(1.0f);
  libpd_finish_message("pd", "dsp");

  // open patch       [; pd open file folder(
  libpd_openfile(filename, dirname);

  // repeat this all for the second instance
  libpd_set_instance(pd2);
  libpd_set_printhook(libpd_print_concatenator);
  libpd_set_concatenated_printhook(pdprint2);
  libpd_set_noteonhook(pdnoteon2);
  libpd_init_audio(1, 2, srate);
  libpd_start_message(1);
  libpd_add_float(1.0f);
  libpd_finish_message("pd", "dsp");
  libpd_openfile(filename, dirname);

    /* the following two messages can be sent without setting the pd instance
    and anyhow the symbols are global so they may affect multiple instances.
    However, if the messages change anything in the pd instance structure
    (DSP state; current time; list of all canvases in our instance) those
    changes will apply to the current Pd instance, so the earlier messages,
    for instance, were sensitive to which was the current one. 
    
    Note also that I'm using the fact that $0 is set to 1003, 1004, ...
    as patches are opened, it would be better to open the patches with 
    settable $1, etc parameters to libpd_openfile().  */

  // [; pd frequency 480 (
  libpd_set_instance(pd1);
  libpd_start_message(1);
  libpd_add_float(480.0f);
  libpd_finish_message("frequency", "float");

  // [; pd frequency -480 (
  libpd_set_instance(pd2);
  libpd_start_message(1);
  libpd_add_float(-480.0f);
  libpd_finish_message("frequency", "float");

  // now run pd for 3 ticks
  int i, j;
  for (i = 0; i < 3; i++) {

    libpd_set_instance(pd1);
    libpd_process_float(1, inbuf, outbuf);
    printf("instance 1, tick %d:\n", i);
    for (j = 0; j < 8; j++) {
      printf("%f ", outbuf[j]);
    }
    printf("... \n");

    libpd_set_instance(pd2);
    libpd_process_float(1, inbuf, outbuf);
    printf("instance 2, tick %d:\n", i);
    for (j = 0; j < 8; j++) {
      printf("%f ", outbuf[j]);
    }
    printf("... \n");
  }

  libpd_free_instance(pd1);
  libpd_free_instance(pd2);

  return 0;
}
