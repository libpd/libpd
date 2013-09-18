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

#include "GEMglNewList.h"

CPPEXTERN_NEW_WITH_TWO_ARGS ( GEMglNewList , t_floatarg, A_DEFFLOAT, t_floatarg, A_DEFFLOAT);

/////////////////////////////////////////////////////////
//
// GEMglViewport
//
/////////////////////////////////////////////////////////
// Constructor
//
GEMglNewList :: GEMglNewList	(t_floatarg arg0=0, t_floatarg arg1=GL_COMPILE_AND_EXECUTE) :
  list(static_cast<GLuint>(arg0)),
		mode(static_cast<GLenum>(arg1))
{
	m_inlet[0] = inlet_new(this->x_obj, &this->x_obj->ob_pd, &s_float, gensym("displaylist"));
	m_inlet[1] = inlet_new(this->x_obj, &this->x_obj->ob_pd, &s_float, gensym("mode"));
}
/////////////////////////////////////////////////////////
// Destructor
//
GEMglNewList :: ~GEMglNewList () {
inlet_free(m_inlet[0]);
inlet_free(m_inlet[1]);
}

/////////////////////////////////////////////////////////
// Render
//
void GEMglNewList :: render(GemState *state) {
	glNewList (list, mode);
}

/////////////////////////////////////////////////////////
// Variables
//
void GEMglNewList :: modeMess (t_float arg1) {	// FUN
	mode = static_cast<GLenum>(arg1);
	setModified();
}
void GEMglNewList :: listMess (t_float arg1) {	// FUN
	list = static_cast<GLuint>(arg1);
	setModified();
}


/////////////////////////////////////////////////////////
// static member functions
//

void GEMglNewList :: obj_setupCallback(t_class *classPtr) {
	 class_addmethod(classPtr, reinterpret_cast<t_method>(&GEMglNewList::listMessCallback),  	gensym("displaylist"), A_DEFFLOAT, A_NULL);
	 class_addmethod(classPtr, reinterpret_cast<t_method>(&GEMglNewList::modeMessCallback),  	gensym("mode"), A_DEFFLOAT, A_NULL);
};

void GEMglNewList :: modeMessCallback (void* data, t_floatarg arg0){
	GetMyClass(data)->modeMess ( static_cast<t_float>(arg0));
}
void GEMglNewList :: listMessCallback (void* data, t_floatarg arg0){
	GetMyClass(data)->listMess ( static_cast<t_float>(arg0));
}
