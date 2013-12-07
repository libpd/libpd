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

#include "GEMglActiveTextureARB.h"

CPPEXTERN_NEW_WITH_ONE_ARG ( GEMglActiveTextureARB , t_floatarg, A_DEFFLOAT);

/////////////////////////////////////////////////////////
//
// GEMglActiveTextureARB
//
/////////////////////////////////////////////////////////
// Constructor
//
GEMglActiveTextureARB :: GEMglActiveTextureARB	(t_floatarg arg0=0) :
		texUnit(static_cast<GLenum>(arg0))
{
	m_inlet[0] = inlet_new(this->x_obj, &this->x_obj->ob_pd, &s_float, gensym("texUnit"));
}
/////////////////////////////////////////////////////////
// Destructor
//
GEMglActiveTextureARB :: ~GEMglActiveTextureARB () {
	inlet_free(m_inlet[0]);
}
//////////////////
// extension check
bool GEMglActiveTextureARB :: isRunnable(void) {
  if(GLEW_ARB_multitexture)return true;
  error("your system does not support the ARB multitexture extension");
  return false;
}
/////////////////////////////////////////////////////////
// Render
//
void GEMglActiveTextureARB :: render(GemState *state) {
	glActiveTextureARB (texUnit);
}

/////////////////////////////////////////////////////////
// Variables
//
void GEMglActiveTextureARB :: texUnitMess (t_float arg1) {	// FUN
	texUnit = static_cast<GLenum>(arg1);
	setModified();
}

/////////////////////////////////////////////////////////
// static member functions
//

void GEMglActiveTextureARB :: obj_setupCallback(t_class *classPtr) {
	 class_addmethod(classPtr, reinterpret_cast<t_method>(&GEMglActiveTextureARB::texUnitMessCallback),  	
									gensym("texUnit"), A_DEFFLOAT, A_NULL);
}

void GEMglActiveTextureARB :: texUnitMessCallback (void* data, t_floatarg arg0){
	GetMyClass(data)->texUnitMess ( static_cast<t_float>(arg0));
}
