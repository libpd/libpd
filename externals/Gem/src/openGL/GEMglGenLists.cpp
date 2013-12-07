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

#include "GEMglGenLists.h"

CPPEXTERN_NEW_WITH_ONE_ARG ( GEMglGenLists , t_floatarg, A_DEFFLOAT);

/////////////////////////////////////////////////////////
//
// GEMglViewport
//
/////////////////////////////////////////////////////////
// Constructor
//
GEMglGenLists :: GEMglGenLists	(t_floatarg arg0=0) :
		range(static_cast<GLsizei>(arg0))
{
	m_inlet = inlet_new(this->x_obj, &this->x_obj->ob_pd, &s_float, gensym("range"));
	m_outlet= outlet_new(this->x_obj, 0);
}
/////////////////////////////////////////////////////////
// Destructor
//
GEMglGenLists :: ~GEMglGenLists () {
   inlet_free(m_inlet);
  outlet_free(m_outlet);
}

/////////////////////////////////////////////////////////
// Render
//
void GEMglGenLists :: render(GemState *state) {
  GLuint i = glGenLists(range);
  outlet_float(m_outlet, static_cast<t_float>(i));
}

/////////////////////////////////////////////////////////
// Variables
//
void GEMglGenLists :: rangeMess (t_float arg1) {	// FUN
  range = static_cast<GLsizei>(arg1);
  setModified();
}


/////////////////////////////////////////////////////////
// static member functions
//

void GEMglGenLists :: obj_setupCallback(t_class *classPtr) {
	 class_addmethod(classPtr, reinterpret_cast<t_method>(&GEMglGenLists::rangeMessCallback),  	gensym("range"), A_DEFFLOAT, A_NULL);
}

void GEMglGenLists :: rangeMessCallback (void* data, t_floatarg arg0){
	GetMyClass(data)->rangeMess (arg0);
}
