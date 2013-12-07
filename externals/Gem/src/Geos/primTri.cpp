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

#include "primTri.h"

#include "Utils/Matrix.h"
#include "Gem/State.h"
#include <string.h>

CPPEXTERN_NEW_WITH_ONE_ARG(primTri, t_floatarg, A_DEFFLOAT);

/////////////////////////////////////////////////////////
//
// primTri
//
/////////////////////////////////////////////////////////
// Constructor
//
/////////////////////////////////////////////////////////
primTri :: primTri(t_floatarg size)
  : GemShape()
{
	mVectors[0][0] = 0.f;
	mVectors[0][1] = 1.f;

	mVectors[1][0] = 1.f;
	mVectors[1][1] = -1.f;

	mVectors[2][0] = -1.f;
	mVectors[2][1] = -1.f;

	mVectors[0][2] = mVectors[1][2] = mVectors[2][2] = 0.f;

	mColors[0][0] = mColors[1][0] = mColors[2][0] = 1.f;
	mColors[0][1] = mColors[1][1] = mColors[2][1] = 1.f;
	mColors[0][2] = mColors[1][2] = mColors[2][2] = 1.f;
	mColors[0][3] = mColors[1][3] = mColors[2][3] = 1.f;

  // create the new inlets
  inlet_new(this->x_obj, &this->x_obj->ob_pd, &s_list, gensym("vect1"));
  inlet_new(this->x_obj, &this->x_obj->ob_pd, &s_list, gensym("vect2"));
  inlet_new(this->x_obj, &this->x_obj->ob_pd, &s_list, gensym("vect3"));
  inlet_new(this->x_obj, &this->x_obj->ob_pd, &s_list, gensym("col1"));
  inlet_new(this->x_obj, &this->x_obj->ob_pd, &s_list, gensym("col2"));
  inlet_new(this->x_obj, &this->x_obj->ob_pd, &s_list, gensym("col3"));
}

/////////////////////////////////////////////////////////
// Destructor
//
/////////////////////////////////////////////////////////
primTri :: ~primTri()
{ }

/////////////////////////////////////////////////////////
// render
//
/////////////////////////////////////////////////////////
void primTri :: renderShape(GemState *state)
{
  if(m_drawType==GL_DEFAULT_GEM)m_drawType=GL_TRIANGLES;
	float norm[3];
	Matrix::generateNormal(mVectors[0], mVectors[1], mVectors[2], norm);
    glNormal3fv(norm);
	if (!GemShape::m_lighting)
		glShadeModel(GL_SMOOTH);

    if (GemShape::m_texType && GemShape::m_texNum)
    {
        int curCoord = 0;
	    glBegin(m_drawType);
	        glTexCoord2f(GemShape::m_texCoords[curCoord].s, GemShape::m_texCoords[curCoord].t);
			glColor4fv(mColors[0]);
   	        glVertex3fv(mVectors[0]);

	        if (GemShape::m_texNum > 1)
				curCoord = 1;
	    	glTexCoord2f(GemShape::m_texCoords[curCoord].s, GemShape::m_texCoords[curCoord].t);
			glColor4fv(mColors[1]);
   	        glVertex3fv(mVectors[1]);

	        if (GemShape::m_texNum > 2)
				curCoord = 2;
	    	glTexCoord2f(GemShape::m_texCoords[curCoord].s, GemShape::m_texCoords[curCoord].t);
			glColor4fv(mColors[2]);
   	        glVertex3fv(mVectors[2]);
	    glEnd();
    }
    else
    {
	    glBegin(m_drawType);
    	        glTexCoord2f(0.f, 0.f);
 				glColor4fv(mColors[0]);
   				glVertex3fv(mVectors[0]);
    	        
				glTexCoord2f(1.f, 0.f);
 				glColor4fv(mColors[1]);
   				glVertex3fv(mVectors[1]);
    	        
				glTexCoord2f(.5f, 1.f);
 				glColor4fv(mColors[2]);
   				glVertex3fv(mVectors[2]);
	    glEnd();
    }

	if (!GemShape::m_lighting)
		glShadeModel(GL_FLAT);
}

/////////////////////////////////////////////////////////
// typeMess
//
/////////////////////////////////////////////////////////
void primTri :: typeMess(t_symbol *type)
{
    if (!strcmp(type->s_name, "line")) 
	    m_drawType = GL_LINE_LOOP;
    else if (!strcmp(type->s_name, "fill")) 
	    m_drawType = GL_TRIANGLES;
    else if (!strcmp(type->s_name, "point"))
	    m_drawType = GL_POINTS;
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
void primTri :: obj_setupCallback(t_class *classPtr)
{
    class_addmethod(classPtr, reinterpret_cast<t_method>(&primTri::vect1MessCallback),
    	    gensym("vect1"), A_FLOAT, A_FLOAT, A_FLOAT, A_NULL); 
    class_addmethod(classPtr, reinterpret_cast<t_method>(&primTri::vect2MessCallback),
    	    gensym("vect2"), A_FLOAT, A_FLOAT, A_FLOAT, A_NULL); 
    class_addmethod(classPtr, reinterpret_cast<t_method>(&primTri::vect3MessCallback),
    	    gensym("vect3"), A_FLOAT, A_FLOAT, A_FLOAT, A_NULL); 
    class_addmethod(classPtr, reinterpret_cast<t_method>(&primTri::col1MessCallback),
    	    gensym("col1"), A_GIMME, A_NULL); 
    class_addmethod(classPtr, reinterpret_cast<t_method>(&primTri::col2MessCallback),
    	    gensym("col2"), A_GIMME, A_NULL); 
    class_addmethod(classPtr, reinterpret_cast<t_method>(&primTri::col3MessCallback),
    	    gensym("col3"), A_GIMME, A_NULL); 
}

void primTri :: vect1MessCallback(void *data, t_floatarg x, t_floatarg y, t_floatarg z)
{
    GetMyClass(data)->vectMess(0, x, y, z);
}
void primTri :: vect2MessCallback(void *data, t_floatarg x, t_floatarg y, t_floatarg z)
{
    GetMyClass(data)->vectMess(1, x, y, z);
}
void primTri :: vect3MessCallback(void *data, t_floatarg x, t_floatarg y, t_floatarg z)
{
    GetMyClass(data)->vectMess(2, x, y, z);
}

void primTri :: col1MessCallback(void *data, t_symbol *, int argc, t_atom *argv)
{
    float alpha = 1.f;
    if (argc == 4)
		alpha = atom_getfloat(&argv[3]);
    GetMyClass(data)->colMess(0, atom_getfloat(&argv[0]), atom_getfloat(&argv[1]),
    	    	    	       atom_getfloat(&argv[2]), alpha);
}
void primTri :: col2MessCallback(void *data, t_symbol *, int argc, t_atom *argv)
{
    float alpha = 1.f;
    if (argc == 4)
		alpha = atom_getfloat(&argv[3]);
    GetMyClass(data)->colMess(1, atom_getfloat(&argv[0]), atom_getfloat(&argv[1]),
    	    	    	       atom_getfloat(&argv[2]), alpha);
}
void primTri :: col3MessCallback(void *data, t_symbol *, int argc, t_atom *argv)
{
    float alpha = 1.f;
    if (argc == 4)
		alpha = atom_getfloat(&argv[3]);
    GetMyClass(data)->colMess(2, atom_getfloat(&argv[0]), atom_getfloat(&argv[1]),
    	    	    	       atom_getfloat(&argv[2]), alpha);
}
