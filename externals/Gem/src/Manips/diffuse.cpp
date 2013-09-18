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

#include "diffuse.h"
#include "Gem/Exception.h"

CPPEXTERN_NEW_WITH_GIMME(diffuse);

/////////////////////////////////////////////////////////
//
// diffuse
//
/////////////////////////////////////////////////////////
// Constructor
//
/////////////////////////////////////////////////////////
diffuse :: diffuse(int argc, t_atom *argv)
{
    if (argc == 4) diffuseMess(atom_getfloat(&argv[0]), atom_getfloat(&argv[1]),
    	    	    	     atom_getfloat(&argv[2]), atom_getfloat(&argv[3]));
    else if (argc == 3) diffuseMess(atom_getfloat(&argv[0]), atom_getfloat(&argv[1]),
    	    	    	          atom_getfloat(&argv[2]), 1.f);
    else if (argc == 0) diffuseMess(0.8f, 0.8f, 0.8f, 1.f);
    else
    {
      throw(GemException("needs 0, 3, or 4 arguments"));
    }

    // create the new inlet
    inlet_new(this->x_obj, &this->x_obj->ob_pd, &s_list, gensym("diffuse"));
}

/////////////////////////////////////////////////////////
// Destructor
//
/////////////////////////////////////////////////////////
diffuse :: ~diffuse()
{ }

/////////////////////////////////////////////////////////
// postrender
//
/////////////////////////////////////////////////////////
void diffuse :: postrender(GemState *)
{
	glEnable(GL_COLOR_MATERIAL);
}

/////////////////////////////////////////////////////////
// render
//
/////////////////////////////////////////////////////////
void diffuse :: render(GemState *)
{
	glDisable(GL_COLOR_MATERIAL);
	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, m_diffuse);
}

/////////////////////////////////////////////////////////
// diffuseMess
//
/////////////////////////////////////////////////////////
void diffuse :: diffuseMess(float red, float green, float blue, float alpha)
{
    m_diffuse[0] = red;
    m_diffuse[1] = green;
    m_diffuse[2] = blue;
    m_diffuse[3] = alpha;
    setModified();
}

/////////////////////////////////////////////////////////
// static member function
//
/////////////////////////////////////////////////////////
void diffuse :: obj_setupCallback(t_class *classPtr)
{
    class_addmethod(classPtr, reinterpret_cast<t_method>(&diffuse::diffuseMessCallback),
    	    gensym("diffuse"), A_GIMME, A_NULL); 
}
void diffuse :: diffuseMessCallback(void *data, t_symbol *, int argc, t_atom *argv)
{
    float alpha = 1;
    if (argc == 4) alpha = atom_getfloat(&argv[3]);
    GetMyClass(data)->diffuseMess(atom_getfloat(&argv[0]), atom_getfloat(&argv[1]),
    	    	    	       atom_getfloat(&argv[2]), alpha);
}

