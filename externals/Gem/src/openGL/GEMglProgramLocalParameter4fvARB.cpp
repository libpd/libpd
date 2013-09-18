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

#include "GEMglProgramLocalParameter4fvARB.h"

CPPEXTERN_NEW_WITH_THREE_ARGS ( GEMglProgramLocalParameter4fvARB , t_floatarg, A_DEFFLOAT, t_floatarg, A_DEFFLOAT, t_floatarg, A_DEFFLOAT);

/////////////////////////////////////////////////////////
//
// GEMglProgramLocalParameter4fvARB
//
/////////////////////////////////////////////////////////
// Constructor
//
GEMglProgramLocalParameter4fvARB :: GEMglProgramLocalParameter4fvARB	(t_floatarg arg0=0, t_floatarg arg1=0, t_floatarg arg2=0) :
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
GEMglProgramLocalParameter4fvARB :: ~GEMglProgramLocalParameter4fvARB () {
	inlet_free(m_inlet[0]);
	inlet_free(m_inlet[1]);
	inlet_free(m_inlet[2]);
}

//////////////////
// extension check
bool GEMglProgramLocalParameter4fvARB :: isRunnable(void) {
  if(GLEW_ARB_vertex_program)return true;
  error("your system does not support the ARB vertex_program extension");
  return false;
}


/////////////////////////////////////////////////////////
// Render
//
void GEMglProgramLocalParameter4fvARB :: render(GemState *state) {
	glProgramLocalParameter4fvARB (target, index, params);
}

/////////////////////////////////////////////////////////
// Variables
//
void GEMglProgramLocalParameter4fvARB :: targetMess (t_float arg1) {	// FUN
	target = static_cast<GLenum>(arg1);
	setModified();
}

void GEMglProgramLocalParameter4fvARB :: indexMess (t_float arg1) {	// FUN
	index = static_cast<GLenum>(arg1);
	setModified();
}

void GEMglProgramLocalParameter4fvARB :: paramsMess (int argc, t_atom*argv) {	// FUN
	if(argc!=4){
		error("GEMglProgramLocalParamter4vARB:  needs 4 elements");
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

void GEMglProgramLocalParameter4fvARB :: obj_setupCallback(t_class *classPtr) {
	 class_addmethod(classPtr, reinterpret_cast<t_method>(&GEMglProgramLocalParameter4fvARB::targetMessCallback),  	gensym("target"), A_DEFFLOAT, A_NULL);
	 class_addmethod(classPtr, reinterpret_cast<t_method>(&GEMglProgramLocalParameter4fvARB::indexMessCallback),  	gensym("index"), A_DEFFLOAT, A_NULL);
	 class_addmethod(classPtr, reinterpret_cast<t_method>(&GEMglProgramLocalParameter4fvARB::paramsMessCallback),  	gensym("params"), A_GIMME, A_NULL);
};

void GEMglProgramLocalParameter4fvARB :: targetMessCallback (void* data, t_floatarg arg0){
	GetMyClass(data)->targetMess ( static_cast<t_float>(arg0));
}
void GEMglProgramLocalParameter4fvARB :: indexMessCallback (void* data, t_floatarg arg0){
	GetMyClass(data)->indexMess ( static_cast<t_float>(arg0));
}
void GEMglProgramLocalParameter4fvARB :: paramsMessCallback (void* data, t_symbol*, int argc, t_atom* argv){
	GetMyClass(data)->paramsMess ( argc, argv);
}
