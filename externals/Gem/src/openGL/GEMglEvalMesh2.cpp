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

#include "GEMglEvalMesh2.h"

CPPEXTERN_NEW_WITH_GIMME ( GEMglEvalMesh2 );

/////////////////////////////////////////////////////////
//
// GEMglViewport
//
/////////////////////////////////////////////////////////
// Constructor
//
GEMglEvalMesh2 :: GEMglEvalMesh2	(int argc, t_atom*argv){
	mode=0;
	i1=i2=j1=j2=0;
	if (argc>0)mode=atom_getint(argv+0);
	if (argc>1)i1  =atom_getint(argv+1);
	if (argc>2)i2  =atom_getint(argv+2);
	if (argc>3)j1  =atom_getint(argv+3);
	if (argc>4)j2  =atom_getint(argv+4);

	m_inlet[0] = inlet_new(this->x_obj, &this->x_obj->ob_pd, &s_float, gensym("mode"));
	m_inlet[1] = inlet_new(this->x_obj, &this->x_obj->ob_pd, &s_float, gensym("i1"));
	m_inlet[2] = inlet_new(this->x_obj, &this->x_obj->ob_pd, &s_float, gensym("i2"));
	m_inlet[3] = inlet_new(this->x_obj, &this->x_obj->ob_pd, &s_float, gensym("j1"));
	m_inlet[4] = inlet_new(this->x_obj, &this->x_obj->ob_pd, &s_float, gensym("j2"));
}
/////////////////////////////////////////////////////////
// Destructor
//
GEMglEvalMesh2 :: ~GEMglEvalMesh2 () {
inlet_free(m_inlet[0]);
inlet_free(m_inlet[1]);
inlet_free(m_inlet[2]);
inlet_free(m_inlet[3]);
inlet_free(m_inlet[4]);
}
//////////////////
// extension check
bool GEMglEvalMesh2 :: isRunnable(void) {
  if(GLEW_VERSION_1_1)return true;
  error("your system does not support OpenGL-1.1");
  return false;
}
/////////////////////////////////////////////////////////
// Render
//
void GEMglEvalMesh2 :: render(GemState *state) {
	glEvalMesh2 (mode, i1, i2, j1, j2);
}

/////////////////////////////////////////////////////////
// Variables
//
void GEMglEvalMesh2 :: modeMess (t_float arg1) {	// FUN
	mode = static_cast<GLenum>(arg1);
	setModified();
}

void GEMglEvalMesh2 :: i1Mess (t_float arg1) {	// FUN
	i1 = static_cast<GLint>(arg1);
	setModified();
}

void GEMglEvalMesh2 :: i2Mess (t_float arg1) {	// FUN
	i2 = static_cast<GLint>(arg1);
	setModified();
}

void GEMglEvalMesh2 :: j1Mess (t_float arg1) {	// FUN
	j1 = static_cast<GLint>(arg1);
	setModified();
}

void GEMglEvalMesh2 :: j2Mess (t_float arg1) {	// FUN
	j2 = static_cast<GLint>(arg1);
	setModified();
}


/////////////////////////////////////////////////////////
// static member functions
//

void GEMglEvalMesh2 :: obj_setupCallback(t_class *classPtr) {
	 class_addmethod(classPtr, reinterpret_cast<t_method>(&GEMglEvalMesh2::modeMessCallback),  	gensym("mode"), A_DEFFLOAT, A_NULL);
	 class_addmethod(classPtr, reinterpret_cast<t_method>(&GEMglEvalMesh2::i1MessCallback),  	gensym("i1"), A_DEFFLOAT, A_NULL);
	 class_addmethod(classPtr, reinterpret_cast<t_method>(&GEMglEvalMesh2::i2MessCallback),  	gensym("i2"), A_DEFFLOAT, A_NULL);
	 class_addmethod(classPtr, reinterpret_cast<t_method>(&GEMglEvalMesh2::j1MessCallback),  	gensym("j1"), A_DEFFLOAT, A_NULL);
	 class_addmethod(classPtr, reinterpret_cast<t_method>(&GEMglEvalMesh2::j2MessCallback),  	gensym("j2"), A_DEFFLOAT, A_NULL);
}

void GEMglEvalMesh2 :: modeMessCallback (void* data, t_floatarg arg0){
	GetMyClass(data)->modeMess ( static_cast<t_float>(arg0));
}
void GEMglEvalMesh2 :: i1MessCallback (void* data, t_floatarg arg0){
	GetMyClass(data)->i1Mess ( static_cast<t_float>(arg0));
}
void GEMglEvalMesh2 :: i2MessCallback (void* data, t_floatarg arg0){
	GetMyClass(data)->i2Mess ( static_cast<t_float>(arg0));
}
void GEMglEvalMesh2 :: j1MessCallback (void* data, t_floatarg arg0){
	GetMyClass(data)->j1Mess ( static_cast<t_float>(arg0));
}
void GEMglEvalMesh2 :: j2MessCallback (void* data, t_floatarg arg0){
	GetMyClass(data)->j2Mess ( static_cast<t_float>(arg0));
}
