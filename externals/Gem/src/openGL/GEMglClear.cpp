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

#include "GEMglClear.h"

CPPEXTERN_NEW_WITH_ONE_ARG ( GEMglClear , t_floatarg, A_DEFFLOAT);

/////////////////////////////////////////////////////////
//
// GEMglViewport
//
/////////////////////////////////////////////////////////
// Constructor
//
GEMglClear :: GEMglClear	(t_floatarg arg0=0) :
		mask(static_cast<GLbitfield>(arg0))
{
	m_inlet[0] = inlet_new(this->x_obj, &this->x_obj->ob_pd, &s_float, gensym("mask"));
}
/////////////////////////////////////////////////////////
// Destructor
//
GEMglClear :: ~GEMglClear () {
inlet_free(m_inlet[0]);
}

/////////////////////////////////////////////////////////
// Render
//
void GEMglClear :: render(GemState *state) {
	glClear (mask);
}

/////////////////////////////////////////////////////////
// Variables
//
void GEMglClear :: maskMess (t_float arg1) {	// FUN
	mask = static_cast<GLbitfield>(arg1);
	setModified();
}


/////////////////////////////////////////////////////////
// static member functions
//

void GEMglClear :: obj_setupCallback(t_class *classPtr) {
	 class_addmethod(classPtr, reinterpret_cast<t_method>(&GEMglClear::maskMessCallback),  	gensym("mask"), A_DEFFLOAT, A_NULL);
};

void GEMglClear :: maskMessCallback (void* data, t_floatarg arg0){
	GetMyClass(data)->maskMess ( static_cast<t_float>(arg0));
}
