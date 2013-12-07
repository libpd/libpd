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

#include "GEMglMultMatrixd.h"

CPPEXTERN_NEW_WITH_ONE_ARG ( GEMglMultMatrixd , t_floatarg, A_DEFFLOAT );

/////////////////////////////////////////////////////////
//
// GEMglMultMatrixd
//
/////////////////////////////////////////////////////////
// Constructor
//
GEMglMultMatrixd :: GEMglMultMatrixd	(t_floatarg arg0=0)
{
	m_inlet = inlet_new(this->x_obj, &this->x_obj->ob_pd, &s_float, gensym("list"));
}
/////////////////////////////////////////////////////////
// Destructor
//
GEMglMultMatrixd :: ~GEMglMultMatrixd () {
	inlet_free(m_inlet);
}

//////////////////
// extension check
bool GEMglMultMatrixd :: isRunnable(void) {
  if(GLEW_VERSION_1_1)return true;
  error("your system does not support OpenGL-1.1");
  return false;
}


/////////////////////////////////////////////////////////
// Render
//
void GEMglMultMatrixd :: render(GemState *state) {
	glMultMatrixd (m_matrix);
}

/////////////////////////////////////////////////////////
// Variables
//
void GEMglMultMatrixd :: matrixMess (int argc, t_atom*argv) {	// FUN
	if(argc!=16){
		error("need 16 (4x4) elements");
		return;
		}
	int i;
	for (i=0;i<16;i++) {
	  m_matrix[i]=static_cast<GLdouble>(atom_getfloat(argv+i));
	}
	setModified();
}

/////////////////////////////////////////////////////////
// static member functions
//

void GEMglMultMatrixd :: obj_setupCallback(t_class *classPtr) {
	 class_addmethod(classPtr, reinterpret_cast<t_method>(&GEMglMultMatrixd::matrixMessCallback),  	
										gensym("list"), A_GIMME, A_NULL);
}

void GEMglMultMatrixd :: matrixMessCallback (void* data, t_symbol*,int argc, t_atom*argv){
	GetMyClass(data)->matrixMess ( argc, argv);
}
