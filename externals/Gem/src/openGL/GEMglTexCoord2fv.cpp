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

#include "GEMglTexCoord2fv.h"

CPPEXTERN_NEW_WITH_TWO_ARGS ( GEMglTexCoord2fv , t_floatarg, A_DEFFLOAT, t_floatarg, A_DEFFLOAT);

/////////////////////////////////////////////////////////
//
// GEMglTexCoord2fv
//
/////////////////////////////////////////////////////////
// Constructor
//
GEMglTexCoord2fv :: GEMglTexCoord2fv	(t_floatarg arg0=0, t_floatarg arg1=0) {
vMess(arg0, arg1);
	m_inlet = inlet_new(this->x_obj, &this->x_obj->ob_pd, &s_float, gensym("v"));
}
/////////////////////////////////////////////////////////
// Destructor
//
GEMglTexCoord2fv :: ~GEMglTexCoord2fv () {
	inlet_free(m_inlet);
}

/////////////////////////////////////////////////////////
// Render
//
void GEMglTexCoord2fv :: render(GemState *state) {
	glTexCoord2fv (v);
}

/////////////////////////////////////////////////////////
// variable
//
void GEMglTexCoord2fv :: vMess (t_float arg0, t_float arg1) {	// FUN
	v[0]=static_cast<GLfloat>(arg0);
	v[1]=static_cast<GLfloat>(arg1);
	setModified();
}

/////////////////////////////////////////////////////////
// static member functions
//

void GEMglTexCoord2fv :: obj_setupCallback(t_class *classPtr) {
	 class_addmethod(classPtr, reinterpret_cast<t_method>(&GEMglTexCoord2fv::vMessCallback),  	gensym("v"), A_DEFFLOAT, A_DEFFLOAT, A_NULL);
}

void GEMglTexCoord2fv :: vMessCallback (void* data, t_floatarg arg0, t_floatarg arg1) {
	GetMyClass(data)->vMess ( arg0, arg1);
}
