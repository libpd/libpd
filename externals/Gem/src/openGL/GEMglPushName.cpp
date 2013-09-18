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

#include "GEMglPushName.h"

CPPEXTERN_NEW_WITH_ONE_ARG ( GEMglPushName , t_floatarg, A_DEFFLOAT);

/////////////////////////////////////////////////////////
//
// GEMglViewport
//
/////////////////////////////////////////////////////////
// Constructor
//
GEMglPushName :: GEMglPushName	(t_floatarg arg0=0) :
		name(static_cast<GLuint>(arg0))
{
	m_inlet[0] = inlet_new(this->x_obj, &this->x_obj->ob_pd, &s_float, gensym("name"));
}
/////////////////////////////////////////////////////////
// Destructor
//
GEMglPushName :: ~GEMglPushName () {
inlet_free(m_inlet[0]);
}

//////////////////
// extension check
bool GEMglPushName :: isRunnable(void) {
  if(GLEW_VERSION_1_1)return true;
  error("your system does not support OpenGL-1.1");
  return false;
}


/////////////////////////////////////////////////////////
// Render
//
void GEMglPushName :: render(GemState *state) {
	glPushName (name);
}

/////////////////////////////////////////////////////////
// Variables
//
void GEMglPushName :: nameMess (t_float arg1) {	// FUN
	name = static_cast<GLuint>(arg1);
	setModified();
}


/////////////////////////////////////////////////////////
// static member functions
//

void GEMglPushName :: obj_setupCallback(t_class *classPtr) {
	 class_addmethod(classPtr, reinterpret_cast<t_method>(&GEMglPushName::nameMessCallback),  	gensym("name"), A_DEFFLOAT, A_NULL);
}

void GEMglPushName :: nameMessCallback (void* data, t_floatarg arg0){
	GetMyClass(data)->nameMess ( static_cast<t_float>(arg0));
}
