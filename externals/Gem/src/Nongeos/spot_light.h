/*-----------------------------------------------------------------
  LOG
  GEM - Graphics Environment for Multimedia

  A world_light

  Copyright (c) 1997-1999 Mark Danks. mark@danks.org
  Copyright (c) Günther Geiger. geiger@epy.co.at
  Copyright (c) 2001-2011 IOhannes m zmölnig. forum::für::umläute. IEM. zmoelnig@iem.at
  For information on usage and redistribution, and for a DISCLAIMER OF ALL
  WARRANTIES, see the file, "GEM.LICENSE.TERMS" in this distribution.

  -----------------------------------------------------------------*/

#ifndef _INCLUDE__GEM_NONGEOS_SPOT_LIGHT_H_
#define _INCLUDE__GEM_NONGEOS_SPOT_LIGHT_H_

#include "Nongeos/world_light.h"

/*-----------------------------------------------------------------
  -------------------------------------------------------------------
  CLASS
  spot_light

  Creates a spot_light - position is at infinity (can be rotated)

  DESCRIPTION

  Inlet for a list - "clrlist"

  "clrlist" - Determines color
  "int" - On/off state

  -----------------------------------------------------------------*/
class GEM_EXTERN spot_light : public world_light
{
  CPPEXTERN_HEADER(spot_light, world_light);

    public:

  //////////
  // Constructor
  spot_light(t_floatarg lightNum);
    	
 protected:

  //////////
  // Destructor
  virtual ~spot_light();

  //////////
  // Do the rendering
  virtual void 	render(GemState *state);

  virtual void 	renderDebug();


  //////////
  // Set the light's parameters (linear attunation, cone angle, decay exponent
  void 		lightParamMess(float linAtt, float cutoff, float exponent);

  //-----------------------------------
  // GROUP:	Member variables
  //-----------------------------------

  // global lighting parameters	
  GLfloat constantAttenuation;
  GLfloat linearAttenuation;
  GLfloat quadraticAttenuation;
  GLfloat spotCutoff;
  GLfloat spotExponent;

  GLfloat spotDirection[3];

 private:

  //////////
  // Static member functions
  static void		lightParamMessCallback(void *data, t_floatarg linAtt, t_floatarg cutoff, t_floatarg exponent);
};



// Define constant position and direction

#endif	// for header file
