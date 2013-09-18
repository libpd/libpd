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

#include "scaleXYZ.h"
#include "Gem/Exception.h"

CPPEXTERN_NEW_WITH_GIMME(scaleXYZ);

/////////////////////////////////////////////////////////
//
// scaleXYZ
//
/////////////////////////////////////////////////////////
// Constructor
//
/////////////////////////////////////////////////////////
scaleXYZ :: scaleXYZ(int argc, t_atom *argv)
{
    if (argc == 3)
    {
        m_vector[0] = atom_getfloat(&argv[0]);
        m_vector[1] = atom_getfloat(&argv[1]);
        m_vector[2] = atom_getfloat(&argv[2]);
    }
    else if (argc == 1)
    {
        m_vector[0] = atom_getfloat(&argv[0]);
        m_vector[1] = atom_getfloat(&argv[0]);
        m_vector[2] = atom_getfloat(&argv[0]);
    }
    else if (argc == 0)
    {
        m_vector[0] = 1.f;
        m_vector[1] = 1.f;
        m_vector[2] = 1.f;
    }
    else
    {
      throw(GemException("needs 0, 1, or 3 arguments"));
    }

    // create the new inlets
    inlet_new(this->x_obj, &this->x_obj->ob_pd, &s_float, gensym("xVal"));
    inlet_new(this->x_obj, &this->x_obj->ob_pd, &s_float, gensym("yVal"));
    inlet_new(this->x_obj, &this->x_obj->ob_pd, &s_float, gensym("zVal"));
}

/////////////////////////////////////////////////////////
// Destructor
//
/////////////////////////////////////////////////////////
scaleXYZ :: ~scaleXYZ()
{ }

/////////////////////////////////////////////////////////
// render
//
/////////////////////////////////////////////////////////
void scaleXYZ :: render(GemState *)
{
    glScalef(m_vector[0], m_vector[1], m_vector[2]);
}

/////////////////////////////////////////////////////////
// xMess
//
/////////////////////////////////////////////////////////
void scaleXYZ :: xMess(float val)
{
    m_vector[0] = val;
    setModified();
}

/////////////////////////////////////////////////////////
// yMess
//
/////////////////////////////////////////////////////////
void scaleXYZ :: yMess(float val)
{
    m_vector[1] = val;
    setModified();
}

/////////////////////////////////////////////////////////
// zMess
//
/////////////////////////////////////////////////////////
void scaleXYZ :: zMess(float val)
{
    m_vector[2] = val;
    setModified();
}

/////////////////////////////////////////////////////////
// static member function
//
/////////////////////////////////////////////////////////
void scaleXYZ :: obj_setupCallback(t_class *classPtr)
{
    class_addmethod(classPtr, reinterpret_cast<t_method>(&scaleXYZ::xMessCallback),
    	    gensym("xVal"), A_FLOAT, A_NULL); 
    class_addmethod(classPtr, reinterpret_cast<t_method>(&scaleXYZ::yMessCallback),
    	    gensym("yVal"), A_FLOAT, A_NULL); 
    class_addmethod(classPtr, reinterpret_cast<t_method>(&scaleXYZ::zMessCallback),
    	    gensym("zVal"), A_FLOAT, A_NULL); 
}
void scaleXYZ :: xMessCallback(void *data, t_floatarg val)
{
    GetMyClass(data)->xMess((float)val);
}
void scaleXYZ :: yMessCallback(void *data, t_floatarg val)
{
    GetMyClass(data)->yMess((float)val);
}
void scaleXYZ :: zMessCallback(void *data, t_floatarg val)
{
    GetMyClass(data)->zMess((float)val);
}

