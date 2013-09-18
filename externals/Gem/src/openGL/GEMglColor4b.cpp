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

#include "GEMglColor4b.h"

CPPEXTERN_NEW_WITH_FOUR_ARGS ( GEMglColor4b , t_floatarg, A_DEFFLOAT, t_floatarg, A_DEFFLOAT, t_floatarg, A_DEFFLOAT, t_floatarg, A_DEFFLOAT);

/////////////////////////////////////////////////////////
//
// GEMglViewport
//
/////////////////////////////////////////////////////////
// Constructor
//
GEMglColor4b :: GEMglColor4b	(t_floatarg arg0=0, t_floatarg arg1=0, t_floatarg arg2=0, t_floatarg arg3=0) :
		red(static_cast<GLbyte>(arg0)), 
		green(static_cast<GLbyte>(arg1)), 
		blue(static_cast<GLbyte>(arg2)), 
		alpha(static_cast<GLbyte>(arg3))
{
	m_inlet[0] = inlet_new(this->x_obj, &this->x_obj->ob_pd, &s_float, gensym("red"));
	m_inlet[1] = inlet_new(this->x_obj, &this->x_obj->ob_pd, &s_float, gensym("green"));
	m_inlet[2] = inlet_new(this->x_obj, &this->x_obj->ob_pd, &s_float, gensym("blue"));
	m_inlet[3] = inlet_new(this->x_obj, &this->x_obj->ob_pd, &s_float, gensym("alpha"));
}
/////////////////////////////////////////////////////////
// Destructor
//
GEMglColor4b :: ~GEMglColor4b () {
inlet_free(m_inlet[0]);
inlet_free(m_inlet[1]);
inlet_free(m_inlet[2]);
inlet_free(m_inlet[3]);
}

/////////////////////////////////////////////////////////
// Render
//
void GEMglColor4b :: render(GemState *state) {
	glColor4b (red, green, blue, alpha);
}

/////////////////////////////////////////////////////////
// Variables
//
void GEMglColor4b :: redMess (t_float arg1) {	// FUN
	red = static_cast<GLbyte>(arg1);
	setModified();
}

void GEMglColor4b :: greenMess (t_float arg1) {	// FUN
	green = static_cast<GLbyte>(arg1);
	setModified();
}

void GEMglColor4b :: blueMess (t_float arg1) {	// FUN
	blue = static_cast<GLbyte>(arg1);
	setModified();
}

void GEMglColor4b :: alphaMess (t_float arg1) {	// FUN
	alpha = static_cast<GLbyte>(arg1);
	setModified();
}


/////////////////////////////////////////////////////////
// static member functions
//

void GEMglColor4b :: obj_setupCallback(t_class *classPtr) {
	 class_addmethod(classPtr, reinterpret_cast<t_method>(&GEMglColor4b::redMessCallback),  	gensym("red"), A_DEFFLOAT, A_NULL);
	 class_addmethod(classPtr, reinterpret_cast<t_method>(&GEMglColor4b::greenMessCallback),  	gensym("green"), A_DEFFLOAT, A_NULL);
	 class_addmethod(classPtr, reinterpret_cast<t_method>(&GEMglColor4b::blueMessCallback),  	gensym("blue"), A_DEFFLOAT, A_NULL);
	 class_addmethod(classPtr, reinterpret_cast<t_method>(&GEMglColor4b::alphaMessCallback),  	gensym("alpha"), A_DEFFLOAT, A_NULL);
};

void GEMglColor4b :: redMessCallback (void* data, t_floatarg arg0){
	GetMyClass(data)->redMess ( static_cast<t_float>(arg0));
}
void GEMglColor4b :: greenMessCallback (void* data, t_floatarg arg0){
	GetMyClass(data)->greenMess ( static_cast<t_float>(arg0));
}
void GEMglColor4b :: blueMessCallback (void* data, t_floatarg arg0){
	GetMyClass(data)->blueMess ( static_cast<t_float>(arg0));
}
void GEMglColor4b :: alphaMessCallback (void* data, t_floatarg arg0){
	GetMyClass(data)->alphaMess ( static_cast<t_float>(arg0));
}
