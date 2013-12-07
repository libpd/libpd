/*-----------------------------------------------------------------
  LOG
  GEM - Graphics Environment for Multimedia

  apply a 2p2z-filter on a sequence of pixBlocks
  
  Copyright (c) 1997-1999 Mark Danks. mark@danks.org
  Copyright (c) Günther Geiger. geiger@epy.co.at
  Copyright (c) 2001-2011 IOhannes m zmölnig. forum::für::umläute. IEM. zmoelnig@iem.at
  For information on usage and redistribution, and for a DISCLAIMER OF ALL
  WARRANTIES, see the file, "GEM.LICENSE.TERMS" in this distribution.
	
  -----------------------------------------------------------------*/

/*-----------------------------------------------------------------
  tv_biquad

  IOhannes m zmoelnig
  mailto:zmoelnig@iem.kug.ac.at
	
  this code is published under the Gnu GeneralPublicLicense that should be distributed with gem & pd
	  
  -----------------------------------------------------------------*/

#ifndef _INCLUDE__GEM_PIXES_PIX_BIQUAD_H_
#define _INCLUDE__GEM_PIXES_PIX_BIQUAD_H_

#include "Base/GemPixObj.h"

/*-----------------------------------------------------------------
  -------------------------------------------------------------------
  CLASS
  pix_biquad

  KEYWORDS
  pix
  
  DESCRIPTION
	
  -----------------------------------------------------------------*/
class GEM_EXTERN pix_biquad : public GemPixObj
{
  CPPEXTERN_HEADER(pix_biquad, GemPixObj);
		
    public:
	
  //////////
  // Constructor
  pix_biquad(int, t_atom*);
	
 protected:
	
  //////////
  // Destructor
  virtual ~pix_biquad();
	
  //////////
  // Do the processing
  virtual void 	processRGBAImage(imageStruct &image);
  virtual void 	processYUVImage(imageStruct &image);
#ifdef __MMX__
  virtual void 	processRGBAMMX(imageStruct &image);
  virtual void 	processYUVMMX (imageStruct &image);
  virtual void 	processGrayMMX(imageStruct &image);
#endif
#ifdef __VEC__
  virtual void 	processYUVAltivec(imageStruct &image);
#endif

  
  //////////
  // the image-latches
  imageStruct    prev;
  imageStruct	 last;

  //////////
  // set-flag: if "set", the buffers (prev&last) are set to the current image
  bool set;
	
  //////////
  // the biquad-factors
  t_float fb0, fb1, fb2, ff1, ff2, ff3;
  void faktorMess(int, t_atom*);
 
  // 0..integer-processing(fast) [default]
  // 1..float-processing(slow)
  int m_mode;
  	
  //////////
  // the methods
  static void setMessCallback(void *data);
  static void modeMessCallback(void *data,float value);
  static void faktorMessCallback(void *data, t_symbol *s, int argc, t_atom* argv);

};

#endif	// for header file
