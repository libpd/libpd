/*-----------------------------------------------------------------
LOG
    GEM - Graphics Environment for Multimedia

    Turn image by 180 degree.
    This is not very interesting apart from the fact, that:
      we write into a separate buffer
      change the image.data-pointer to point at th new buffer
      pass on the new data to the next processor
      after rendering (postrender), we change the pointer back to the original data
    So we can chain various effects, that need to make copies of the original data

    Copyright (c) 1997-1999 Mark Danks. mark@danks.org
    Copyright (c) 2001-2011 IOhannes m zmölnig. forum::für::umläute. IEM. zmoelnig@iem.at
    For information on usage and redistribution, and for a DISCLAIMER OF ALL
    WARRANTIES, see the file, "GEM.LICENSE.TERMS" in this distribution.

-----------------------------------------------------------------*/

#ifndef _INCLUDE__GEM_PIXES_PIX_TEST_H_
#define _INCLUDE__GEM_PIXES_PIX_TEST_H_

#include "Base/GemPixObj.h"

/*-----------------------------------------------------------------
  -------------------------------------------------------------------
  CLASS
  pix_test
    
  Change pix to greyscale

  KEYWORDS
  pix
    
  DESCRIPTION
   
  -----------------------------------------------------------------*/
class GEM_EXTERN pix_test : public GemPixObj
{
  CPPEXTERN_HEADER(pix_test, GemPixObj);

    public:

  //////////
  // Constructor
  pix_test();
    	
 protected:
    	
  //////////
  // Destructor
  virtual ~pix_test();

  //////////
  // Do the processing
  virtual void 	processImage(imageStruct &image);

  //////////
  // Do the processing - this is a no-op
  virtual void 	processGrayImage(imageStruct &image) { }

  imageStruct    myImage;
  imageStruct  *orgImage;
  unsigned char*orgdata;

  unsigned char off;
  unsigned char csize;

  static void floatMessCallback(void *data, float n);
  static void csizeMessCallback(void *data, float n);
};

#endif	// for header file
