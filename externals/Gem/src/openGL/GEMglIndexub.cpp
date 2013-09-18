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

#include "GEMglIndexub.h"

CPPEXTERN_NEW_WITH_ONE_ARG ( GEMglIndexub , t_floatarg, A_DEFFLOAT);

/////////////////////////////////////////////////////////
//
// GEMglViewport
//
/////////////////////////////////////////////////////////
// Constructor
//
GEMglIndexub :: GEMglIndexub	(t_floatarg arg0=0) :
		c(static_cast<GLubyte>(arg0))
{
	m_inlet[0] = inlet_new(this->x_obj, &this->x_obj->ob_pd, &s_float, gensym("c"));
}
/////////////////////////////////////////////////////////
// Destructor
//
GEMglIndexub :: ~GEMglIndexub () {
inlet_free(m_inlet[0]);
}

//////////////////
// extension check
bool GEMglIndexub :: isRunnable(void) {
  if(GLEW_VERSION_1_1)return true;
  error("your system does not support OpenGL-1.1");
  return false;
}

/////////////////////////////////////////////////////////
// Render
//
void GEMglIndexub :: render(GemState *state) {
	glIndexub (c);
}

/////////////////////////////////////////////////////////
// Variables
//
void GEMglIndexub :: cMess (t_float arg1) {	// FUN
	c = static_cast<GLubyte>(arg1);
	setModified();
}


/////////////////////////////////////////////////////////
// static member functions
//

void GEMglIndexub :: obj_setupCallback(t_class *classPtr) {
	 class_addmethod(classPtr, reinterpret_cast<t_method>(&GEMglIndexub::cMessCallback),  	gensym("c"), A_DEFFLOAT, A_NULL);
}

void GEMglIndexub :: cMessCallback (void* data, t_floatarg arg0){
	GetMyClass(data)->cMess ( static_cast<t_float>(arg0));
}
