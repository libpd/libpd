////////////////////////////////////////////////////////
//
// GEM - Graphics Environment for Multimedia
//
// zmoelnig@iem.kug.ac.at
//
//    Copyright (c) 1997-2000 Mark Danks.
//    Copyright (c) Günther Geiger.
//    Copyright (c) 2001-2011 IOhannes m zmölnig. forum::für::umläute. IEM. zmoelnig@iem.at
//    For information on usage and redistribution, and for a DISCLAIMER OF ALL
//    WARRANTIES, see the file, "GEM.LICENSE.TERMS" in this distribution.
//
/////////////////////////////////////////////////////////

#include "curve.h"
#include "Gem/State.h"
#include <string.h>

CPPEXTERN_NEW_WITH_ONE_ARG(curve, t_floatarg, A_DEFFLOAT);

/////////////////////////////////////////////////////////
//
// curve
//
/////////////////////////////////////////////////////////
// Constructor
//
/////////////////////////////////////////////////////////
curve :: curve(t_floatarg numInputs)
	   : polygon(numInputs)
{
    m_drawType = GL_LINE_STRIP;
    m_resolution = 30;

    m_texCoords[0][0]=0;m_texCoords[0][1]=0;
    m_texCoords[1][0]=1;m_texCoords[1][1]=0;
    m_texCoords[2][0]=0;m_texCoords[2][1]=1;
    m_texCoords[3][0]=1;m_texCoords[3][1]=1;
}

/////////////////////////////////////////////////////////
// Destructor
//
/////////////////////////////////////////////////////////
curve :: ~curve()
{ }

/////////////////////////////////////////////////////////
// render
//
/////////////////////////////////////////////////////////
void curve :: render(GemState *state)
{
  if(m_numVertices<1)
    return;

  TexCoord*texCoords=NULL;
  int texType=0;
  int texNum=0;
  bool lighting=false;
  state->get(GemState::_GL_TEX_COORDS, texCoords);
  state->get(GemState::_GL_TEX_TYPE, texType);
  state->get(GemState::_GL_TEX_NUMCOORDS, texNum);

  if(m_drawType==GL_DEFAULT_GEM)m_drawType=GL_LINE_STRIP;
    glNormal3f(0.0f, 0.0f, 1.0f);
    glLineWidth(m_linewidth);
    if(texType){
      switch(texNum){
      default:
        m_texCoords[0][0]=texCoords[0].s;m_texCoords[0][1]=texCoords[0].t;
        m_texCoords[1][0]=texCoords[1].s;m_texCoords[1][1]=texCoords[1].t;
        m_texCoords[2][0]=texCoords[2].s;m_texCoords[2][1]=texCoords[2].t;
        m_texCoords[3][0]=texCoords[3].s;m_texCoords[3][1]=texCoords[3].t;
        break;
      case 0: case 1: case 2: case 3:
        m_texCoords[0][0]=0.f;m_texCoords[0][1]=0.f;
        m_texCoords[1][0]=1.f;m_texCoords[1][1]=0.f;
        m_texCoords[2][0]=1.f;m_texCoords[2][1]=1.f;
        m_texCoords[3][0]=0.f;m_texCoords[3][1]=1.f;
        break;
      }
      
      glEnable(GL_MAP1_TEXTURE_COORD_2);
      glMap1f(GL_MAP1_TEXTURE_COORD_2, 0,   1,   2, m_numVertices, &m_texCoords[0][0]);
    }
    glMap1f(GL_MAP1_VERTEX_3, 0.0, 1.0, 3, m_numVertices, &(m_vert[0][0]));
    glEnable(GL_MAP1_VERTEX_3);
    glBegin(m_drawType);
    for (int n = 0; n <= m_resolution; n++)
      glEvalCoord1f(static_cast<GLfloat>(n)/static_cast<GLfloat>(m_resolution));
    glEnd();
    glLineWidth(1.0);
}

/////////////////////////////////////////////////////////
// resolutionMess
//
/////////////////////////////////////////////////////////
void curve :: resolutionMess(int resolution)
{
    m_resolution = (resolution < 1) ? 1 : resolution;
    setModified();
}

/////////////////////////////////////////////////////////
// static member function
//
/////////////////////////////////////////////////////////
void curve :: obj_setupCallback(t_class *classPtr)
{
    class_addmethod(classPtr, reinterpret_cast<t_method>(&curve::resolutionMessCallback),
    	    gensym("res"), A_FLOAT, A_NULL);
}
void curve :: resolutionMessCallback(void *data, t_floatarg res)
{
    GetMyClass(data)->resolutionMess(static_cast<int>(res));
}
