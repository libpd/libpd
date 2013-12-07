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

#include "rectangle.h"

#include "Gem/State.h"
#include <string.h>

CPPEXTERN_NEW_WITH_TWO_ARGS(rectangle, t_floatarg, A_DEFFLOAT, t_floatarg, A_DEFFLOAT);

/////////////////////////////////////////////////////////
//
// rectangle
//
/////////////////////////////////////////////////////////
// Constructor
//
/////////////////////////////////////////////////////////
rectangle :: rectangle(t_floatarg width, t_floatarg height)
		   : GemShape(width), m_height(height)
{
    if (m_height == 0.f)
		m_height = 1.f;

    // the height inlet
    m_inletH = inlet_new(this->x_obj, &this->x_obj->ob_pd, &s_float, gensym("ft2"));
}

/////////////////////////////////////////////////////////
// Destructor
//
/////////////////////////////////////////////////////////
rectangle :: ~rectangle()
{
    inlet_free(m_inletH);
}

/////////////////////////////////////////////////////////
// renderShape
//
/////////////////////////////////////////////////////////
void rectangle :: renderShape(GemState *state)
{
#ifdef __GNUC__
# warning rectangle: look at SetVertex
#endif /* __GNUC__ */

  if(m_drawType==GL_DEFAULT_GEM)m_drawType=GL_QUADS;

  glNormal3f(0.0f, 0.0f, 1.0f);

  if (GemShape::m_texType && GemShape::m_texNum)
		{
			glBegin(m_drawType);
			SetVertex(state, -m_size,  -m_height, 0.0f,0.,0.,0);
			SetVertex(state, m_size,  -m_height, 0.0f,1.,0.,1);
			SetVertex(state, m_size,  m_height, 0.0f,1.,1.,2);
			SetVertex(state, -m_size,  m_height, 0.0f,0.,1.,3);
			glEnd();
		}
  else
    {
	    glBegin(m_drawType);
	        glTexCoord2f(0.0f, 0.0f);
                glVertex3f(-m_size, -m_height, 0.0f);
	        glTexCoord2f(1.0f, 0.0f);
                glVertex3f( m_size, -m_height, 0.0f);
	        glTexCoord2f(1.0f, 1.0f);
                glVertex3f( m_size,  m_height, 0.0f);
	        glTexCoord2f(0.0f, 1.0f);
                glVertex3f(-m_size,  m_height, 0.0f);
	    glEnd();
    }
}

/////////////////////////////////////////////////////////
// heightMess
//
/////////////////////////////////////////////////////////
void rectangle :: heightMess(float size)
{
    m_height = size;
    setModified();
}

/////////////////////////////////////////////////////////
// typeMess
//
/////////////////////////////////////////////////////////
void rectangle :: typeMess(t_symbol *type)
{
    if (!strcmp(type->s_name, "line")) 
	    m_drawType = GL_LINE_LOOP;
    else if (!strcmp(type->s_name, "fill")) 
	    m_drawType = GL_QUADS;
    else if (!strcmp(type->s_name, "point"))
	    m_drawType = GL_POINTS;
    else
    {
	    error ("GEM: square draw style");
	    return;
    }
    setModified();
}

/////////////////////////////////////////////////////////
// static member function
//
/////////////////////////////////////////////////////////
void rectangle :: obj_setupCallback(t_class *classPtr)
{
    class_addmethod(classPtr, reinterpret_cast<t_method>(&rectangle::heightMessCallback),
    	    gensym("ft2"), A_FLOAT, A_NULL);
}

void rectangle :: heightMessCallback(void *data, t_floatarg size)
{
    GetMyClass(data)->heightMess(size);
}
