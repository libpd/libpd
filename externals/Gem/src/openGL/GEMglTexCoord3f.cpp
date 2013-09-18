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

#include "GEMglTexCoord3f.h"

CPPEXTERN_NEW_WITH_THREE_ARGS ( GEMglTexCoord3f , t_floatarg, A_DEFFLOAT, t_floatarg, A_DEFFLOAT, t_floatarg, A_DEFFLOAT);

/////////////////////////////////////////////////////////
//
// GEMglViewport
//
/////////////////////////////////////////////////////////
// Constructor
//
GEMglTexCoord3f :: GEMglTexCoord3f	(t_floatarg arg0=0, t_floatarg arg1=0, t_floatarg arg2=0) :
		s(static_cast<GLfloat>(arg0)), 
		t(static_cast<GLfloat>(arg1)), 
		r(static_cast<GLfloat>(arg2))
{
	m_inlet[0] = inlet_new(this->x_obj, &this->x_obj->ob_pd, &s_float, gensym("s"));
	m_inlet[1] = inlet_new(this->x_obj, &this->x_obj->ob_pd, &s_float, gensym("t"));
	m_inlet[2] = inlet_new(this->x_obj, &this->x_obj->ob_pd, &s_float, gensym("r"));
}
/////////////////////////////////////////////////////////
// Destructor
//
GEMglTexCoord3f :: ~GEMglTexCoord3f () {
inlet_free(m_inlet[0]);
inlet_free(m_inlet[1]);
inlet_free(m_inlet[2]);
}

/////////////////////////////////////////////////////////
// Render
//
void GEMglTexCoord3f :: render(GemState *state) {
	glTexCoord3f (s, t, r);
}

/////////////////////////////////////////////////////////
// Variables
//
void GEMglTexCoord3f :: sMess (t_float arg1) {	// FUN
	s = static_cast<GLfloat>(arg1);
	setModified();
}

void GEMglTexCoord3f :: tMess (t_float arg1) {	// FUN
	t = static_cast<GLfloat>(arg1);
	setModified();
}

void GEMglTexCoord3f :: rMess (t_float arg1) {	// FUN
	r = static_cast<GLfloat>(arg1);
	setModified();
}


/////////////////////////////////////////////////////////
// static member functions
//

void GEMglTexCoord3f :: obj_setupCallback(t_class *classPtr) {
	 class_addmethod(classPtr, reinterpret_cast<t_method>(&GEMglTexCoord3f::sMessCallback),  	gensym("s"), A_DEFFLOAT, A_NULL);
	 class_addmethod(classPtr, reinterpret_cast<t_method>(&GEMglTexCoord3f::tMessCallback),  	gensym("t"), A_DEFFLOAT, A_NULL);
	 class_addmethod(classPtr, reinterpret_cast<t_method>(&GEMglTexCoord3f::rMessCallback),  	gensym("r"), A_DEFFLOAT, A_NULL);
};

void GEMglTexCoord3f :: sMessCallback (void* data, t_floatarg arg0){
	GetMyClass(data)->sMess ( static_cast<t_float>(arg0));
}
void GEMglTexCoord3f :: tMessCallback (void* data, t_floatarg arg0){
	GetMyClass(data)->tMess ( static_cast<t_float>(arg0));
}
void GEMglTexCoord3f :: rMessCallback (void* data, t_floatarg arg0){
	GetMyClass(data)->rMess ( static_cast<t_float>(arg0));
}
