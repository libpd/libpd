////////////////////////////////////////////////////////
//
// GEM - Graphics Environment for Multimedia
//
// zmoelnig@iem.kug.ac.at
//
// Implementation file
//
//    Copyright (c) 1997-1999 Mark Danks.
//    Copyright (c) Günther Geiger.
//    Copyright (c) 2001-2011 IOhannes m zmölnig. forum::für::umläute. IEM. zmoelnig@iem.at
//    For information on usage and redistribution, and for a DISCLAIMER OF ALL
//    WARRANTIES, see the file, "GEM.LICENSE.TERMS" in this distribution.
//
/////////////////////////////////////////////////////////

#include "colorSquare.h"
#include <string.h>

#include "Gem/State.h"

CPPEXTERN_NEW_WITH_ONE_ARG(colorSquare, t_floatarg, A_DEFFLOAT);

/////////////////////////////////////////////////////////
//
// colorSquare
//
/////////////////////////////////////////////////////////
// Constructor
//
/////////////////////////////////////////////////////////
colorSquare :: colorSquare(t_floatarg size)
             : GemShape(size)
{
    m_drawType = GL_QUADS;

    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 3; j++)
            m_color[i][j] = 1.f;
    }

    inlet_new(this->x_obj, &this->x_obj->ob_pd, &s_list, gensym("vert0"));
    inlet_new(this->x_obj, &this->x_obj->ob_pd, &s_list, gensym("vert1"));
    inlet_new(this->x_obj, &this->x_obj->ob_pd, &s_list, gensym("vert2"));
    inlet_new(this->x_obj, &this->x_obj->ob_pd, &s_list, gensym("vert3"));
}

/////////////////////////////////////////////////////////
// Destructor
//
/////////////////////////////////////////////////////////
colorSquare :: ~colorSquare()
{ }

/////////////////////////////////////////////////////////
// renderShape
//
/////////////////////////////////////////////////////////
void colorSquare :: renderShape(GemState *state)
{
  if(m_drawType==GL_DEFAULT_GEM)m_drawType=GL_QUADS;

  if (!GemShape::m_lighting) glShadeModel(GL_SMOOTH);

  glNormal3f(0.0f, 0.0f, 1.0f);
    if (GemShape::m_texType && GemShape::m_texNum)
    {
        int curCoord = 0;

	    glBegin(m_drawType);
	    	glTexCoord2f(GemShape::m_texCoords[curCoord].s, GemShape::m_texCoords[curCoord].t);
            glColor3fv(m_color[0]);
                glVertex3f(-m_size, -m_size, 0.0f);

	        if (GemShape::m_texNum > 1) curCoord = 1;
	    	glTexCoord2f(GemShape::m_texCoords[curCoord].s, GemShape::m_texCoords[curCoord].t);
            glColor3fv(m_color[1]);
                glVertex3f( m_size, -m_size, 0.0f);

	        if (GemShape::m_texNum > 2) curCoord = 2;
	    	glTexCoord2f(GemShape::m_texCoords[curCoord].s, GemShape::m_texCoords[curCoord].t);
            glColor3fv(m_color[2]);
                glVertex3f( m_size,  m_size, 0.0f);

	        if (GemShape::m_texNum > 3) curCoord = 3;
	    	glTexCoord2f(GemShape::m_texCoords[curCoord].s, GemShape::m_texCoords[curCoord].t);
            glColor3fv(m_color[3]);
                glVertex3f(-m_size,  m_size, 0.0f);
	    glEnd();
    }
    else
    {
	    glBegin(m_drawType);
            glColor3fv(m_color[0]);
	        glTexCoord2f(0.0f, 0.0f);
                glVertex3f(-m_size, -m_size, 0.0f);
            glColor3fv(m_color[1]);
	        glTexCoord2f(1.0f, 0.0f);
                glVertex3f( m_size, -m_size, 0.0f);
            glColor3fv(m_color[2]);
	        glTexCoord2f(1.0f, 1.0f);
                glVertex3f( m_size,  m_size, 0.0f);
            glColor3fv(m_color[3]);
	        glTexCoord2f(0.0f, 1.0f);
                glVertex3f(-m_size,  m_size, 0.0f);
	    glEnd();
    }
}

/////////////////////////////////////////////////////////
// postrenderShape
//
/////////////////////////////////////////////////////////
void colorSquare :: postrenderShape(GemState *state)
{
    if (!GemShape::m_lighting) glShadeModel(GL_FLAT);
}

/////////////////////////////////////////////////////////
// vertColorMess
//
/////////////////////////////////////////////////////////
void colorSquare :: vertColorMess(int whichVert, float r, float g, float b)
{
    m_color[whichVert][0] = r;
    m_color[whichVert][1] = g;
    m_color[whichVert][2] = b;
}

/////////////////////////////////////////////////////////
// static member function
//
/////////////////////////////////////////////////////////
void colorSquare :: obj_setupCallback(t_class *classPtr)
{
    class_addmethod(classPtr, reinterpret_cast<t_method>(&colorSquare::vert0MessCallback),
    	    gensym("vert0"), A_FLOAT, A_FLOAT, A_FLOAT, A_NULL);
    class_addmethod(classPtr, reinterpret_cast<t_method>(&colorSquare::vert1MessCallback),
    	    gensym("vert1"), A_FLOAT, A_FLOAT, A_FLOAT, A_NULL);
    class_addmethod(classPtr, reinterpret_cast<t_method>(&colorSquare::vert2MessCallback),
    	    gensym("vert2"), A_FLOAT, A_FLOAT, A_FLOAT, A_NULL);
    class_addmethod(classPtr, reinterpret_cast<t_method>(&colorSquare::vert3MessCallback),
    	    gensym("vert3"), A_FLOAT, A_FLOAT, A_FLOAT, A_NULL);
}
void colorSquare :: vert0MessCallback(void *data, t_floatarg r, t_floatarg g, t_floatarg b)
{
    GetMyClass(data)->vertColorMess(0, r, g, b);
}
void colorSquare :: vert1MessCallback(void *data, t_floatarg r, t_floatarg g, t_floatarg b)
{
    GetMyClass(data)->vertColorMess(1, r, g, b);
}
void colorSquare :: vert2MessCallback(void *data, t_floatarg r, t_floatarg g, t_floatarg b)
{
    GetMyClass(data)->vertColorMess(2, r, g, b);
}
void colorSquare :: vert3MessCallback(void *data, t_floatarg r, t_floatarg g, t_floatarg b)
{
    GetMyClass(data)->vertColorMess(3, r, g, b);
}
