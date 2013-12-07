////////////////////////////////////////////////////////
//
// GEM - Graphics Environment for Multimedia
//
// zmoelnig@iem.kug.ac.at
//
//    Copyright (c) 1997-2000 Mark Danks.
//    Copyright (c) Günther Geiger.
//    Copyright (c) 2001-2011 IOhannes m zmölnig. forum::für::umläute. IEM. zmoelnig@iem.at
//    For information on usage and redistribution, and for a DISCLAIMER OF ALL
//    WARRANTIES, see the file, "GEM.LICENSE.TERMS" in this distribution.
//
/////////////////////////////////////////////////////////

#include "part_targetcolor.h"

#include "Gem/Exception.h"

#include "papi.h"

CPPEXTERN_NEW_WITH_GIMME(part_targetcolor);

/////////////////////////////////////////////////////////
//
// part_targetcolor
//
/////////////////////////////////////////////////////////
// Constructor
//
/////////////////////////////////////////////////////////
part_targetcolor :: part_targetcolor(int argc, t_atom *argv)
{
	scaleMess(.05f);

	if (argc == 5)
	{
		colorMess(atom_getfloat(&argv[0]), atom_getfloat(&argv[1]),
				  atom_getfloat(&argv[2]), atom_getfloat(&argv[3]));
		scaleMess(atom_getfloat(&argv[4]));
	}
  else if (argc == 4) colorMess(atom_getfloat(&argv[0]), atom_getfloat(&argv[1]),
                                atom_getfloat(&argv[2]), atom_getfloat(&argv[3]));
  else if (argc == 3) colorMess(atom_getfloat(&argv[0]), atom_getfloat(&argv[1]),
                                atom_getfloat(&argv[2]), 1.f);
  else if (argc == 0) colorMess(1.f, 1.f, 1.f, 1.f);
  else
    {
      throw(GemException("needs 0, 3, 4, or 5 arguments"));
    }

  // create the new inlet
  inlet_new(this->x_obj, &this->x_obj->ob_pd, &s_list, gensym("color"));
  inlet_new(this->x_obj, &this->x_obj->ob_pd, &s_float, gensym("ft1"));
}

/////////////////////////////////////////////////////////
// Destructor
//
/////////////////////////////////////////////////////////
part_targetcolor :: ~part_targetcolor()
{ }

/////////////////////////////////////////////////////////
// renderParticles
//
/////////////////////////////////////////////////////////
void part_targetcolor :: renderParticles(GemState *state)
{
  if (m_tickTime > 0.f)
	{
		pTargetColor(m_color[0], m_color[1], m_color[2], m_color[3], m_scale);
	}
}

/////////////////////////////////////////////////////////
// scaleMess
//
/////////////////////////////////////////////////////////
void part_targetcolor :: scaleMess(float scale)
{
    m_scale = scale;
    setModified();
}

/////////////////////////////////////////////////////////
// colorMess
//
/////////////////////////////////////////////////////////
void part_targetcolor :: colorMess(float red, float green, float blue, float alpha)
{
    m_color[0] = red;
    m_color[1] = green;
    m_color[2] = blue;
    m_color[3] = alpha;
    setModified();
}

/////////////////////////////////////////////////////////
// static member function
//
/////////////////////////////////////////////////////////
void part_targetcolor :: obj_setupCallback(t_class *classPtr)
{
    class_addmethod(classPtr, reinterpret_cast<t_method>(&part_targetcolor::colorMessCallback),
    	    gensym("color"), A_GIMME, A_NULL); 
    class_addmethod(classPtr, reinterpret_cast<t_method>(&part_targetcolor::scaleMessCallback),
    	    gensym("ft1"), A_FLOAT, A_NULL); 
}
void part_targetcolor :: colorMessCallback(void *data, t_symbol *, int argc, t_atom *argv)
{
    float alpha = 1;
    if (argc == 4) alpha = atom_getfloat(&argv[3]);
    GetMyClass(data)->colorMess(atom_getfloat(&argv[0]), atom_getfloat(&argv[1]),
    	    	    	       atom_getfloat(&argv[2]), alpha);
}
void part_targetcolor :: scaleMessCallback(void *data, t_floatarg scale)
{
    GetMyClass(data)->scaleMess((float)scale);
}
