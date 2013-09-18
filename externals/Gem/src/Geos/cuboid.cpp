////////////////////////////////////////////////////////
//
// GEM - Graphics Environment for Multimedia
//
// mark@danks.org
//
// Implementation file
//
//    Copyright (c) 1997-2000 Mark Danks.
//    For information on usage and redistribution, and for a DISCLAIMER OF ALL
//    WARRANTIES, see the file, "GEM.LICENSE.TERMS" in this distribution.
//
/////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////
//
// modified cube geos into cuboid by erich berger 2001 rat@telecoma.net
//
////////////////////////////////////////////////////////
////////////////////////////////////////////////////////

#include "cuboid.h"
#include "Gem/State.h"
#include "string.h"

CPPEXTERN_NEW_WITH_THREE_ARGS(cuboid, t_floatarg, A_DEFFLOAT, t_floatarg, A_DEFFLOAT, t_floatarg, A_DEFFLOAT );

/////////////////////////////////////////////////////////
//
// cuboid
//
/////////////////////////////////////////////////////////
// Constructor
//
/////////////////////////////////////////////////////////
cuboid :: cuboid(t_floatarg sizex, t_floatarg sizey, t_floatarg sizez)
  : GemShape(sizex), m_sizey(sizey), m_sizez(sizez)
{
  if (m_sizey == 0.f)
    m_sizey = 1.f;
  if (m_sizez == 0.f)
    m_sizez = 0.f;

  m_inletY = inlet_new(this->x_obj, &this->x_obj->ob_pd, &s_float, gensym("ft2"));
  m_inletZ = inlet_new(this->x_obj, &this->x_obj->ob_pd, &s_float, gensym("ft3"));
}

/////////////////////////////////////////////////////////
// Destructor
//
/////////////////////////////////////////////////////////
cuboid :: ~cuboid()
{
  inlet_free(m_inletY);
  inlet_free(m_inletZ);
}

/////////////////////////////////////////////////////////
// renderShape
//
/////////////////////////////////////////////////////////
void cuboid :: renderShape(GemState *state)
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
		        glVertex3d(v[faces[i][0]][0] * m_size, v[faces[i][0]][1] * m_sizey, v[faces[i][0]][2] * m_sizez);
		        glVertex3d(v[faces[i][1]][0] * m_size, v[faces[i][1]][1] * m_sizey, v[faces[i][1]][2] * m_sizez);
		        glVertex3d(v[faces[i][2]][0] * m_size, v[faces[i][2]][1] * m_sizey, v[faces[i][2]][2] * m_sizez);
		        glVertex3d(v[faces[i][3]][0] * m_size, v[faces[i][3]][1] * m_sizey, v[faces[i][3]][2] * m_sizez);
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
		        glVertex3f(v[faces[i][0]][0] * m_size, v[faces[i][0]][1] * m_sizey, v[faces[i][0]][2] * m_sizez);
				
				if (GemShape::m_texNum > 1)
					curCoord = 1;
	    		glTexCoord2f(GemShape::m_texCoords[curCoord].s, GemShape::m_texCoords[curCoord].t);
//    	        glTexCoord2f(1.0, 0.0);
		        glVertex3f(v[faces[i][1]][0] * m_size, v[faces[i][1]][1] * m_sizey, v[faces[i][1]][2] * m_sizez);
				
				if (GemShape::m_texNum > 2)
					curCoord = 2;
	    		glTexCoord2f(GemShape::m_texCoords[curCoord].s, GemShape::m_texCoords[curCoord].t);
//    	        glTexCoord2f(1.0, 1.0);
		        glVertex3f(v[faces[i][2]][0] * m_size, v[faces[i][2]][1] * m_sizey, v[faces[i][2]][2] * m_sizez);
				
				if (GemShape::m_texNum > 3)
					curCoord = 3;
				glTexCoord2f(GemShape::m_texCoords[curCoord].s, GemShape::m_texCoords[curCoord].t);
//    	        glTexCoord2f(0.0, 1.0);
		        glVertex3f(v[faces[i][3]][0] * m_size, v[faces[i][3]][1] * m_sizey, v[faces[i][3]][2] * m_sizez);
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
		        glVertex3f(v[faces[i][0]][0] * m_size, v[faces[i][0]][1] * m_sizey, v[faces[i][0]][2] * m_sizez);
    	        glTexCoord2f(1.0, 0.0);
		        glVertex3f(v[faces[i][1]][0] * m_size, v[faces[i][1]][1] * m_sizey, v[faces[i][1]][2] * m_sizez);
    	        glTexCoord2f(1.0, 1.0);
		        glVertex3f(v[faces[i][2]][0] * m_size, v[faces[i][2]][1] * m_sizey, v[faces[i][2]][2] * m_sizez);
    	        glTexCoord2f(0.0, 1.0);
		        glVertex3f(v[faces[i][3]][0] * m_size, v[faces[i][3]][1] * m_sizey, v[faces[i][3]][2] * m_sizez);
	        }
	    glEnd();
    }
}
/////////////////////////////////////////////////////////
// heightMess
//
/////////////////////////////////////////////////////////
void cuboid :: heightMess(float sizey)
{
    m_sizey = sizey;
    setModified();
}
/////////////////////////////////////////////////////////
// widthMess
//
/////////////////////////////////////////////////////////
void cuboid :: widthMess(float sizez)
{
    m_sizez = sizez;
    setModified();
}

/////////////////////////////////////////////////////////
// static member function
//
/////////////////////////////////////////////////////////
void cuboid :: obj_setupCallback(t_class *classPtr)
{
    class_addmethod(classPtr, reinterpret_cast<t_method>(&cuboid::heightMessCallback),
    	    gensym("ft2"), A_FLOAT, A_NULL);
    class_addmethod(classPtr, reinterpret_cast<t_method>(&cuboid::widthMessCallback),
	    gensym("ft3"), A_FLOAT, A_NULL);
}

void cuboid :: heightMessCallback(void *data, t_floatarg size)
{
    GetMyClass(data)->heightMess(size);  
}

void cuboid :: widthMessCallback(void *data, t_floatarg size)
{
    GetMyClass(data)->widthMess(size);  
}
