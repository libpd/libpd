/*-----------------------------------------------------------------
LOG
    GEM - Graphics Environment for Multimedia

	Draw a part_render group

    Copyright (c) 1997-1999 Mark Danks. mark@danks.org
    Copyright (c) Günther Geiger. geiger@epy.co.at
    Copyright (c) 2001-2011 IOhannes m zmölnig. forum::für::umläute. IEM. zmoelnig@iem.at
    For information on usage and redistribution, and for a DISCLAIMER OF ALL
    WARRANTIES, see the file, "GEM.LICENSE.TERMS" in this distribution.

-----------------------------------------------------------------*/

#ifndef _INCLUDE__GEM_PARTICLES_PART_RENDER_H_
#define _INCLUDE__GEM_PARTICLES_PART_RENDER_H_

#include "Particles/partlib_base.h"

/*-----------------------------------------------------------------
-------------------------------------------------------------------
CLASS

	part_render
    
	Draw a part_render group

DESCRIPTION

-----------------------------------------------------------------*/
class GEM_EXTERN part_render : public partlib_base
{
  CPPEXTERN_HEADER(part_render, partlib_base);

    public:

  //////////
  // Constructor
  part_render();
    	
  //////////
  virtual void 	renderParticles(GemState *state);
  virtual void 	postrender(GemState *state);

 protected:
    	
  //////////
  // Destructor
  virtual ~part_render();

  // How the object should be drawn
  GLfloat        *m_pos;
  void		colorMess(int state);
  bool		m_colorize;
  GLfloat        *m_colors;
  void		sizeMess(int state);
  bool		m_sizing;
  GLfloat        *m_sizes;

  int           m_number;

 private:
  //////////
  // static member functions
  static void 	colorMessCallback(void *data,  t_floatarg state);
  static void 	sizeMessCallback(void *data,  t_floatarg state);
};

#endif	// for header file
