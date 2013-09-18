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

#include "GEMglDepthRange.h"

CPPEXTERN_NEW_WITH_TWO_ARGS ( GEMglDepthRange , t_floatarg, A_DEFFLOAT, t_floatarg, A_DEFFLOAT);

/////////////////////////////////////////////////////////
//
// GEMglViewport
//
/////////////////////////////////////////////////////////
// Constructor
//
GEMglDepthRange :: GEMglDepthRange	(t_floatarg arg0=0, t_floatarg arg1=0) :
		near_val(static_cast<GLclampd>(arg0)), 
		far_val(static_cast<GLclampd>(arg1))
{
	m_inlet[0] = inlet_new(this->x_obj, &this->x_obj->ob_pd, &s_float, gensym("near_val"));
	m_inlet[1] = inlet_new(this->x_obj, &this->x_obj->ob_pd, &s_float, gensym("far_val"));
}
/////////////////////////////////////////////////////////
// Destructor
//
GEMglDepthRange :: ~GEMglDepthRange () {
inlet_free(m_inlet[0]);
inlet_free(m_inlet[1]);
}

/////////////////////////////////////////////////////////
// Render
//
void GEMglDepthRange :: render(GemState *state) {
	glDepthRange (near_val, far_val);
}

/////////////////////////////////////////////////////////
// Variables
//
void GEMglDepthRange :: near_valMess (GLclampd arg1) {	// FUN
	near_val = static_cast<GLclampd>(arg1);
	setModified();
}

void GEMglDepthRange :: far_valMess (GLclampd arg1) {	// FUN
	far_val = static_cast<GLclampd>(arg1);
	setModified();
}


/////////////////////////////////////////////////////////
// static member functions
//

void GEMglDepthRange :: obj_setupCallback(t_class *classPtr) {
	 class_addmethod(classPtr, reinterpret_cast<t_method>(&GEMglDepthRange::near_valMessCallback),  	gensym("near_val"), A_DEFFLOAT, A_NULL);
	 class_addmethod(classPtr, reinterpret_cast<t_method>(&GEMglDepthRange::far_valMessCallback),  	gensym("far_val"), A_DEFFLOAT, A_NULL);
};

void GEMglDepthRange :: near_valMessCallback (void* data, GLclampd arg0){
	GetMyClass(data)->near_valMess ( static_cast<GLclampd>(arg0));
}
void GEMglDepthRange :: far_valMessCallback (void* data, GLclampd arg0){
	GetMyClass(data)->far_valMess ( static_cast<GLclampd>(arg0));
}
