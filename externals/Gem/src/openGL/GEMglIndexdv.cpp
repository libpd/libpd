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

#include "GEMglIndexdv.h"

CPPEXTERN_NEW_WITH_ONE_ARG ( GEMglIndexdv , t_floatarg, A_DEFFLOAT);

/////////////////////////////////////////////////////////
//
// GEMglIndexdv
//
/////////////////////////////////////////////////////////
// Constructor
//
GEMglIndexdv :: GEMglIndexdv	(t_floatarg arg0=0) {
	cMess(arg0);
	m_inlet = inlet_new(this->x_obj, &this->x_obj->ob_pd, &s_float, gensym("v"));
}
/////////////////////////////////////////////////////////
// Destructor
//
GEMglIndexdv :: ~GEMglIndexdv () {
	inlet_free(m_inlet);
}

/////////////////////////////////////////////////////////
// Render
//
void GEMglIndexdv :: render(GemState *state) {
	glIndexdv (c);
}

/////////////////////////////////////////////////////////
// variable
//
void GEMglIndexdv :: cMess (t_float arg0) {	// FUN
  c[0]=static_cast<GLdouble>(arg0);
  setModified();
}

/////////////////////////////////////////////////////////
// static member functions
//

void GEMglIndexdv :: obj_setupCallback(t_class *classPtr) {
  class_addmethod(classPtr, reinterpret_cast<t_method>(&GEMglIndexdv::cMessCallback),  	gensym("c"), A_DEFFLOAT, A_NULL);
}

void GEMglIndexdv :: cMessCallback (void* data, t_floatarg arg0) {
	GetMyClass(data)->cMess (arg0);
}
