/*
 *  pix_roll.h
 *  gem_darwin
 *
 *  Created by chris clepper on Mon Oct 07 2002.
 *  Copyright (c) 2002 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef _INCLUDE__GEM_PIXES_PIX_ROLL_H_ 
#define _INCLUDE__GEM_PIXES_PIX_ROLL_H_ 

#include "Base/GemPixObj.h"

/*-----------------------------------------------------------------
-------------------------------------------------------------------
CLASS
    pix_roll
    
    

KEYWORDS
    pix
    yuv
    
DESCRIPTION

   template for yuv_ objects
   
-----------------------------------------------------------------*/

class GEM_EXTERN pix_roll : public GemPixObj
{
  CPPEXTERN_HEADER(pix_roll, GemPixObj);

    public:

  //////////
  // Constructor
  pix_roll();
    	
 protected:
    	
  //////////
  // Destructor
  virtual ~pix_roll();

  //////////
  // Do the processing
  virtual void 	processImage(imageStruct &image);
  virtual void 	processRGBAImage(imageStruct &image);

  //////////
  // Do the YUV processing
  virtual void 	processYUVImage(imageStruct &image);
        
  unsigned char  *saved;
  int		m_vroll,m_axis;
  int		m_blurH,m_blurW,m_blurSize,m_blurBpp;
  t_inlet         *inletBlur;
        
 private:
    
  //////////
  // Static member functions
    	
  static void rollCallback       (void *data, t_floatarg value);
  static void axisCallback       (void *data, t_floatarg value);
};

#endif

