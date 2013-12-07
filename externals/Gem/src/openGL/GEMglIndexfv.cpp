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

#include "GEMglIndexfv.h"

CPPEXTERN_NEW_WITH_ONE_ARG ( GEMglIndexfv , t_floatarg, A_DEFFLOAT);

/////////////////////////////////////////////////////////
//
// GEMglIndexfv
//
/////////////////////////////////////////////////////////
// Constructor
//
GEMglIndexfv :: GEMglIndexfv	(t_floatarg arg0=0) {
	cMess(arg0);
	m_inlet = inlet_new(this->x_obj, &this->x_obj->ob_pd, &s_float, gensym("v"));
}
/////////////////////////////////////////////////////////
// Destructor
//
GEMglIndexfv :: ~GEMglIndexfv () {
	inlet_free(m_inlet);
}

/////////////////////////////////////////////////////////
// Render
//
void GEMglIndexfv :: render(GemState *state) {
	glIndexfv (c);
}

/////////////////////////////////////////////////////////
// variable
//
void GEMglIndexfv :: cMess (t_float arg0) {	// FUN
  c[0]=static_cast<GLfloat>(arg0);
  setModified();
}

/////////////////////////////////////////////////////////
// static member functions
//

void GEMglIndexfv :: obj_setupCallback(t_class *classPtr) {
  class_addmethod(classPtr, reinterpret_cast<t_method>(&GEMglIndexfv::cMessCallback),  	gensym("c"), A_DEFFLOAT, A_NULL);
}

void GEMglIndexfv :: cMessCallback (void* data, t_floatarg arg0) {
	GetMyClass(data)->cMess (arg0);
}
