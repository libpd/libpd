////////////////////////////////////////////////////////
//
// GEM - Graphics Environment for Multimedia
//
// zmoelnig@iem.kug.ac.at
//
// Implementation file
//
//    Copyright (c) 1997-1998 Mark Danks.
//    Copyright (c) Günther Geiger.
//    Copyright (c) 2001-2011 IOhannes m zmölnig. forum::für::umläute. IEM. zmoelnig@iem.at
//    For information on usage and redistribution, and for a DISCLAIMER OF ALL
//    WARRANTIES, see the file, "GEM.LICENSE.TERMS" in this distribution.
//
/////////////////////////////////////////////////////////

#include "circle.h"

#include <math.h>

#include "Gem/State.h"

#define NUM_PNTS 100
GLfloat *circle::m_cos = NULL;
GLfloat *circle::m_sin = NULL;

CPPEXTERN_NEW_WITH_ONE_ARG(circle, t_floatarg, A_DEFFLOAT);

/////////////////////////////////////////////////////////
//
// circle
//
/////////////////////////////////////////////////////////
// Constructor
//
/////////////////////////////////////////////////////////
circle :: circle(t_floatarg size)
        : GemShape(size)
{
    m_drawType = GL_POLYGON;
}

/////////////////////////////////////////////////////////
// Destructor
//
/////////////////////////////////////////////////////////
circle :: ~circle()
{ }

/////////////////////////////////////////////////////////
// renderShape
//
/////////////////////////////////////////////////////////
void circle :: renderShape(GemState *state)
{
  if(m_drawType==GL_DEFAULT_GEM)m_drawType=GL_POLYGON;
    glNormal3f(0.0f, 0.0f, 1.0f);
    glLineWidth(m_linewidth);
    glBegin(m_drawType);
	    if (GemShape::m_texType)
	    {
	      GLfloat xsize  = 1.f;
	      GLfloat ysize0 = 0.f;
	      GLfloat ysize  = 1.f;
	      if(GemShape::m_texNum>=3){
		xsize  = GemShape::m_texCoords[1].s;
		ysize0 = GemShape::m_texCoords[2].t;
		ysize  = GemShape::m_texCoords[1].t;
	      }
	        for (int n = 0; n < NUM_PNTS; n++)
	        {
		  glTexCoord2f(xsize*(m_cos[n] + 1) / 2.f, (ysize0-ysize)*(m_sin[n] + 1) / 2.f+ysize);
		        glVertex3f(m_cos[n] * m_size,
			           m_sin[n] * m_size,
			           0.0);
	        }
	    }
	    else
	    {
	        for (int n = 0; n < NUM_PNTS; n++)
		        glVertex3f(m_cos[n] * m_size,
			           m_sin[n] * m_size,
			           0.0);
	    }
    glEnd();
    glLineWidth(1.0);
}

/////////////////////////////////////////////////////////
// static member function
//
/////////////////////////////////////////////////////////
void circle :: obj_setupCallback(t_class *)
{
    m_cos = new GLfloat [NUM_PNTS];
    m_sin = new GLfloat [NUM_PNTS];
    double TWO_PI = 8. * atan(1.f);
    // compute sin/cos lookup table
    for(int i = 0; i < NUM_PNTS; i++)
    {
	    m_cos[i] = cos(TWO_PI * i / NUM_PNTS);
	    m_sin[i] = sin(TWO_PI * i / NUM_PNTS);
    }
}

