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

#include "GEMglVertex2d.h"

CPPEXTERN_NEW_WITH_TWO_ARGS ( GEMglVertex2d , t_floatarg, A_DEFFLOAT, t_floatarg, A_DEFFLOAT);

/////////////////////////////////////////////////////////
//
// GEMglViewport
//
/////////////////////////////////////////////////////////
// Constructor
//
GEMglVertex2d :: GEMglVertex2d	(t_floatarg arg0=0, t_floatarg arg1=0) :
		x(static_cast<GLdouble>(arg0)), 
		y(static_cast<GLdouble>(arg1))
{
	m_inlet[0] = inlet_new(this->x_obj, &this->x_obj->ob_pd, &s_float, gensym("x"));
	m_inlet[1] = inlet_new(this->x_obj, &this->x_obj->ob_pd, &s_float, gensym("y"));
}
/////////////////////////////////////////////////////////
// Destructor
//
GEMglVertex2d :: ~GEMglVertex2d () {
inlet_free(m_inlet[0]);
inlet_free(m_inlet[1]);
}

/////////////////////////////////////////////////////////
// Render
//
void GEMglVertex2d :: render(GemState *state) {
	glVertex2d (x, y);
}

/////////////////////////////////////////////////////////
// Variables
//
void GEMglVertex2d :: xMess (t_float arg1) {	// FUN
	x = static_cast<GLdouble>(arg1);
	setModified();
}

void GEMglVertex2d :: yMess (t_float arg1) {	// FUN
	y = static_cast<GLdouble>(arg1);
	setModified();
}


/////////////////////////////////////////////////////////
// static member functions
//

void GEMglVertex2d :: obj_setupCallback(t_class *classPtr) {
	 class_addmethod(classPtr, reinterpret_cast<t_method>(&GEMglVertex2d::xMessCallback),  	gensym("x"), A_DEFFLOAT, A_NULL);
	 class_addmethod(classPtr, reinterpret_cast<t_method>(&GEMglVertex2d::yMessCallback),  	gensym("y"), A_DEFFLOAT, A_NULL);
};

void GEMglVertex2d :: xMessCallback (void* data, t_floatarg arg0){
	GetMyClass(data)->xMess ( static_cast<t_float>(arg0));
}
void GEMglVertex2d :: yMessCallback (void* data, t_floatarg arg0){
	GetMyClass(data)->yMess ( static_cast<t_float>(arg0));
}
