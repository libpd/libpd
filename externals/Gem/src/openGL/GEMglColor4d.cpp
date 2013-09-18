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

#include "GEMglColor4d.h"

CPPEXTERN_NEW_WITH_FOUR_ARGS ( GEMglColor4d , t_floatarg, A_DEFFLOAT, t_floatarg, A_DEFFLOAT, t_floatarg, A_DEFFLOAT, t_floatarg, A_DEFFLOAT);

/////////////////////////////////////////////////////////
//
// GEMglViewport
//
/////////////////////////////////////////////////////////
// Constructor
//
GEMglColor4d :: GEMglColor4d	(t_floatarg arg0=0, t_floatarg arg1=0, t_floatarg arg2=0, t_floatarg arg3=0) :
		red(static_cast<GLdouble>(arg0)), 
		green(static_cast<GLdouble>(arg1)), 
		blue(static_cast<GLdouble>(arg2)), 
		alpha(static_cast<GLdouble>(arg3))
{
	m_inlet[0] = inlet_new(this->x_obj, &this->x_obj->ob_pd, &s_float, gensym("red"));
	m_inlet[1] = inlet_new(this->x_obj, &this->x_obj->ob_pd, &s_float, gensym("green"));
	m_inlet[2] = inlet_new(this->x_obj, &this->x_obj->ob_pd, &s_float, gensym("blue"));
	m_inlet[3] = inlet_new(this->x_obj, &this->x_obj->ob_pd, &s_float, gensym("alpha"));
}
/////////////////////////////////////////////////////////
// Destructor
//
GEMglColor4d :: ~GEMglColor4d () {
inlet_free(m_inlet[0]);
inlet_free(m_inlet[1]);
inlet_free(m_inlet[2]);
inlet_free(m_inlet[3]);
}

/////////////////////////////////////////////////////////
// Render
//
void GEMglColor4d :: render(GemState *state) {
	glColor4d (red, green, blue, alpha);
}

/////////////////////////////////////////////////////////
// Variables
//
void GEMglColor4d :: redMess (t_float arg1) {	// FUN
	red = static_cast<GLdouble>(arg1);
	setModified();
}

void GEMglColor4d :: greenMess (t_float arg1) {	// FUN
	green = static_cast<GLdouble>(arg1);
	setModified();
}

void GEMglColor4d :: blueMess (t_float arg1) {	// FUN
	blue = static_cast<GLdouble>(arg1);
	setModified();
}

void GEMglColor4d :: alphaMess (t_float arg1) {	// FUN
	alpha = static_cast<GLdouble>(arg1);
	setModified();
}


/////////////////////////////////////////////////////////
// static member functions
//

void GEMglColor4d :: obj_setupCallback(t_class *classPtr) {
	 class_addmethod(classPtr, reinterpret_cast<t_method>(&GEMglColor4d::redMessCallback),  	gensym("red"), A_DEFFLOAT, A_NULL);
	 class_addmethod(classPtr, reinterpret_cast<t_method>(&GEMglColor4d::greenMessCallback),  	gensym("green"), A_DEFFLOAT, A_NULL);
	 class_addmethod(classPtr, reinterpret_cast<t_method>(&GEMglColor4d::blueMessCallback),  	gensym("blue"), A_DEFFLOAT, A_NULL);
	 class_addmethod(classPtr, reinterpret_cast<t_method>(&GEMglColor4d::alphaMessCallback),  	gensym("alpha"), A_DEFFLOAT, A_NULL);
};

void GEMglColor4d :: redMessCallback (void* data, t_floatarg arg0){
	GetMyClass(data)->redMess ( static_cast<t_float>(arg0));
}
void GEMglColor4d :: greenMessCallback (void* data, t_floatarg arg0){
	GetMyClass(data)->greenMess ( static_cast<t_float>(arg0));
}
void GEMglColor4d :: blueMessCallback (void* data, t_floatarg arg0){
	GetMyClass(data)->blueMess ( static_cast<t_float>(arg0));
}
void GEMglColor4d :: alphaMessCallback (void* data, t_floatarg arg0){
	GetMyClass(data)->alphaMess ( static_cast<t_float>(arg0));
}
