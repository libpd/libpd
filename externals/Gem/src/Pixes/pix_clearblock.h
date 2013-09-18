/*-----------------------------------------------------------------
LOG
    GEM - Graphics Environment for Multimedia

    Clear an image without destroying glTexture

    Copyright (c) 1997-1998 Mark Danks. mark@danks.org
    Copyright (c) Günther Geiger. geiger@epy.co.at
    Copyright (c) 2001-2011 IOhannes m zmölnig. forum::für::umläute. IEM. zmoelnig@iem.at
    For information on usage and redistribution, and for a DISCLAIMER OF ALL
    WARRANTIES, see the file, "GEM.LICENSE.TERMS" in this distribution.

    -----------------------------------------------------------------*/

#ifndef _INCLUDE__GEM_PIXES_PIX_CLEARBLOCK_H_
#define _INCLUDE__GEM_PIXES_PIX_CLEARBLOCK_H_

#include "Base/GemBase.h"
#include "Gem/Image.h"

class GemCache;

/*-----------------------------------------------------------------
  -------------------------------------------------------------------
  CLASS
  pix_clearblock
    
  Clear an image without destroying glTexture

  KEYWORDS
  pix
    
  -----------------------------------------------------------------*/
class GEM_EXTERN pix_clearblock : public GemBase
{
  CPPEXTERN_HEADER(pix_clearblock, GemBase);

    public:

  //////////
  // Constructor
  pix_clearblock();
    	
 protected:
    	
  //////////
  // Destructor
  virtual ~pix_clearblock();

  //////////
  // Do the rendering
  virtual void 	render(GemState *state);

  //////////
  // After the rendering
  virtual void 	postrender(GemState *state);

  //////////
  virtual void	startRendering();
    	
  //////////
  virtual void	stopRendering();
    	
  //////////
  // The old cache
  GemCache    	*m_oldcache;
};

#endif	// for header file
