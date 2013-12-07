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

#include "GEMglTexCoord1dv.h"

CPPEXTERN_NEW_WITH_ONE_ARG ( GEMglTexCoord1dv , t_floatarg, A_DEFFLOAT);

/////////////////////////////////////////////////////////
//
// GEMglTexCoord1dv
//
/////////////////////////////////////////////////////////
// Constructor
//
GEMglTexCoord1dv :: GEMglTexCoord1dv	(t_floatarg arg0=0) {
vMess(arg0);
	m_inlet = inlet_new(this->x_obj, &this->x_obj->ob_pd, &s_float, gensym("v"));
}
/////////////////////////////////////////////////////////
// Destructor
//
GEMglTexCoord1dv :: ~GEMglTexCoord1dv () {
	inlet_free(m_inlet);
}

/////////////////////////////////////////////////////////
// Render
//
void GEMglTexCoord1dv :: render(GemState *state) {
	glTexCoord1dv (v);
}

/////////////////////////////////////////////////////////
// variable
//
void GEMglTexCoord1dv :: vMess (t_float arg0) {	// FUN
	v[0]=static_cast<GLdouble>(arg0);
	setModified();
}

/////////////////////////////////////////////////////////
// static member functions
//

void GEMglTexCoord1dv :: obj_setupCallback(t_class *classPtr) {
	 class_addmethod(classPtr, reinterpret_cast<t_method>(&GEMglTexCoord1dv::vMessCallback),  	gensym("v"), A_DEFFLOAT, A_NULL);
}

void GEMglTexCoord1dv :: vMessCallback (void* data, t_floatarg arg0) {
	GetMyClass(data)->vMess ( arg0);
}
