#include <SDL2/SDL.h>
#include <SDL2/SDL_audio.h>

#include <GL/Regal.h>
#include "GL/glu.h"

#include <emscripten.h>

#include <stdio.h>
#include "z_libpd.h"

#include "gem_init.c"
extern void Gem_setup(void);

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

void main1(void)
{
  // nop
}

int main(int argc, char **argv)
{
  // initialize SDL2 audio
  SDL_Init(SDL_INIT_AUDIO);
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
  libpd_init();
  libpd_init_audio(1, 2, have.freq);
  Gem_init();
  Gem_setup();

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

#if 1

#undef glXGetProcAddressARB
void *glXGetProcAddressARB(const GLubyte *s) { return eglGetProcAddress((const char *) s); }

#undef glBegin
void glBegin(GLenum mode) { rglBegin(mode); }
#undef glColor3f
void glColor3f(GLfloat x, GLfloat y, GLfloat z) { rglColor3f(x, y, z); }
#undef glColor3fv
void glColor3fv(const GLfloat *v) { rglColor3fv(v); }
#undef glColor4f
void glColor4f(GLfloat x, GLfloat y, GLfloat z, GLfloat w) { rglColor4f(x, y, z, w); }
#undef glColorMaterial
void glColorMaterial(GLenum face, GLenum mode) { rglColorMaterial(face, mode); }
#undef glDisable
void glDisable(GLenum mode) { rglDisable(mode); }
#undef glDrawBuffer
void glDrawBuffer(GLenum mode) { rglDrawBuffer(mode); }
#undef glEnable
void glEnable(GLenum mode) { rglEnable(mode); }
#undef glEnd
void glEnd(void) { rglEnd(); }
#undef glFogf
void glFogf(GLenum n, GLfloat v) { rglFogf(n, v); }
#undef glFogfv
void glFogfv(GLenum n, const GLfloat *v) { rglFogfv(n, v); }
#undef glFrustum
void glFrustum(GLdouble a, GLdouble b, GLdouble c, GLdouble d, GLdouble e, GLdouble f) { rglFrustum(a,b,c,d,e,f); }
#undef glGetBooleanv
void glGetBooleanv(GLenum pname, GLboolean *params) { return rglGetBooleanv(pname, params); }
#undef glGetDoublev
void glGetDoublev(GLenum pname, GLdouble *params) { return rglGetDoublev(pname, params); }
#undef glGetFloatv
void glGetFloatv(GLenum pname, GLfloat *params) { return rglGetFloatv(pname, params); }
#undef glGetIntegerv
void glGetIntegerv(GLenum pname, GLint *params) { return rglGetIntegerv(pname, params); }
#undef glIsEnabled
GLboolean glIsEnabled(GLenum cap) { return rglIsEnabled(cap); }
#undef glLightfv
void glLightfv(GLenum l, GLenum n, const GLfloat *v) { rglLightfv(l, n, v); }
#undef glLightModeli
void glLightModeli(GLenum pname, GLint param) { rglLightModeli(pname, param); }
#undef glLoadIdentity
void glLoadIdentity(void) { rglLoadIdentity(); }
#undef glMaterialfv
void glMaterialfv(GLenum f, GLenum p, const GLfloat *v) { rglMaterialfv(f, p, v); }
#undef glMatrixMode
void glMatrixMode(GLenum mode) { rglMatrixMode(mode); }
#undef glMultMatrixf
void glMultMatrixf(const GLfloat * m) { rglMultMatrixf(m); }
#undef glNormal3f
void glNormal3f(GLfloat x, GLfloat y, GLfloat z) { rglNormal3f(x, y, z); }
#undef glNormal3fv
void glNormal3fv(const GLfloat *v) { rglNormal3fv(v); }
#undef glPopMatrix
void glPopMatrix(void) { rglPopMatrix(); }
#undef glPushMatrix
void glPushMatrix(void) { rglPushMatrix(); }
#undef glRotatef
void glRotatef(GLfloat x, GLfloat y, GLfloat z, GLfloat w) { rglRotatef(x,y,z,w); }
#undef glScalef
void glScalef(GLfloat x, GLfloat y, GLfloat z) { rglScalef(x,y,z); }
#undef glShadeModel
void glShadeModel(GLenum mode) { rglShadeModel(mode); }
#undef glTexCoord2f
void glTexCoord2f(GLfloat x, GLfloat y) { rglTexCoord2f(x,y); }
#undef glTranslated
void glTranslated(GLdouble x, GLdouble y, GLdouble z) { rglTranslated(x,y,z); }
#undef glTranslatef
void glTranslatef(GLfloat x, GLfloat y, GLfloat z) { rglTranslatef(x,y,z); }
#undef glVertex2f
void glVertex2f(GLfloat x, GLfloat y) { rglVertex2f(x,y); }
#undef glVertex3f
void glVertex3f(GLfloat x, GLfloat y, GLfloat z) { rglVertex3f(x,y,z); }
#undef glVertex3d
void glVertex3d(GLdouble x, GLdouble y, GLdouble z) { rglVertex3d(x, y, z); }

// GLU
void mgluLookAt(GLdouble eyeX, GLdouble eyeY, GLdouble eyeZ, GLdouble centerX, GLdouble centerY, GLdouble centerZ, GLdouble upX, GLdouble upY, GLdouble upZ) { gluLookAt(eyeX, eyeY, eyeZ, centerX, centerY, centerZ, upX, upY, upZ); }
const GLubyte *gluErrorString(GLenum error) { return mgluErrorString(error); }
GLUquadric *gluNewQuadric(void) { return mgluNewQuadric(); }
void gluDeleteQuadric(GLUquadric* quad) { mgluDeleteQuadric(quad); }
void gluQuadricDrawStyle(GLUquadric* quad, GLenum draw) { mgluQuadricDrawStyle(quad, draw); }
void gluQuadricTexture(GLUquadric* quad, GLboolean texture) { mgluQuadricTexture(quad, texture); }
void gluCylinder(GLUquadric* quad, GLdouble base, GLdouble top, GLdouble height, GLint slices, GLint stacks) { mgluCylinder(quad, base, top, height, slices, stacks); }

#endif
