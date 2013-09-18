////////////////////////////////////////////////////////
//
// GEM - Graphics Environment for Multimedia
//
// Implementation file
//
// Copyright (c) 2004-2005 tigital@mac.com
//  For information on usage and redistribution, and for a DISCLAIMER
//  *  OF ALL WARRANTIES, see the file, "GEM.LICENSE.TERMS"
//
////////////////////////////////////////////////////////

#include "GEMglBindProgramARB.h"

CPPEXTERN_NEW_WITH_TWO_ARGS ( GEMglBindProgramARB , t_floatarg, A_DEFFLOAT, t_floatarg, A_DEFFLOAT);

/////////////////////////////////////////////////////////
//
// GEMglBindProgramARB
//
/////////////////////////////////////////////////////////
// Constructor
//
GEMglBindProgramARB :: GEMglBindProgramARB	(t_floatarg arg0=0, t_floatarg arg1=0) :
		target(static_cast<GLenum>(arg0)), 
		program(static_cast<GLuint>(arg1))
{
	m_inlet[0] = inlet_new(this->x_obj, &this->x_obj->ob_pd, &s_float, gensym("target"));
	m_inlet[1] = inlet_new(this->x_obj, &this->x_obj->ob_pd, &s_float, gensym("program"));
}
/////////////////////////////////////////////////////////
// Destructor
//
GEMglBindProgramARB :: ~GEMglBindProgramARB () {
inlet_free(m_inlet[0]);
inlet_free(m_inlet[1]);
}
//////////////////
// extension check
bool GEMglBindProgramARB :: isRunnable(void) {
  if(GLEW_ARB_vertex_program)return true;
  error("your system does not support the ARB vertex_program extension");
  return false;
}
/////////////////////////////////////////////////////////
// Render
//
void GEMglBindProgramARB :: render(GemState *state) {
	glBindProgramARB (target, program);
}

/////////////////////////////////////////////////////////
// Variables
//
void GEMglBindProgramARB :: targetMess (t_float arg1) {	// FUN
	target = static_cast<GLenum>(arg1);
	setModified();
}

void GEMglBindProgramARB :: programMess (t_float arg1) {	// FUN
	program = static_cast<GLuint>(arg1);
	setModified();
}


/////////////////////////////////////////////////////////
// static member functions
//

void GEMglBindProgramARB :: obj_setupCallback(t_class *classPtr) {
	 class_addmethod(classPtr, reinterpret_cast<t_method>(&GEMglBindProgramARB::targetMessCallback),  	gensym("target"), A_DEFFLOAT, A_NULL);
	 class_addmethod(classPtr, reinterpret_cast<t_method>(&GEMglBindProgramARB::programMessCallback),  	gensym("program"), A_DEFFLOAT, A_NULL);
}

void GEMglBindProgramARB :: targetMessCallback (void* data, t_floatarg arg0){
	GetMyClass(data)->targetMess ( static_cast<t_float>(arg0));
}
void GEMglBindProgramARB :: programMessCallback (void* data, t_floatarg arg0){
	GetMyClass(data)->programMess ( static_cast<t_float>(arg0));
}
