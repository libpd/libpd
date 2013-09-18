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

#include "GEMglScissor.h"

CPPEXTERN_NEW_WITH_FOUR_ARGS ( GEMglScissor , t_floatarg, A_DEFFLOAT, t_floatarg, A_DEFFLOAT, t_floatarg, A_DEFFLOAT, t_floatarg, A_DEFFLOAT);

/////////////////////////////////////////////////////////
//
// GEMglViewport
//
/////////////////////////////////////////////////////////
// Constructor
//
GEMglScissor :: GEMglScissor	(t_floatarg arg0=0, t_floatarg arg1=0, t_floatarg arg2=0, t_floatarg arg3=0) :
		x(static_cast<GLint>(arg0)), 
		y(static_cast<GLint>(arg1)), 
		width(static_cast<GLsizei>(arg2)), 
		height(static_cast<GLsizei>(arg3))
{
	m_inlet[0] = inlet_new(this->x_obj, &this->x_obj->ob_pd, &s_float, gensym("x"));
	m_inlet[1] = inlet_new(this->x_obj, &this->x_obj->ob_pd, &s_float, gensym("y"));
	m_inlet[2] = inlet_new(this->x_obj, &this->x_obj->ob_pd, &s_float, gensym("width"));
	m_inlet[3] = inlet_new(this->x_obj, &this->x_obj->ob_pd, &s_float, gensym("height"));
}
/////////////////////////////////////////////////////////
// Destructor
//
GEMglScissor :: ~GEMglScissor () {
inlet_free(m_inlet[0]);
inlet_free(m_inlet[1]);
inlet_free(m_inlet[2]);
inlet_free(m_inlet[3]);
}

/////////////////////////////////////////////////////////
// Render
//
void GEMglScissor :: render(GemState *state) {
	glScissor (x, y, width, height);
}

/////////////////////////////////////////////////////////
// Variables
//
void GEMglScissor :: xMess (t_float arg1) {	// FUN
	x = static_cast<GLint>(arg1);
	setModified();
}

void GEMglScissor :: yMess (t_float arg1) {	// FUN
	y = static_cast<GLint>(arg1);
	setModified();
}

void GEMglScissor :: widthMess (t_float arg1) {	// FUN
	width = static_cast<GLsizei>(arg1);
	setModified();
}

void GEMglScissor :: heightMess (t_float arg1) {	// FUN
	height = static_cast<GLsizei>(arg1);
	setModified();
}


/////////////////////////////////////////////////////////
// static member functions
//

void GEMglScissor :: obj_setupCallback(t_class *classPtr) {
	 class_addmethod(classPtr, reinterpret_cast<t_method>(&GEMglScissor::xMessCallback),  	gensym("x"), A_DEFFLOAT, A_NULL);
	 class_addmethod(classPtr, reinterpret_cast<t_method>(&GEMglScissor::yMessCallback),  	gensym("y"), A_DEFFLOAT, A_NULL);
	 class_addmethod(classPtr, reinterpret_cast<t_method>(&GEMglScissor::widthMessCallback),  	gensym("width"), A_DEFFLOAT, A_NULL);
	 class_addmethod(classPtr, reinterpret_cast<t_method>(&GEMglScissor::heightMessCallback),  	gensym("height"), A_DEFFLOAT, A_NULL);
};

void GEMglScissor :: xMessCallback (void* data, t_floatarg arg0){
	GetMyClass(data)->xMess ( static_cast<t_float>(arg0));
}
void GEMglScissor :: yMessCallback (void* data, t_floatarg arg0){
	GetMyClass(data)->yMess ( static_cast<t_float>(arg0));
}
void GEMglScissor :: widthMessCallback (void* data, t_floatarg arg0){
	GetMyClass(data)->widthMess ( static_cast<t_float>(arg0));
}
void GEMglScissor :: heightMessCallback (void* data, t_floatarg arg0){
	GetMyClass(data)->heightMess ( static_cast<t_float>(arg0));
}
