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

#include "GEMglProgramEnvParameter4fvARB.h"

CPPEXTERN_NEW_WITH_THREE_ARGS ( GEMglProgramEnvParameter4fvARB , t_floatarg, A_DEFFLOAT, t_floatarg, A_DEFFLOAT, t_floatarg, A_DEFFLOAT);

/////////////////////////////////////////////////////////
//
// GEMglProgramEnvParameter4fvARB
//
/////////////////////////////////////////////////////////
// Constructor
//
GEMglProgramEnvParameter4fvARB :: GEMglProgramEnvParameter4fvARB	(t_floatarg arg0=0, t_floatarg arg1=0, t_floatarg arg2=0) :
		target(static_cast<GLenum>(arg0)), 
		index(static_cast<GLenum>(arg1)) 
		//params(static_cast<GLfloat>(arg2))
{
	m_inlet[0] = inlet_new(this->x_obj, &this->x_obj->ob_pd, &s_float, gensym("target"));
	m_inlet[1] = inlet_new(this->x_obj, &this->x_obj->ob_pd, &s_float, gensym("index"));
	m_inlet[2] = inlet_new(this->x_obj, &this->x_obj->ob_pd, &s_float, gensym("params"));
}
/////////////////////////////////////////////////////////
// Destructor
//
GEMglProgramEnvParameter4fvARB :: ~GEMglProgramEnvParameter4fvARB () {
inlet_free(m_inlet[0]);
inlet_free(m_inlet[1]);
inlet_free(m_inlet[2]);
}

//////////////////
// extension check
bool GEMglProgramEnvParameter4fvARB :: isRunnable(void) {
  if(GLEW_ARB_vertex_program)return true;
  error("your system does not the ARB vertex_program extension");
  return false;
}


/////////////////////////////////////////////////////////
// Render
//
void GEMglProgramEnvParameter4fvARB :: render(GemState *state) {
	glProgramEnvParameter4fvARB (target, index, params);
}

/////////////////////////////////////////////////////////
// Variables
//
void GEMglProgramEnvParameter4fvARB :: targetMess (t_float arg1) {	// FUN
	target = static_cast<GLenum>(arg1);
	setModified();
}

void GEMglProgramEnvParameter4fvARB :: indexMess (t_float arg1) {	// FUN
	index = static_cast<GLenum>(arg1);
	setModified();
}

void GEMglProgramEnvParameter4fvARB :: paramsMess (int argc, t_atom*argv) {	// FUN
	if(argc!=4){
		error("GEMglProgramEnvParameter4vARB:  needs 4 elements");
		return;
	}
	int i;
	for (i=0;i<4;i++){
		params[i] = static_cast<GLfloat>(atom_getfloat(argv+i));
		//post("params[%i] = %f\n",i,params[i]);
	}
	setModified();
}


/////////////////////////////////////////////////////////
// static member functions
//

void GEMglProgramEnvParameter4fvARB :: obj_setupCallback(t_class *classPtr) {
	 class_addmethod(classPtr, reinterpret_cast<t_method>(&GEMglProgramEnvParameter4fvARB::targetMessCallback),  	gensym("target"), A_DEFFLOAT, A_NULL);
	 class_addmethod(classPtr, reinterpret_cast<t_method>(&GEMglProgramEnvParameter4fvARB::indexMessCallback),  	gensym("index"), A_DEFFLOAT, A_NULL);
	 class_addmethod(classPtr, reinterpret_cast<t_method>(&GEMglProgramEnvParameter4fvARB::paramsMessCallback),  	gensym("params"), A_GIMME, A_NULL);
};

void GEMglProgramEnvParameter4fvARB :: targetMessCallback (void* data, t_floatarg arg0){
	GetMyClass(data)->targetMess ( static_cast<t_float>(arg0));
}
void GEMglProgramEnvParameter4fvARB :: indexMessCallback (void* data, t_floatarg arg0){
	GetMyClass(data)->indexMess ( static_cast<t_float>(arg0));
}
void GEMglProgramEnvParameter4fvARB :: paramsMessCallback (void* data, t_symbol*,int argc, t_atom*argv){
	GetMyClass(data)->paramsMess ( argc, argv );
}
