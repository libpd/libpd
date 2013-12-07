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

#include "GEMglPixelZoom.h"

CPPEXTERN_NEW_WITH_TWO_ARGS ( GEMglPixelZoom , t_floatarg, A_DEFFLOAT, t_floatarg, A_DEFFLOAT);

/////////////////////////////////////////////////////////
//
// GEMglViewport
//
/////////////////////////////////////////////////////////
// Constructor
//
GEMglPixelZoom :: GEMglPixelZoom	(t_floatarg arg0=0, t_floatarg arg1=0) :
		xfactor(static_cast<GLfloat>(arg0)), 
		yfactor(static_cast<GLfloat>(arg1))
{
	m_inlet[0] = inlet_new(this->x_obj, &this->x_obj->ob_pd, &s_float, gensym("xfactor"));
	m_inlet[1] = inlet_new(this->x_obj, &this->x_obj->ob_pd, &s_float, gensym("yfactor"));
}
/////////////////////////////////////////////////////////
// Destructor
//
GEMglPixelZoom :: ~GEMglPixelZoom () {
inlet_free(m_inlet[0]);
inlet_free(m_inlet[1]);
}

/////////////////////////////////////////////////////////
// Render
//
void GEMglPixelZoom :: render(GemState *state) {
	glPixelZoom (xfactor, yfactor);
}

/////////////////////////////////////////////////////////
// Variables
//
void GEMglPixelZoom :: xfactorMess (t_float arg1) {	// FUN
	xfactor = static_cast<GLfloat>(arg1);
	setModified();
}

void GEMglPixelZoom :: yfactorMess (t_float arg1) {	// FUN
	yfactor = static_cast<GLfloat>(arg1);
	setModified();
}


/////////////////////////////////////////////////////////
// static member functions
//

void GEMglPixelZoom :: obj_setupCallback(t_class *classPtr) {
	 class_addmethod(classPtr, reinterpret_cast<t_method>(&GEMglPixelZoom::xfactorMessCallback),  	gensym("xfactor"), A_DEFFLOAT, A_NULL);
	 class_addmethod(classPtr, reinterpret_cast<t_method>(&GEMglPixelZoom::yfactorMessCallback),  	gensym("yfactor"), A_DEFFLOAT, A_NULL);
};

void GEMglPixelZoom :: xfactorMessCallback (void* data, t_floatarg arg0){
	GetMyClass(data)->xfactorMess ( static_cast<t_float>(arg0));
}
void GEMglPixelZoom :: yfactorMessCallback (void* data, t_floatarg arg0){
	GetMyClass(data)->yfactorMess ( static_cast<t_float>(arg0));
}
