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

#include "GEMglArrayElement.h"

CPPEXTERN_NEW_WITH_ONE_ARG ( GEMglArrayElement , t_floatarg, A_DEFFLOAT);

/////////////////////////////////////////////////////////
//
// GEMglViewport
//
/////////////////////////////////////////////////////////
// Constructor
//
GEMglArrayElement :: GEMglArrayElement	(t_floatarg arg0=0) :
		i(static_cast<GLint>(arg0))
{
	m_inlet[0] = inlet_new(this->x_obj, &this->x_obj->ob_pd, &s_float, gensym("i"));
}
/////////////////////////////////////////////////////////
// Destructor
//
GEMglArrayElement :: ~GEMglArrayElement () {
inlet_free(m_inlet[0]);
}
//////////////////
// extension check
bool GEMglArrayElement :: isRunnable(void) {
  if(GLEW_VERSION_1_1)return true;
  error("your system does not support OpenGL-1.1");
  return false;
}
/////////////////////////////////////////////////////////
// Render
//
void GEMglArrayElement :: render(GemState *state) {
	glArrayElement (i);
}

/////////////////////////////////////////////////////////
// Variables
//
void GEMglArrayElement :: iMess (t_float arg1) {	// FUN
	i = static_cast<GLint>(arg1);
	setModified();
}


/////////////////////////////////////////////////////////
// static member functions
//

void GEMglArrayElement :: obj_setupCallback(t_class *classPtr) {
	 class_addmethod(classPtr, reinterpret_cast<t_method>(&GEMglArrayElement::iMessCallback),  	gensym("i"), A_DEFFLOAT, A_NULL);
}

void GEMglArrayElement :: iMessCallback (void* data, t_floatarg arg0){
	GetMyClass(data)->iMess ( static_cast<t_float>(arg0));
}
