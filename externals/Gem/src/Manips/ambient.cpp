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

#include "ambient.h"
#include "Gem/Exception.h"

CPPEXTERN_NEW_WITH_GIMME(ambient);

/////////////////////////////////////////////////////////
//
// ambient
//
/////////////////////////////////////////////////////////
// Constructor
//
/////////////////////////////////////////////////////////
ambient :: ambient(int argc, t_atom *argv)
{
    if (argc == 4) ambientMess(atom_getfloat(&argv[0]), atom_getfloat(&argv[1]),
    	    	    	     atom_getfloat(&argv[2]), atom_getfloat(&argv[3]));
    else if (argc == 3) ambientMess(atom_getfloat(&argv[0]), atom_getfloat(&argv[1]),
    	    	    	          atom_getfloat(&argv[2]), 1.f);
    else if (argc == 0) ambientMess(0.2f, 0.2f, 0.2f, 1.f);
    else
    {
      throw(GemException("needs 0, 3, or 4 arguments"));
    }

    // create the new inlet
    inlet_new(this->x_obj, &this->x_obj->ob_pd, &s_list, gensym("ambient"));
}

/////////////////////////////////////////////////////////
// Destructor
//
/////////////////////////////////////////////////////////
ambient :: ~ambient()
{ }

/////////////////////////////////////////////////////////
// postrender
//
/////////////////////////////////////////////////////////
void ambient :: postrender(GemState *)
{
	glEnable(GL_COLOR_MATERIAL);
}

/////////////////////////////////////////////////////////
// render
//
/////////////////////////////////////////////////////////
void ambient :: render(GemState *)
{
	glDisable(GL_COLOR_MATERIAL);
	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, m_ambient);
}

/////////////////////////////////////////////////////////
// ambientMess
//
/////////////////////////////////////////////////////////
void ambient :: ambientMess(float red, float green, float blue, float alpha)
{
    m_ambient[0] = red;
    m_ambient[1] = green;
    m_ambient[2] = blue;
    m_ambient[3] = alpha;
    setModified();
}

/////////////////////////////////////////////////////////
// static member function
//
/////////////////////////////////////////////////////////
void ambient :: obj_setupCallback(t_class *classPtr)
{
    class_addmethod(classPtr, reinterpret_cast<t_method>(&ambient::ambientMessCallback),
    	    gensym("ambient"), A_GIMME, A_NULL); 
}
void ambient :: ambientMessCallback(void *data, t_symbol *, int argc, t_atom *argv)
{
    float alpha = 1.0;
    if (argc == 4) alpha = atom_getfloat(&argv[3]);
    GetMyClass(data)->ambientMess(atom_getfloat(&argv[0]), atom_getfloat(&argv[1]),
    	    	    	       atom_getfloat(&argv[2]), alpha);
}

