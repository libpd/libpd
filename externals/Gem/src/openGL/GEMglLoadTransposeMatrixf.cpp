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

#include "GEMglLoadTransposeMatrixf.h"

CPPEXTERN_NEW_WITH_ONE_ARG ( GEMglLoadTransposeMatrixf , t_floatarg, A_DEFFLOAT );

/////////////////////////////////////////////////////////
//
// GEMglLoadTransposeMatrixf
//
/////////////////////////////////////////////////////////
// Constructor
//
GEMglLoadTransposeMatrixf :: GEMglLoadTransposeMatrixf	(t_floatarg arg0=0)
{
	m_inlet = inlet_new(this->x_obj, &this->x_obj->ob_pd, &s_list, gensym("list"));
}
/////////////////////////////////////////////////////////
// Destructor
//
GEMglLoadTransposeMatrixf :: ~GEMglLoadTransposeMatrixf () {
inlet_free(m_inlet);
}

//////////////////
// extension check
bool GEMglLoadTransposeMatrixf :: isRunnable(void) {
  if(GLEW_VERSION_1_3)return true;
  error("your system does not support OpenGL-1.3");
  return false;
}

/////////////////////////////////////////////////////////
// Render
//
void GEMglLoadTransposeMatrixf :: render(GemState *state) {
	glLoadTransposeMatrixf (m_matrix);
}

/////////////////////////////////////////////////////////
// Variables
//
void GEMglLoadTransposeMatrixf :: matrixMess (int argc, t_atom*argv) {	// FUN
	if(argc!=16){
		error("need 16 (4x4) elements");
		return;
		}
	int i;
	for (i=0;i<16;i++) {
	  m_matrix[i]=static_cast<GLfloat>(atom_getfloat(argv+i));
	}
	setModified();
}

/////////////////////////////////////////////////////////
// static member functions
//

void GEMglLoadTransposeMatrixf :: obj_setupCallback(t_class *classPtr) {
	 class_addmethod(classPtr, reinterpret_cast<t_method>(&GEMglLoadTransposeMatrixf::matrixMessCallback),  	gensym("list"), A_GIMME, A_NULL);
}

void GEMglLoadTransposeMatrixf :: matrixMessCallback (void* data, t_symbol*, int argc, t_atom*argv){
	GetMyClass(data)->matrixMess ( argc, argv);
}
