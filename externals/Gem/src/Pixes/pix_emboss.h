/*
 *  pix_emboss.h
 *  gem_darwin
 *
 *  Created by chris clepper on Mon Oct 07 2002.
 *  Copyright (c) 2002 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef _INCLUDE__GEM_PIXES_PIX_EMBOSS_H_ 
#define _INCLUDE__GEM_PIXES_PIX_EMBOSS_H_ 

#include "Base/GemPixObj.h"

/*-----------------------------------------------------------------
-------------------------------------------------------------------
CLASS
    pix_emboss
    
    

KEYWORDS
    pix
    yuv
    
DESCRIPTION

   embossment of images
   only YUV for now
   
-----------------------------------------------------------------*/

class GEM_EXTERN pix_emboss : public GemPixObj
{
  CPPEXTERN_HEADER(pix_emboss, GemPixObj);

    public:

  //////////
  // Constructor
  pix_emboss();
    	
 protected:
    	
  //////////
  // Destructor
  virtual ~pix_emboss();

  //////////
  // Do the YUV processing
  virtual void 	processYUVImage(imageStruct &image);

 private:
    
  //////////
  // Static member functions
};

#endif

