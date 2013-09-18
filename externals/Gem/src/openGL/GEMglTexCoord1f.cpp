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

#include "GEMglTexCoord1f.h"

CPPEXTERN_NEW_WITH_ONE_ARG ( GEMglTexCoord1f , t_floatarg, A_DEFFLOAT);

/////////////////////////////////////////////////////////
//
// GEMglViewport
//
/////////////////////////////////////////////////////////
// Constructor
//
GEMglTexCoord1f :: GEMglTexCoord1f	(t_floatarg arg0=0) :
		s(static_cast<GLfloat>(arg0))
{
	m_inlet[0] = inlet_new(this->x_obj, &this->x_obj->ob_pd, &s_float, gensym("s"));
}
/////////////////////////////////////////////////////////
// Destructor
//
GEMglTexCoord1f :: ~GEMglTexCoord1f () {
inlet_free(m_inlet[0]);
}

/////////////////////////////////////////////////////////
// Render
//
void GEMglTexCoord1f :: render(GemState *state) {
	glTexCoord1f (s);
}

/////////////////////////////////////////////////////////
// Variables
//
void GEMglTexCoord1f :: sMess (t_float arg1) {	// FUN
	s = static_cast<GLfloat>(arg1);
	setModified();
}


/////////////////////////////////////////////////////////
// static member functions
//

void GEMglTexCoord1f :: obj_setupCallback(t_class *classPtr) {
	 class_addmethod(classPtr, reinterpret_cast<t_method>(&GEMglTexCoord1f::sMessCallback),  	gensym("s"), A_DEFFLOAT, A_NULL);
};

void GEMglTexCoord1f :: sMessCallback (void* data, t_floatarg arg0){
	GetMyClass(data)->sMess ( static_cast<t_float>(arg0));
}
