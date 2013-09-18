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

#include "GEMglMultiTexCoord2fARB.h"

CPPEXTERN_NEW_WITH_THREE_ARGS ( GEMglMultiTexCoord2fARB , t_floatarg, A_DEFFLOAT, t_floatarg, A_DEFFLOAT, t_floatarg, A_DEFFLOAT);

/////////////////////////////////////////////////////////
//
// GEMglMultiTexCoord2fARB
//
/////////////////////////////////////////////////////////
// Constructor
//
GEMglMultiTexCoord2fARB :: GEMglMultiTexCoord2fARB	(t_floatarg arg0=0, t_floatarg arg1=0, t_floatarg arg2=0) :
		texUnit(static_cast<GLenum>(arg0)), 
		s(static_cast<GLfloat>(arg1)), 
		t(static_cast<GLfloat>(arg2))
{
	m_inlet[0] = inlet_new(this->x_obj, &this->x_obj->ob_pd, &s_float, gensym("texUnit"));
	m_inlet[1] = inlet_new(this->x_obj, &this->x_obj->ob_pd, &s_float, gensym("s"));
	m_inlet[2] = inlet_new(this->x_obj, &this->x_obj->ob_pd, &s_float, gensym("t"));
}
/////////////////////////////////////////////////////////
// Destructor
//
GEMglMultiTexCoord2fARB :: ~GEMglMultiTexCoord2fARB () {
	inlet_free(m_inlet[0]);
	inlet_free(m_inlet[1]);
	inlet_free(m_inlet[2]);
}

//////////////////
// extension check
bool GEMglMultiTexCoord2fARB :: isRunnable(void) {
  if(GLEW_ARB_multitexture)return true;
  error("your system does not support the ARB multitexture extension");
  return false;
}


/////////////////////////////////////////////////////////
// Render
//
void GEMglMultiTexCoord2fARB :: render(GemState *state) {
	glMultiTexCoord2fARB (texUnit, s, t);
}

/////////////////////////////////////////////////////////
// Variables
//
void GEMglMultiTexCoord2fARB :: texUnitMess (t_float arg1) {	// FUN
	texUnit = static_cast<GLenum>(arg1);
	setModified();
}

void GEMglMultiTexCoord2fARB :: sMess (t_float arg1) {	// FUN
	s = static_cast<GLfloat>(arg1);
	setModified();
}

void GEMglMultiTexCoord2fARB :: tMess (t_float arg1) {	// FUN
	t = static_cast<GLfloat>(arg1);
	setModified();
}


/////////////////////////////////////////////////////////
// static member functions
//

void GEMglMultiTexCoord2fARB :: obj_setupCallback(t_class *classPtr) {
	 class_addmethod(classPtr, reinterpret_cast<t_method>(&GEMglMultiTexCoord2fARB::texUnitMessCallback),  	
					gensym("texUnit"), A_DEFFLOAT, A_NULL);
	 class_addmethod(classPtr, reinterpret_cast<t_method>(&GEMglMultiTexCoord2fARB::sMessCallback),  	
					gensym("s"), A_DEFFLOAT, A_NULL);
	 class_addmethod(classPtr, reinterpret_cast<t_method>(&GEMglMultiTexCoord2fARB::tMessCallback),  	
					gensym("t"), A_DEFFLOAT, A_NULL);
};

void GEMglMultiTexCoord2fARB :: texUnitMessCallback (void* data, t_floatarg arg0){
	GetMyClass(data)->texUnitMess ( static_cast<t_float>(arg0));
}
void GEMglMultiTexCoord2fARB :: sMessCallback (void* data, t_floatarg arg0){
	GetMyClass(data)->sMess ( static_cast<t_float>(arg0));
}
void GEMglMultiTexCoord2fARB :: tMessCallback (void* data, t_floatarg arg0){
	GetMyClass(data)->tMess ( static_cast<t_float>(arg0));
}
