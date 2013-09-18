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

#include "triangle.h"
#include <string.h>

#include "Gem/State.h"

CPPEXTERN_NEW_WITH_ONE_ARG(triangle, t_floatarg, A_DEFFLOAT);

/////////////////////////////////////////////////////////
//
// triangle
//
/////////////////////////////////////////////////////////
// Constructor
//
/////////////////////////////////////////////////////////
triangle :: triangle(t_floatarg size)
          : GemShape(size)
{
    m_drawType = GL_TRIANGLES;
}

/////////////////////////////////////////////////////////
// Destructor
//
/////////////////////////////////////////////////////////
triangle :: ~triangle()
{ }

/////////////////////////////////////////////////////////
// renderShape
//
/////////////////////////////////////////////////////////
void triangle :: renderShape(GemState *state)
{
  if(m_drawType==GL_DEFAULT_GEM)m_drawType=GL_TRIANGLES;
    glNormal3f(0.0f, 0.0f, 1.0f);
    if (m_drawType == GL_LINE_LOOP)
        glLineWidth(m_linewidth);

    if (GemShape::m_texType && GemShape::m_texNum)
    {
        int curCoord = 0;
	    glBegin(m_drawType);
	        glTexCoord2f(GemShape::m_texCoords[curCoord].s, GemShape::m_texCoords[curCoord].t);
   	        glVertex3f(-m_size, -m_size, 0.f);

	        if (GemShape::m_texNum > 1) curCoord = 1;
	    	glTexCoord2f(GemShape::m_texCoords[curCoord].s, GemShape::m_texCoords[curCoord].t);
   	        glVertex3f(m_size, -m_size, 0.f);

	        if (GemShape::m_texNum > 2) curCoord = 2;
	    	glTexCoord2f(GemShape::m_texCoords[curCoord].s, GemShape::m_texCoords[curCoord].t);
   	        glVertex3f(0.f,	m_size, 0.f);
	    glEnd();
    }
    else
    {
	    glBegin(m_drawType);
    	        glTexCoord2f(0.f, 0.f); glVertex3f(-m_size, -m_size, 0.f);
    	        glTexCoord2f(1.f, 0.f); glVertex3f( m_size, -m_size, 0.f);
    	        glTexCoord2f(.5f, 1.f); glVertex3f( 0.f,     m_size, 0.f);
	    glEnd();
    }
    if (m_drawType == GL_LINE_LOOP)
        glLineWidth(1.0);
}

/////////////////////////////////////////////////////////
// static member function
//
/////////////////////////////////////////////////////////
void triangle :: obj_setupCallback(t_class *)
{ }

