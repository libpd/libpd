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

#include "GEMglPrioritizeTextures.h"

CPPEXTERN_NEW_WITH_ONE_ARG ( GEMglPrioritizeTextures , t_floatarg, A_DEFFLOAT);

/////////////////////////////////////////////////////////
//
// GEMglViewport
//
/////////////////////////////////////////////////////////
// Constructor
//
GEMglPrioritizeTextures :: GEMglPrioritizeTextures	(t_floatarg arg0=16) :
		n(static_cast<GLsizei>(arg0)) {
	if (n>0) t_len=p_len=n;
	else t_len=p_len=16;

	textures=new GLuint[t_len];
	priorities=new GLclampf[p_len];

	m_inlet[0] = inlet_new(this->x_obj, &this->x_obj->ob_pd, &s_float, gensym("n"));
	m_inlet[1] = inlet_new(this->x_obj, &this->x_obj->ob_pd, &s_float, gensym("textures"));
	m_inlet[2] = inlet_new(this->x_obj, &this->x_obj->ob_pd, &s_float, gensym("priorities"));
}
/////////////////////////////////////////////////////////
// Destructor
//
GEMglPrioritizeTextures :: ~GEMglPrioritizeTextures () {
inlet_free(m_inlet[0]);
inlet_free(m_inlet[1]);
inlet_free(m_inlet[2]);
}

//////////////////
// extension check
bool GEMglPrioritizeTextures :: isRunnable(void) {
  if(GLEW_VERSION_1_1)return true;
  error("your system does not support OpenGL-1.1");
  return false;
}

/////////////////////////////////////////////////////////
// Render
//
void GEMglPrioritizeTextures :: render(GemState *state) {
  int N=n;
  if (t_len<N)N=t_len;
  if (p_len<N)N=p_len;
  glPrioritizeTextures (N, textures, priorities);
}

/////////////////////////////////////////////////////////
// Variables
//
void GEMglPrioritizeTextures :: nMess (t_float arg1) {	// FUN
	n = static_cast<GLsizei>(arg1);
	setModified();
}

void GEMglPrioritizeTextures :: texturesMess (int argc,t_atom*argv) {	// FUN
  if (argc>t_len){
    t_len=argc;
    delete [] textures;
    textures = new GLuint[t_len];
  }
  while(argc--)textures[argc]=static_cast<GLuint>(atom_getint(argv+argc));
  setModified();
}

void GEMglPrioritizeTextures :: prioritiesMess (int argc, t_atom*argv) {	// FUN
  if (argc>p_len){
    p_len=argc;
    delete [] priorities;
    priorities = new GLclampf[p_len];
  }
  while(argc--)priorities[argc]=static_cast<GLclampf>(atom_getfloat(argv+argc));
  setModified();
}


/////////////////////////////////////////////////////////
// static member functions
//

void GEMglPrioritizeTextures :: obj_setupCallback(t_class *classPtr) {
	 class_addmethod(classPtr, reinterpret_cast<t_method>(&GEMglPrioritizeTextures::nMessCallback),  	gensym("n"), A_DEFFLOAT, A_NULL);
	 class_addmethod(classPtr, reinterpret_cast<t_method>(&GEMglPrioritizeTextures::texturesMessCallback),  	gensym("textures"), A_GIMME, A_NULL);
	 class_addmethod(classPtr, reinterpret_cast<t_method>(&GEMglPrioritizeTextures::prioritiesMessCallback),  	gensym("priorities"), A_GIMME, A_NULL);
}

void GEMglPrioritizeTextures :: nMessCallback (void* data, t_floatarg arg0){
	GetMyClass(data)->nMess (arg0);
}
void GEMglPrioritizeTextures :: texturesMessCallback (void* data, t_symbol*, int argc, t_atom*argv){
	GetMyClass(data)->texturesMess (argc,argv);
}
void GEMglPrioritizeTextures :: prioritiesMessCallback (void* data, t_symbol*, int argc, t_atom*argv){
	GetMyClass(data)->prioritiesMess (argc,argv);
}
