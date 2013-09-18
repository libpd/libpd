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

#include "GEMglLineStipple.h"

CPPEXTERN_NEW_WITH_TWO_ARGS ( GEMglLineStipple , t_floatarg, A_DEFFLOAT, t_floatarg, A_DEFFLOAT);

/////////////////////////////////////////////////////////
//
// GEMglViewport
//
/////////////////////////////////////////////////////////
// Constructor
//
GEMglLineStipple :: GEMglLineStipple	(t_floatarg arg0=0, t_floatarg arg1=0) :
		factor(static_cast<GLint>(arg0)), 
		pattern(static_cast<GLushort>(arg1))
{
	m_inlet[0] = inlet_new(this->x_obj, &this->x_obj->ob_pd, &s_float, gensym("factor"));
	m_inlet[1] = inlet_new(this->x_obj, &this->x_obj->ob_pd, &s_float, gensym("pattern"));
}
/////////////////////////////////////////////////////////
// Destructor
//
GEMglLineStipple :: ~GEMglLineStipple () {
inlet_free(m_inlet[0]);
inlet_free(m_inlet[1]);
}

/////////////////////////////////////////////////////////
// Render
//
void GEMglLineStipple :: render(GemState *state) {
	glLineStipple (factor, pattern);
}

/////////////////////////////////////////////////////////
// Variables
//
void GEMglLineStipple :: factorMess (t_float arg1) {	// FUN
	factor = static_cast<GLint>(arg1);
	setModified();
}

void GEMglLineStipple :: patternMess (t_float arg1) {	// FUN
	pattern = static_cast<GLushort>(arg1);
	setModified();
}


/////////////////////////////////////////////////////////
// static member functions
//

void GEMglLineStipple :: obj_setupCallback(t_class *classPtr) {
	 class_addmethod(classPtr, reinterpret_cast<t_method>(&GEMglLineStipple::factorMessCallback),  	gensym("factor"), A_DEFFLOAT, A_NULL);
	 class_addmethod(classPtr, reinterpret_cast<t_method>(&GEMglLineStipple::patternMessCallback),  	gensym("pattern"), A_DEFFLOAT, A_NULL);
};

void GEMglLineStipple :: factorMessCallback (void* data, t_floatarg arg0){
	GetMyClass(data)->factorMess ( static_cast<t_float>(arg0));
}
void GEMglLineStipple :: patternMessCallback (void* data, t_floatarg arg0){
	GetMyClass(data)->patternMess ( static_cast<t_float>(arg0));
}
