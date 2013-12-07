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

#include "GEMglNormal3s.h"

CPPEXTERN_NEW_WITH_THREE_ARGS ( GEMglNormal3s , t_floatarg, A_DEFFLOAT, t_floatarg, A_DEFFLOAT, t_floatarg, A_DEFFLOAT);

/////////////////////////////////////////////////////////
//
// GEMglViewport
//
/////////////////////////////////////////////////////////
// Constructor
//
GEMglNormal3s :: GEMglNormal3s	(t_floatarg arg0=0, t_floatarg arg1=0, t_floatarg arg2=0) :
		nx(static_cast<GLshort>(arg0)), 
		ny(static_cast<GLshort>(arg1)), 
		nz(static_cast<GLshort>(arg2))
{
	m_inlet[0] = inlet_new(this->x_obj, &this->x_obj->ob_pd, &s_float, gensym("nx"));
	m_inlet[1] = inlet_new(this->x_obj, &this->x_obj->ob_pd, &s_float, gensym("ny"));
	m_inlet[2] = inlet_new(this->x_obj, &this->x_obj->ob_pd, &s_float, gensym("nz"));
}
/////////////////////////////////////////////////////////
// Destructor
//
GEMglNormal3s :: ~GEMglNormal3s () {
inlet_free(m_inlet[0]);
inlet_free(m_inlet[1]);
inlet_free(m_inlet[2]);
}

/////////////////////////////////////////////////////////
// Render
//
void GEMglNormal3s :: render(GemState *state) {
	glNormal3s (nx, ny, nz);
}

/////////////////////////////////////////////////////////
// Variables
//
void GEMglNormal3s :: nxMess (t_float arg1) {	// FUN
	nx = static_cast<GLshort>(arg1);
	setModified();
}

void GEMglNormal3s :: nyMess (t_float arg1) {	// FUN
	ny = static_cast<GLshort>(arg1);
	setModified();
}

void GEMglNormal3s :: nzMess (t_float arg1) {	// FUN
	nz = static_cast<GLshort>(arg1);
	setModified();
}


/////////////////////////////////////////////////////////
// static member functions
//

void GEMglNormal3s :: obj_setupCallback(t_class *classPtr) {
	 class_addmethod(classPtr, reinterpret_cast<t_method>(&GEMglNormal3s::nxMessCallback),  	gensym("nx"), A_DEFFLOAT, A_NULL);
	 class_addmethod(classPtr, reinterpret_cast<t_method>(&GEMglNormal3s::nyMessCallback),  	gensym("ny"), A_DEFFLOAT, A_NULL);
	 class_addmethod(classPtr, reinterpret_cast<t_method>(&GEMglNormal3s::nzMessCallback),  	gensym("nz"), A_DEFFLOAT, A_NULL);
};

void GEMglNormal3s :: nxMessCallback (void* data, t_floatarg arg0){
	GetMyClass(data)->nxMess ( static_cast<t_float>(arg0));
}
void GEMglNormal3s :: nyMessCallback (void* data, t_floatarg arg0){
	GetMyClass(data)->nyMess ( static_cast<t_float>(arg0));
}
void GEMglNormal3s :: nzMessCallback (void* data, t_floatarg arg0){
	GetMyClass(data)->nzMess ( static_cast<t_float>(arg0));
}
