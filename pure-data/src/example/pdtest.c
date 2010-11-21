#include <stdio.h>
#include "z_libpd.h"

void pdprint(const char *s) {
  printf("%s", s);
}

int main(int argc, char **argv) {
  if (argc < 3) {
    fprintf(stderr, "usage: %s file folder\n", argv[0]);
    return -1;
  }

  // init pd
  int srate = 44100;
  libpd_printhook = (t_libpd_printhook) pdprint;
  libpd_init();
  libpd_init_audio(1, 2, srate, 1);
  float inbuf[64], outbuf[128];  // one input channel, two output channels
                                 // block size 64, one tick per buffer

  // compute audio    [; pd dsp 1(
  libpd_start_message();
  libpd_add_float(1.0f);
  libpd_finish_message("pd", "dsp");

  // open patch       [; pd open file folder(
  libpd_start_message();
  libpd_add_symbol(argv[1]);
  libpd_add_symbol(argv[2]);
  libpd_finish_message("pd", "open");

  // now run pd for ten seconds (logical time)
  int i;
  for (i = 0; i < 10 * srate / 64; i++) {
    // fill inbuf here
    libpd_process_float(inbuf, outbuf);
    // use outbuf here
  }

  return 0;
}

