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

#include "GEMglCallList.h"

CPPEXTERN_NEW_WITH_ONE_ARG ( GEMglCallList , t_floatarg, A_DEFFLOAT);

/////////////////////////////////////////////////////////
//
// GEMglViewport
//
/////////////////////////////////////////////////////////
// Constructor
//
GEMglCallList :: GEMglCallList	(t_floatarg arg0=0) : list(static_cast<GLuint>(arg0)) {
  m_inlet = inlet_new(this->x_obj, &this->x_obj->ob_pd, &s_float, gensym("gllist"));
}
/////////////////////////////////////////////////////////
// Destructor
//
GEMglCallList :: ~GEMglCallList () {
  inlet_free(m_inlet);
}

/////////////////////////////////////////////////////////
// Render
//
void GEMglCallList :: render(GemState *state) {
  glCallList (list);
}

/////////////////////////////////////////////////////////
// Variables
//
void GEMglCallList :: listMess (t_float arg1) {	// FUN
  list = static_cast<GLuint>(arg1);
  setModified();
}


/////////////////////////////////////////////////////////
// static member functions
//

void GEMglCallList :: obj_setupCallback(t_class *classPtr) {
	 class_addmethod(classPtr, reinterpret_cast<t_method>(&GEMglCallList::listMessCallback),  	gensym("gllist"), A_DEFFLOAT, A_NULL);
}

void GEMglCallList :: listMessCallback (void* data, t_floatarg arg0){
	GetMyClass(data)->listMess (arg0);
}
