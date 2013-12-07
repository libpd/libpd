////////////////////////////////////////////////////////
//
// GEM - Graphics Environment for Multimedia
//
// zmoelnig@iem.kug.ac.at
//
// Implementation file
//
//    Copyright (c) 1997-2000 Mark Danks.
//    Copyright (c) Günther Geiger.
//    Copyright (c) 2001-2011 IOhannes m zmölnig. forum::für::umläute. IEM. zmoelnig@iem.at
//    For information on usage and redistribution, and for a DISCLAIMER OF ALL
//    WARRANTIES, see the file, "GEM.LICENSE.TERMS" in this distribution.
//
/////////////////////////////////////////////////////////

#include "GemSplash.h"
#include "Gem/State.h"

CPPEXTERN_NEW(GemSplash);

/////////////////////////////////////////////////////////
//
// GemSplash
//
/////////////////////////////////////////////////////////
// Constructor
//
/////////////////////////////////////////////////////////
GemSplash :: GemSplash()
{
}

/////////////////////////////////////////////////////////
// Destructor
//
/////////////////////////////////////////////////////////
GemSplash :: ~GemSplash()
{ }

/////////////////////////////////////////////////////////
// render
//
/////////////////////////////////////////////////////////
void GemSplash :: render(GemState *state)
{
  // this should do something cool.
  // and it should display "Gem" and the version-number
  // probably the core people that were involved
  // should be mentioned too

  // probably we should do a GemSplash contest
}
 
/////////////////////////////////////////////////////////
// static member function
//
/////////////////////////////////////////////////////////
void GemSplash :: obj_setupCallback(t_class *classPtr)
{
  class_addcreator(reinterpret_cast<t_newmethod>(create_GemSplash), gensym("Gem"), A_NULL); 
}
