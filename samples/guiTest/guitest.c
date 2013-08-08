/*
 * Copyright (c) 2013 Kjetil Matheussen
 *
 * BSD Simplified License.
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.
 *
 */


#include <stdio.h>
#include "z_libpd.h"

void pdprint(const char *s) {
  printf("%s", s);
}

void pdnoteon(int ch, int pitch, int vel) {
  printf("noteon: %d %d %d\n", ch, pitch, vel);
}

float inbuf[64], outbuf[128];  // one input channel, two output channels
                               // block size 64, one tick per buffer

static void runawhile(int num_seconds){
  int msecsleft = num_seconds*1000;
  while(msecsleft > 0) {
    // fill inbuf here
    libpd_process_float(1, inbuf, outbuf);
    // use outbuf here
    usleep(1000);
    msecsleft -= 1;
  }
}

int main(int argc, char **argv) {
  if (argc < 3) {
    fprintf(stderr, "usage: %s file folder\n", argv[0]);
    return -1;
  }

  // init pd
  int srate = 44100;
  libpd_printhook = (t_libpd_printhook) pdprint;
  libpd_noteonhook = (t_libpd_noteonhook) pdnoteon;
  libpd_init(true, "../../pure-data");
  libpd_init_audio(1, 2, srate);

  // compute audio    [; pd dsp 1(
  libpd_start_message(1); // one entry in list
  libpd_add_float(1.0f);
  libpd_finish_message("pd", "dsp");

  // open patch       [; pd open file folder(
  void *file = libpd_openfile(argv[1], argv[2]);

  // now run pd for 12 seconds (logical time)
  int iterations;
  for(iterations=0;iterations<2;iterations++) {
    printf("Showing gui for 5 seconds\n");
    libpd_show_gui();
    runawhile(5);

    printf("Hiding gui for 2 seconds\n");
    libpd_hide_gui();
    runawhile(2);
  }

  printf("Closing file\n");
  libpd_closefile(file);

  printf("Cleaning up\n");
  libpd_cleanup();

  return 0;
}

