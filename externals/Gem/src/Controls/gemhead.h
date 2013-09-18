/*-----------------------------------------------------------------
  LOG
  GEM - Graphics Environment for Multimedia

  The starting point for all graphics trees

  Copyright (c) 1997-1998 Mark Danks. mark@danks.org
  Copyright (c) Günther Geiger. geiger@epy.co.at
  Copyright (c) 2001-2011 IOhannes m zmölnig. forum::für::umläute. IEM. zmoelnig@iem.at
  For information on usage and redistribution, and for a DISCLAIMER OF ALL
  WARRANTIES, see the file, "GEM.LICENSE.TERMS" in this distribution.

  -----------------------------------------------------------------*/

#ifndef _INCLUDE__GEM_CONTROLS_GEMHEAD_H_
#define _INCLUDE__GEM_CONTROLS_GEMHEAD_H_

#include "Base/CPPExtern.h"
#include "gemreceive.h"

class GemState;
class GemCache;

/*-----------------------------------------------------------------
  -------------------------------------------------------------------
  CLASS
  gemhead
    
  The starting point for all graphics trees

  DESCRIPTION
    
  "bang" - sends out a state list
    
  -----------------------------------------------------------------*/
class GEM_EXTERN gemhead : public gemreceive
{
  CPPEXTERN_HEADER(gemhead, CPPExtern);

 public:

  //////////
  // Constructor
  gemhead(int, t_atom*);
    	
  //////////
  // A render message
  void	    	renderGL(GemState *state);
    	    	
  //////////
  // Set up the modifying flags
  virtual void	    	startRendering();

  //////////
  // Clean up the modifying flags
  virtual void	    	stopRendering();


  //////////
  // change the priority
  void	    	setMess(t_float priority);

  virtual void receive(t_symbol*s, int argc, t_atom*argv);

 protected:
    	
  //////////
  // Destructor
  virtual ~gemhead();

  std::string m_basename;
    	
 private:

  t_outlet    	*m_out1;

  GemCache    	*m_cache;   	    	    // The cache information

  void            outputRenderOnOff(int state);
    	
  void	    	renderOnOff(int state);     // Turn rendering on and off
  int 	    	m_renderOn;

  void	    	bangMess();
};

#endif	// for header file
