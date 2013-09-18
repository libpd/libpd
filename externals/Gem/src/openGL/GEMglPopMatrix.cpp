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

#include "GEMglPopMatrix.h"

CPPEXTERN_NEW ( GEMglPopMatrix );

/////////////////////////////////////////////////////////
//
// GEMglViewport
//
/////////////////////////////////////////////////////////
// Constructor
//
GEMglPopMatrix :: GEMglPopMatrix	(){}
/////////////////////////////////////////////////////////
// Destructor
//
GEMglPopMatrix :: ~GEMglPopMatrix () {}

/////////////////////////////////////////////////////////
// Render
//
void GEMglPopMatrix :: render(GemState *state) {
	glPopMatrix ();
}
/////////////////////////////////////////////////////////
// static member function
void GEMglPopMatrix :: obj_setupCallback(t_class *classPtr) {}
