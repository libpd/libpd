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

#include "GEMglMapGrid1f.h"

CPPEXTERN_NEW_WITH_THREE_ARGS ( GEMglMapGrid1f , t_floatarg, A_DEFFLOAT, t_floatarg, A_DEFFLOAT, t_floatarg, A_DEFFLOAT);

/////////////////////////////////////////////////////////
//
// GEMglViewport
//
/////////////////////////////////////////////////////////
// Constructor
//
GEMglMapGrid1f :: GEMglMapGrid1f	(t_floatarg arg0=0, t_floatarg arg1=0, t_floatarg arg2=0) :
		un(static_cast<GLint>(arg0)), 
		u1(static_cast<GLfloat>(arg1)), 
		u2(static_cast<GLfloat>(arg2))
{
	m_inlet[0] = inlet_new(this->x_obj, &this->x_obj->ob_pd, &s_float, gensym("un"));
	m_inlet[1] = inlet_new(this->x_obj, &this->x_obj->ob_pd, &s_float, gensym("u1"));
	m_inlet[2] = inlet_new(this->x_obj, &this->x_obj->ob_pd, &s_float, gensym("u2"));
}
/////////////////////////////////////////////////////////
// Destructor
//
GEMglMapGrid1f :: ~GEMglMapGrid1f () {
inlet_free(m_inlet[0]);
inlet_free(m_inlet[1]);
inlet_free(m_inlet[2]);
}

//////////////////
// extension check
bool GEMglMapGrid1f :: isRunnable(void) {
  if(GLEW_VERSION_1_1)return true;
  error("your system does not support OpenGL-1.1");
  return false;
}

/////////////////////////////////////////////////////////
// Render
//
void GEMglMapGrid1f :: render(GemState *state) {
	glMapGrid1f (un, u1, u2);
}

/////////////////////////////////////////////////////////
// Variables
//
void GEMglMapGrid1f :: unMess (t_float arg1) {	// FUN
	un = static_cast<GLint>(arg1);
	setModified();
}

void GEMglMapGrid1f :: u1Mess (t_float arg1) {	// FUN
	u1 = static_cast<GLfloat>(arg1);
	setModified();
}

void GEMglMapGrid1f :: u2Mess (t_float arg1) {	// FUN
	u2 = static_cast<GLfloat>(arg1);
	setModified();
}


/////////////////////////////////////////////////////////
// static member functions
//

void GEMglMapGrid1f :: obj_setupCallback(t_class *classPtr) {
	 class_addmethod(classPtr, reinterpret_cast<t_method>(&GEMglMapGrid1f::unMessCallback),  	gensym("un"), A_DEFFLOAT, A_NULL);
	 class_addmethod(classPtr, reinterpret_cast<t_method>(&GEMglMapGrid1f::u1MessCallback),  	gensym("u1"), A_DEFFLOAT, A_NULL);
	 class_addmethod(classPtr, reinterpret_cast<t_method>(&GEMglMapGrid1f::u2MessCallback),  	gensym("u2"), A_DEFFLOAT, A_NULL);
}

void GEMglMapGrid1f :: unMessCallback (void* data, t_floatarg arg0){
	GetMyClass(data)->unMess ( static_cast<t_float>(arg0));
}
void GEMglMapGrid1f :: u1MessCallback (void* data, t_floatarg arg0){
	GetMyClass(data)->u1Mess ( static_cast<t_float>(arg0));
}
void GEMglMapGrid1f :: u2MessCallback (void* data, t_floatarg arg0){
	GetMyClass(data)->u2Mess ( static_cast<t_float>(arg0));
}
