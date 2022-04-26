/*
  this tests libpd's double-precision mode
*/
#include <stdio.h>
#include <math.h>
#include "z_libpd.h"

// note: pd uses %g to convert numbers to strings so the number representation
// will likely be limited to 6 digits
void pdprint(const char *s) {
  printf("%s", s);
}

// force 12 digit representation to show more precision
void pddouble(const char *s, double x) {
  printf("double: %.12f\n", x);
}

int main(int argc, char **argv) {
  if (argc < 3) {
    fprintf(stderr, "usage: %s file folder\n", argv[0]);
    return -1;
  }

  // check if double-precision
  // note: for printing t_float size this file is built with PD_FLOATSIZE,
  // otherwise it's fine to leave it off in client code
  printf("double-precision %d\n", (int)(sizeof(t_float)/8));
  printf("float size %d\n", (int)PD_FLOATSIZE);

  // init pd
  int srate = 44100;
  libpd_set_printhook(pdprint);
  libpd_set_doublehook(pddouble);
  libpd_init();
  libpd_init_audio(1, 2, srate);
  libpd_bind("toC");
  double inbuf[64], outbuf[128];  // one input channel, two output channels
                                  // block size 64, one tick per buffer

  // compute audio    [; pd dsp 1(
  libpd_start_message(1); // one entry in list
  libpd_add_double(1.0f);
  libpd_finish_message("pd", "dsp");

  // open patch       [; pd open file folder(
  if (!libpd_openfile(argv[1], argv[2]))
    return -1;

  // send a value with many digits...
  printf("MI_PI: %.12f\n", M_PI); 
  libpd_double("fromC", M_PI);

  // now run pd for ten seconds (logical time)
  int i;
  for (i = 0; i < 10 * srate / 64; i++) {
    // fill inbuf here
    libpd_process_double(1, inbuf, outbuf);
    // use outbuf here
  }
  for (i = 0; i < 10; i++)
    printf("%.12f\n", outbuf[i]);
  return 0;
}
