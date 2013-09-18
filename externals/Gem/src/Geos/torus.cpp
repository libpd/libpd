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

/////////////////////////////////////////////////////////
// 2105:forum::für::umläute:2000
/////////////////////////////////////////////////////////
// added the gluPartialDisk
/////////////////////////////////////////////////////////

#include "torus.h"
#include "Gem/State.h"
#include "Gem/Exception.h"

CPPEXTERN_NEW_WITH_GIMME(torus);

/////////////////////////////////////////////////////////
//
// torus
//
/////////////////////////////////////////////////////////
// Constructor
//
/////////////////////////////////////////////////////////
torus :: torus(int argc, t_atom *argv)
      : GemGluObj(1.f)
{
  float size = 1.f;
  int numSlices = 10;
  m_innerRadius = 0.5f;

  switch(argc){
  case 3:
    size = atom_getfloat(&argv[0]);
    numSlices = atom_getint(&argv[1]);
    m_innerRadius = atom_getfloat(&argv[2]);
    break;
  case 2:
    size = atom_getfloat(&argv[0]);
    numSlices = atom_getint(&argv[1]);
    break;
  case 1:
    size = atom_getfloat(&argv[0]);
  case 0:
    break;
  default:
    throw(GemException("needs 0, 1, 2 or 3 arguments"));
  }
	
  sizeMess(size);
  numSlicesMess(numSlices);
  inlet_new(this->x_obj, &this->x_obj->ob_pd, gensym("float"), gensym("inner"));
}

/////////////////////////////////////////////////////////
// Destructor
//
/////////////////////////////////////////////////////////
torus :: ~torus()
{ }

/////////////////////////////////////////////////////////
// innerRadius
//
/////////////////////////////////////////////////////////
void torus :: innerRadius(float radius)
{
    m_innerRadius = radius;
    setModified();
}

/////////////////////////////////////////////////////////
// render
//
/////////////////////////////////////////////////////////
void torus :: render(GemState *state)
{
  TexCoord*texCoords=NULL;
  bool lighting = false;
  int texNum=0;
  int texType=0;
  state->get(GemState::_GL_TEX_COORDS, texCoords);
  state->get(GemState::_GL_TEX_TYPE, texType);
  state->get(GemState::_GL_TEX_NUMCOORDS, texNum);
  state->get(GemState::_GL_LIGHTING, lighting);

  GLenum type = m_drawType;
  switch(m_drawType){
  case GL_LINE_LOOP: type=GL_LINE;  break;    
  case GL_POINTS   : type=GL_POINT; break;
  case GL_DEFAULT_GEM: // default
  case GL_POLYGON  : type=GL_FILL;  break;
  }
#ifdef GLU_TRUE
  switch(m_drawType){
  case GLU_LINE : type=GL_LINE;  break;    
  case GLU_POINT: type=GL_POINT; break;
  case GLU_FILL : type=GL_FILL;  break;
  }
#endif

  GLfloat xsize = 1.0, xsize0 = 0.0;
  GLfloat ysize = 1.0, ysize0 = 0.0;
  if(texType && texNum>=3){
    xsize0 = texCoords[0].s;
    xsize  = texCoords[1].s-xsize0;
    ysize0 = texCoords[1].t;
    ysize  = texCoords[2].t-ysize0;
  }

  //gluTorus(m_thing, m_innerRadius, m_size, m_numSlices, m_numSlices);
  GLint rings = m_numSlices;
  GLint nsides= m_numSlices;

  GLfloat r = m_innerRadius;
  GLfloat R = m_size;

  int i, j;
  GLfloat theta, phi, theta1;
  GLfloat cosTheta, sinTheta;
  GLfloat cosTheta1, sinTheta1;

  const GLfloat ringDelta = 2.0 * M_PI / rings;
  const GLfloat sideDelta = 2.0 * M_PI / nsides;

  const GLfloat ds = 1.0 / rings;
  const GLfloat dt = 1.0 / nsides;
  GLfloat s, t;

  glPushAttrib(GL_POLYGON_BIT);
  glPolygonMode(GL_FRONT_AND_BACK, type); 

  theta = 0.0;
  cosTheta = 1.0;
  sinTheta = 0.0;
  t = 0.0;
  for (i = rings - 1; i >= 0; i--) {
    theta1 = theta + ringDelta;
    cosTheta1 = cos(theta1);
    sinTheta1 = sin(theta1);
    glBegin(GL_QUAD_STRIP);
    phi = 0.0;
    s = 0.0;
    for (j = nsides; j >= 0; j--) {
      GLfloat cosPhi, sinPhi, dist;

      phi += sideDelta;
      cosPhi = cos(phi);
      sinPhi = sin(phi);
      dist = R + r * cosPhi;

      if(lighting)glNormal3f(cosTheta1 * cosPhi, -sinTheta1 * cosPhi, sinPhi);
      if(texType)glTexCoord2f(s*xsize+xsize0, t*ysize+ysize0);
      glVertex3f(cosTheta1 * dist, -sinTheta1 * dist, r * sinPhi);

      if(lighting)glNormal3f(cosTheta * cosPhi, -sinTheta * cosPhi, sinPhi);
      if(texType)glTexCoord2f(s*xsize+xsize0, (t - dt)*ysize+ysize0);
      glVertex3f(cosTheta * dist, -sinTheta * dist,  r * sinPhi);

      s+=ds;
    }
    glEnd();
    theta = theta1;
    cosTheta = cosTheta1;
    sinTheta = sinTheta1;
    t += dt;
  }
  glPopAttrib();
}

/////////////////////////////////////////////////////////
// static member functions
//
/////////////////////////////////////////////////////////
void torus :: obj_setupCallback(t_class *classPtr)
{
    class_addmethod(classPtr, reinterpret_cast<t_method>(&torus::innerRadiusCallback),
    	    gensym("inner"), A_FLOAT, A_NULL); 
}
void torus :: innerRadiusCallback(void *data, t_floatarg radius)
{
    GetMyClass(data)->innerRadius(radius);
}

