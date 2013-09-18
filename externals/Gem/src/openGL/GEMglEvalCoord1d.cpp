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

#include "GEMglEvalCoord1d.h"

CPPEXTERN_NEW_WITH_ONE_ARG ( GEMglEvalCoord1d , t_floatarg, A_DEFFLOAT);

/////////////////////////////////////////////////////////
//
// GEMglViewport
//
/////////////////////////////////////////////////////////
// Constructor
//
GEMglEvalCoord1d :: GEMglEvalCoord1d	(t_floatarg arg0=0) :
		u(static_cast<GLdouble>(arg0))
{
	m_inlet[0] = inlet_new(this->x_obj, &this->x_obj->ob_pd, &s_float, gensym("u"));
}
/////////////////////////////////////////////////////////
// Destructor
//
GEMglEvalCoord1d :: ~GEMglEvalCoord1d () {
inlet_free(m_inlet[0]);
}
//////////////////
// extension check
bool GEMglEvalCoord1d :: isRunnable(void) {
  if(GLEW_VERSION_1_1)return true;
  error("your system does not support OpenGL-1.1");
  return false;
}
/////////////////////////////////////////////////////////
// Render
//
void GEMglEvalCoord1d :: render(GemState *state) {
	glEvalCoord1d (u);
}

/////////////////////////////////////////////////////////
// Variables
//
void GEMglEvalCoord1d :: uMess (t_float arg1) {	// FUN
	u = static_cast<GLdouble>(arg1);
	setModified();
}


/////////////////////////////////////////////////////////
// static member functions
//

void GEMglEvalCoord1d :: obj_setupCallback(t_class *classPtr) {
	 class_addmethod(classPtr, reinterpret_cast<t_method>(&GEMglEvalCoord1d::uMessCallback),  	gensym("u"), A_DEFFLOAT, A_NULL);
}

void GEMglEvalCoord1d :: uMessCallback (void* data, t_floatarg arg0){
	GetMyClass(data)->uMess ( static_cast<t_float>(arg0));
}
