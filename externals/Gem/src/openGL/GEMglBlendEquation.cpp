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

#include "GEMglBlendEquation.h"

CPPEXTERN_NEW_WITH_ONE_ARG ( GEMglBlendEquation , t_floatarg, A_DEFFLOAT);

/////////////////////////////////////////////////////////
//
// GEMglViewport
//
/////////////////////////////////////////////////////////
// Constructor
//
GEMglBlendEquation :: GEMglBlendEquation	(t_floatarg arg0=0) :
  mode(static_cast<GLenum>(arg0))
{
  m_inlet[0] = inlet_new(this->x_obj, &this->x_obj->ob_pd, &s_float, gensym("mode"));
}
////////////////////////////////////////////////////////
// Destructor
//
GEMglBlendEquation :: ~GEMglBlendEquation () {
  inlet_free(m_inlet[0]);
}
//////////////////
// extension check
bool GEMglBlendEquation :: isRunnable(void) {
  if(GLEW_VERSION_1_4)return true;
  error("your system does not support OpenGL-1.4");
  return false;
}
/////////////////////////////////////////////////////////
// Render
//
void GEMglBlendEquation :: render(GemState *state) {
  glBlendEquation (mode);
}

/////////////////////////////////////////////////////////
// Variables
//
void GEMglBlendEquation :: modeMess (t_float arg1) {	// FUN
  mode = static_cast<GLenum>(arg1);
  setModified();
}


/////////////////////////////////////////////////////////
// static member functions
//

void GEMglBlendEquation :: obj_setupCallback(t_class *classPtr) {
  class_addmethod(classPtr, reinterpret_cast<t_method>(&GEMglBlendEquation::modeMessCallback),  	gensym("mode"), A_DEFFLOAT, A_NULL);
};

void GEMglBlendEquation :: modeMessCallback (void* data, t_floatarg arg0){
  GetMyClass(data)->modeMess ( static_cast<t_float>(arg0));
}
