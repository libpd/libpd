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

#include "GEMglProgramStringARB.h"

CPPEXTERN_NEW_WITH_GIMME ( GEMglProgramStringARB );

/////////////////////////////////////////////////////////
//
// GEMglProgramStringARB
//
/////////////////////////////////////////////////////////
// Constructor
//
GEMglProgramStringARB :: GEMglProgramStringARB	(int argc, t_atom*argv) :
		target(static_cast<GLenum>(0)), 
		format(static_cast<GLenum>(0)), 
		len(static_cast<GLsizei>(0)),
                string(NULL)
{
        switch (argc) {
        default:
        case 4:
          string = reinterpret_cast<GLvoid*>(atom_getsymbol(argv+3)->s_name);
        case 3:
          len=atom_getint(argv+2);
        case 2:
          format=atom_getint(argv+1);
        case 1:
          target=atom_getint(argv+0);
        case 0:
          break;
        }

	m_inlet[0] = inlet_new(this->x_obj, &this->x_obj->ob_pd, &s_float, gensym("target"));
	m_inlet[1] = inlet_new(this->x_obj, &this->x_obj->ob_pd, &s_float, gensym("format"));
	m_inlet[2] = inlet_new(this->x_obj, &this->x_obj->ob_pd, &s_float, gensym("len"));
	m_inlet[3] = inlet_new(this->x_obj, &this->x_obj->ob_pd, &s_symbol, gensym("string"));
}
/////////////////////////////////////////////////////////
// Destructor
//
GEMglProgramStringARB :: ~GEMglProgramStringARB ()
{
	inlet_free(m_inlet[0]);
	inlet_free(m_inlet[1]);
	inlet_free(m_inlet[2]);
	inlet_free(m_inlet[3]);
}
//////////////////
// extension check
bool GEMglProgramStringARB :: isRunnable(void) {
  if(GLEW_ARB_vertex_program)return true;
  error("your system does not support the ARB vertex_program extension");
  return false;
}

/////////////////////////////////////////////////////////
// Render
//
void GEMglProgramStringARB :: render(GemState *state) 
{
	glProgramStringARB (target, format, len, string);
}

/////////////////////////////////////////////////////////
// Variables
//
void GEMglProgramStringARB :: targetMess (t_float arg1) {	// FUN
	target = static_cast<GLenum>(arg1);
	setModified();
}

void GEMglProgramStringARB :: formatMess (t_float arg1) {	// FUN
	format = static_cast<GLenum>(arg1);
	setModified();
}

void GEMglProgramStringARB :: lenMess (t_float arg1) {	// FUN
	len = static_cast<GLsizei>(arg1);
	setModified();
}

void GEMglProgramStringARB :: stringMess (t_symbol* arg1) {	// FUN
        string = reinterpret_cast<GLvoid*>(arg1->s_name);
	setModified();
}


/////////////////////////////////////////////////////////
// static member functions
//

void GEMglProgramStringARB :: obj_setupCallback(t_class *classPtr) {
	 class_addmethod(classPtr, reinterpret_cast<t_method>(&GEMglProgramStringARB::targetMessCallback),
									gensym("target"), A_DEFFLOAT, A_NULL);
	 class_addmethod(classPtr, reinterpret_cast<t_method>(&GEMglProgramStringARB::formatMessCallback), 
									gensym("format"), A_DEFFLOAT, A_NULL);
	 class_addmethod(classPtr, reinterpret_cast<t_method>(&GEMglProgramStringARB::lenMessCallback), 
									gensym("len"), A_DEFFLOAT, A_NULL);
	 class_addmethod(classPtr, reinterpret_cast<t_method>(&GEMglProgramStringARB::stringMessCallback), 
									gensym("string"), A_DEFSYMBOL, A_NULL);
};

void GEMglProgramStringARB :: targetMessCallback (void* data, t_floatarg arg0){
	GetMyClass(data)->targetMess ( arg0 );
}
void GEMglProgramStringARB :: formatMessCallback (void* data, t_floatarg arg1){
	GetMyClass(data)->formatMess ( arg1 );
}
void GEMglProgramStringARB :: lenMessCallback (void* data, t_floatarg arg2){
	GetMyClass(data)->lenMess ( arg2 );
}
void GEMglProgramStringARB :: stringMessCallback (void* data, t_symbol* arg3){
	GetMyClass(data)->stringMess ( arg3 );
}
