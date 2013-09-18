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

#include "GEMglColor4ub.h"

CPPEXTERN_NEW_WITH_FOUR_ARGS ( GEMglColor4ub , t_floatarg, A_DEFFLOAT, t_floatarg, A_DEFFLOAT, t_floatarg, A_DEFFLOAT, t_floatarg, A_DEFFLOAT);

/////////////////////////////////////////////////////////
//
// GEMglViewport
//
/////////////////////////////////////////////////////////
// Constructor
//
GEMglColor4ub :: GEMglColor4ub	(t_floatarg arg0=0, t_floatarg arg1=0, t_floatarg arg2=0, t_floatarg arg3=0) :
		red(static_cast<GLubyte>(arg0)), 
		green(static_cast<GLubyte>(arg1)), 
		blue(static_cast<GLubyte>(arg2)), 
		alpha(static_cast<GLubyte>(arg3))
{
	m_inlet[0] = inlet_new(this->x_obj, &this->x_obj->ob_pd, &s_float, gensym("red"));
	m_inlet[1] = inlet_new(this->x_obj, &this->x_obj->ob_pd, &s_float, gensym("green"));
	m_inlet[2] = inlet_new(this->x_obj, &this->x_obj->ob_pd, &s_float, gensym("blue"));
	m_inlet[3] = inlet_new(this->x_obj, &this->x_obj->ob_pd, &s_float, gensym("alpha"));
}
/////////////////////////////////////////////////////////
// Destructor
//
GEMglColor4ub :: ~GEMglColor4ub () {
inlet_free(m_inlet[0]);
inlet_free(m_inlet[1]);
inlet_free(m_inlet[2]);
inlet_free(m_inlet[3]);
}

/////////////////////////////////////////////////////////
// Render
//
void GEMglColor4ub :: render(GemState *state) {
	glColor4ub (red, green, blue, alpha);
}

/////////////////////////////////////////////////////////
// Variables
//
void GEMglColor4ub :: redMess (t_float arg1) {	// FUN
	red = static_cast<GLubyte>(arg1);
	setModified();
}

void GEMglColor4ub :: greenMess (t_float arg1) {	// FUN
	green = static_cast<GLubyte>(arg1);
	setModified();
}

void GEMglColor4ub :: blueMess (t_float arg1) {	// FUN
	blue = static_cast<GLubyte>(arg1);
	setModified();
}

void GEMglColor4ub :: alphaMess (t_float arg1) {	// FUN
	alpha = static_cast<GLubyte>(arg1);
	setModified();
}


/////////////////////////////////////////////////////////
// static member functions
//

void GEMglColor4ub :: obj_setupCallback(t_class *classPtr) {
	 class_addmethod(classPtr, reinterpret_cast<t_method>(&GEMglColor4ub::redMessCallback),  	gensym("red"), A_DEFFLOAT, A_NULL);
	 class_addmethod(classPtr, reinterpret_cast<t_method>(&GEMglColor4ub::greenMessCallback),  	gensym("green"), A_DEFFLOAT, A_NULL);
	 class_addmethod(classPtr, reinterpret_cast<t_method>(&GEMglColor4ub::blueMessCallback),  	gensym("blue"), A_DEFFLOAT, A_NULL);
	 class_addmethod(classPtr, reinterpret_cast<t_method>(&GEMglColor4ub::alphaMessCallback),  	gensym("alpha"), A_DEFFLOAT, A_NULL);
};

void GEMglColor4ub :: redMessCallback (void* data, t_floatarg arg0){
	GetMyClass(data)->redMess ( static_cast<t_float>(arg0));
}
void GEMglColor4ub :: greenMessCallback (void* data, t_floatarg arg0){
	GetMyClass(data)->greenMess ( static_cast<t_float>(arg0));
}
void GEMglColor4ub :: blueMessCallback (void* data, t_floatarg arg0){
	GetMyClass(data)->blueMess ( static_cast<t_float>(arg0));
}
void GEMglColor4ub :: alphaMessCallback (void* data, t_floatarg arg0){
	GetMyClass(data)->alphaMess ( static_cast<t_float>(arg0));
}
