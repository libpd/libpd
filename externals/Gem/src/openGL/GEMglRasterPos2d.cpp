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

#include "GEMglRasterPos2d.h"

CPPEXTERN_NEW_WITH_TWO_ARGS ( GEMglRasterPos2d , t_floatarg, A_DEFFLOAT, t_floatarg, A_DEFFLOAT);

/////////////////////////////////////////////////////////
//
// GEMglViewport
//
/////////////////////////////////////////////////////////
// Constructor
//
GEMglRasterPos2d :: GEMglRasterPos2d	(t_floatarg arg0=0, t_floatarg arg1=0) :
		x(static_cast<GLdouble>(arg0)), 
		y(static_cast<GLdouble>(arg1))
{
	m_inlet[0] = inlet_new(this->x_obj, &this->x_obj->ob_pd, &s_float, gensym("x"));
	m_inlet[1] = inlet_new(this->x_obj, &this->x_obj->ob_pd, &s_float, gensym("y"));
}
/////////////////////////////////////////////////////////
// Destructor
//
GEMglRasterPos2d :: ~GEMglRasterPos2d () {
inlet_free(m_inlet[0]);
inlet_free(m_inlet[1]);
}

/////////////////////////////////////////////////////////
// Render
//
void GEMglRasterPos2d :: render(GemState *state) {
	glRasterPos2d (x, y);
}

/////////////////////////////////////////////////////////
// Variables
//
void GEMglRasterPos2d :: xMess (t_float arg1) {	// FUN
	x = static_cast<GLdouble>(arg1);
	setModified();
}

void GEMglRasterPos2d :: yMess (t_float arg1) {	// FUN
	y = static_cast<GLdouble>(arg1);
	setModified();
}


/////////////////////////////////////////////////////////
// static member functions
//

void GEMglRasterPos2d :: obj_setupCallback(t_class *classPtr) {
	 class_addmethod(classPtr, reinterpret_cast<t_method>(&GEMglRasterPos2d::xMessCallback),  	gensym("x"), A_DEFFLOAT, A_NULL);
	 class_addmethod(classPtr, reinterpret_cast<t_method>(&GEMglRasterPos2d::yMessCallback),  	gensym("y"), A_DEFFLOAT, A_NULL);
};

void GEMglRasterPos2d :: xMessCallback (void* data, t_floatarg arg0){
	GetMyClass(data)->xMess ( static_cast<t_float>(arg0));
}
void GEMglRasterPos2d :: yMessCallback (void* data, t_floatarg arg0){
	GetMyClass(data)->yMess ( static_cast<t_float>(arg0));
}
