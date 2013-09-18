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

#include "GEMglColor4f.h"

CPPEXTERN_NEW_WITH_FOUR_ARGS ( GEMglColor4f , t_floatarg, A_DEFFLOAT, t_floatarg, A_DEFFLOAT, t_floatarg, A_DEFFLOAT, t_floatarg, A_DEFFLOAT);

/////////////////////////////////////////////////////////
//
// GEMglViewport
//
/////////////////////////////////////////////////////////
// Constructor
//
GEMglColor4f :: GEMglColor4f	(t_floatarg arg0=0, t_floatarg arg1=0, t_floatarg arg2=0, t_floatarg arg3=0) :
		red(static_cast<GLfloat>(arg0)), 
		green(static_cast<GLfloat>(arg1)), 
		blue(static_cast<GLfloat>(arg2)), 
		alpha(static_cast<GLfloat>(arg3))
{
	m_inlet[0] = inlet_new(this->x_obj, &this->x_obj->ob_pd, &s_float, gensym("red"));
	m_inlet[1] = inlet_new(this->x_obj, &this->x_obj->ob_pd, &s_float, gensym("green"));
	m_inlet[2] = inlet_new(this->x_obj, &this->x_obj->ob_pd, &s_float, gensym("blue"));
	m_inlet[3] = inlet_new(this->x_obj, &this->x_obj->ob_pd, &s_float, gensym("alpha"));
}
/////////////////////////////////////////////////////////
// Destructor
//
GEMglColor4f :: ~GEMglColor4f () {
inlet_free(m_inlet[0]);
inlet_free(m_inlet[1]);
inlet_free(m_inlet[2]);
inlet_free(m_inlet[3]);
}

/////////////////////////////////////////////////////////
// Render
//
void GEMglColor4f :: render(GemState *state) {
	glColor4f (red, green, blue, alpha);
}

/////////////////////////////////////////////////////////
// Variables
//
void GEMglColor4f :: redMess (t_float arg1) {	// FUN
	red = static_cast<GLfloat>(arg1);
	setModified();
}

void GEMglColor4f :: greenMess (t_float arg1) {	// FUN
	green = static_cast<GLfloat>(arg1);
	setModified();
}

void GEMglColor4f :: blueMess (t_float arg1) {	// FUN
	blue = static_cast<GLfloat>(arg1);
	setModified();
}

void GEMglColor4f :: alphaMess (t_float arg1) {	// FUN
	alpha = static_cast<GLfloat>(arg1);
	setModified();
}


/////////////////////////////////////////////////////////
// static member functions
//

void GEMglColor4f :: obj_setupCallback(t_class *classPtr) {
	 class_addmethod(classPtr, reinterpret_cast<t_method>(&GEMglColor4f::redMessCallback),  	gensym("red"), A_DEFFLOAT, A_NULL);
	 class_addmethod(classPtr, reinterpret_cast<t_method>(&GEMglColor4f::greenMessCallback),  	gensym("green"), A_DEFFLOAT, A_NULL);
	 class_addmethod(classPtr, reinterpret_cast<t_method>(&GEMglColor4f::blueMessCallback),  	gensym("blue"), A_DEFFLOAT, A_NULL);
	 class_addmethod(classPtr, reinterpret_cast<t_method>(&GEMglColor4f::alphaMessCallback),  	gensym("alpha"), A_DEFFLOAT, A_NULL);
};

void GEMglColor4f :: redMessCallback (void* data, t_floatarg arg0){
	GetMyClass(data)->redMess ( static_cast<t_float>(arg0));
}
void GEMglColor4f :: greenMessCallback (void* data, t_floatarg arg0){
	GetMyClass(data)->greenMess ( static_cast<t_float>(arg0));
}
void GEMglColor4f :: blueMessCallback (void* data, t_floatarg arg0){
	GetMyClass(data)->blueMess ( static_cast<t_float>(arg0));
}
void GEMglColor4f :: alphaMessCallback (void* data, t_floatarg arg0){
	GetMyClass(data)->alphaMess ( static_cast<t_float>(arg0));
}
