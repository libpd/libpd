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

#include "GEMglGetString.h"

CPPEXTERN_NEW_WITH_ONE_ARG ( GEMglGetString , t_floatarg, A_DEFFLOAT);

/////////////////////////////////////////////////////////
//
// GEMglViewport
//
/////////////////////////////////////////////////////////
// Constructor
//
GEMglGetString :: GEMglGetString	(t_floatarg arg0=0) :
  name(static_cast<GLenum>(arg0)) {
  m_inlet = inlet_new(this->x_obj, &this->x_obj->ob_pd, &s_float, gensym("name"));
  m_outlet=outlet_new(this->x_obj, 0);
}
/////////////////////////////////////////////////////////
// Destructor
//
GEMglGetString :: ~GEMglGetString () {
  inlet_free (m_inlet);
  outlet_free(m_outlet);
}

/////////////////////////////////////////////////////////
// Render
//
void GEMglGetString :: render(GemState *state) {
  const char* test=reinterpret_cast<const char*>(glGetString (name));
  if(test!=NULL)outlet_symbol(m_outlet, gensym(test));
}

/////////////////////////////////////////////////////////
// Variables
//
void GEMglGetString :: nameMess (t_atom arg) {	// FUN
  name = static_cast<GLenum>(getGLdefine(&arg));
  setModified();
}

/////////////////////////////////////////////////////////
// static member functions
//
void GEMglGetString :: obj_setupCallback(t_class *classPtr) {
  class_addmethod(classPtr, reinterpret_cast<t_method>(&GEMglGetString::nameMessCallback), gensym("name"), A_GIMME, A_NULL);
}

void GEMglGetString :: nameMessCallback (void* data, t_symbol*, int argc, t_atom*argv){
  if(argc==1)GetMyClass(data)->nameMess ( argv[0]);
}
