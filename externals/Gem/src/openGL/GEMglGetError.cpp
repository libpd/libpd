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

#include "GEMglGetError.h"

CPPEXTERN_NEW ( GEMglGetError );

/////////////////////////////////////////////////////////
//
// GEMglViewport
//
/////////////////////////////////////////////////////////
// Constructor
//
GEMglGetError :: GEMglGetError	() {
  m_outlet = outlet_new(this->x_obj, 0);
}
/////////////////////////////////////////////////////////
// Destructor
//
GEMglGetError :: ~GEMglGetError () {
  outlet_free(m_outlet);
}

/////////////////////////////////////////////////////////
// Render
//
void GEMglGetError :: render(GemState *state) {
  GLenum i=glGetError();
  outlet_float(m_outlet, static_cast<t_float>(i));
}

/////////////////////////////////////////////////////////
// static member functions
//

void GEMglGetError :: obj_setupCallback(t_class *classPtr) {}
