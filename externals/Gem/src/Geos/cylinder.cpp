////////////////////////////////////////////////////////
//
// GEM - Graphics Environment for Multimedia
//
// zmoelnig@iem.kug.ac.at
//
// Implementation file
//
//    Copyright (c) 1997-2000 Mark Danks.
//    Copyright (c) Günther Geiger.
//    Copyright (c) 2001-2011 IOhannes m zmölnig. forum::für::umläute. IEM. zmoelnig@iem.at
//    For information on usage and redistribution, and for a DISCLAIMER OF ALL
//    WARRANTIES, see the file, "GEM.LICENSE.TERMS" in this distribution.
//
/////////////////////////////////////////////////////////

#include "cylinder.h"
#include "Gem/State.h"

CPPEXTERN_NEW_WITH_TWO_ARGS(cylinder, t_floatarg, A_DEFFLOAT, t_floatarg, A_DEFFLOAT);

#define normal3f glNormal3f

/////////////////////////////////////////////////////////
//
// cylinder
//
/////////////////////////////////////////////////////////
// Constructor
//
/////////////////////////////////////////////////////////
  cylinder :: cylinder(t_floatarg size,t_floatarg slize)
    : GemGluObj(size,slize),
      baseRadius(size),
      topRadius(size),
      height(size*2),
      slices(slize),
      stacks(slize)
{ }

////////////////////////////////////////////////////////
// Destructor
//
/////////////////////////////////////////////////////////
cylinder :: ~cylinder()
{ }

////////////////////////////////////////////////////////
// render
//
/////////////////////////////////////////////////////////
void cylinder :: setupParameters(void){
  baseRadius=m_size;
  topRadius=m_size;
  height=m_size*2;
  slices=m_numSlices;
  stacks=m_numSlices;
}
void cylinder :: render(GemState *state)
{
  setupParameters();
  if(m_drawType==GL_DEFAULT_GEM)m_drawType=GL_FILL;

  GLdouble da, r, dr, dz;
  GLfloat x, y, z, nz;
  GLint i, j;
  GLboolean orientation = false; /* true=INSIDE */
  GLfloat nsign = (orientation)?-1.0:1.0;

  TexCoord*texCoords=NULL;
  int texType=0;
  int texNum=0;
  bool lighting=false;
  state->get(GemState::_GL_TEX_COORDS, texCoords);
  state->get(GemState::_GL_TEX_TYPE, texType);
  state->get(GemState::_GL_TEX_NUMCOORDS, texNum);
  state->get(GemState::_GL_LIGHTING, lighting);


  GLfloat xsize = 1.0, xsize0 = 0.0;
  GLfloat ysize = 1.0, ysize0 = 0.0;
  if(texType && texNum>=3){
    xsize0 = texCoords[0].s;
    xsize  = texCoords[1].s-xsize0;
    ysize0 = texCoords[1].t;
    ysize  = texCoords[2].t-ysize0;
  }
    
  glPushMatrix();
  glTranslatef(0.f, 0.f, -m_size);

  // gluCylinder(m_thing, m_size, m_size, m_size * 2, m_numSlices, m_numSlices);
  da = 2.0 * M_PI / slices;
  dr = (topRadius - baseRadius) / stacks;
  dz = height / stacks;
  nz = (baseRadius - topRadius) / height;	/* Z component of normal vectors */
  
  if (m_drawType == GL_POINT) {
    glBegin(GL_POINTS);
    for (i = 0; i < slices; i++) {
      x = cos(i * da);
      y = sin(i * da);
      normal3f(x * nsign, y * nsign, nz * nsign);

      z = 0.0;
      r = baseRadius;
      for (j = 0; j <= stacks; j++) {
        glVertex3f(x * r, y * r, z);
        z += dz;
        r += dr;
      }
    }
    glEnd();
  }
  else if (m_drawType == GL_LINE || m_drawType == GLU_SILHOUETTE) {
    /* Draw rings */
    if (m_drawType == GL_LINE) {
      z = 0.0;
      r = baseRadius;
      for (j = 0; j <= stacks; j++) {
        glBegin(GL_LINE_LOOP);
        for (i = 0; i < slices; i++) {
          x = cos(i * da);
          y = sin(i * da);
          normal3f(x * nsign, y * nsign, nz * nsign);
          glVertex3f(x * r, y * r, z);
        }
        glEnd();
        z += dz;
        r += dr;
      }
    }
    else {
      /* draw one ring at each end */
      if (baseRadius != 0.0) {
        glBegin(GL_LINE_LOOP);
        for (i = 0; i < slices; i++) {
          x = cos(i * da);
          y = sin(i * da);
          normal3f(x * nsign, y * nsign, nz * nsign);
          glVertex3f(x * baseRadius, y * baseRadius, 0.0);
        }
        glEnd();
        glBegin(GL_LINE_LOOP);
        for (i = 0; i < slices; i++) {
          x = cos(i * da);
          y = sin(i * da);
          normal3f(x * nsign, y * nsign, nz * nsign);
          glVertex3f(x * topRadius, y * topRadius, height);
        }
        glEnd();
      }
    }
    /* draw length lines */
    glBegin(GL_LINES);
    for (i = 0; i < slices; i++) {
      x = cos(i * da);
      y = sin(i * da);
      normal3f(x * nsign, y * nsign, nz * nsign);
      glVertex3f(x * baseRadius, y * baseRadius, 0.0);
      glVertex3f(x * topRadius, y * topRadius, height);
    }
    glEnd();
  }
  else if (m_drawType == GL_FILL) {
    GLfloat ds = 1.0 / slices;
    GLfloat dt = 1.0 / stacks;
    GLfloat t = 0.0;
    z = 0.0;
    r = baseRadius;
    for (j = 0; j < stacks; j++) {
      GLfloat s = 0.0;
      glBegin(GL_QUAD_STRIP);
      for (i = 0; i <= slices; i++) {
        GLfloat x, y;
        if (i == slices) {
          x = sin(0.0);
          y = cos(0.0);
        }
        else {
          x = sin(i * da);
          y = cos(i * da);
        }
        if (nsign == 1.0) {
          normal3f(x * nsign, y * nsign, nz * nsign);
          if(texType)glTexCoord2f(s*xsize+xsize0, t*ysize+ysize0);
          glVertex3f(x * r, y * r, z);
          normal3f(x * nsign, y * nsign, nz * nsign);
          if(texType)glTexCoord2f(s*xsize+xsize0, (t + dt)*ysize+ysize0);
          glVertex3f(x * (r + dr), y * (r + dr), z + dz);
        }
        else {
          normal3f(x * nsign, y * nsign, nz * nsign);
          if(texType)glTexCoord2f(s*xsize+xsize0, t*ysize+ysize0);
          glVertex3f(x * r, y * r, z);
          normal3f(x * nsign, y * nsign, nz * nsign);
          if(texType)glTexCoord2f(s*xsize+xsize0, (t + dt)*ysize+ysize0);
          glVertex3f(x * (r + dr), y * (r + dr), z + dz);
        }
        s += ds;
      }			/* for slices */
      glEnd();
      r += dr;
      t += dt;
      z += dz;
    }				/* for stacks */
  }
  glPopMatrix();
}

/////////////////////////////////////////////////////////
// static member functions
//
/////////////////////////////////////////////////////////
void cylinder :: obj_setupCallback(t_class *)
{ }
