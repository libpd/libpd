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

#include "GEMglClipPlane.h"

CPPEXTERN_NEW_WITH_FOUR_ARGS ( GEMglClipPlane , t_floatarg, A_DEFFLOAT, t_floatarg, A_DEFFLOAT, t_floatarg, A_DEFFLOAT, t_floatarg, A_DEFFLOAT);

/////////////////////////////////////////////////////////
//
// GEMglClipPlane
//
/////////////////////////////////////////////////////////
// Constructor
//
GEMglClipPlane :: GEMglClipPlane	(t_floatarg arg0=0, t_floatarg arg1=0, t_floatarg arg2=0, t_floatarg arg3=0) {
  vMess(arg0, arg1, arg2, arg3);
  m_inlet[0] = inlet_new(this->x_obj, &this->x_obj->ob_pd, &s_float, gensym("plane"));
  m_inlet[1] = inlet_new(this->x_obj, &this->x_obj->ob_pd, &s_float, gensym("v"));
}
/////////////////////////////////////////////////////////
// Destructor
//
GEMglClipPlane :: ~GEMglClipPlane () {
  inlet_free(m_inlet[0]);
  inlet_free(m_inlet[1]);
}

/////////////////////////////////////////////////////////
// Render
//
void GEMglClipPlane :: render(GemState *state) {
  glClipPlane (plane, v);
}

/////////////////////////////////////////////////////////
// variable
//
void GEMglClipPlane :: vMess (t_float arg0, t_float arg1, t_float arg2, t_float arg3) {	// FUN
  v[0]=static_cast<GLdouble>(arg0);
  v[1]=static_cast<GLdouble>(arg1);
  v[2]=static_cast<GLdouble>(arg2);
  v[3]=static_cast<GLdouble>(arg3);
  setModified();
}

void GEMglClipPlane :: planeMess (t_float arg0) {	// FUN
  plane=static_cast<GLenum>(arg0);
  setModified();
}

/////////////////////////////////////////////////////////
// static member functions
//

void GEMglClipPlane :: obj_setupCallback(t_class *classPtr) {
  class_addmethod(classPtr, reinterpret_cast<t_method>(&GEMglClipPlane::vMessCallback),  	gensym("v"), A_DEFFLOAT, A_DEFFLOAT, A_DEFFLOAT, A_DEFFLOAT, A_NULL);
  class_addmethod(classPtr, reinterpret_cast<t_method>(&GEMglClipPlane::planeMessCallback),  	gensym("plane"), A_DEFFLOAT, A_NULL);
}

void GEMglClipPlane :: vMessCallback (void* data, t_floatarg arg0, t_floatarg arg1, t_floatarg arg2, t_floatarg arg3) {
  GetMyClass(data)->vMess ( arg0, arg1, arg2, arg3);
}

void GEMglClipPlane :: planeMessCallback (void* data, t_floatarg arg0) {
  GetMyClass(data)->planeMess ( arg0);
}
