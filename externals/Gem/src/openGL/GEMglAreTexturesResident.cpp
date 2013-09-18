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

#include "GEMglAreTexturesResident.h"

CPPEXTERN_NEW_WITH_GIMME ( GEMglAreTexturesResident );

/////////////////////////////////////////////////////////
//
// GEMglViewport
//
/////////////////////////////////////////////////////////
// Constructor
//
GEMglAreTexturesResident :: GEMglAreTexturesResident	(int argc, t_atom*argv) {
	len=32;
	textures  =new GLuint   [len];
	residences=new GLboolean[len];
	m_buffer  =new t_atom   [len];
	texturesMess(argc, argv);
	
	m_inlet = inlet_new(this->x_obj, &this->x_obj->ob_pd, &s_list, gensym("textures"));
	m_out1 = outlet_new(this->x_obj, 0);
	m_out2 = outlet_new(this->x_obj, 0);
}
/////////////////////////////////////////////////////////
// Destructor
//
GEMglAreTexturesResident :: ~GEMglAreTexturesResident () {
  inlet_free(m_inlet);
  outlet_free(m_out1);
  outlet_free(m_out2);
}
//////////////////
// extension check
bool GEMglAreTexturesResident :: isRunnable(void) {
  if(GLEW_VERSION_1_1)return true;
  error("your system does not support OpenGL-1.1");
  return false;
}
/////////////////////////////////////////////////////////
// Render
//
void GEMglAreTexturesResident :: render(GemState *state) {
  GLboolean ok = glAreTexturesResident (n, textures, residences);
  int i=n;
  while(i--){
    t_float f = residences[i]?1.0:0.0;
    SETFLOAT(m_buffer+i, f);
  }
  outlet_list(m_out2, &s_list, n, m_buffer);
  outlet_float(m_out1, (ok?1.0:0.0));
}

/////////////////////////////////////////////////////////
// Variables
//
void GEMglAreTexturesResident :: texturesMess (int argc, t_atom*argv) {
  if (argc>len){
    len=argc;
    delete[]textures;   textures  =new GLuint   [len];
    delete[]residences; residences=new GLboolean[len];
    delete[]m_buffer;   m_buffer  =new t_atom   [len];
  }
  n=argc;
  while(argc--)textures[argc]=atom_getint(argv+argc);
  setModified();
}

/////////////////////////////////////////////////////////
// static member functions
//

void GEMglAreTexturesResident :: obj_setupCallback(t_class *classPtr) {
	 class_addmethod(classPtr, reinterpret_cast<t_method>(&GEMglAreTexturesResident::texturesMessCallback),  	gensym("textures"), A_GIMME, A_NULL);
}
void GEMglAreTexturesResident :: texturesMessCallback (void* data, t_symbol*,int argc, t_atom*argv){
	GetMyClass(data)->texturesMess (argc,argv);
}
