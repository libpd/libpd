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

#include "GEMglDepthMask.h"

CPPEXTERN_NEW_WITH_ONE_ARG ( GEMglDepthMask , t_floatarg, A_DEFFLOAT);

/////////////////////////////////////////////////////////
//
// GEMglViewport
//
/////////////////////////////////////////////////////////
// Constructor
//
GEMglDepthMask :: GEMglDepthMask	(t_floatarg arg0=0) :
		flag(static_cast<GLboolean>(arg0))
{
	m_inlet[0] = inlet_new(this->x_obj, &this->x_obj->ob_pd, &s_float, gensym("flag"));
}
/////////////////////////////////////////////////////////
// Destructor
//
GEMglDepthMask :: ~GEMglDepthMask () {
inlet_free(m_inlet[0]);
}

/////////////////////////////////////////////////////////
// Render
//
void GEMglDepthMask :: render(GemState *state) {
	glDepthMask (flag);
}

/////////////////////////////////////////////////////////
// Variables
//
void GEMglDepthMask :: flagMess (t_float arg1) {	// FUN
	flag = static_cast<GLboolean>(arg1);
	setModified();
}


/////////////////////////////////////////////////////////
// static member functions
//

void GEMglDepthMask :: obj_setupCallback(t_class *classPtr) {
	 class_addmethod(classPtr, reinterpret_cast<t_method>(&GEMglDepthMask::flagMessCallback),  	gensym("flag"), A_DEFFLOAT, A_NULL);
};

void GEMglDepthMask :: flagMessCallback (void* data, t_floatarg arg0){
	GetMyClass(data)->flagMess ( static_cast<t_float>(arg0));
}
