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

#include "GEMglIsTexture.h"

CPPEXTERN_NEW_WITH_ONE_ARG ( GEMglIsTexture , t_floatarg, A_DEFFLOAT);

/////////////////////////////////////////////////////////
//
// GEMglViewport
//
/////////////////////////////////////////////////////////
// Constructor
//
GEMglIsTexture :: GEMglIsTexture	(t_floatarg arg0=0) : texture(static_cast<GLuint>(arg0)) {
  m_inlet = inlet_new(this->x_obj, &this->x_obj->ob_pd, &s_float, gensym("texture"));
  m_outlet=outlet_new(this->x_obj, 0);
}
/////////////////////////////////////////////////////////
// Destructor
//
GEMglIsTexture :: ~GEMglIsTexture () {
  inlet_free(m_inlet);
  outlet_free(m_outlet);
}

/////////////////////////////////////////////////////////
// Render
//
void GEMglIsTexture :: render(GemState *state) {
  GLboolean b = glIsTexture (texture);
  outlet_float(m_outlet, b?1.0:0.0);
}

/////////////////////////////////////////////////////////
// Variables
//
void GEMglIsTexture :: textureMess (t_float arg1) {	// FUN
  texture = static_cast<GLuint>(arg1);
  setModified();
}


/////////////////////////////////////////////////////////
// static member functions
//

void GEMglIsTexture :: obj_setupCallback(t_class *classPtr) {
	 class_addmethod(classPtr, reinterpret_cast<t_method>(&GEMglIsTexture::textureMessCallback),  	gensym("texture"), A_DEFFLOAT, A_NULL);
}

void GEMglIsTexture :: textureMessCallback (void* data, t_floatarg arg0){
	GetMyClass(data)->textureMess (arg0);
}
