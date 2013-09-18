/*-----------------------------------------------------------------
  LOG
  GEM - Graphics Environment for Multimedia

  Convert the HSV(A)-Channels of a pixBuf into RGB(A)

  Copyright (c) 1997-1999 Mark Danks. mark@danks.org
  Copyright (c) Günther Geiger. geiger@epy.co.at
  Copyright (c) 2001-2011 IOhannes m zmölnig. forum::für::umläute. IEM. zmoelnig@iem.at
  Copyright (c) 2002 James Tittle & Chris Clepper
  For information on usage and redistribution, and for a DISCLAIMER OF ALL
  WARRANTIES, see the file, "GEM.LICENSE.TERMS" in this distribution.

  -----------------------------------------------------------------*/

#ifndef _INCLUDE__GEM_PIXES_PIX_HSV_RGB_H_
#define _INCLUDE__GEM_PIXES_PIX_HSV_RGB_H_

#include "Base/GemPixObj.h"

/*-----------------------------------------------------------------
  -------------------------------------------------------------------
  CLASS
  pix_hsv2rgb
    
  Change pix to greyscale

  KEYWORDS
  pix
    
  DESCRIPTION
   
  -----------------------------------------------------------------*/
class GEM_EXTERN pix_hsv2rgb : public GemPixObj
{
  CPPEXTERN_HEADER(pix_hsv2rgb, GemPixObj);

    public:

  //////////
  // Constructor
  pix_hsv2rgb();
    	
 protected:
    	
  //////////
  // Destructor
  virtual ~pix_hsv2rgb();

  //////////
  // Do the processing
  virtual void 	processRGBAImage(imageStruct &image);
};

#endif	// for header file
