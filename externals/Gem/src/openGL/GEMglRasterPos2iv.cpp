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

#include "GEMglRasterPos2iv.h"

CPPEXTERN_NEW_WITH_TWO_ARGS ( GEMglRasterPos2iv , t_floatarg, A_DEFFLOAT, t_floatarg, A_DEFFLOAT);

/////////////////////////////////////////////////////////
//
// GEMglRasterPos2iv
//
/////////////////////////////////////////////////////////
// Constructor
//
GEMglRasterPos2iv :: GEMglRasterPos2iv	(t_floatarg arg0=0, t_floatarg arg1=0) {
vMess(arg0, arg1);
	m_inlet = inlet_new(this->x_obj, &this->x_obj->ob_pd, &s_float, gensym("v"));
}
/////////////////////////////////////////////////////////
// Destructor
//
GEMglRasterPos2iv :: ~GEMglRasterPos2iv () {
	inlet_free(m_inlet);
}

/////////////////////////////////////////////////////////
// Render
//
void GEMglRasterPos2iv :: render(GemState *state) {
	glRasterPos2iv (v);
}

/////////////////////////////////////////////////////////
// variable
//
void GEMglRasterPos2iv :: vMess (t_float arg0, t_float arg1) {	// FUN
	v[0]=static_cast<GLint>(arg0);
	v[1]=static_cast<GLint>(arg1);
	setModified();
}

/////////////////////////////////////////////////////////
// static member functions
//

void GEMglRasterPos2iv :: obj_setupCallback(t_class *classPtr) {
	 class_addmethod(classPtr, reinterpret_cast<t_method>(&GEMglRasterPos2iv::vMessCallback),  	gensym("v"), A_DEFFLOAT, A_DEFFLOAT, A_NULL);
}

void GEMglRasterPos2iv :: vMessCallback (void* data, t_floatarg arg0, t_floatarg arg1) {
	GetMyClass(data)->vMess ( arg0, arg1);
}
