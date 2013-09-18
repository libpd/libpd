////////////////////////////////////////////////////////
//
// GEM - Graphics Environment for Multimedia
//
// Implementation file
//
// Copyright (c) 2004 tigital@mac.com
//  For information on usage and redistribution, and for a DISCLAIMER
//  *  OF ALL WARRANTIES, see the file, "GEM.LICENSE.TERMS"
//
////////////////////////////////////////////////////////

#include "GEMglProgramEnvParameter4dARB.h"

CPPEXTERN_NEW_WITH_GIMME ( GEMglProgramEnvParameter4dARB );

/////////////////////////////////////////////////////////
//
// GEMglProgramEnvParameter4dARB
//
/////////////////////////////////////////////////////////
// Constructor
//
GEMglProgramEnvParameter4dARB :: GEMglProgramEnvParameter4dARB	(int argc, t_atom*argv ) :
		target(0), index(0), 
		m_x(0), m_y(0), m_z(0), m_w(0)
{
	m_inlet[0] = inlet_new(this->x_obj, &this->x_obj->ob_pd, &s_float, gensym("target"));
	m_inlet[1] = inlet_new(this->x_obj, &this->x_obj->ob_pd, &s_float, gensym("index"));
	m_inlet[2] = inlet_new(this->x_obj, &this->x_obj->ob_pd, &s_float, gensym("x"));
	m_inlet[3] = inlet_new(this->x_obj, &this->x_obj->ob_pd, &s_float, gensym("y"));
	m_inlet[4] = inlet_new(this->x_obj, &this->x_obj->ob_pd, &s_float, gensym("z"));
	m_inlet[5] = inlet_new(this->x_obj, &this->x_obj->ob_pd, &s_float, gensym("w"));
}
/////////////////////////////////////////////////////////
// Destructor
//
GEMglProgramEnvParameter4dARB :: ~GEMglProgramEnvParameter4dARB () {
inlet_free(m_inlet[0]);
inlet_free(m_inlet[1]);
inlet_free(m_inlet[2]);
inlet_free(m_inlet[3]);
inlet_free(m_inlet[4]);
inlet_free(m_inlet[5]);
}

//////////////////
// extension check
bool GEMglProgramEnvParameter4dARB :: isRunnable(void) {
  if(GLEW_ARB_vertex_program)return true;
  error("your system does not the ARB vertex_program extension");
  return false;
}

/////////////////////////////////////////////////////////
// Render
//
void GEMglProgramEnvParameter4dARB :: render(GemState *state) {
	glProgramEnvParameter4dARB (target, index, m_x, m_y, m_z, m_w);
}

/////////////////////////////////////////////////////////
// Variables
//
void GEMglProgramEnvParameter4dARB :: targetMess (t_float arg1) {	// FUN
	target = static_cast<GLenum>(arg1);
	setModified();
}

void GEMglProgramEnvParameter4dARB :: indexMess (t_float arg2) {	// FUN
	index = static_cast<GLenum>(arg2);
	setModified();
}

void GEMglProgramEnvParameter4dARB :: xMess (t_float arg2) {	// FUN
	m_x = static_cast<GLdouble>(arg2);
	setModified();
}

void GEMglProgramEnvParameter4dARB :: yMess (t_float arg3) {	// FUN
	m_y = static_cast<GLdouble>(arg3);
	setModified();
}

void GEMglProgramEnvParameter4dARB :: zMess (t_float arg4) {	// FUN
	m_z = static_cast<GLdouble>(arg4);
	setModified();
}
void GEMglProgramEnvParameter4dARB :: wMess (t_float arg5) {	// FUN
	m_w = static_cast<GLdouble>(arg5);
	setModified();
}
/////////////////////////////////////////////////////////
// static member functions
//

void GEMglProgramEnvParameter4dARB :: obj_setupCallback(t_class *classPtr) {
	 class_addmethod(classPtr, reinterpret_cast<t_method>(&GEMglProgramEnvParameter4dARB::targetMessCallback),  	gensym("target"), A_DEFFLOAT, A_NULL);
	 class_addmethod(classPtr, reinterpret_cast<t_method>(&GEMglProgramEnvParameter4dARB::indexMessCallback),  	gensym("index"), A_DEFFLOAT, A_NULL);
	 class_addmethod(classPtr, reinterpret_cast<t_method>(&GEMglProgramEnvParameter4dARB::xMessCallback),  	gensym("x"), A_DEFFLOAT, A_NULL);
	 class_addmethod(classPtr, reinterpret_cast<t_method>(&GEMglProgramEnvParameter4dARB::yMessCallback),  	gensym("y"), A_DEFFLOAT, A_NULL);
	 class_addmethod(classPtr, reinterpret_cast<t_method>(&GEMglProgramEnvParameter4dARB::zMessCallback),  	gensym("z"), A_DEFFLOAT, A_NULL);
	 class_addmethod(classPtr, reinterpret_cast<t_method>(&GEMglProgramEnvParameter4dARB::wMessCallback),  	gensym("w"), A_DEFFLOAT, A_NULL);
};

void GEMglProgramEnvParameter4dARB :: targetMessCallback (void* data, t_floatarg arg0){
	GetMyClass(data)->targetMess ( static_cast<t_float>(arg0));
}
void GEMglProgramEnvParameter4dARB :: indexMessCallback (void* data, t_floatarg arg1){
	GetMyClass(data)->indexMess ( static_cast<t_float>(arg1));
}
void GEMglProgramEnvParameter4dARB :: xMessCallback (void* data, t_floatarg arg2){
	GetMyClass(data)->xMess ( static_cast<t_float>(arg2));
}
void GEMglProgramEnvParameter4dARB :: yMessCallback (void* data, t_floatarg arg3){
	GetMyClass(data)->yMess ( static_cast<t_float>(arg3));
}
void GEMglProgramEnvParameter4dARB :: zMessCallback (void* data, t_floatarg arg4){
	GetMyClass(data)->zMess ( static_cast<t_float>(arg4));
}
void GEMglProgramEnvParameter4dARB :: wMessCallback (void* data, t_floatarg arg5){
	GetMyClass(data)->wMess ( static_cast<t_float>(arg5));
}
