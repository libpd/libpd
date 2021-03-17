/*
  this tests pd's currently *experimental* multi instance support 
*/
#include <stdio.h>
#include "z_libpd.h"
#include "m_imp.h"
#define TIMEUNITPERMSEC (32. * 441.)

void pdprint(const char *s) {
  printf("%s", s);
}

void pdnoteon(int ch, int pitch, int vel) {
  printf("noteon: %d %d %d\n", ch, pitch, vel);
}

int main(int argc, char **argv) {
  t_pdinstance *pd1, *pd2;
  int srate = 44100;
  float inbuf[64], outbuf[128];  // one input channel, two output channels
                                 // block size 64, one tick per buffer
  if (argc < 3) {
    fprintf(stderr, "usage: %s file folder\n", argv[0]);
    return -1;
  }

  libpd_init();
  
  // maybe these two calls should be available per-instance somehow:
  libpd_set_printhook(pdprint);   
  libpd_set_noteonhook(pdnoteon);

  pd1 = libpd_new_instance();
  pd2 = libpd_new_instance();
  printf("%d instances\n", libpd_num_instances());

  libpd_set_instance(pd1); // talk to first pd instance

  libpd_init_audio(1, 2, srate);
  // compute audio    [; pd dsp 1(
  libpd_start_message(1); // one entry in list
  libpd_add_float(1.0f);
  libpd_finish_message("pd", "dsp");

  // open patch       [; pd open file folder(
  libpd_openfile(argv[1], argv[2]);

  libpd_set_instance(pd2);

  libpd_init_audio(1, 2, srate);
  // compute audio    [; pd dsp 1(
  libpd_start_message(1); // one entry in list
  libpd_add_float(1.0f);
  libpd_finish_message("pd", "dsp");

  // open patch       [; pd open file folder(
  libpd_openfile(argv[1], argv[2]);

    /* Note that I'm using the fact that $0 is set to 1003, 1004, ...
    as patches are opened, it would be better to open the patches with 
    settable $1, etc parameters to libpd_openfile().  */

  libpd_set_instance(pd1);
  // [; pd frequency 1 (
  libpd_start_message(1); // one entry in list
  libpd_add_float(1.0f);
  fprintf(stderr, "x 1\n");
  libpd_finish_message("frequency", "float");
  fprintf(stderr, "x 2\n");

  libpd_set_instance(pd2);
  // [; pd frequency 2 (
  libpd_start_message(1); // one entry in list
  libpd_add_float(2.0f);
  fprintf(stderr, "x 3\n");
  libpd_finish_message("frequency", "float");
  fprintf(stderr, "x 4\n");

  // now run pd for ten seconds (logical time)
  int i, j;
  for (i = 0; i < 3; i++) {
    // fill inbuf here
    libpd_set_instance(pd1);
    libpd_process_float(1, inbuf, outbuf);
    if (i < 2)
    {
        for (j = 0; j < 8; j++)
            printf("%f ", outbuf[j]);
        printf("\n");
    }
    libpd_set_instance(pd2);
    libpd_process_float(1, inbuf, outbuf);
    if (i < 2)
    {
        for (j = 0; j < 8; j++)
            printf("%f ", outbuf[j]);
        printf("\n");
    }
  }

  libpd_free_instance(pd1);
  libpd_free_instance(pd2);

  return 0;
}
