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
  t_pdinstance *pd1 = pdinstance_new(), *pd2 = pdinstance_new();
  if (argc < 3) {
    fprintf(stderr, "usage: %s file folder\n", argv[0]);
    return -1;
  }
  
  int srate = 44100;
    // maybe these two calls should be available per-instance somehow:
  libpd_set_printhook(pdprint);   
  libpd_set_noteonhook(pdnoteon);
    /* set a "current" instance before libpd_init() or else Pd will make
    an unnecessary third "default" instance. */
  pd_setinstance(pd1);
  libpd_init();
    /* ... here we'd sure like to be able to have number of channels be
    per-instance.  The sample rate is still global within Pd but we might
    also consider relaxing that restrction. */
  libpd_init_audio(1, 2, srate);

  float inbuf[64], outbuf[128];  // one input channel, two output channels
                                 // block size 64, one tick per buffer

  pd_setinstance(pd1);  // talk to first pd instance 

  // compute audio    [; pd dsp 1(
  libpd_start_message(1); // one entry in list
  libpd_add_float(1.0f);
  libpd_finish_message("pd", "dsp");

  // open patch       [; pd open file folder(
  libpd_openfile(argv[1], argv[2]);

  pd_setinstance(pd2);

  // compute audio    [; pd dsp 1(
  libpd_start_message(1); // one entry in list
  libpd_add_float(1.0f);
  libpd_finish_message("pd", "dsp");

  // open patch       [; pd open file folder(
  libpd_openfile(argv[1], argv[2]);

    /* the following two messages can be sent without setting the pd instance
    and anyhow the symbols are global so they may affect multiple instances.
    However, if the messages change anything in the pd instance structure
    (DSP state; current time; list of all canvases n our instance) those
    changes will apply to the current Pd nstance, so the earlier messages,
    for instance, were sensitive to which was the current one. 
    
    Note also that I'm using the fact that $0 is set to 1003, 1004, ...
    as patches are opened, it would be better to open the patches with 
    settable $1, etc parameters to libpd_openfile().  */
    
  // [; pd frequency 1 (
  libpd_start_message(1); // one entry in list
  libpd_add_float(1.0f);
  libpd_finish_message("1003-frequency", "float");

  // [; pd frequency 1 (
  libpd_start_message(1); // one entry in list
  libpd_add_float(2.0f);
  libpd_finish_message("1004-frequency", "float");

  // now run pd for ten seconds (logical time)
  int i, j;
  for (i = 0; i < 3; i++) {
    // fill inbuf here
    pd_setinstance(pd1);
    libpd_process_float(1, inbuf, outbuf);
    if (i < 2)
    {
        for (j = 0; j < 8; j++)
            printf("%f ", outbuf[j]);
        printf("\n");
    }
    pd_setinstance(pd2);
    libpd_process_float(1, inbuf, outbuf);
    if (i < 2)
    {
        for (j = 0; j < 8; j++)
            printf("%f ", outbuf[j]);
        printf("\n");
    }
  }

  return 0;
}