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

#include "GEMglMaterialfv.h"
#include "Gem/Exception.h"

CPPEXTERN_NEW_WITH_GIMME ( GEMglMaterialfv);

/////////////////////////////////////////////////////////
//
// GEMglViewport
//
/////////////////////////////////////////////////////////
// Constructor
//
GEMglMaterialfv :: GEMglMaterialfv	(int argc, t_atom*argv) :
		face(0), 
		pname(0)
{
  int i=0;
  for(i=0; i<4; i++)
    param[i]=0;

  switch(argc) {
  case 0: break;
  case 1:
    throw GemException("GEMglMaterialfv: invalid number of parameters");
  default:
    paramMess(argc-2, argv+2);
  case 2: 
    face=getGLdefine(argv);
    pname=getGLdefine(argv+1);
    break;
  }


	m_inlet[0] = inlet_new(this->x_obj, &this->x_obj->ob_pd, &s_float, gensym("face"));
	m_inlet[1] = inlet_new(this->x_obj, &this->x_obj->ob_pd, &s_float, gensym("pname"));
	m_inlet[2] = inlet_new(this->x_obj, &this->x_obj->ob_pd, &s_float, gensym("param"));
}
/////////////////////////////////////////////////////////
// Destructor
//
GEMglMaterialfv :: ~GEMglMaterialfv () {
inlet_free(m_inlet[0]);
inlet_free(m_inlet[1]);
inlet_free(m_inlet[2]);
}

/////////////////////////////////////////////////////////
// Render
//
void GEMglMaterialfv :: render(GemState *state) {
	glMaterialfv (face, pname, param);
}

/////////////////////////////////////////////////////////
// Variables
//
void GEMglMaterialfv :: faceMess (t_atom arg1) {	// FUN
	face = static_cast<GLenum>(getGLdefine(&arg1));
	setModified();
}

void GEMglMaterialfv :: pnameMess (t_atom arg1) {	// FUN
	pname = static_cast<GLenum>(getGLdefine(&arg1));
	setModified();
}

void GEMglMaterialfv :: paramMess (int argc, t_atom*argv) {	// FUN
  int i=0;
  for(i=0; (i<argc) && (i<4); i++) {
    param[i]=static_cast<GLfloat>(atom_getfloat(argv+i));
  }
	setModified();
}


/////////////////////////////////////////////////////////
// static member functions
//

void GEMglMaterialfv :: obj_setupCallback(t_class *classPtr) {
	 class_addmethod(classPtr, reinterpret_cast<t_method>(&GEMglMaterialfv::faceMessCallback),  	gensym("face"), A_GIMME, A_NULL);
	 class_addmethod(classPtr, reinterpret_cast<t_method>(&GEMglMaterialfv::pnameMessCallback),  	gensym("pname"), A_GIMME, A_NULL);
	 class_addmethod(classPtr, reinterpret_cast<t_method>(&GEMglMaterialfv::paramMessCallback),  	gensym("param"), A_GIMME, A_NULL);
};

void GEMglMaterialfv :: faceMessCallback (void* data, t_symbol*, int argc, t_atom*argv){
  if(argc==1) GetMyClass(data)->faceMess (*argv);
}
void GEMglMaterialfv :: pnameMessCallback (void* data, t_symbol*, int argc, t_atom*argv){
	if(argc==1) GetMyClass(data)->pnameMess (*argv);
}
void GEMglMaterialfv :: paramMessCallback (void* data, t_symbol*, int argc, t_atom*argv){
	GetMyClass(data)->paramMess (argc, argv);
}
