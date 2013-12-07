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

#include "GEMglPolygonOffset.h"

CPPEXTERN_NEW_WITH_TWO_ARGS ( GEMglPolygonOffset , t_floatarg, A_DEFFLOAT, t_floatarg, A_DEFFLOAT);

/////////////////////////////////////////////////////////
//
// GEMglViewport
//
/////////////////////////////////////////////////////////
// Constructor
//
GEMglPolygonOffset :: GEMglPolygonOffset	(t_floatarg arg0=0, t_floatarg arg1=0) :
		factor(static_cast<GLfloat>(arg0)), 
		units(static_cast<GLfloat>(arg1))
{
	m_inlet[0] = inlet_new(this->x_obj, &this->x_obj->ob_pd, &s_float, gensym("factor"));
	m_inlet[1] = inlet_new(this->x_obj, &this->x_obj->ob_pd, &s_float, gensym("units"));
}
/////////////////////////////////////////////////////////
// Destructor
//
GEMglPolygonOffset :: ~GEMglPolygonOffset () {
inlet_free(m_inlet[0]);
inlet_free(m_inlet[1]);
}

/////////////////////////////////////////////////////////
// Render
//
void GEMglPolygonOffset :: render(GemState *state) {
	glPolygonOffset (factor, units);
}

/////////////////////////////////////////////////////////
// Variables
//
void GEMglPolygonOffset :: factorMess (t_float arg1) {	// FUN
	factor = static_cast<GLfloat>(arg1);
	setModified();
}

void GEMglPolygonOffset :: unitsMess (t_float arg1) {	// FUN
	units = static_cast<GLfloat>(arg1);
	setModified();
}


/////////////////////////////////////////////////////////
// static member functions
//

void GEMglPolygonOffset :: obj_setupCallback(t_class *classPtr) {
	 class_addmethod(classPtr, reinterpret_cast<t_method>(&GEMglPolygonOffset::factorMessCallback),  	gensym("factor"), A_DEFFLOAT, A_NULL);
	 class_addmethod(classPtr, reinterpret_cast<t_method>(&GEMglPolygonOffset::unitsMessCallback),  	gensym("units"), A_DEFFLOAT, A_NULL);
};

void GEMglPolygonOffset :: factorMessCallback (void* data, t_floatarg arg0){
	GetMyClass(data)->factorMess ( static_cast<t_float>(arg0));
}
void GEMglPolygonOffset :: unitsMessCallback (void* data, t_floatarg arg0){
	GetMyClass(data)->unitsMess ( static_cast<t_float>(arg0));
}
