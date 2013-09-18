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

#include "GEMglLighti.h"

CPPEXTERN_NEW_WITH_THREE_ARGS ( GEMglLighti , t_floatarg, A_DEFFLOAT, t_floatarg, A_DEFFLOAT, t_floatarg, A_DEFFLOAT);

/////////////////////////////////////////////////////////
//
// GEMglViewport
//
/////////////////////////////////////////////////////////
// Constructor
//
GEMglLighti :: GEMglLighti	(t_floatarg arg0=0, t_floatarg arg1=0, t_floatarg arg2=0) :
		light(static_cast<GLenum>(arg0)), 
		pname(static_cast<GLenum>(arg1)), 
		param(static_cast<GLint>(arg2))
{
	m_inlet[0] = inlet_new(this->x_obj, &this->x_obj->ob_pd, &s_float, gensym("light"));
	m_inlet[1] = inlet_new(this->x_obj, &this->x_obj->ob_pd, &s_float, gensym("pname"));
	m_inlet[2] = inlet_new(this->x_obj, &this->x_obj->ob_pd, &s_float, gensym("param"));
}
/////////////////////////////////////////////////////////
// Destructor
//
GEMglLighti :: ~GEMglLighti () {
inlet_free(m_inlet[0]);
inlet_free(m_inlet[1]);
inlet_free(m_inlet[2]);
}

/////////////////////////////////////////////////////////
// Render
//
void GEMglLighti :: render(GemState *state) {
	glLighti (light, pname, param);
}

/////////////////////////////////////////////////////////
// Variables
//
void GEMglLighti :: lightMess (t_float arg1) {	// FUN
	light = static_cast<GLenum>(arg1);
	setModified();
}

void GEMglLighti :: pnameMess (t_float arg1) {	// FUN
	pname = static_cast<GLenum>(arg1);
	setModified();
}

void GEMglLighti :: paramMess (t_float arg1) {	// FUN
	param = static_cast<GLint>(arg1);
	setModified();
}


/////////////////////////////////////////////////////////
// static member functions
//

void GEMglLighti :: obj_setupCallback(t_class *classPtr) {
	 class_addmethod(classPtr, reinterpret_cast<t_method>(&GEMglLighti::lightMessCallback),  	gensym("light"), A_DEFFLOAT, A_NULL);
	 class_addmethod(classPtr, reinterpret_cast<t_method>(&GEMglLighti::pnameMessCallback),  	gensym("pname"), A_DEFFLOAT, A_NULL);
	 class_addmethod(classPtr, reinterpret_cast<t_method>(&GEMglLighti::paramMessCallback),  	gensym("param"), A_DEFFLOAT, A_NULL);
};

void GEMglLighti :: lightMessCallback (void* data, t_floatarg arg0){
	GetMyClass(data)->lightMess ( static_cast<t_float>(arg0));
}
void GEMglLighti :: pnameMessCallback (void* data, t_floatarg arg0){
	GetMyClass(data)->pnameMess ( static_cast<t_float>(arg0));
}
void GEMglLighti :: paramMessCallback (void* data, t_floatarg arg0){
	GetMyClass(data)->paramMess ( static_cast<t_float>(arg0));
}
