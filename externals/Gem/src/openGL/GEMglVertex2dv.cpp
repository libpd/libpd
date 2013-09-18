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

#include "GEMglVertex2dv.h"

CPPEXTERN_NEW_WITH_TWO_ARGS ( GEMglVertex2dv , t_floatarg, A_DEFFLOAT, t_floatarg, A_DEFFLOAT);

/////////////////////////////////////////////////////////
//
// GEMglVertex2dv
//
/////////////////////////////////////////////////////////
// Constructor
//
GEMglVertex2dv :: GEMglVertex2dv	(t_floatarg arg0=0, t_floatarg arg1=0) {
vMess(arg0, arg1);
	m_inlet = inlet_new(this->x_obj, &this->x_obj->ob_pd, &s_float, gensym("v"));
}
/////////////////////////////////////////////////////////
// Destructor
//
GEMglVertex2dv :: ~GEMglVertex2dv () {
	inlet_free(m_inlet);
}

/////////////////////////////////////////////////////////
// Render
//
void GEMglVertex2dv :: render(GemState *state) {
	glVertex2dv (v);
}

/////////////////////////////////////////////////////////
// variable
//
void GEMglVertex2dv :: vMess (t_float arg0, t_float arg1) {	// FUN
	v[0]=static_cast<GLdouble>(arg0);
	v[1]=static_cast<GLdouble>(arg1);
	setModified();
}

/////////////////////////////////////////////////////////
// static member functions
//

void GEMglVertex2dv :: obj_setupCallback(t_class *classPtr) {
	 class_addmethod(classPtr, reinterpret_cast<t_method>(&GEMglVertex2dv::vMessCallback),  	gensym("v"), A_DEFFLOAT, A_DEFFLOAT, A_NULL);
}

void GEMglVertex2dv :: vMessCallback (void* data, t_floatarg arg0, t_floatarg arg1) {
	GetMyClass(data)->vMess ( arg0, arg1);
}
