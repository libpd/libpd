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

#include "GEMglFogfv.h"

CPPEXTERN_NEW_WITH_GIMME ( GEMglFogfv );

/////////////////////////////////////////////////////////
//
// GEMglFogfv
//
/////////////////////////////////////////////////////////
// Constructor
//
GEMglFogfv :: GEMglFogfv	(int argc, t_atom *argv) {
	int i=FOG_ARRAY_LENGTH;
	while(i--)params[i]=0.0;

	pnameMess(atom_getfloat(argv));
	if (argc>0)paramsMess(argc-1, argv+1);

	m_inlet[0] = inlet_new(this->x_obj, &this->x_obj->ob_pd, &s_float, gensym("pname"));
	m_inlet[1] = inlet_new(this->x_obj, &this->x_obj->ob_pd, &s_list, gensym("params"));
}
/////////////////////////////////////////////////////////
// Destructor
//
GEMglFogfv :: ~GEMglFogfv () {
	inlet_free(m_inlet[0]);
	inlet_free(m_inlet[1]);
}

//////////////////
// extension check
bool GEMglFogfv :: isRunnable(void) {
  if(GLEW_VERSION_1_1)return true;
  error("your system does not support OpenGL-1.1");
  return false;
}

/////////////////////////////////////////////////////////
// Render
//
void GEMglFogfv :: render(GemState *state) {
	glFogfv (pname, params);
}

/////////////////////////////////////////////////////////
// variable
//
void GEMglFogfv :: pnameMess (t_float arg0) {	// FUN
  pname=static_cast<GLenum>(arg0);
  setModified();
}
void GEMglFogfv :: paramsMess (int argc, t_atom*argv) {	// FUN
  int i = (argc<FOG_ARRAY_LENGTH)?argc:FOG_ARRAY_LENGTH;
  while(i--)params[i]=atom_getfloat(argv+i);
  setModified();
}


/////////////////////////////////////////////////////////
// static member functions
//

void GEMglFogfv :: obj_setupCallback(t_class *classPtr) {
	 class_addmethod(classPtr, reinterpret_cast<t_method>(&GEMglFogfv::pnameMessCallback),  	gensym("pname"), A_DEFFLOAT, A_NULL);
	 class_addmethod(classPtr, reinterpret_cast<t_method>(&GEMglFogfv::paramsMessCallback),  	gensym("params"), A_GIMME, A_NULL);
}

void GEMglFogfv :: pnameMessCallback (void* data, t_floatarg arg0) {
	GetMyClass(data)->pnameMess (arg0);
}
void GEMglFogfv :: paramsMessCallback (void* data, t_symbol*, int argc, t_atom* argv) {
	GetMyClass(data)->paramsMess (argc, argv);
}

