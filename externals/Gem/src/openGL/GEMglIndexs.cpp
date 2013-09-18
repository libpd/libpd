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

#include "GEMglIndexs.h"

CPPEXTERN_NEW_WITH_ONE_ARG ( GEMglIndexs , t_floatarg, A_DEFFLOAT);

/////////////////////////////////////////////////////////
//
// GEMglViewport
//
/////////////////////////////////////////////////////////
// Constructor
//
GEMglIndexs :: GEMglIndexs	(t_floatarg arg0=0) :
		c(static_cast<GLshort>(arg0))
{
	m_inlet[0] = inlet_new(this->x_obj, &this->x_obj->ob_pd, &s_float, gensym("c"));
}
/////////////////////////////////////////////////////////
// Destructor
//
GEMglIndexs :: ~GEMglIndexs () {
inlet_free(m_inlet[0]);
}

/////////////////////////////////////////////////////////
// Render
//
void GEMglIndexs :: render(GemState *state) {
	glIndexs (c);
}

/////////////////////////////////////////////////////////
// Variables
//
void GEMglIndexs :: cMess (t_float arg1) {	// FUN
	c = static_cast<GLshort>(arg1);
	setModified();
}


/////////////////////////////////////////////////////////
// static member functions
//

void GEMglIndexs :: obj_setupCallback(t_class *classPtr) {
	 class_addmethod(classPtr, reinterpret_cast<t_method>(&GEMglIndexs::cMessCallback),  	gensym("c"), A_DEFFLOAT, A_NULL);
};

void GEMglIndexs :: cMessCallback (void* data, t_floatarg arg0){
	GetMyClass(data)->cMess ( static_cast<t_float>(arg0));
}
