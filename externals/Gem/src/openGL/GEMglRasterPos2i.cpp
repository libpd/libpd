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

#include "GEMglRasterPos2i.h"

CPPEXTERN_NEW_WITH_TWO_ARGS ( GEMglRasterPos2i , t_floatarg, A_DEFFLOAT, t_floatarg, A_DEFFLOAT);

/////////////////////////////////////////////////////////
//
// GEMglViewport
//
/////////////////////////////////////////////////////////
// Constructor
//
GEMglRasterPos2i :: GEMglRasterPos2i	(t_floatarg arg0=0, t_floatarg arg1=0) :
		x(static_cast<GLint>(arg0)), 
		y(static_cast<GLint>(arg1))
{
	m_inlet[0] = inlet_new(this->x_obj, &this->x_obj->ob_pd, &s_float, gensym("x"));
	m_inlet[1] = inlet_new(this->x_obj, &this->x_obj->ob_pd, &s_float, gensym("y"));
}
/////////////////////////////////////////////////////////
// Destructor
//
GEMglRasterPos2i :: ~GEMglRasterPos2i () {
inlet_free(m_inlet[0]);
inlet_free(m_inlet[1]);
}

/////////////////////////////////////////////////////////
// Render
//
void GEMglRasterPos2i :: render(GemState *state) {
	glRasterPos2i (x, y);
}

/////////////////////////////////////////////////////////
// Variables
//
void GEMglRasterPos2i :: xMess (t_float arg1) {	// FUN
	x = static_cast<GLint>(arg1);
	setModified();
}

void GEMglRasterPos2i :: yMess (t_float arg1) {	// FUN
	y = static_cast<GLint>(arg1);
	setModified();
}


/////////////////////////////////////////////////////////
// static member functions
//

void GEMglRasterPos2i :: obj_setupCallback(t_class *classPtr) {
	 class_addmethod(classPtr, reinterpret_cast<t_method>(&GEMglRasterPos2i::xMessCallback),  	gensym("x"), A_DEFFLOAT, A_NULL);
	 class_addmethod(classPtr, reinterpret_cast<t_method>(&GEMglRasterPos2i::yMessCallback),  	gensym("y"), A_DEFFLOAT, A_NULL);
};

void GEMglRasterPos2i :: xMessCallback (void* data, t_floatarg arg0){
	GetMyClass(data)->xMess ( static_cast<t_float>(arg0));
}
void GEMglRasterPos2i :: yMessCallback (void* data, t_floatarg arg0){
	GetMyClass(data)->yMess ( static_cast<t_float>(arg0));
}
