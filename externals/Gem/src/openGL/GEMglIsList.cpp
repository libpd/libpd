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

#include "GEMglIsList.h"

CPPEXTERN_NEW_WITH_ONE_ARG ( GEMglIsList , t_floatarg, A_DEFFLOAT);

/////////////////////////////////////////////////////////
//
// GEMglViewport
//
/////////////////////////////////////////////////////////
// Constructor
//
GEMglIsList :: GEMglIsList	(t_floatarg arg0=0) : list(static_cast<GLuint>(arg0)) {
  m_inlet = inlet_new(this->x_obj, &this->x_obj->ob_pd, &s_float, gensym("gllist"));
  m_outlet=outlet_new(this->x_obj, 0);
}
/////////////////////////////////////////////////////////
// Destructor
//
GEMglIsList :: ~GEMglIsList () {
  inlet_free(m_inlet);
  outlet_free(m_outlet);
}

/////////////////////////////////////////////////////////
// Render
//
void GEMglIsList :: render(GemState *state) {
  GLboolean b = glIsList (list);
  outlet_float(m_outlet, b?1.0:0.0);
}

/////////////////////////////////////////////////////////
// Variables
//
void GEMglIsList :: listMess (t_float arg1) {	// FUN
  list = static_cast<GLuint>(arg1);
  setModified();
}


/////////////////////////////////////////////////////////
// static member functions
//

void GEMglIsList :: obj_setupCallback(t_class *classPtr) {
	 class_addmethod(classPtr, reinterpret_cast<t_method>(&GEMglIsList::listMessCallback),  	gensym("gllist"), A_DEFFLOAT, A_NULL);
}

void GEMglIsList :: listMessCallback (void* data, t_floatarg arg0){
	GetMyClass(data)->listMess (arg0);
}
