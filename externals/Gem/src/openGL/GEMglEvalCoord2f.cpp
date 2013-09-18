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

#include "GEMglEvalCoord2f.h"

CPPEXTERN_NEW_WITH_TWO_ARGS ( GEMglEvalCoord2f , t_floatarg, A_DEFFLOAT, t_floatarg, A_DEFFLOAT);

/////////////////////////////////////////////////////////
//
// GEMglViewport
//
/////////////////////////////////////////////////////////
// Constructor
//
GEMglEvalCoord2f :: GEMglEvalCoord2f	(t_floatarg arg0=0, t_floatarg arg1=0) :
		u(static_cast<GLfloat>(arg0)), 
		v(static_cast<GLfloat>(arg1))
{
	m_inlet[0] = inlet_new(this->x_obj, &this->x_obj->ob_pd, &s_float, gensym("u"));
	m_inlet[1] = inlet_new(this->x_obj, &this->x_obj->ob_pd, &s_float, gensym("v"));
}
/////////////////////////////////////////////////////////
// Destructor
//
GEMglEvalCoord2f :: ~GEMglEvalCoord2f () {
inlet_free(m_inlet[0]);
inlet_free(m_inlet[1]);
}
//////////////////
// extension check
bool GEMglEvalCoord2f :: isRunnable(void) {
  if(GLEW_VERSION_1_1)return true;
  error("your system does not support OpenGL-1.1");
  return false;
}
/////////////////////////////////////////////////////////
// Render
//
void GEMglEvalCoord2f :: render(GemState *state) {
	glEvalCoord2f (u, v);
}

/////////////////////////////////////////////////////////
// Variables
//
void GEMglEvalCoord2f :: uMess (t_float arg1) {	// FUN
	u = static_cast<GLfloat>(arg1);
	setModified();
}

void GEMglEvalCoord2f :: vMess (t_float arg1) {	// FUN
	v = static_cast<GLfloat>(arg1);
	setModified();
}


/////////////////////////////////////////////////////////
// static member functions
//

void GEMglEvalCoord2f :: obj_setupCallback(t_class *classPtr) {
	 class_addmethod(classPtr, reinterpret_cast<t_method>(&GEMglEvalCoord2f::uMessCallback),  	gensym("u"), A_DEFFLOAT, A_NULL);
	 class_addmethod(classPtr, reinterpret_cast<t_method>(&GEMglEvalCoord2f::vMessCallback),  	gensym("v"), A_DEFFLOAT, A_NULL);
}

void GEMglEvalCoord2f :: uMessCallback (void* data, t_floatarg arg0){
	GetMyClass(data)->uMess ( static_cast<t_float>(arg0));
}
void GEMglEvalCoord2f :: vMessCallback (void* data, t_floatarg arg0){
	GetMyClass(data)->vMess ( static_cast<t_float>(arg0));
}
