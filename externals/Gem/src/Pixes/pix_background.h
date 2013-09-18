/*
 *  pix_background.h
 *  gem_darwin
 *
 *  Created by chris clepper on Mon Oct 07 2002.
 *  Copyright (c) 2002 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef _INCLUDE__GEM_PIXES_PIX_BACKGROUND_H_ 
#define _INCLUDE__GEM_PIXES_PIX_BACKGROUND_H_ 

#include "Base/GemPixObj.h"

/*-----------------------------------------------------------------
  -------------------------------------------------------------------
  CLASS
  pix_background
    
    

  KEYWORDS
  pix
  yuv
    
  DESCRIPTION

  template for yuv_ objects
   
  -----------------------------------------------------------------*/

class GEM_EXTERN pix_background : public GemPixObj
{
  CPPEXTERN_HEADER(pix_background, GemPixObj);

    public:

  //////////
  // Constructor
  pix_background(int argc, t_atom*argv);
    	
 protected:
    	
  //////////
  // Destructor
  virtual ~pix_background();

  //////////
  // Do the processing
  virtual void 	processRGBAImage(imageStruct &image);
  virtual void 	processGrayImage(imageStruct &image);
  virtual void 	processYUVImage (imageStruct &image);
#ifdef __MMX__
  virtual void processRGBAMMX   (imageStruct &image);
  virtual void processYUVMMX    (imageStruct &image);
  virtual void processGrayMMX   (imageStruct &image);
#endif 
#ifdef __VEC__
  //////////
  // Do the YUV Altivec processing
  virtual void 	processYUVAltivec(imageStruct &image);
#endif

  virtual void rangeNMess(int argc, t_atom*argv);
        
  imageStruct   m_savedImage;
  int		m_Yrange,m_Urange,m_Vrange, m_Arange;
  t_inlet      *inletRange;
  int		m_reset;

        
 private:
    
  //////////
  // Static member functions
    	
  static void rangeCallback       (void *data, t_floatarg Y, t_floatarg U, t_floatarg V);
  //  static void rangeNCallback      (void *data, t_floatarg Y, t_floatarg U, t_floatarg V);
  static void rangeNCallback      (void *data, t_symbol*,int,t_atom*);
  static void resetCallback       (void *data);


};

#endif

