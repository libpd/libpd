/*
Based on:
<https://gitlab.freedesktop.org/mesa/glu/blob/master/src/libutil/error.c>
<https://gitlab.freedesktop.org/mesa/glu/blob/master/src/libutil/project.c>
with minor modifications:
- use double throughout lookAt
- no nurbs/tess error strings
*/
/*
 * SGI FREE SOFTWARE LICENSE B (Version 2.0, Sept. 18, 2008)
 * Copyright (C) 1991-2000 Silicon Graphics, Inc. All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice including the dates of first publication and
 * either this permission notice or a reference to
 * http://oss.sgi.com/projects/FreeB/
 * shall be included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * SILICON GRAPHICS, INC. BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
 * OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * Except as contained in this notice, the name of Silicon Graphics, Inc.
 * shall not be used in advertising or otherwise to promote the sale, use or
 * other dealings in this Software without prior written authorization from
 * Silicon Graphics, Inc.
 */

#include <GL/Regal.h>
#include <GL/RegalGLU.h>

#include <math.h>

static void makeIdentity(GLdouble m[16])
{
    m[0+4*0] = 1; m[0+4*1] = 0; m[0+4*2] = 0; m[0+4*3] = 0;
    m[1+4*0] = 0; m[1+4*1] = 1; m[1+4*2] = 0; m[1+4*3] = 0;
    m[2+4*0] = 0; m[2+4*1] = 0; m[2+4*2] = 1; m[2+4*3] = 0;
    m[3+4*0] = 0; m[3+4*1] = 0; m[3+4*2] = 0; m[3+4*3] = 1;
}

static void normalize(double v[3])
{
    double r;

    r = sqrt( v[0]*v[0] + v[1]*v[1] + v[2]*v[2] );
    if (r == 0.0) return;

    v[0] /= r;
    v[1] /= r;
    v[2] /= r;
}

static void cross(double v1[3], double v2[3], double result[3])
{
    result[0] = v1[1]*v2[2] - v1[2]*v2[1];
    result[1] = v1[2]*v2[0] - v1[0]*v2[2];
    result[2] = v1[0]*v2[1] - v1[1]*v2[0];
}

extern void gluLookAt
  ( GLdouble eyex, GLdouble eyey, GLdouble eyez
  , GLdouble centerx, GLdouble centery, GLdouble centerz
  , GLdouble upx, GLdouble upy, GLdouble upz
  )
{
    double forward[3], side[3], up[3];
    GLdouble m[4][4];

    forward[0] = centerx - eyex;
    forward[1] = centery - eyey;
    forward[2] = centerz - eyez;

    up[0] = upx;
    up[1] = upy;
    up[2] = upz;

    normalize(forward);

    /* Side = forward x up */
    cross(forward, up, side);
    normalize(side);

    /* Recompute up as: up = side x forward */
    cross(side, forward, up);

    makeIdentity(&m[0][0]);
    m[0][0] = side[0];
    m[1][0] = side[1];
    m[2][0] = side[2];

    m[0][1] = up[0];
    m[1][1] = up[1];
    m[2][1] = up[2];

    m[0][2] = -forward[0];
    m[1][2] = -forward[1];
    m[2][2] = -forward[2];

    glMultMatrixd(&m[0][0]);
    glTranslated(-eyex, -eyey, -eyez);
}

extern const GLubyte *gluErrorString(GLenum err)
{
  switch (err)
  {
  case GL_NO_ERROR: return (const GLubyte *)"no error";
  case GL_INVALID_ENUM: return (const GLubyte *)"invalid enumerant";
  case GL_INVALID_VALUE: return (const GLubyte *)"invalid value";
  case GL_INVALID_OPERATION: return (const GLubyte *)"invalid operation";
  case GL_STACK_OVERFLOW: return (const GLubyte *)"stack overflow";
  case GL_STACK_UNDERFLOW: return (const GLubyte *)"stack underflow";
  case GL_OUT_OF_MEMORY: return (const GLubyte *)"out of memory";
  case GL_TABLE_TOO_LARGE: return (const GLubyte *)"table too large";
#ifdef GL_EXT_framebuffer_object
  case GL_INVALID_FRAMEBUFFER_OPERATION_EXT: return (const GLubyte *)"invalid framebuffer operation";
#endif
  default: return 0;
  }
}
