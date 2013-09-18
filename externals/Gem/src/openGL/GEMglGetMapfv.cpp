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

#include "GEMglGetMapfv.h"

CPPEXTERN_NEW_WITH_TWO_ARGS ( GEMglGetMapfv, t_floatarg, A_DEFFLOAT, t_floatarg, A_DEFFLOAT);

/////////////////////////////////////////////////////////
//
// GEMglGetMapfv
//
/////////////////////////////////////////////////////////
// Constructor
//
GEMglGetMapfv :: GEMglGetMapfv	(t_floatarg arg0=0, t_floatarg arg1=0) {
	targetMess(arg0);
	queryMess(arg1);
	m_inlet[0] = inlet_new(this->x_obj, &this->x_obj->ob_pd, &s_float, gensym("target"));
	m_inlet[1] = inlet_new(this->x_obj, &this->x_obj->ob_pd, &s_float, gensym("query"));
}
/////////////////////////////////////////////////////////
// Destructor
//
GEMglGetMapfv :: ~GEMglGetMapfv () {
	inlet_free(m_inlet[0]);
	inlet_free(m_inlet[1]);
}

//////////////////
// extension check
bool GEMglGetMapfv :: isRunnable(void) {
  if(GLEW_VERSION_1_1)return true;
  error("your system does not support OpenGL-1.1");
  return false;
}

/////////////////////////////////////////////////////////
// Render
//
void GEMglGetMapfv :: render(GemState *state) {
  glGetMapfv (target, query, v);
  post("not really implemented:: got data @ %X, what should i do with it", v);
}


/////////////////////////////////////////////////////////
// variable
//
void GEMglGetMapfv :: targetMess (t_float arg0) {	// FUN
  target=static_cast<GLenum>(arg0);
  setModified();
}
void GEMglGetMapfv :: queryMess (t_float arg0) {	// FUN
  query=static_cast<GLenum>(arg0);
  setModified();
}

/////////////////////////////////////////////////////////
// static member functions
//

void GEMglGetMapfv :: obj_setupCallback(t_class *classPtr) {
  class_addmethod(classPtr, reinterpret_cast<t_method>(&GEMglGetMapfv::targetMessCallback), gensym("target"), A_DEFFLOAT, A_NULL);
  class_addmethod(classPtr, reinterpret_cast<t_method>(&GEMglGetMapfv::queryMessCallback), gensym("query"), A_DEFFLOAT, A_NULL);
}

void GEMglGetMapfv :: targetMessCallback (void* data, t_floatarg arg0) {
	GetMyClass(data)->targetMess (arg0 );
}
void GEMglGetMapfv :: queryMessCallback (void* data, t_floatarg arg0) {
	GetMyClass(data)->queryMess (arg0 );
}
