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

#include "GEMglTexGeni.h"

CPPEXTERN_NEW_WITH_THREE_ARGS ( GEMglTexGeni , t_floatarg, A_DEFFLOAT, t_floatarg, A_DEFFLOAT, t_floatarg, A_DEFFLOAT);

/////////////////////////////////////////////////////////
//
// GEMglViewport
//
/////////////////////////////////////////////////////////
// Constructor
//
GEMglTexGeni :: GEMglTexGeni	(t_floatarg arg0=0, t_floatarg arg1=0, t_floatarg arg2=0) :
		coord(static_cast<GLenum>(arg0)), 
		pname(static_cast<GLenum>(arg1)), 
		param(static_cast<GLint>(arg2))
{
	m_inlet[0] = inlet_new(this->x_obj, &this->x_obj->ob_pd, &s_float, gensym("coord"));
	m_inlet[1] = inlet_new(this->x_obj, &this->x_obj->ob_pd, &s_float, gensym("pname"));
	m_inlet[2] = inlet_new(this->x_obj, &this->x_obj->ob_pd, &s_float, gensym("param"));
}
/////////////////////////////////////////////////////////
// Destructor
//
GEMglTexGeni :: ~GEMglTexGeni () {
inlet_free(m_inlet[0]);
inlet_free(m_inlet[1]);
inlet_free(m_inlet[2]);
}

/////////////////////////////////////////////////////////
// Render
//
void GEMglTexGeni :: render(GemState *state) {
	glTexGeni (coord, pname, param);
}

/////////////////////////////////////////////////////////
// Variables
//
void GEMglTexGeni :: coordMess (t_float arg1) {	// FUN
	coord = static_cast<GLenum>(arg1);
	setModified();
}

void GEMglTexGeni :: pnameMess (t_float arg1) {	// FUN
	pname = static_cast<GLenum>(arg1);
	setModified();
}

void GEMglTexGeni :: paramMess (t_float arg1) {	// FUN
	param = static_cast<GLint>(arg1);
	setModified();
}


/////////////////////////////////////////////////////////
// static member functions
//

void GEMglTexGeni :: obj_setupCallback(t_class *classPtr) {
	 class_addmethod(classPtr, reinterpret_cast<t_method>(&GEMglTexGeni::coordMessCallback),  	gensym("coord"), A_DEFFLOAT, A_NULL);
	 class_addmethod(classPtr, reinterpret_cast<t_method>(&GEMglTexGeni::pnameMessCallback),  	gensym("pname"), A_DEFFLOAT, A_NULL);
	 class_addmethod(classPtr, reinterpret_cast<t_method>(&GEMglTexGeni::paramMessCallback),  	gensym("param"), A_DEFFLOAT, A_NULL);
};

void GEMglTexGeni :: coordMessCallback (void* data, t_floatarg arg0){
	GetMyClass(data)->coordMess ( static_cast<t_float>(arg0));
}
void GEMglTexGeni :: pnameMessCallback (void* data, t_floatarg arg0){
	GetMyClass(data)->pnameMess ( static_cast<t_float>(arg0));
}
void GEMglTexGeni :: paramMessCallback (void* data, t_floatarg arg0){
	GetMyClass(data)->paramMess ( static_cast<t_float>(arg0));
}
