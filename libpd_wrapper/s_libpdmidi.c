/* Copyright (c) 1997-2010 Miller Puckette and others.
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

#include "m_pd.h"
#include "z_libpd.h"

#define CHANNEL ((port << 4) | channel)

void outmidi_noteon(int port, int channel, int pitch, int velo) {
  if (libpd_noteonhook)
    libpd_noteonhook(CHANNEL, pitch, velo);
}

void outmidi_controlchange(int port, int channel, int ctl, int value) {
  if (libpd_controlchangehook)
    libpd_controlchangehook(CHANNEL, ctl, value);
}

void outmidi_programchange(int port, int channel, int value) {
  if (libpd_programchangehook)
    libpd_programchangehook(CHANNEL, value);
}

void outmidi_pitchbend(int port, int channel, int value) {
  if (libpd_pitchbendhook)
    libpd_pitchbendhook(CHANNEL, value - 8192); // remove offset
}

void outmidi_aftertouch(int port, int channel, int value) {
  if (libpd_aftertouchhook)
    libpd_aftertouchhook(CHANNEL, value);
}

void outmidi_polyaftertouch(int port, int channel, int pitch, int value) {
  if (libpd_polyaftertouchhook)
    libpd_polyaftertouchhook(CHANNEL, pitch, value);
}


// The rest is not relevant to libpd.
void outmidi_byte(int port, int value) {}
void sys_get_midi_apis(char *buf) {}
void sys_listmididevs(void) {}
void sys_get_midi_params(int *pnmidiindev, int *pmidiindev,
    int *pnmidioutdev, int *pmidioutdev) {}
void sys_open_midi(int nmidiindev, int *midiindev,
    int nmidioutdev, int *midioutdev, int enable) {}
void sys_reopen_midi(void) {}
void sys_initmidiqueue(void) {}
void sys_pollmidiqueue(void) {}
void sys_setmiditimediff(double inbuftime, double outbuftime) {}
void glob_midi_setapi(void *dummy, t_floatarg f) {}
void glob_midi_properties(t_pd *dummy, t_floatarg flongform) {}
void glob_midi_dialog(t_pd *dummy, t_symbol *s, int argc, t_atom *argv) {}

