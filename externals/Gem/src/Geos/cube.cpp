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

#include "cube.h"
#include "Gem/State.h"

CPPEXTERN_NEW_WITH_ONE_ARG(cube, t_floatarg, A_DEFFLOAT);

/////////////////////////////////////////////////////////
//
// cube
//
/////////////////////////////////////////////////////////
// Constructor
//
/////////////////////////////////////////////////////////
cube :: cube(t_floatarg size)
      : GemShape(size)
{}

/////////////////////////////////////////////////////////
// Destructor
//
/////////////////////////////////////////////////////////
cube :: ~cube()
{ }

/////////////////////////////////////////////////////////
// renderShape
//
/////////////////////////////////////////////////////////
void cube :: renderShape(GemState *state)
{
  if(m_drawType==GL_DEFAULT_GEM)m_drawType=GL_QUADS;
    static GLfloat n[6][3] =
    {
	{ 0.0f, 0.0f, 1.0f}, {1.0f, 0.0f, 0.0f}, {0.0f,  0.0f, -1.0f},
	{-1.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, -1.0f,  0.0f}
    };
    static GLfloat v[8][3] =
    {
	{-1.0f, -1.0f,  1.0f}, { 1.0f, -1.0f,  1.0f}, { 1.0f, 1.0f,  1.0f}, {-1.0f, 1.0f,  1.0f},
	{ 1.0f, -1.0f, -1.0f}, {-1.0f, -1.0f, -1.0f}, {-1.0f, 1.0f, -1.0f}, { 1.0f, 1.0f, -1.0f}
    };
    static GLint faces[6][4] =
    {
	{ 0, 1, 2, 3 }, { 1, 4, 7, 2 }, { 4, 5, 6, 7 },
	{ 5, 0, 3, 6 }, { 3, 2, 7, 6 }, { 1, 0, 5, 4 }
    };
    if (m_drawType == GL_LINE_LOOP)
    {
	        for (int i = 0; i < 6; i++)
	        {
	            glBegin(m_drawType);
		        glNormal3f(0.0f, 0.0f, 1.0f);
		        glVertex3d(v[faces[i][0]][0] * m_size, v[faces[i][0]][1] * m_size, v[faces[i][0]][2] * m_size);
		        glVertex3d(v[faces[i][1]][0] * m_size, v[faces[i][1]][1] * m_size, v[faces[i][1]][2] * m_size);
		        glVertex3d(v[faces[i][2]][0] * m_size, v[faces[i][2]][1] * m_size, v[faces[i][2]][2] * m_size);
		        glVertex3d(v[faces[i][3]][0] * m_size, v[faces[i][3]][1] * m_size, v[faces[i][3]][2] * m_size);
	            glEnd();
	        }
	    glLineWidth(1.0);
    }
	else if (GemShape::m_texType && GemShape::m_texNum)
    {
	    glBegin(m_drawType);
	        for (int i = 0; i < 6; i++)
	        {
				int curCoord = 0;
	            glNormal3fv(&n[i][0]);
				glTexCoord2f(GemShape::m_texCoords[curCoord].s, GemShape::m_texCoords[curCoord].t);
//    	        glTexCoord2f(0.0, 0.0);
		        glVertex3f(v[faces[i][0]][0] * m_size, v[faces[i][0]][1] * m_size, v[faces[i][0]][2] * m_size);
				
				if (GemShape::m_texNum > 1)
					curCoord = 1;
	    		glTexCoord2f(GemShape::m_texCoords[curCoord].s, GemShape::m_texCoords[curCoord].t);
//    	        glTexCoord2f(1.0, 0.0);
		        glVertex3f(v[faces[i][1]][0] * m_size, v[faces[i][1]][1] * m_size, v[faces[i][1]][2] * m_size);
				
				if (GemShape::m_texNum > 2)
					curCoord = 2;
	    		glTexCoord2f(GemShape::m_texCoords[curCoord].s, GemShape::m_texCoords[curCoord].t);
//    	        glTexCoord2f(1.0, 1.0);
		        glVertex3f(v[faces[i][2]][0] * m_size, v[faces[i][2]][1] * m_size, v[faces[i][2]][2] * m_size);
				
				if (GemShape::m_texNum > 3)
					curCoord = 3;
				glTexCoord2f(GemShape::m_texCoords[curCoord].s, GemShape::m_texCoords[curCoord].t);
//    	        glTexCoord2f(0.0, 1.0);
		        glVertex3f(v[faces[i][3]][0] * m_size, v[faces[i][3]][1] * m_size, v[faces[i][3]][2] * m_size);
	        }
	    glEnd();
	}
    else
    {
	    glBegin(m_drawType);
	        for (int i = 0; i < 6; i++)
	        {
	            glNormal3fv(&n[i][0]);
    	        glTexCoord2f(0.0, 0.0);
		        glVertex3f(v[faces[i][0]][0] * m_size, v[faces[i][0]][1] * m_size, v[faces[i][0]][2] * m_size);
    	        glTexCoord2f(1.0, 0.0);
		        glVertex3f(v[faces[i][1]][0] * m_size, v[faces[i][1]][1] * m_size, v[faces[i][1]][2] * m_size);
    	        glTexCoord2f(1.0, 1.0);
		        glVertex3f(v[faces[i][2]][0] * m_size, v[faces[i][2]][1] * m_size, v[faces[i][2]][2] * m_size);
    	        glTexCoord2f(0.0, 1.0);
		        glVertex3f(v[faces[i][3]][0] * m_size, v[faces[i][3]][1] * m_size, v[faces[i][3]][2] * m_size);
	        }
	    glEnd();
    }
}

/////////////////////////////////////////////////////////
// static member function
//
/////////////////////////////////////////////////////////
void cube :: obj_setupCallback(t_class *classPtr)
{ }
