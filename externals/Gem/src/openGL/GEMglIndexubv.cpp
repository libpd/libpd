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

#include "GEMglIndexubv.h"

CPPEXTERN_NEW_WITH_ONE_ARG ( GEMglIndexubv , t_floatarg, A_DEFFLOAT);

/////////////////////////////////////////////////////////
//
// GEMglIndexubv
//
/////////////////////////////////////////////////////////
// Constructor
//
GEMglIndexubv :: GEMglIndexubv	(t_floatarg arg0=0) {
	cMess(arg0);
	m_inlet = inlet_new(this->x_obj, &this->x_obj->ob_pd, &s_float, gensym("v"));
}
/////////////////////////////////////////////////////////
// Destructor
//
GEMglIndexubv :: ~GEMglIndexubv () {
	inlet_free(m_inlet);
}

//////////////////
// extension check
bool GEMglIndexubv :: isRunnable(void) {
  if(GLEW_VERSION_1_1)return true;
  error("your system does not support OpenGL-1.1");
  return false;
}

/////////////////////////////////////////////////////////
// Render
//
void GEMglIndexubv :: render(GemState *state) {
	glIndexubv (c);
}

/////////////////////////////////////////////////////////
// variable
//
void GEMglIndexubv :: cMess (t_float arg0) {	// FUN
  c[0]=static_cast<GLubyte>(arg0);
  setModified();
}

/////////////////////////////////////////////////////////
// static member functions
//

void GEMglIndexubv :: obj_setupCallback(t_class *classPtr) {
  class_addmethod(classPtr, reinterpret_cast<t_method>(&GEMglIndexubv::cMessCallback),  	gensym("c"), A_DEFFLOAT, A_NULL);
}

void GEMglIndexubv :: cMessCallback (void* data, t_floatarg arg0) {
	GetMyClass(data)->cMess (arg0);
}
