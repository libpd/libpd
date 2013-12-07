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

#include "GEMglEvalCoord1dv.h"

CPPEXTERN_NEW_WITH_ONE_ARG ( GEMglEvalCoord1dv , t_floatarg, A_DEFFLOAT);

/////////////////////////////////////////////////////////
//
// GEMglEvalCoord1dv
//
/////////////////////////////////////////////////////////
// Constructor
//
GEMglEvalCoord1dv :: GEMglEvalCoord1dv	(t_floatarg arg0=0) {
vMess(arg0);
	m_inlet = inlet_new(this->x_obj, &this->x_obj->ob_pd, &s_float, gensym("v"));
}
/////////////////////////////////////////////////////////
// Destructor
//
GEMglEvalCoord1dv :: ~GEMglEvalCoord1dv () {
	inlet_free(m_inlet);
}
//////////////////
// extension check
bool GEMglEvalCoord1dv :: isRunnable(void) {
  if(GLEW_VERSION_1_1)return true;
  error("your system does not support OpenGL-1.1");
  return false;
}
/////////////////////////////////////////////////////////
// Render
//
void GEMglEvalCoord1dv :: render(GemState *state) {
	glEvalCoord1dv (v);
}

/////////////////////////////////////////////////////////
// variable
//
void GEMglEvalCoord1dv :: vMess (t_float arg0) {	// FUN
	v[0]=static_cast<GLdouble>(arg0);
	setModified();
}

/////////////////////////////////////////////////////////
// static member functions
//

void GEMglEvalCoord1dv :: obj_setupCallback(t_class *classPtr) {
	 class_addmethod(classPtr, reinterpret_cast<t_method>(&GEMglEvalCoord1dv::vMessCallback),  	gensym("v"), A_DEFFLOAT, A_NULL);
}

void GEMglEvalCoord1dv :: vMessCallback (void* data, t_floatarg arg0) {
	GetMyClass(data)->vMess ( arg0);
}
