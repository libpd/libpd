///////////////////////////////////////////////////////
//
// GEM - Graphics Environment for Multimedia
//
// zmoelnig@iem.kug.ac.at
//
// Implementation file
//
//    Copyright (c) 2009-2011 IOhannes m zmölnig. forum::für::umläute. IEM. zmoelnig@iem.at
//    For information on usage and redistribution, and for a DISCLAIMER OF ALL
//    WARRANTIES, see the file, "GEM.LICENSE.TERMS" in this distribution.
//



#include "ContextData.h"
#include "Base/GemContext.h"

using namespace gem;

/* LATER, when we have multiple contexts, this should really be "-1" or such
 */
const int ContextDataBase::INVALID_CONTEXT=0;

int ContextDataBase::getCurContext(void) {
  /* this should get an integer-index of the current context from GemContext */
  int id=0;
  id=gem::Context::getContextId();
  return id;
}

ContextDataBase::~ContextDataBase(void) {
}
