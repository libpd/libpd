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

#include "GEMglRotated.h"

CPPEXTERN_NEW_WITH_FOUR_ARGS ( GEMglRotated , t_floatarg, A_DEFFLOAT, t_floatarg, A_DEFFLOAT, t_floatarg, A_DEFFLOAT, t_floatarg, A_DEFFLOAT);

/////////////////////////////////////////////////////////
//
// GEMglViewport
//
/////////////////////////////////////////////////////////
// Constructor
//
GEMglRotated :: GEMglRotated	(t_floatarg arg0=0, t_floatarg arg1=0, t_floatarg arg2=0, t_floatarg arg3=0) :
		angle(static_cast<GLdouble>(arg0)), 
		x(static_cast<GLdouble>(arg1)), 
		y(static_cast<GLdouble>(arg2)), 
		z(static_cast<GLdouble>(arg3))
{
	m_inlet[0] = inlet_new(this->x_obj, &this->x_obj->ob_pd, &s_float, gensym("angle"));
	m_inlet[1] = inlet_new(this->x_obj, &this->x_obj->ob_pd, &s_float, gensym("x"));
	m_inlet[2] = inlet_new(this->x_obj, &this->x_obj->ob_pd, &s_float, gensym("y"));
	m_inlet[3] = inlet_new(this->x_obj, &this->x_obj->ob_pd, &s_float, gensym("z"));
}
/////////////////////////////////////////////////////////
// Destructor
//
GEMglRotated :: ~GEMglRotated () {
inlet_free(m_inlet[0]);
inlet_free(m_inlet[1]);
inlet_free(m_inlet[2]);
inlet_free(m_inlet[3]);
}

/////////////////////////////////////////////////////////
// Render
//
void GEMglRotated :: render(GemState *state) {
	glRotated (angle, x, y, z);
}

/////////////////////////////////////////////////////////
// Variables
//
void GEMglRotated :: angleMess (t_float arg1) {	// FUN
	angle = static_cast<GLdouble>(arg1);
	setModified();
}

void GEMglRotated :: xMess (t_float arg1) {	// FUN
	x = static_cast<GLdouble>(arg1);
	setModified();
}

void GEMglRotated :: yMess (t_float arg1) {	// FUN
	y = static_cast<GLdouble>(arg1);
	setModified();
}

void GEMglRotated :: zMess (t_float arg1) {	// FUN
	z = static_cast<GLdouble>(arg1);
	setModified();
}


/////////////////////////////////////////////////////////
// static member functions
//

void GEMglRotated :: obj_setupCallback(t_class *classPtr) {
	 class_addmethod(classPtr, reinterpret_cast<t_method>(&GEMglRotated::angleMessCallback),  	gensym("angle"), A_DEFFLOAT, A_NULL);
	 class_addmethod(classPtr, reinterpret_cast<t_method>(&GEMglRotated::xMessCallback),  	gensym("x"), A_DEFFLOAT, A_NULL);
	 class_addmethod(classPtr, reinterpret_cast<t_method>(&GEMglRotated::yMessCallback),  	gensym("y"), A_DEFFLOAT, A_NULL);
	 class_addmethod(classPtr, reinterpret_cast<t_method>(&GEMglRotated::zMessCallback),  	gensym("z"), A_DEFFLOAT, A_NULL);
};

void GEMglRotated :: angleMessCallback (void* data, t_floatarg arg0){
	GetMyClass(data)->angleMess ( static_cast<t_float>(arg0));
}
void GEMglRotated :: xMessCallback (void* data, t_floatarg arg0){
	GetMyClass(data)->xMess ( static_cast<t_float>(arg0));
}
void GEMglRotated :: yMessCallback (void* data, t_floatarg arg0){
	GetMyClass(data)->yMess ( static_cast<t_float>(arg0));
}
void GEMglRotated :: zMessCallback (void* data, t_floatarg arg0){
	GetMyClass(data)->zMess ( static_cast<t_float>(arg0));
}
