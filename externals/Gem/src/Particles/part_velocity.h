/*-----------------------------------------------------------------
  LOG
  GEM - Graphics Environment for Multimedia

  Set the velocity domain for particles

  Copyright (c) 1997-1999 Mark Danks. mark@danks.org
  Copyright (c) Günther Geiger. geiger@epy.co.at
  Copyright (c) 2001-2011 IOhannes m zmölnig. forum::für::umläute. IEM. zmoelnig@iem.at
  For information on usage and redistribution, and for a DISCLAIMER OF ALL
  WARRANTIES, see the file, "GEM.LICENSE.TERMS" in this distribution.

  -----------------------------------------------------------------*/

#ifndef _INCLUDE__GEM_PARTICLES_PART_VELOCITY_H_
#define _INCLUDE__GEM_PARTICLES_PART_VELOCITY_H_

#include "papi.h"

#include "Particles/partlib_base.h"

/*-----------------------------------------------------------------
  -------------------------------------------------------------------
  CLASS

  part_velocity
    
  Set the velocity domain for particles

  DESCRIPTION

  -----------------------------------------------------------------*/
class GEM_EXTERN part_velocity : public partlib_base
{
  CPPEXTERN_HEADER(part_velocity, partlib_base);

    public:

  //////////
  // Constructor
  part_velocity(int,t_atom*);
    	
  //////////
  virtual void 	renderParticles(GemState *state);

 protected:
    	
  //////////
  // Destructor
  virtual ~part_velocity();

  //////////
  void		vectorMess(int argc, t_atom*argv);
  void		domainMess(t_symbol*s);

  //////////
  float		m_arg[9];
  PDomainEnum	m_domain;

 private:

  //////////
  // static member functions
  static void	domainMessCallback(void *data, t_symbol*s);
  static void	vectorMessCallback(void *data, t_symbol*, int, t_atom*);
};

#endif	// for header file
