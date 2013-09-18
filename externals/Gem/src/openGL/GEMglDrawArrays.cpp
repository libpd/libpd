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

#include "GEMglDrawArrays.h"

CPPEXTERN_NEW_WITH_THREE_ARGS ( GEMglDrawArrays , t_floatarg, A_DEFFLOAT, t_floatarg, A_DEFFLOAT, t_floatarg, A_DEFFLOAT);

/////////////////////////////////////////////////////////
//
// GEMglViewport
//
/////////////////////////////////////////////////////////
// Constructor
//
GEMglDrawArrays :: GEMglDrawArrays	(t_floatarg arg0=0, t_floatarg arg1=0, t_floatarg arg2=0) :
		mode(static_cast<GLenum>(arg0)), 
		first(static_cast<GLint>(arg1)), 
		count(static_cast<GLsizei>(arg2))
{
	m_inlet[0] = inlet_new(this->x_obj, &this->x_obj->ob_pd, &s_float, gensym("mode"));
	m_inlet[1] = inlet_new(this->x_obj, &this->x_obj->ob_pd, &s_float, gensym("first"));
	m_inlet[2] = inlet_new(this->x_obj, &this->x_obj->ob_pd, &s_float, gensym("count"));
}
/////////////////////////////////////////////////////////
// Destructor
//
GEMglDrawArrays :: ~GEMglDrawArrays () {
inlet_free(m_inlet[0]);
inlet_free(m_inlet[1]);
inlet_free(m_inlet[2]);
}
//////////////////
// extension check
bool GEMglDrawArrays :: isRunnable(void) {
  if(GLEW_VERSION_1_1)return true;
  error("your system does not support OpenGL-1.1");
  return false;
}
/////////////////////////////////////////////////////////
// Render
//
void GEMglDrawArrays :: render(GemState *state) {
	glDrawArrays (mode, first, count);
}

/////////////////////////////////////////////////////////
// Variables
//
void GEMglDrawArrays :: modeMess (t_float arg1) {	// FUN
	mode = static_cast<GLenum>(arg1);
	setModified();
}

void GEMglDrawArrays :: firstMess (t_float arg1) {	// FUN
	first = static_cast<GLint>(arg1);
	setModified();
}

void GEMglDrawArrays :: countMess (t_float arg1) {	// FUN
	count = static_cast<GLsizei>(arg1);
	setModified();
}


/////////////////////////////////////////////////////////
// static member functions
//

void GEMglDrawArrays :: obj_setupCallback(t_class *classPtr) {
	 class_addmethod(classPtr, reinterpret_cast<t_method>(&GEMglDrawArrays::modeMessCallback),  	gensym("mode"), A_DEFFLOAT, A_NULL);
	 class_addmethod(classPtr, reinterpret_cast<t_method>(&GEMglDrawArrays::firstMessCallback),  	gensym("first"), A_DEFFLOAT, A_NULL);
	 class_addmethod(classPtr, reinterpret_cast<t_method>(&GEMglDrawArrays::countMessCallback),  	gensym("count"), A_DEFFLOAT, A_NULL);
}

void GEMglDrawArrays :: modeMessCallback (void* data, t_floatarg arg0){
	GetMyClass(data)->modeMess ( static_cast<t_float>(arg0));
}
void GEMglDrawArrays :: firstMessCallback (void* data, t_floatarg arg0){
	GetMyClass(data)->firstMess ( static_cast<t_float>(arg0));
}
void GEMglDrawArrays :: countMessCallback (void* data, t_floatarg arg0){
	GetMyClass(data)->countMess ( static_cast<t_float>(arg0));
}
