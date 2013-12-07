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

#include "GEMglLoadName.h"

CPPEXTERN_NEW_WITH_ONE_ARG ( GEMglLoadName , t_floatarg, A_DEFFLOAT);

/////////////////////////////////////////////////////////
//
// GEMglViewport
//
/////////////////////////////////////////////////////////
// Constructor
//
GEMglLoadName :: GEMglLoadName	(t_floatarg arg0=0) :
		name(static_cast<GLuint>(arg0))
{
	m_inlet[0] = inlet_new(this->x_obj, &this->x_obj->ob_pd, &s_float, gensym("name"));
}
/////////////////////////////////////////////////////////
// Destructor
//
GEMglLoadName :: ~GEMglLoadName () {
inlet_free(m_inlet[0]);
}

//////////////////
// extension check
bool GEMglLoadName :: isRunnable(void) {
  if(GLEW_VERSION_1_1)return true;
  error("your system does not support OpenGL-1.1");
  return false;
}

/////////////////////////////////////////////////////////
// Render
//
void GEMglLoadName :: render(GemState *state) {
	glLoadName (name);
}

/////////////////////////////////////////////////////////
// Variables
//
void GEMglLoadName :: nameMess (t_float arg1) {	// FUN
	name = static_cast<GLuint>(arg1);
	setModified();
}


/////////////////////////////////////////////////////////
// static member functions
//

void GEMglLoadName :: obj_setupCallback(t_class *classPtr) {
	 class_addmethod(classPtr, reinterpret_cast<t_method>(&GEMglLoadName::nameMessCallback),  	gensym("name"), A_DEFFLOAT, A_NULL);
}

void GEMglLoadName :: nameMessCallback (void* data, t_floatarg arg0){
	GetMyClass(data)->nameMess ( static_cast<t_float>(arg0));
}
