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

#include "GEMglGenTextures.h"

CPPEXTERN_NEW_WITH_GIMME ( GEMglGenTextures);

/////////////////////////////////////////////////////////
//
// GEMglViewport
//
/////////////////////////////////////////////////////////
// Constructor
//
GEMglGenTextures :: GEMglGenTextures	(int argc, t_atom*argv) :
  n(0), textures(NULL)
{
	if(argc)texturesMess(argc, argv);
	
	m_inlet = inlet_new(this->x_obj, &this->x_obj->ob_pd, &s_float, gensym("textures"));

}
/////////////////////////////////////////////////////////
// Destructor
//
GEMglGenTextures :: ~GEMglGenTextures () {
inlet_free(m_inlet);
}

//////////////////
// extension check
bool GEMglGenTextures :: isRunnable(void) {
  if(GLEW_VERSION_1_1)return true;
  error("your system does not support OpenGL-1.1");
  return false;
}

/////////////////////////////////////////////////////////
// Render
//
void GEMglGenTextures :: render(GemState *state) {
	glGenTextures (n, textures);
}

/////////////////////////////////////////////////////////
// Variables
//
void GEMglGenTextures :: texturesMess (int argc, t_atom*argv) {	// FUN
  if(!argc) {
    error("no textures specified!");
    return;
  }
  n=0;
  if(textures) delete [] textures;
  textures = new GLuint[argc];
  while(argc--){
    if(argv->a_type == A_FLOAT)textures[n++] = static_cast<GLuint>(atom_getint(argv));
    argv++;
  }
  setModified();
}



/////////////////////////////////////////////////////////
// static member functions
//

void GEMglGenTextures :: obj_setupCallback(t_class *classPtr) {
	 class_addmethod(classPtr, reinterpret_cast<t_method>(&GEMglGenTextures::texturesMessCallback),  	gensym("textures"), A_GIMME, A_NULL);
}
void GEMglGenTextures :: texturesMessCallback (void* data, t_symbol*, int argc, t_atom*argv){
	GetMyClass(data)->texturesMess (argc,argv);
}
