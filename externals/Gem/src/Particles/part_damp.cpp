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

#include "part_damp.h"


CPPEXTERN_NEW_WITH_THREE_ARGS(part_damp, t_floatarg, A_DEFFLOAT, t_floatarg, A_DEFFLOAT, t_floatarg, A_DEFFLOAT);

/////////////////////////////////////////////////////////
//
// part_damp
//
/////////////////////////////////////////////////////////
// Constructor
//
/////////////////////////////////////////////////////////
part_damp :: part_damp(t_floatarg xpos, t_floatarg ypos, t_floatarg zpos)
{
	m_vector[0] = xpos;
	m_vector[1] = ypos;
	m_vector[2] = zpos;

    inlet_new(this->x_obj, &this->x_obj->ob_pd, gensym("list"), gensym("vector"));
}

/////////////////////////////////////////////////////////
// Destructor
//
/////////////////////////////////////////////////////////
part_damp :: ~part_damp()
{ }

/////////////////////////////////////////////////////////
// renderParticles
//
/////////////////////////////////////////////////////////
void part_damp :: renderParticles(GemState *state)
{
	if (m_tickTime > 0.f)
	{
		pDamping(m_vector[0], m_vector[1], m_vector[2]);
	}
}

/////////////////////////////////////////////////////////
// vectorMess
//
/////////////////////////////////////////////////////////
void part_damp :: vectorMess(float x, float y, float z)
{
    m_vector[0] = x;
    m_vector[1] = y;
    m_vector[2] = z;
}

/////////////////////////////////////////////////////////
// static member functions
//
/////////////////////////////////////////////////////////
void part_damp :: obj_setupCallback(t_class *classPtr)
{
    class_addmethod(classPtr, reinterpret_cast<t_method>(&part_damp::vectorMessCallback),
    	    gensym("vector"), A_FLOAT, A_FLOAT, A_FLOAT, A_NULL); 
}
void part_damp :: vectorMessCallback(void *data, t_floatarg x, t_floatarg y, t_floatarg z)
{
    GetMyClass(data)->vectorMess((float)x, (float)y, (float)z);
}
