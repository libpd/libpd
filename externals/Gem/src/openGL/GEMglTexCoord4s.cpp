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

#include "GEMglTexCoord4s.h"

CPPEXTERN_NEW_WITH_FOUR_ARGS ( GEMglTexCoord4s , t_floatarg, A_DEFFLOAT, t_floatarg, A_DEFFLOAT, t_floatarg, A_DEFFLOAT, t_floatarg, A_DEFFLOAT);

/////////////////////////////////////////////////////////
//
// GEMglViewport
//
/////////////////////////////////////////////////////////
// Constructor
//
GEMglTexCoord4s :: GEMglTexCoord4s	(t_floatarg arg0=0, t_floatarg arg1=0, t_floatarg arg2=0, t_floatarg arg3=0) :
		s(static_cast<GLshort>(arg0)), 
		t(static_cast<GLshort>(arg1)), 
		r(static_cast<GLshort>(arg2)), 
		q(static_cast<GLshort>(arg3))
{
	m_inlet[0] = inlet_new(this->x_obj, &this->x_obj->ob_pd, &s_float, gensym("s"));
	m_inlet[1] = inlet_new(this->x_obj, &this->x_obj->ob_pd, &s_float, gensym("t"));
	m_inlet[2] = inlet_new(this->x_obj, &this->x_obj->ob_pd, &s_float, gensym("r"));
	m_inlet[3] = inlet_new(this->x_obj, &this->x_obj->ob_pd, &s_float, gensym("q"));
}
/////////////////////////////////////////////////////////
// Destructor
//
GEMglTexCoord4s :: ~GEMglTexCoord4s () {
inlet_free(m_inlet[0]);
inlet_free(m_inlet[1]);
inlet_free(m_inlet[2]);
inlet_free(m_inlet[3]);
}

/////////////////////////////////////////////////////////
// Render
//
void GEMglTexCoord4s :: render(GemState *state) {
	glTexCoord4s (s, t, r, q);
}

/////////////////////////////////////////////////////////
// Variables
//
void GEMglTexCoord4s :: sMess (t_float arg1) {	// FUN
	s = static_cast<GLshort>(arg1);
	setModified();
}

void GEMglTexCoord4s :: tMess (t_float arg1) {	// FUN
	t = static_cast<GLshort>(arg1);
	setModified();
}

void GEMglTexCoord4s :: rMess (t_float arg1) {	// FUN
	r = static_cast<GLshort>(arg1);
	setModified();
}

void GEMglTexCoord4s :: qMess (t_float arg1) {	// FUN
	q = static_cast<GLshort>(arg1);
	setModified();
}


/////////////////////////////////////////////////////////
// static member functions
//

void GEMglTexCoord4s :: obj_setupCallback(t_class *classPtr) {
	 class_addmethod(classPtr, reinterpret_cast<t_method>(&GEMglTexCoord4s::sMessCallback),  	gensym("s"), A_DEFFLOAT, A_NULL);
	 class_addmethod(classPtr, reinterpret_cast<t_method>(&GEMglTexCoord4s::tMessCallback),  	gensym("t"), A_DEFFLOAT, A_NULL);
	 class_addmethod(classPtr, reinterpret_cast<t_method>(&GEMglTexCoord4s::rMessCallback),  	gensym("r"), A_DEFFLOAT, A_NULL);
	 class_addmethod(classPtr, reinterpret_cast<t_method>(&GEMglTexCoord4s::qMessCallback),  	gensym("q"), A_DEFFLOAT, A_NULL);
};

void GEMglTexCoord4s :: sMessCallback (void* data, t_floatarg arg0){
	GetMyClass(data)->sMess ( static_cast<t_float>(arg0));
}
void GEMglTexCoord4s :: tMessCallback (void* data, t_floatarg arg0){
	GetMyClass(data)->tMess ( static_cast<t_float>(arg0));
}
void GEMglTexCoord4s :: rMessCallback (void* data, t_floatarg arg0){
	GetMyClass(data)->rMess ( static_cast<t_float>(arg0));
}
void GEMglTexCoord4s :: qMessCallback (void* data, t_floatarg arg0){
	GetMyClass(data)->qMess ( static_cast<t_float>(arg0));
}
