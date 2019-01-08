#include <SDL2/SDL.h>
#include <SDL2/SDL_audio.h>
#include <SDL2/SDL_video.h>
#include <GL/gl.h>
#include <emscripten.h>

#include <stdio.h>
#include "z_libpd.h"

void audio(void *userdata, Uint8 *stream, int len)
{
  float inbuf[64], outbuf[64][2];
  float *b = (float *) stream;
  int m = len / sizeof(float) / 2;
  int k = 0;
  while (m > 0)
  {
    for (int i = 0; i < 64; ++i)
      inbuf[i] = 0;
    libpd_process_float(1, &inbuf[0], &outbuf[0][0]);
    for (int i = 0; i < 64; ++i)
      for (int j = 0; j < 2; ++j)
        b[k++] = outbuf[i][j];
    m -= 64;
  }
  if (m < 0)
  {
    fprintf(stderr, "buffer overflow, m went negative: %d\n", m);
  }
}

void pdprint(const char *s) {
  printf("%s", s);
}

void pdnoteon(int ch, int pitch, int vel) {
  printf("noteon: %d %d %d\n", ch, pitch, vel);
}

SDL_Window *window;

void main1(void)
{
  int x = 0, y = 0, w = 0, h = 0;
  SDL_GetWindowSize(window, &w, &h);
  Uint32 buttons = SDL_GetMouseState(&x, &y);
  glClearColor(x * 1.0 / w, y * 1.0 / h, 0.5, 1);
  glClear(GL_COLOR_BUFFER_BIT);
  SDL_GL_SwapWindow(window);
  libpd_start_message(1);
  libpd_add_float(24 + 60.0 * x / w);
  libpd_finish_message("note", "float");
  libpd_start_message(1);
  libpd_add_float(100.0 - 100.0 * y / h);
  libpd_finish_message("db", "float");
}

int main(int argc, char **argv)
{
  // initialize SDL2 audio
  SDL_Init(SDL_INIT_AUDIO | SDL_INIT_VIDEO);
  window = SDL_CreateWindow("pdtest_gui", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 640, 480, SDL_WINDOW_OPENGL);
  SDL_GLContext glcontext = SDL_GL_CreateContext(window);

  SDL_AudioSpec want, have;
  want.freq = 48000;
  want.format = AUDIO_F32;
  want.channels = 2;
  want.samples = 4096;
  want.callback = audio;
  SDL_AudioDeviceID dev = SDL_OpenAudioDevice(NULL, 0, &want, &have, SDL_AUDIO_ALLOW_ANY_CHANGE);
  printf("want: %d %d %d %d\n", want.freq, want.format, want.channels, want.samples);
  printf("have: %d %d %d %d\n", have.freq, have.format, have.channels, have.samples);

  // initialize libpd
  libpd_set_printhook(pdprint);
  libpd_set_noteonhook(pdnoteon);
  libpd_init();
  libpd_init_audio(1, 2, have.freq);

  // compute audio    [; pd dsp 1(
  libpd_start_message(1); // one entry in list
  libpd_add_float(1.0f);
  libpd_finish_message("pd", "dsp");

  // open patch       [; pd open file folder(
  libpd_openfile("test.pd", ".");

  // start audio processing
  SDL_PauseAudioDevice(dev, 0);
  emscripten_set_main_loop(main1, 0, 1);
  return 0;
}
