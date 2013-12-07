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

#include "GEMglEvalMesh1.h"

CPPEXTERN_NEW_WITH_THREE_ARGS ( GEMglEvalMesh1 , t_floatarg, A_DEFFLOAT, t_floatarg, A_DEFFLOAT, t_floatarg, A_DEFFLOAT);

/////////////////////////////////////////////////////////
//
// GEMglViewport
//
/////////////////////////////////////////////////////////
// Constructor
//
GEMglEvalMesh1 :: GEMglEvalMesh1	(t_floatarg arg0=0, t_floatarg arg1=0, t_floatarg arg2=0) :
		mode(static_cast<GLenum>(arg0)), 
		i1(static_cast<GLint>(arg1)), 
		i2(static_cast<GLint>(arg2))
{
	m_inlet[0] = inlet_new(this->x_obj, &this->x_obj->ob_pd, &s_float, gensym("mode"));
	m_inlet[1] = inlet_new(this->x_obj, &this->x_obj->ob_pd, &s_float, gensym("i1"));
	m_inlet[2] = inlet_new(this->x_obj, &this->x_obj->ob_pd, &s_float, gensym("i2"));
}
/////////////////////////////////////////////////////////
// Destructor
//
GEMglEvalMesh1 :: ~GEMglEvalMesh1 () {
inlet_free(m_inlet[0]);
inlet_free(m_inlet[1]);
inlet_free(m_inlet[2]);
}
//////////////////
// extension check
bool GEMglEvalMesh1 :: isRunnable(void) {
  if(GLEW_VERSION_1_1)return true;
  error("your system does not support OpenGL-1.1");
  return false;
}
/////////////////////////////////////////////////////////
// Render
//
void GEMglEvalMesh1 :: render(GemState *state) {
	glEvalMesh1 (mode, i1, i2);
}

/////////////////////////////////////////////////////////
// Variables
//
void GEMglEvalMesh1 :: modeMess (t_float arg1) {	// FUN
	mode = static_cast<GLenum>(arg1);
	setModified();
}

void GEMglEvalMesh1 :: i1Mess (t_float arg1) {	// FUN
	i1 = static_cast<GLint>(arg1);
	setModified();
}

void GEMglEvalMesh1 :: i2Mess (t_float arg1) {	// FUN
	i2 = static_cast<GLint>(arg1);
	setModified();
}


/////////////////////////////////////////////////////////
// static member functions
//

void GEMglEvalMesh1 :: obj_setupCallback(t_class *classPtr) {
	 class_addmethod(classPtr, reinterpret_cast<t_method>(&GEMglEvalMesh1::modeMessCallback),  	gensym("mode"), A_DEFFLOAT, A_NULL);
	 class_addmethod(classPtr, reinterpret_cast<t_method>(&GEMglEvalMesh1::i1MessCallback),  	gensym("i1"), A_DEFFLOAT, A_NULL);
	 class_addmethod(classPtr, reinterpret_cast<t_method>(&GEMglEvalMesh1::i2MessCallback),  	gensym("i2"), A_DEFFLOAT, A_NULL);
}

void GEMglEvalMesh1 :: modeMessCallback (void* data, t_floatarg arg0){
	GetMyClass(data)->modeMess ( static_cast<t_float>(arg0));
}
void GEMglEvalMesh1 :: i1MessCallback (void* data, t_floatarg arg0){
	GetMyClass(data)->i1Mess ( static_cast<t_float>(arg0));
}
void GEMglEvalMesh1 :: i2MessCallback (void* data, t_floatarg arg0){
	GetMyClass(data)->i2Mess ( static_cast<t_float>(arg0));
}
