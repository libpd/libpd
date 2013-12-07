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

#include "GEMglStencilFunc.h"

CPPEXTERN_NEW_WITH_THREE_ARGS ( GEMglStencilFunc , t_floatarg, A_DEFFLOAT, t_floatarg, A_DEFFLOAT, t_floatarg, A_DEFFLOAT);

/////////////////////////////////////////////////////////
//
// GEMglViewport
//
/////////////////////////////////////////////////////////
// Constructor
//
GEMglStencilFunc :: GEMglStencilFunc	(t_floatarg arg0=0, t_floatarg arg1=0, t_floatarg arg2=0) :
		func(static_cast<GLenum>(arg0)), 
		ref(static_cast<GLint>(arg1)), 
		mask(static_cast<GLuint>(arg2))
{
	m_inlet[0] = inlet_new(this->x_obj, &this->x_obj->ob_pd, &s_float, gensym("func"));
	m_inlet[1] = inlet_new(this->x_obj, &this->x_obj->ob_pd, &s_float, gensym("ref"));
	m_inlet[2] = inlet_new(this->x_obj, &this->x_obj->ob_pd, &s_float, gensym("mask"));
}
/////////////////////////////////////////////////////////
// Destructor
//
GEMglStencilFunc :: ~GEMglStencilFunc () {
inlet_free(m_inlet[0]);
inlet_free(m_inlet[1]);
inlet_free(m_inlet[2]);
}

/////////////////////////////////////////////////////////
// Render
//
void GEMglStencilFunc :: render(GemState *state) {
	glStencilFunc (func, ref, mask);
}

/////////////////////////////////////////////////////////
// Variables
//
void GEMglStencilFunc :: funcMess (t_float arg1) {	// FUN
	func = static_cast<GLenum>(arg1);
	setModified();
}

void GEMglStencilFunc :: refMess (t_float arg1) {	// FUN
	ref = static_cast<GLint>(arg1);
	setModified();
}

void GEMglStencilFunc :: maskMess (t_float arg1) {	// FUN
	mask = static_cast<GLuint>(arg1);
	setModified();
}


/////////////////////////////////////////////////////////
// static member functions
//

void GEMglStencilFunc :: obj_setupCallback(t_class *classPtr) {
	 class_addmethod(classPtr, reinterpret_cast<t_method>(&GEMglStencilFunc::funcMessCallback),  	gensym("func"), A_DEFFLOAT, A_NULL);
	 class_addmethod(classPtr, reinterpret_cast<t_method>(&GEMglStencilFunc::refMessCallback),  	gensym("ref"), A_DEFFLOAT, A_NULL);
	 class_addmethod(classPtr, reinterpret_cast<t_method>(&GEMglStencilFunc::maskMessCallback),  	gensym("mask"), A_DEFFLOAT, A_NULL);
};

void GEMglStencilFunc :: funcMessCallback (void* data, t_floatarg arg0){
	GetMyClass(data)->funcMess ( static_cast<t_float>(arg0));
}
void GEMglStencilFunc :: refMessCallback (void* data, t_floatarg arg0){
	GetMyClass(data)->refMess ( static_cast<t_float>(arg0));
}
void GEMglStencilFunc :: maskMessCallback (void* data, t_floatarg arg0){
	GetMyClass(data)->maskMess ( static_cast<t_float>(arg0));
}
