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

#include "GEMglIndexiv.h"

CPPEXTERN_NEW_WITH_ONE_ARG ( GEMglIndexiv , t_floatarg, A_DEFFLOAT);

/////////////////////////////////////////////////////////
//
// GEMglIndexiv
//
/////////////////////////////////////////////////////////
// Constructor
//
GEMglIndexiv :: GEMglIndexiv	(t_floatarg arg0=0) {
	cMess(arg0);
	m_inlet = inlet_new(this->x_obj, &this->x_obj->ob_pd, &s_float, gensym("v"));
}
/////////////////////////////////////////////////////////
// Destructor
//
GEMglIndexiv :: ~GEMglIndexiv () {
	inlet_free(m_inlet);
}

/////////////////////////////////////////////////////////
// Render
//
void GEMglIndexiv :: render(GemState *state) {
	glIndexiv (c);
}

/////////////////////////////////////////////////////////
// variable
//
void GEMglIndexiv :: cMess (t_float arg0) {	// FUN
  c[0]=static_cast<GLint>(arg0);
  setModified();
}

/////////////////////////////////////////////////////////
// static member functions
//

void GEMglIndexiv :: obj_setupCallback(t_class *classPtr) {
  class_addmethod(classPtr, reinterpret_cast<t_method>(&GEMglIndexiv::cMessCallback),  	gensym("c"), A_DEFFLOAT, A_NULL);
}

void GEMglIndexiv :: cMessCallback (void* data, t_floatarg arg0) {
	GetMyClass(data)->cMess (arg0);
}
