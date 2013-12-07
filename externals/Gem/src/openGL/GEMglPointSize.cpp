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

#include "GEMglPointSize.h"

CPPEXTERN_NEW_WITH_ONE_ARG ( GEMglPointSize , t_floatarg, A_DEFFLOAT);

/////////////////////////////////////////////////////////
//
// GEMglViewport
//
/////////////////////////////////////////////////////////
// Constructor
//
GEMglPointSize :: GEMglPointSize	(t_floatarg arg0=0) :
		size(static_cast<GLfloat>(arg0))
{
	m_inlet[0] = inlet_new(this->x_obj, &this->x_obj->ob_pd, &s_float, gensym("size"));
}
/////////////////////////////////////////////////////////
// Destructor
//
GEMglPointSize :: ~GEMglPointSize () {
inlet_free(m_inlet[0]);
}

/////////////////////////////////////////////////////////
// Render
//
void GEMglPointSize :: render(GemState *state) {
	glPointSize (size);
}

/////////////////////////////////////////////////////////
// Variables
//
void GEMglPointSize :: sizeMess (t_float arg1) {	// FUN
	size = static_cast<GLfloat>(arg1);
	setModified();
}


/////////////////////////////////////////////////////////
// static member functions
//

void GEMglPointSize :: obj_setupCallback(t_class *classPtr) {
	 class_addmethod(classPtr, reinterpret_cast<t_method>(&GEMglPointSize::sizeMessCallback),  	gensym("size"), A_DEFFLOAT, A_NULL);
};

void GEMglPointSize :: sizeMessCallback (void* data, t_floatarg arg0){
	GetMyClass(data)->sizeMess ( static_cast<t_float>(arg0));
}
