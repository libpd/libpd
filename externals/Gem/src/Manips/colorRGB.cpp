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

#include "colorRGB.h"
#include "Gem/Exception.h"

CPPEXTERN_NEW_WITH_GIMME(colorRGB);

/////////////////////////////////////////////////////////
//
// colorRGB
//
/////////////////////////////////////////////////////////
// Constructor
//
/////////////////////////////////////////////////////////
colorRGB :: colorRGB(int argc, t_atom *argv)
{
    if (argc == 4)
    {
        m_vector[0] = atom_getfloat(&argv[0]);
        m_vector[1] = atom_getfloat(&argv[1]);
        m_vector[2] = atom_getfloat(&argv[2]);
        m_vector[3] = atom_getfloat(&argv[3]);
    }
    else if (argc == 3)
    {
        m_vector[0] = atom_getfloat(&argv[0]);
        m_vector[1] = atom_getfloat(&argv[1]);
        m_vector[2] = atom_getfloat(&argv[2]);
        m_vector[3] = 1;
    }
    else if (argc == 0)
    {
        m_vector[0] = 1.f;
        m_vector[1] = 1.f;
        m_vector[2] = 1.f;
        m_vector[3] = 1.f;
    }
    else
    {
      throw(GemException("needs 0, 3 or 4 arguments"));
    }

    // create the new inlets
    inlet_new(this->x_obj, &this->x_obj->ob_pd, &s_float, gensym("rVal"));
    inlet_new(this->x_obj, &this->x_obj->ob_pd, &s_float, gensym("gVal"));
    inlet_new(this->x_obj, &this->x_obj->ob_pd, &s_float, gensym("bVal"));
    inlet_new(this->x_obj, &this->x_obj->ob_pd, &s_float, gensym("aVal"));
}

/////////////////////////////////////////////////////////
// Destructor
//
/////////////////////////////////////////////////////////
colorRGB :: ~colorRGB()
{ }

/////////////////////////////////////////////////////////
// render
//
/////////////////////////////////////////////////////////
void colorRGB :: render(GemState *)
{
    glColor4f(m_vector[0], m_vector[1], m_vector[2], m_vector[3]);
}

/////////////////////////////////////////////////////////
// rMess
//
/////////////////////////////////////////////////////////
void colorRGB :: rMess(float val)
{
    m_vector[0] = val;
    setModified();
}

/////////////////////////////////////////////////////////
// gMess
//
/////////////////////////////////////////////////////////
void colorRGB :: gMess(float val)
{
    m_vector[1] = val;
    setModified();
}

/////////////////////////////////////////////////////////
// bMess
//
/////////////////////////////////////////////////////////
void colorRGB :: bMess(float val)
{
    m_vector[2] = val;
    setModified();
}

/////////////////////////////////////////////////////////
// aMess
//
/////////////////////////////////////////////////////////
void colorRGB :: aMess(float val)
{
    m_vector[3] = val;
    setModified();
}

/////////////////////////////////////////////////////////
// static member function
//
/////////////////////////////////////////////////////////
void colorRGB :: obj_setupCallback(t_class *classPtr)
{
  class_addcreator(reinterpret_cast<t_newmethod>(create_colorRGB), 
		   gensym("colourRGB"), A_GIMME, A_NULL);
    class_addmethod(classPtr, reinterpret_cast<t_method>(&colorRGB::rMessCallback),
    	    gensym("rVal"), A_FLOAT, A_NULL); 
    class_addmethod(classPtr, reinterpret_cast<t_method>(&colorRGB::gMessCallback),
    	    gensym("gVal"), A_FLOAT, A_NULL); 
    class_addmethod(classPtr, reinterpret_cast<t_method>(&colorRGB::bMessCallback),
    	    gensym("bVal"), A_FLOAT, A_NULL); 
    class_addmethod(classPtr, reinterpret_cast<t_method>(&colorRGB::aMessCallback),
    	    gensym("aVal"), A_FLOAT, A_NULL); 
}
void colorRGB :: rMessCallback(void *data, t_floatarg val)
{
    GetMyClass(data)->rMess((float)val);
}
void colorRGB :: gMessCallback(void *data, t_floatarg val)
{
    GetMyClass(data)->gMess((float)val);
}
void colorRGB :: bMessCallback(void *data, t_floatarg val)
{
    GetMyClass(data)->bMess((float)val);
}
void colorRGB :: aMessCallback(void *data, t_floatarg val)
{
    GetMyClass(data)->aMess((float)val);
}
