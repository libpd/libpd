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

#include "GEMglLightModeli.h"

CPPEXTERN_NEW_WITH_TWO_ARGS ( GEMglLightModeli , t_floatarg, A_DEFFLOAT, t_floatarg, A_DEFFLOAT);

/////////////////////////////////////////////////////////
//
// GEMglViewport
//
/////////////////////////////////////////////////////////
// Constructor
//
GEMglLightModeli :: GEMglLightModeli	(t_floatarg arg0=0, t_floatarg arg1=0) :
		pname(static_cast<GLenum>(arg0)), 
		param(static_cast<GLint>(arg1))
{
	m_inlet[0] = inlet_new(this->x_obj, &this->x_obj->ob_pd, &s_float, gensym("pname"));
	m_inlet[1] = inlet_new(this->x_obj, &this->x_obj->ob_pd, &s_float, gensym("param"));
}
/////////////////////////////////////////////////////////
// Destructor
//
GEMglLightModeli :: ~GEMglLightModeli () {
inlet_free(m_inlet[0]);
inlet_free(m_inlet[1]);
}

/////////////////////////////////////////////////////////
// Render
//
void GEMglLightModeli :: render(GemState *state) {
	glLightModeli (pname, param);
}

/////////////////////////////////////////////////////////
// Variables
//
void GEMglLightModeli :: pnameMess (t_float arg1) {	// FUN
	pname = static_cast<GLenum>(arg1);
	setModified();
}

void GEMglLightModeli :: paramMess (t_float arg1) {	// FUN
	param = static_cast<GLint>(arg1);
	setModified();
}


/////////////////////////////////////////////////////////
// static member functions
//

void GEMglLightModeli :: obj_setupCallback(t_class *classPtr) {
	 class_addmethod(classPtr, reinterpret_cast<t_method>(&GEMglLightModeli::pnameMessCallback),  	gensym("pname"), A_DEFFLOAT, A_NULL);
	 class_addmethod(classPtr, reinterpret_cast<t_method>(&GEMglLightModeli::paramMessCallback),  	gensym("param"), A_DEFFLOAT, A_NULL);
};

void GEMglLightModeli :: pnameMessCallback (void* data, t_floatarg arg0){
	GetMyClass(data)->pnameMess ( static_cast<t_float>(arg0));
}
void GEMglLightModeli :: paramMessCallback (void* data, t_floatarg arg0){
	GetMyClass(data)->paramMess ( static_cast<t_float>(arg0));
}
