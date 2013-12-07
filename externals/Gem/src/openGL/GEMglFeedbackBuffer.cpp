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

#include "GEMglFeedbackBuffer.h"

CPPEXTERN_NEW_WITH_TWO_ARGS ( GEMglFeedbackBuffer , t_floatarg, A_DEFFLOAT, t_floatarg, A_DEFFLOAT);

/////////////////////////////////////////////////////////
//
// GEMglViewport
//
/////////////////////////////////////////////////////////
// Constructor
//
GEMglFeedbackBuffer :: GEMglFeedbackBuffer	(t_floatarg arg0=128, t_floatarg arg1=0) :
		size(static_cast<GLsizei>(arg0)), type(static_cast<GLenum>(arg1))
{
	len=(size>0)?size:128;
	buffer = new float[len];

	m_inlet[0] = inlet_new(this->x_obj, &this->x_obj->ob_pd, &s_float, gensym("size"));
	m_inlet[1] = inlet_new(this->x_obj, &this->x_obj->ob_pd, &s_float, gensym("type"));
}
/////////////////////////////////////////////////////////
// Destructor
//
GEMglFeedbackBuffer :: ~GEMglFeedbackBuffer () {
inlet_free(m_inlet[0]);
inlet_free(m_inlet[1]);
}
//////////////////
// extension check
bool GEMglFeedbackBuffer :: isRunnable(void) {
  if(GLEW_VERSION_1_1)return true;
  error("your system does not support OpenGL-1.1");
  return false;
}
/////////////////////////////////////////////////////////
// Render
//
void GEMglFeedbackBuffer :: render(GemState *state) {
  glFeedbackBuffer (size, type, buffer);
  error("i got data @ %X, but i don't know what to do with it!", buffer);
}

/////////////////////////////////////////////////////////
// Variables
//
void GEMglFeedbackBuffer :: sizeMess (t_float arg1) {	// FUN
	size = static_cast<GLsizei>(arg1);
	if (size>len){
	  len=size;
	  delete[]buffer;
	  buffer = new float[len];
	}	  
	setModified();
}

void GEMglFeedbackBuffer :: typeMess (t_float arg1) {	// FUN
	type = static_cast<GLenum>(arg1);
	setModified();
}

/////////////////////////////////////////////////////////
// static member functions
//

void GEMglFeedbackBuffer :: obj_setupCallback(t_class *classPtr) {
	 class_addmethod(classPtr, reinterpret_cast<t_method>(&GEMglFeedbackBuffer::sizeMessCallback),  	gensym("size"), A_DEFFLOAT, A_NULL);
	 class_addmethod(classPtr, reinterpret_cast<t_method>(&GEMglFeedbackBuffer::typeMessCallback),  	gensym("type"), A_DEFFLOAT, A_NULL);
}

void GEMglFeedbackBuffer :: sizeMessCallback (void* data, t_floatarg arg0){
	GetMyClass(data)->sizeMess ( static_cast<t_float>(arg0));
}
void GEMglFeedbackBuffer :: typeMessCallback (void* data, t_floatarg arg0){
	GetMyClass(data)->typeMess ( static_cast<t_float>(arg0));
}
