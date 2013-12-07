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

#include "GEMglNormal3f.h"

CPPEXTERN_NEW_WITH_THREE_ARGS ( GEMglNormal3f , t_floatarg, A_DEFFLOAT, t_floatarg, A_DEFFLOAT, t_floatarg, A_DEFFLOAT);

/////////////////////////////////////////////////////////
//
// GEMglViewport
//
/////////////////////////////////////////////////////////
// Constructor
//
GEMglNormal3f :: GEMglNormal3f	(t_floatarg arg0=0, t_floatarg arg1=0, t_floatarg arg2=0) :
		nx(static_cast<GLfloat>(arg0)), 
		ny(static_cast<GLfloat>(arg1)), 
		nz(static_cast<GLfloat>(arg2))
{
	m_inlet[0] = inlet_new(this->x_obj, &this->x_obj->ob_pd, &s_float, gensym("nx"));
	m_inlet[1] = inlet_new(this->x_obj, &this->x_obj->ob_pd, &s_float, gensym("ny"));
	m_inlet[2] = inlet_new(this->x_obj, &this->x_obj->ob_pd, &s_float, gensym("nz"));
}
/////////////////////////////////////////////////////////
// Destructor
//
GEMglNormal3f :: ~GEMglNormal3f () {
inlet_free(m_inlet[0]);
inlet_free(m_inlet[1]);
inlet_free(m_inlet[2]);
}

/////////////////////////////////////////////////////////
// Render
//
void GEMglNormal3f :: render(GemState *state) {
	glNormal3f (nx, ny, nz);
}

/////////////////////////////////////////////////////////
// Variables
//
void GEMglNormal3f :: nxMess (t_float arg1) {	// FUN
	nx = static_cast<GLfloat>(arg1);
	setModified();
}

void GEMglNormal3f :: nyMess (t_float arg1) {	// FUN
	ny = static_cast<GLfloat>(arg1);
	setModified();
}

void GEMglNormal3f :: nzMess (t_float arg1) {	// FUN
	nz = static_cast<GLfloat>(arg1);
	setModified();
}


/////////////////////////////////////////////////////////
// static member functions
//

void GEMglNormal3f :: obj_setupCallback(t_class *classPtr) {
	 class_addmethod(classPtr, reinterpret_cast<t_method>(&GEMglNormal3f::nxMessCallback),  	gensym("nx"), A_DEFFLOAT, A_NULL);
	 class_addmethod(classPtr, reinterpret_cast<t_method>(&GEMglNormal3f::nyMessCallback),  	gensym("ny"), A_DEFFLOAT, A_NULL);
	 class_addmethod(classPtr, reinterpret_cast<t_method>(&GEMglNormal3f::nzMessCallback),  	gensym("nz"), A_DEFFLOAT, A_NULL);
};

void GEMglNormal3f :: nxMessCallback (void* data, t_floatarg arg0){
	GetMyClass(data)->nxMess ( static_cast<t_float>(arg0));
}
void GEMglNormal3f :: nyMessCallback (void* data, t_floatarg arg0){
	GetMyClass(data)->nyMess ( static_cast<t_float>(arg0));
}
void GEMglNormal3f :: nzMessCallback (void* data, t_floatarg arg0){
	GetMyClass(data)->nzMess ( static_cast<t_float>(arg0));
}
