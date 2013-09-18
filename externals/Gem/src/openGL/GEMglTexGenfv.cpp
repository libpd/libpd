////////////////////////////////////////////////////////
//
// GEM - Graphics Environment for Multimedia
//
// Implementation file
//
// Copyright (c) 2006 james tittle, tigital@mac.com
//	
//  For information on usage and redistribution, and for a DISCLAIMER
//  *  OF ALL WARRANTIES, see the file, "GEM.LICENSE.TERMS"
//
////////////////////////////////////////////////////////

#include "GEMglTexGenfv.h"

CPPEXTERN_NEW_WITH_THREE_ARGS ( GEMglTexGenfv , t_floatarg, A_DEFFLOAT, t_floatarg, A_DEFFLOAT, t_floatarg, A_DEFFLOAT);

/////////////////////////////////////////////////////////
//
// GEMglTexGenfv
//
/////////////////////////////////////////////////////////
// Constructor
//
GEMglTexGenfv :: GEMglTexGenfv	(t_floatarg arg0=0, t_floatarg arg1=0, t_floatarg arg2=0) :
		coord(static_cast<GLenum>(arg0)), 
		pname(static_cast<GLenum>(arg1))
{
	m_inlet[0] = inlet_new(this->x_obj, &this->x_obj->ob_pd, &s_float, gensym("coord"));
	m_inlet[1] = inlet_new(this->x_obj, &this->x_obj->ob_pd, &s_float, gensym("pname"));
	m_inlet[2] = inlet_new(this->x_obj, &this->x_obj->ob_pd, &s_float, gensym("params"));
}
/////////////////////////////////////////////////////////
// Destructor
//
GEMglTexGenfv :: ~GEMglTexGenfv () {
inlet_free(m_inlet[0]);
inlet_free(m_inlet[1]);
inlet_free(m_inlet[2]);
}

/////////////////////////////////////////////////////////
// Render
//
void GEMglTexGenfv :: render(GemState *state) {
	glTexGenfv (coord, pname, params);
}

/////////////////////////////////////////////////////////
// Variables
//
void GEMglTexGenfv :: coordMess (t_float arg1) {	// FUN
	coord = static_cast<GLenum>(arg1);
	setModified();
}

void GEMglTexGenfv :: pnameMess (t_float arg1) {	// FUN
	pname = static_cast<GLenum>(arg1);
	setModified();
}

void GEMglTexGenfv :: paramsMess (int argc, t_atom*argv) {	// FUN
	if(argc!=4){
		error("needs 4 elements");
		return;
	}
	int i;
	for (i=0;i<4;i++){
		params[i] = static_cast<GLfloat>(atom_getfloat(argv+i));
	}
	setModified();
}


/////////////////////////////////////////////////////////
// static member functions
//

void GEMglTexGenfv :: obj_setupCallback(t_class *classPtr) {
	 class_addmethod(classPtr, reinterpret_cast<t_method>(&GEMglTexGenfv::coordMessCallback),  	gensym("coord"), A_DEFFLOAT, A_NULL);
	 class_addmethod(classPtr, reinterpret_cast<t_method>(&GEMglTexGenfv::pnameMessCallback),  	gensym("pname"), A_DEFFLOAT, A_NULL);
	 class_addmethod(classPtr, reinterpret_cast<t_method>(&GEMglTexGenfv::paramsMessCallback),  	gensym("params"), A_GIMME, A_NULL);
};

void GEMglTexGenfv :: coordMessCallback (void* data, t_floatarg arg0){
	GetMyClass(data)->coordMess ( static_cast<t_float>(arg0));
}
void GEMglTexGenfv :: pnameMessCallback (void* data, t_floatarg arg0){
	GetMyClass(data)->pnameMess ( static_cast<t_float>(arg0));
}
void GEMglTexGenfv :: paramsMessCallback (void* data, t_symbol*,int argc, t_atom*argv){
	GetMyClass(data)->paramsMess ( argc, argv );
}
