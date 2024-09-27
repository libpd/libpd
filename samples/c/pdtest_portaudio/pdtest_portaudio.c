/*
  run libpd with audio input/output using portaudio for 5 seconds
  Dan Wilcox <danomatika.com> 2023
*/
#include <stdio.h>
#include "z_libpd.h"
#include "portaudio.h"

// PORTAUDIO

// returns number of input channels for device index or 0 if none
static int pa_get_inputs(PaDeviceIndex dev) {
    const PaDeviceInfo *info = Pa_GetDeviceInfo(dev);
    return (info ? info->maxInputChannels : 0);
}

// returns number of output channels for device index or 0 if none
static int pa_get_outputs(PaDeviceIndex dev) {
    const PaDeviceInfo *info = Pa_GetDeviceInfo(dev);
    return (info ? info->maxOutputChannels : 0);
}

// portaudio sample render callback
static int pa_callback(const void *inputBuffer, void *outputBuffer,
                       unsigned long framesPerBuffer,
                       const PaStreamCallbackTimeInfo* timeInfo,
                       PaStreamCallbackFlags statusFlags, void *userData) {
  // assumes blocksize is *always* a multiple of libpd_blocksize(),
  // if not, then additional buffer is required
  int ticks = framesPerBuffer / libpd_blocksize(); 
  libpd_process_float(ticks, inputBuffer, outputBuffer);
  return 0;
}

// UTIL

#include <unistd.h>
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

// cross-platform sleep
// from https://gist.github.com/rafaelglikis/ee7275bf80956a5308af5accb4871135
void sleep_ms(int ms) {
  #ifdef _WIN32
    Sleep(ms);
  #elif _POSIX_C_SOURCE >= 199309L
    struct timespec ts;
    ts.tv_sec = ms / 1000;
    ts.tv_nsec = (ms % 1000) * 1000000;
    nanosleep(&ts, NULL);
  #else
    usleep(ms * 1000);
  #endif
}

// MAIN

void pdprint(const char *s) {
  printf("%s", s);
}

void pdnoteon(int ch, int pitch, int vel) {
  printf("noteon: %d %d %d\n", ch, pitch, vel);
}

int main(int argc, char **argv) {
  if (argc < 3) {
    fprintf(stderr, "usage: %s file folder\n", argv[0]);
    return -1;
  }

  // init portaudio: default devices, 32 bit float samples
  PaError error = Pa_Initialize();
  if (error != paNoError) {
      printf("portaudio init error: %s\n", Pa_GetErrorText(error));
      return -1;
  }
  int inputdev = Pa_GetDefaultInputDevice();
  int outputdev = Pa_GetDefaultOutputDevice();
  int inputchan = pa_get_inputs(inputdev);
  int outputchan = pa_get_outputs(outputdev);
  int samplerate = 44100;
  int buffersize = 512;
  PaStream *stream = NULL;
  PaStreamParameters inputParams = {
      .device = (PaDeviceIndex)inputdev,
      .channelCount = inputchan,
      .sampleFormat = paFloat32,
      0,
      NULL
  };
  PaStreamParameters outputParams = {
      .device = (PaDeviceIndex)outputdev,
      .channelCount = outputchan,
      .sampleFormat = paFloat32,
      0,
      NULL
  };
  error = Pa_OpenStream(
      &stream,
      (inputchan > 0 ? &inputParams : NULL),
      (outputchan > 0 ? &outputParams : NULL),
      samplerate,
      buffersize,
      0,
      pa_callback,
      NULL
  );
  if (error != paNoError) {
      fprintf(stderr, "portaudio open error: %s\n", Pa_GetErrorText(error));
      return -1;
  }
  {
    const PaDeviceInfo *info = Pa_GetDeviceInfo(inputdev);
    if(info) {
      printf("input dev:  %d %s\n", inputdev, info->name);
    }
  }
  {
    const PaDeviceInfo *info = Pa_GetDeviceInfo(outputdev);
    if(info) {
      printf("output dev: %d %s\n", outputdev, info->name);
      printf("output cha: %d\n", outputchan);
    }
  }

  // init pd, match portaudio channels and samplerate
  libpd_set_printhook(pdprint);
  libpd_set_noteonhook(pdnoteon);
  libpd_init();
  libpd_init_audio(inputchan, outputchan, samplerate);

  // compute audio    [; pd dsp 1(
  libpd_start_message(1); // one entry in list
  libpd_add_float(1.0f);
  libpd_finish_message("pd", "dsp");

  // open patch       [; pd open file folder(
  if (!libpd_openfile(argv[1], argv[2]))
    return -1;

  // start audio processing
  error = Pa_StartStream(stream);
  if (error != paNoError) {
      fprintf(stderr, "portaudio start error: %s\n", Pa_GetErrorText(error));
      return -1;
  }

  // now run dummy main loop for ten seconds (actual time),
  // samples are processed by pd in audio thread (pa_callback)
  for (int i = 0; i < 10; i++) {
    sleep_ms(1000);
    printf("sleep: %d\n", i+1);
  }

  // stop audio processing
  error = Pa_StopStream(stream);
  if (error != paNoError) {
      fprintf(stderr, "portaudio stop error: %s\n", Pa_GetErrorText(error));
      return -1;
  }

  // done
  error = Pa_Terminate();
  if (error != paNoError) {
      fprintf(stderr, "portaudio terminate error: %s\n", Pa_GetErrorText(error));
      return -1;
  }

  return 0;
}
