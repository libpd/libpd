/*-----------------------------------------------------------------
  LOG
  GEM - Graphics Environment for Multimedia

  A light

  Copyright (c) 1997-1999 Mark Danks. mark@danks.org
  Copyright (c) Günther Geiger. geiger@epy.co.at
  Copyright (c) 2001-2011 IOhannes m zmölnig. forum::für::umläute. IEM. zmoelnig@iem.at
  For information on usage and redistribution, and for a DISCLAIMER OF ALL
  WARRANTIES, see the file, "GEM.LICENSE.TERMS" in this distribution.

  -----------------------------------------------------------------*/

#ifndef _INCLUDE__GEM_NONGEOS_LIGHT_H_
#define _INCLUDE__GEM_NONGEOS_LIGHT_H_

#include "Nongeos/world_light.h"

/*-----------------------------------------------------------------
  -------------------------------------------------------------------
  CLASS
  light
    
  Creates a light which can be positioned

  DESCRIPTION
    
  Inlet for a list - "poslist"
    
  "poslist" - Determines position

  -----------------------------------------------------------------*/
class GEM_EXTERN light : public world_light
{
  CPPEXTERN_HEADER(light, world_light);

    public:

  //////////
  // Constructor
  light(t_floatarg lightNum);
        
 protected:
        
  //////////
  // Destructor
  virtual ~light();

  virtual void    renderDebug();
};

#endif  // for header file
