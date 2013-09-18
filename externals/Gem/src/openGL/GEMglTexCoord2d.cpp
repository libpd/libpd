////////////////////////////////////////////////////////
//
// GEM - Graphics Environment for Multimedia
//
// Implementation file
//
// Copyright (c) 2002-2011 IOhannes m zmölnig. forum::für::umläute. IEM. zmoelnig@iem.at
//	zmoelnig@iem.kug.ac.at
//  For information on usage and redistribution, and for a DISCLAIMER
//  *  OF ALL WARRANTIES, see the file, "GEM.LICENSE.TERMS"
//
//  this file has been generated...
////////////////////////////////////////////////////////

#include "GEMglTexCoord2d.h"

CPPEXTERN_NEW_WITH_TWO_ARGS ( GEMglTexCoord2d , t_floatarg, A_DEFFLOAT, t_floatarg, A_DEFFLOAT);

/////////////////////////////////////////////////////////
//
// GEMglViewport
//
/////////////////////////////////////////////////////////
// Constructor
//
GEMglTexCoord2d :: GEMglTexCoord2d	(t_floatarg arg0=0, t_floatarg arg1=0) :
		s(static_cast<GLdouble>(arg0)), 
		t(static_cast<GLdouble>(arg1))
{
	m_inlet[0] = inlet_new(this->x_obj, &this->x_obj->ob_pd, &s_float, gensym("s"));
	m_inlet[1] = inlet_new(this->x_obj, &this->x_obj->ob_pd, &s_float, gensym("t"));
}
/////////////////////////////////////////////////////////
// Destructor
//
GEMglTexCoord2d :: ~GEMglTexCoord2d () {
inlet_free(m_inlet[0]);
inlet_free(m_inlet[1]);
}

/////////////////////////////////////////////////////////
// Render
//
void GEMglTexCoord2d :: render(GemState *state) {
	glTexCoord2d (s, t);
}

/////////////////////////////////////////////////////////
// Variables
//
void GEMglTexCoord2d :: sMess (t_float arg1) {	// FUN
	s = static_cast<GLdouble>(arg1);
	setModified();
}

void GEMglTexCoord2d :: tMess (t_float arg1) {	// FUN
	t = static_cast<GLdouble>(arg1);
	setModified();
}


/////////////////////////////////////////////////////////
// static member functions
//

void GEMglTexCoord2d :: obj_setupCallback(t_class *classPtr) {
	 class_addmethod(classPtr, reinterpret_cast<t_method>(&GEMglTexCoord2d::sMessCallback),  	gensym("s"), A_DEFFLOAT, A_NULL);
	 class_addmethod(classPtr, reinterpret_cast<t_method>(&GEMglTexCoord2d::tMessCallback),  	gensym("t"), A_DEFFLOAT, A_NULL);
};

void GEMglTexCoord2d :: sMessCallback (void* data, t_floatarg arg0){
	GetMyClass(data)->sMess ( static_cast<t_float>(arg0));
}
void GEMglTexCoord2d :: tMessCallback (void* data, t_floatarg arg0){
	GetMyClass(data)->tMess ( static_cast<t_float>(arg0));
}
