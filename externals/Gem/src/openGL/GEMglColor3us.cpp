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

#include "GEMglColor3us.h"

CPPEXTERN_NEW_WITH_THREE_ARGS ( GEMglColor3us , t_floatarg, A_DEFFLOAT, t_floatarg, A_DEFFLOAT, t_floatarg, A_DEFFLOAT);

/////////////////////////////////////////////////////////
//
// GEMglViewport
//
/////////////////////////////////////////////////////////
// Constructor
//
GEMglColor3us :: GEMglColor3us	(t_floatarg arg0=0, t_floatarg arg1=0, t_floatarg arg2=0) :
		red(static_cast<GLushort>(arg0)), 
		green(static_cast<GLushort>(arg1)), 
		blue(static_cast<GLushort>(arg2))
{
	m_inlet[0] = inlet_new(this->x_obj, &this->x_obj->ob_pd, &s_float, gensym("red"));
	m_inlet[1] = inlet_new(this->x_obj, &this->x_obj->ob_pd, &s_float, gensym("green"));
	m_inlet[2] = inlet_new(this->x_obj, &this->x_obj->ob_pd, &s_float, gensym("blue"));
}
/////////////////////////////////////////////////////////
// Destructor
//
GEMglColor3us :: ~GEMglColor3us () {
inlet_free(m_inlet[0]);
inlet_free(m_inlet[1]);
inlet_free(m_inlet[2]);
}

/////////////////////////////////////////////////////////
// Render
//
void GEMglColor3us :: render(GemState *state) {
	glColor3us (red, green, blue);
}

/////////////////////////////////////////////////////////
// Variables
//
void GEMglColor3us :: redMess (t_float arg1) {	// FUN
	red = static_cast<GLushort>(arg1);
	setModified();
}

void GEMglColor3us :: greenMess (t_float arg1) {	// FUN
	green = static_cast<GLushort>(arg1);
	setModified();
}

void GEMglColor3us :: blueMess (t_float arg1) {	// FUN
	blue = static_cast<GLushort>(arg1);
	setModified();
}


/////////////////////////////////////////////////////////
// static member functions
//

void GEMglColor3us :: obj_setupCallback(t_class *classPtr) {
	 class_addmethod(classPtr, reinterpret_cast<t_method>(&GEMglColor3us::redMessCallback),  	gensym("red"), A_DEFFLOAT, A_NULL);
	 class_addmethod(classPtr, reinterpret_cast<t_method>(&GEMglColor3us::greenMessCallback),  	gensym("green"), A_DEFFLOAT, A_NULL);
	 class_addmethod(classPtr, reinterpret_cast<t_method>(&GEMglColor3us::blueMessCallback),  	gensym("blue"), A_DEFFLOAT, A_NULL);
};

void GEMglColor3us :: redMessCallback (void* data, t_floatarg arg0){
	GetMyClass(data)->redMess ( static_cast<t_float>(arg0));
}
void GEMglColor3us :: greenMessCallback (void* data, t_floatarg arg0){
	GetMyClass(data)->greenMess ( static_cast<t_float>(arg0));
}
void GEMglColor3us :: blueMessCallback (void* data, t_floatarg arg0){
	GetMyClass(data)->blueMess ( static_cast<t_float>(arg0));
}
