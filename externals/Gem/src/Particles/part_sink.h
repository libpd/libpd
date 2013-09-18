/*-----------------------------------------------------------------
  LOG
  GEM - Graphics Environment for Multimedia

  Generate particles

  Copyright (c) 1997-1999 Mark Danks. mark@danks.org
  Copyright (c) Günther Geiger. geiger@epy.co.at
  Copyright (c) 2001-2011 IOhannes m zmölnig. forum::für::umläute. IEM. zmoelnig@iem.at
  For information on usage and redistribution, and for a DISCLAIMER OF ALL
  WARRANTIES, see the file, "GEM.LICENSE.TERMS" in this distribution.

  -----------------------------------------------------------------*/

#ifndef _INCLUDE__GEM_PARTICLES_PART_SINK_H_
#define _INCLUDE__GEM_PARTICLES_PART_SINK_H_

#include "Particles/partlib_base.h"
#include "papi.h"

/*-----------------------------------------------------------------
  -------------------------------------------------------------------
  CLASS

  part_sink
    
  Generate particles

  DESCRIPTION

  -----------------------------------------------------------------*/
class GEM_EXTERN part_sink : public partlib_base
{
  CPPEXTERN_HEADER(part_sink, partlib_base);

    public:

  //////////
  // Constructor
  part_sink(int argc, t_atom*argv);
    	
  //////////
  virtual void 	renderParticles(GemState *state);

 protected:
    	
  //////////
  // Destructor
  virtual ~part_sink();
		
  //////////
  // kill inside?
  void			killMess(int num);
  bool			m_kill;

  //////////
  // vel.domain
  PDomainEnum           m_domain;
  void                  domainMess(t_symbol *s);
  float			m_arg[9];
  void			vectorMess(int argc, t_atom*argv);
	
 private:

  //////////
  // static member functions
  static void		killMessCallback(void *data, t_floatarg num);
  static void		domainMessCallback(void *data, t_symbol*s);
  static void		vectorMessCallback(void *data, t_symbol*, int, t_atom*);
};

#endif	// for header file
