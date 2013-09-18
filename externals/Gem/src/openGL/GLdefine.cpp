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

#include "GLdefine.h"

CPPEXTERN_NEW_WITH_GIMME ( GLdefine );

/////////////////////////////////////////////////////////
//
// GLdefine
//
/////////////////////////////////////////////////////////
// Constructor
//
GLdefine :: GLdefine	(int argc, t_atom*argv) :
  m_argc(0), m_argv(NULL),
  m_outlet(NULL)
{
  listMess(argc, argv);
  m_outlet = outlet_new(this->x_obj, &s_float);
}
/////////////////////////////////////////////////////////
// Destructor
//
GLdefine :: ~GLdefine () {
  outlet_free(m_outlet);
}

/////////////////////////////////////////////////////////
// Variables
//
void GLdefine :: symMess (t_symbol *s) {	// FUN
  t_atom ap;
  SETSYMBOL(&ap, s);
  listMess(1, &ap);
}
void GLdefine :: bangMess () {
  if(m_outlet) {
    switch (m_argc) {
    case 0:
      outlet_bang(m_outlet);
      break;
    case 1:
      outlet_float(m_outlet, atom_getfloat(m_argv));
      break;
    default:
      outlet_list(m_outlet, 0, m_argc, m_argv);
      break;
    }
  }
}

void GLdefine :: listMess (int argc, t_atom*argv) {
  int i=0;

  if(m_argc) {
    if(m_argv)freebytes(m_argv, sizeof(t_atom)*m_argc);
    m_argv=NULL;
    m_argc=0;
  }
  m_argc=argc;
  m_argv=(t_atom*)getbytes(sizeof(t_atom)*m_argc);

  for(i=0; i<argc; i++) {
    t_float x = getGLdefine(argv+i);
    SETFLOAT(m_argv+i, x);
  }

  bangMess();
}


/////////////////////////////////////////////////////////
// static member functions
//

void GLdefine :: obj_setupCallback(t_class *classPtr) {
	 class_addsymbol(classPtr, GLdefine::symMessCallback);
	 class_addbang(classPtr, GLdefine::bangMessCallback);
	 class_addlist(classPtr, GLdefine::listMessCallback);
	 class_addanything(classPtr, GLdefine::anyMessCallback);
};

void GLdefine :: symMessCallback (void* data, t_symbol *arg0){
	GetMyClass(data)->symMess (arg0);
}
void GLdefine :: anyMessCallback (void* data, t_symbol *arg0, int argc, t_atom*argv){
	GetMyClass(data)->symMess (arg0);
}
void GLdefine :: bangMessCallback (void* data){
	GetMyClass(data)->bangMess ();
}
void GLdefine :: listMessCallback (void* data, t_symbol *arg0, int argc, t_atom*argv){
	GetMyClass(data)->listMess (argc, argv);
}
