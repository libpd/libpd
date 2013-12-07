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

#include "GEMglGenProgramsARB.h"

CPPEXTERN_NEW_WITH_GIMME ( GEMglGenProgramsARB);

/////////////////////////////////////////////////////////
//
// GEMglGenProgramsARB
//
/////////////////////////////////////////////////////////
// Constructor
//
GEMglGenProgramsARB :: GEMglGenProgramsARB	(int argc, t_atom*argv) :
  n(0), programs(NULL)
{
	programsMess(argc, argv);
	
	m_inlet = inlet_new(this->x_obj, &this->x_obj->ob_pd, &s_float, gensym("programs"));

}
/////////////////////////////////////////////////////////
// Destructor
//
GEMglGenProgramsARB :: ~GEMglGenProgramsARB () {
inlet_free(m_inlet);
}
//////////////////
// extension check
bool GEMglGenProgramsARB :: isRunnable(void) {
  if(GLEW_ARB_vertex_program)return true;
  error("your system does not support the ARB vertex_program extension");
  return false;
}
/////////////////////////////////////////////////////////
// Render
//
void GEMglGenProgramsARB :: render(GemState *state) {
	glGenProgramsARB (n, programs);
}

/////////////////////////////////////////////////////////
// Variables
//
void GEMglGenProgramsARB :: programsMess (int argc, t_atom*argv) {	// FUN
  n=0;
  delete [] programs;
  programs = new GLuint[argc];
  while(argc--){
    if(argv->a_type == A_FLOAT)programs[n++] = static_cast<GLuint>(atom_getint(argv));
    argv++;
  }
  setModified();
}



/////////////////////////////////////////////////////////
// static member functions
//

void GEMglGenProgramsARB :: obj_setupCallback(t_class *classPtr) {
	 class_addmethod(classPtr, reinterpret_cast<t_method>(&GEMglGenProgramsARB::programsMessCallback),  	gensym("programs"), A_GIMME, A_NULL);
}
void GEMglGenProgramsARB :: programsMessCallback (void* data, t_symbol*, int argc, t_atom*argv){
	GetMyClass(data)->programsMess (argc,argv);
}
