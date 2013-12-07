/*
 * Copyright (c) 2013 Kjetil Matheussen
 *
 * BSD Simplified License.
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.
 *
 */

#include <stdio.h>
#include <unistd.h>
#include <libpds.h>

#if 0
void pdprint(const char *s) {
  printf("%s", s);
}

void pdnoteon(int ch, int pitch, int vel) {
  printf("noteon: %d %d %d\n", ch, pitch, vel);
}
#endif

float inbuf[64], outbuf[128];  // one input channel, two output channels
                               // block size 64, one tick per buffer

static void runawhile(pd_t *pd1, pd_t *pd2, int num_seconds){
  int msecsleft = num_seconds*1000;
  while(msecsleft > 0) {
    // fill inbuf here
    libpds_process_float(pd1, 1, inbuf, outbuf);
    libpds_process_float(pd2, 1, inbuf, outbuf);
    // use outbuf here
    usleep(1000);
    msecsleft -= 1;
  }
}

static void *file1;
static void *file2;

static pd_t *start_instance(const char *filename, void **file) {
  pd_t *pd = libpds_create(true, "../../pure-data");

  if(pd==NULL){
    fprintf(stderr, "libpds_create returned NULL. Message: \"%s\"\n", libpds_strerror());
    return NULL;
  }
  
  // init pd
  int srate = 44100;
  //libpds_set_printhook(pd, pdprint);
  //libpds_set_noteonhook(pd, pdnoteon);
  libpds_init_audio(pd, 1, 2, srate);

  // compute audio    [; pd dsp 1(
  libpds_start_message(pd, 1); // one entry in list
  libpds_add_float(pd, 1.0f);
  libpds_finish_message(pd, "pd", "dsp");

  // open patch       [; pd open file folder(
  *file = libpds_openfile(pd, filename, ".");

  return pd;
}

int main(int argc, char **argv) {
  pd_t *pd1 = start_instance("test1.pd", &file1);
  pd_t *pd2 = start_instance("test2.pd", &file2);

  if(pd1==NULL || pd1==NULL)
    return -1;

  // now run pd for 14 seconds (logical time)
  int iterations;
  for(iterations=0;iterations<2;iterations++) {
    printf("Showing guis for 5 seconds\n");
    libpds_show_gui(pd1);
    libpds_show_gui(pd2);
    runawhile(pd1, pd2, 500);

    printf("Hiding guis for 2 seconds\n");
    libpds_hide_gui(pd1);
    libpds_hide_gui(pd2);
    runawhile(pd1, pd2, 2);
  }

  printf("Closing files\n");
  libpds_closefile(pd1, file1);
  libpds_closefile(pd2, file2);

  printf("Cleaning up\n");
  libpds_delete(pd1);
  libpds_delete(pd2);

  return 0;
}
