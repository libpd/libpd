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

#include "polygon.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "Gem/State.h"

CPPEXTERN_NEW_WITH_ONE_ARG(polygon, t_floatarg, A_DEFFLOAT);

/////////////////////////////////////////////////////////
//
// polygon
//
/////////////////////////////////////////////////////////
// Constructor
//
/////////////////////////////////////////////////////////
polygon :: polygon(t_floatarg numInputs)
  : GemShape(), 
    m_numVertices(0),
    m_vertarray(NULL),
    m_vert(NULL), 
    m_numInputs(0),
    m_inlet(NULL)
{
  int i;
  int realNum = static_cast<int>(numInputs);

  // configure the inlets
	if(realNum>0) {
		createVertices(realNum);
		
		m_numInputs=realNum;
		m_inlet=new t_inlet*[m_numInputs];
		
		char tempVt[7];
		// create the proper number of inputs
		for (i = 0; i < realNum; i++) {
			sprintf(tempVt, "%d", i+1);
			m_inlet[i]=inlet_new(this->x_obj, &this->x_obj->ob_pd, &s_list, gensym(tempVt) );
		}
	} else {
		logpost(NULL, 5, "variable number of vertices");
	}
}

/////////////////////////////////////////////////////////
// Destructor
//
/////////////////////////////////////////////////////////
polygon :: ~polygon()
{
  if(m_vert)
		delete[]m_vert;
  if(m_vertarray)
		delete[]m_vertarray;
	if(m_inlet) {
		int i=0;
		for(i=0; i<m_numInputs; i++) {
			inlet_free(m_inlet[i]);
			m_inlet[i]=NULL;
		}
		delete[]m_inlet;
	}
}


void polygon :: createVertices(int num) {
	if(m_vert)
		delete[]m_vert;
	if(m_vertarray)
		delete[]m_vertarray;
	m_numVertices=0;

		
	if(num>0) {
		int i=0;

		m_numVertices = num;
		m_vert = new float*[num];
		m_vertarray = new float[num*3];
		
		for (i = 0; i < num*3; i++)  {
			m_vertarray[i]=0.0f;
		}
		for (i = 0; i < num; i++)  {
			m_vert[i]=m_vertarray+3*i;
		}
	}
}

/////////////////////////////////////////////////////////
// render
//
/////////////////////////////////////////////////////////
void polygon :: renderShape(GemState *state)
{
  if(m_drawType==GL_DEFAULT_GEM)m_drawType=GL_POLYGON;

  glNormal3f(0.0f, 0.0f, 1.0f);
  if (GemShape::m_texType && GemShape::m_texNum)
    {
      glBegin(m_drawType);
      for (int i = 0; i < m_numVertices; i++)
        {
          if (GemShape::m_texNum < i)
            glTexCoord2f(GemShape::m_texCoords[GemShape::m_texNum - 1].s,
                         GemShape::m_texCoords[GemShape::m_texNum - 1].t);
          else
            glTexCoord2f(GemShape::m_texCoords[i].s,
                         GemShape::m_texCoords[i].t);
          glVertex3fv(m_vert[i]);
        }
      glEnd();
    }
  else
    {
      float maxVal[2];
      maxVal[0] = maxVal[1] = 0;
      if (GemShape::m_texType)
        {
          for (int i = 0; i < m_numVertices; i++)
            {
              for (int j = 0; j < 2; j++)
                {
                  if (m_vert[i][j] < 0)
                    {
                      if (-m_vert[i][j] > maxVal[j])
                        maxVal[j] = -m_vert[i][j];
                    }
                  else
                    {
                      if (m_vert[i][j] > maxVal[j])
                        maxVal[j] = m_vert[i][j];
                    }
                }
            }
        }
      glBegin(m_drawType);
      for(int n=0; n < m_numVertices; n++)
        {
          if (GemShape::m_texType)
            glTexCoord2f(m_vert[n][0] / maxVal[0],
                         m_vert[n][1] / maxVal[1]);
          glVertex3fv(m_vert[n]);
        }
      glEnd();
    }
}


void polygon :: listMess(int argc, t_atom*argv) {
	if(0==m_numInputs) {
		if(argc%3) {
			error("list must contain 3 elements for each vertex!");
			return;
		}
		createVertices(argc/3);
	}
	
	if(m_numVertices*3==argc) {
		int i=0;
		for(i=0; i<m_numVertices; i++) {
			setVert(i, 
							atom_getfloat(argv+0),
							atom_getfloat(argv+1),
							atom_getfloat(argv+2)
							);
			argv+=3;
		}
	} else {
		error("vertex-list must have exactly %d numbers", m_numVertices*3);
	}
}

/////////////////////////////////////////////////////////
// setVert
//
/////////////////////////////////////////////////////////
void polygon :: setVert(int whichOne, float x, float y, float z)
{
	if(whichOne>=0 && whichOne<m_numVertices) {
    m_vert[whichOne][0] = x;
    m_vert[whichOne][1] = y;
    m_vert[whichOne][2] = z;
    setModified();
	} else {
		error("cannot set vertex#%d of %d", whichOne, m_numVertices);
	}
}

/////////////////////////////////////////////////////////
// typeMess
//
/////////////////////////////////////////////////////////
void polygon :: typeMess(t_symbol *type)
{
    if (!strcmp(type->s_name, "default")) 
	    m_drawType = GL_DEFAULT_GEM;
    else if (!strcmp(type->s_name, "line")) 
	    m_drawType = GL_LINE_LOOP;
    else if (!strcmp(type->s_name, "fill")) 
	    m_drawType = GL_POLYGON;
    else if (!strcmp(type->s_name, "point"))
	    m_drawType = GL_POINTS;
    else if (!strcmp(type->s_name, "linestrip")) 
	    m_drawType = GL_LINE_STRIP;
    else if (!strcmp(type->s_name, "tri")) 
	    m_drawType = GL_TRIANGLES;
    else if (!strcmp(type->s_name, "tristrip")) 
	    m_drawType = GL_TRIANGLE_STRIP;
    else if (!strcmp(type->s_name, "trifan")) 
	    m_drawType = GL_TRIANGLE_FAN;
    else if (!strcmp(type->s_name, "quad")) 
	    m_drawType = GL_QUADS;
    else if (!strcmp(type->s_name, "quadstrip")) 
	    m_drawType = GL_QUAD_STRIP;
    else
    {
	    error ("unknown draw style");
	    return;
    }
    setModified();
}

/////////////////////////////////////////////////////////
// static member function
//
/////////////////////////////////////////////////////////
void polygon :: obj_setupCallback(t_class *classPtr)
{
	class_addlist(classPtr, reinterpret_cast<t_method>(&polygon::listCallback));
	class_addmethod(classPtr, reinterpret_cast<t_method>(&polygon::vertexCallback), gensym("vertex"), A_FLOAT, A_FLOAT, A_FLOAT, A_FLOAT, A_NULL);
	class_addanything(classPtr, reinterpret_cast<t_method>(&polygon::vertCallback));
}

void polygon :: listCallback(void *data, t_symbol*s, int argc, t_atom*argv)
{
	GetMyClass(data)->listMess(argc, argv);
}

void polygon :: vertexCallback(void *data, t_floatarg id, t_floatarg x, t_floatarg y, t_floatarg z)
{
	GetMyClass(data)->setVert(id, x, y, z);
}

void polygon :: vertCallback(void *data, t_symbol*s, int argc, t_atom*argv)
{
  int i = atoi(s->s_name);
  if (i>0 && argc==3){
    GetMyClass(data)->setVert(i-1, atom_getfloat(argv), atom_getfloat(argv+1), atom_getfloat(argv+2));
  }
}
