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

#include "translate.h"
#include "Gem/Exception.h"

CPPEXTERN_NEW_WITH_GIMME(translate);

/////////////////////////////////////////////////////////
//
// translate
//
/////////////////////////////////////////////////////////
// Constructor
//
/////////////////////////////////////////////////////////
translate :: translate(int argc, t_atom *argv)
{
    m_distance  = 0.0;
    if (argc == 4)
    {
        m_distance = atom_getfloat(&argv[0]);
        vectorMess(atom_getfloat(&argv[1]), atom_getfloat(&argv[2]), atom_getfloat(&argv[3]));
    }
    else if (argc == 3) vectorMess(atom_getfloat(&argv[0]), atom_getfloat(&argv[1]),
                                   atom_getfloat(&argv[2]));

    else if (argc == 0) vectorMess(1, 0, 0);
    else
    {
      throw(GemException("needs 0, 3, or 4 arguments"));
    }

    // create the new inlets
    inlet_new(this->x_obj, &this->x_obj->ob_pd, &s_float, gensym("ft1"));
    inlet_new(this->x_obj, &this->x_obj->ob_pd, &s_list, gensym("vector"));
}

/////////////////////////////////////////////////////////
// Destructor
//
/////////////////////////////////////////////////////////
translate :: ~translate()
{ }

/////////////////////////////////////////////////////////
// render
//
/////////////////////////////////////////////////////////
void translate :: render(GemState *)
{
    glTranslatef(m_vector[0] * m_distance, m_vector[1] * m_distance, m_vector[2] * m_distance);
}

/////////////////////////////////////////////////////////
// distanceMess
//
/////////////////////////////////////////////////////////
void translate :: distanceMess(float distance)
{
    m_distance = distance;
    setModified();
}

/////////////////////////////////////////////////////////
// vectorMess
//
/////////////////////////////////////////////////////////
void translate :: vectorMess(float x, float y, float z)
{
    m_vector[0] = x;
    m_vector[1] = y;
    m_vector[2] = z;
    setModified();
}

/////////////////////////////////////////////////////////
// static member function
//
/////////////////////////////////////////////////////////
void translate :: obj_setupCallback(t_class *classPtr)
{
    class_addmethod(classPtr, reinterpret_cast<t_method>(&translate::distanceMessCallback),
    	    gensym("ft1"), A_FLOAT, A_NULL); 
    class_addmethod(classPtr, reinterpret_cast<t_method>(&translate::vectorMessCallback),
    	    gensym("vector"), A_FLOAT, A_FLOAT, A_FLOAT, A_NULL); 
}
void translate :: distanceMessCallback(void *data, t_floatarg distance)
{
    GetMyClass(data)->distanceMess((float)distance);
}
void translate :: vectorMessCallback(void *data, t_floatarg x, t_floatarg y, t_floatarg z)
{
    GetMyClass(data)->vectorMess((float)x, (float)y, (float)z);
}

