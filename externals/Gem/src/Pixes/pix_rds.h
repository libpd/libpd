/*-----------------------------------------------------------------
LOG
    GEM - Graphics Environment for Multimedia

    random dot stereogram
    This object is an based on the RandomDotStereoTV effect from EffecTV
    Originally written by Fukuchi Kentarou
    Copyright (C) 2002 FUKUCHI Kentarou                         

    ported by tigital@mac.com
    
    Copyright (c) 2003 James Tittle tigital@mac.com
    For information on usage and redistribution, and for a DISCLAIMER OF ALL
    WARRANTIES, see the file, "GEM.LICENSE.TERMS" in this distribution.

-----------------------------------------------------------------*/

#ifndef _INCLUDE__GEM_PIXES_PIX_RDS_H_
#define _INCLUDE__GEM_PIXES_PIX_RDS_H_

#include "Base/GemPixObj.h"

/*-----------------------------------------------------------------
  -------------------------------------------------------------------
  CLASS
  pix_rds
  
  creates random dot stereogram from luminance
  
  KEYWORDS
  pix
    
  DESCRIPTION
   
   send a "toggle" to input for switching between methods
   method 0 = crosseyed
   method 1 = walleyed
  -----------------------------------------------------------------*/
class GEM_EXTERN pix_rds : public GemPixObj
{
  CPPEXTERN_HEADER(pix_rds, GemPixObj);

    public:

  //////////
  // Constructor
  pix_rds();
    	
 protected:
    	
  //////////
  // Destructor
  virtual ~pix_rds();

  //////////
  // Do the processing
  virtual void processRGBAImage(imageStruct &image);
  virtual void processGrayImage(imageStruct &image);
  virtual void processYUVImage(imageStruct &image);

  imageStruct    myImage;
  int 	doDots;
  int 	stride;
  int	method;
  int	fastrand_val;

 private:
  
  //////////
  // static member functions
  static void methMessCallback(void *data, t_floatarg state);
  static void strideMessCallback(void *data, t_floatarg state);
  static void seedMessCallback(void *data, t_floatarg state);
};

#endif	// for header file
