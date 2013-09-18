/*-----------------------------------------------------------------
LOG
GEM - Graphics Environment for Multimedia

set the Alpha-channel of a pixBlock depending on the difference between the current and the last pixBlock
  
Copyright (c) 1997-1999 Mark Danks. mark@danks.org
Copyright (c) Günther Geiger. geiger@epy.co.at
Copyright (c) 2001-2011 IOhannes m zmölnig. forum::für::umläute. IEM. zmoelnig@iem.at
For information on usage and redistribution, and for a DISCLAIMER OF ALL
WARRANTIES, see the file, "GEM.LICENSE.TERMS" in this distribution.
	
-----------------------------------------------------------------*/

/*-----------------------------------------------------------------
pix_movement

  movement-transform a series of pictures (especially movies, videos,...)
  you have to use pix_film instead of pix_movie to make this work with prerecorded videos
 
  2803:forum::für::umläute:2000
  0409:forum::für::umläute:2000
  1801:forum::für::umläute:2001  added the second mode
  IOhannes m zmoelnig
  mailto:zmoelnig@iem.kug.ac.at
  
  this code is published under the Gnu GeneralPublicLicense that should be distributed with gem & pd
	  
-----------------------------------------------------------------*/

#ifndef _INCLUDE__GEM_PIXES_PIX_MOVEMENT_H_
#define _INCLUDE__GEM_PIXES_PIX_MOVEMENT_H_

#include "Base/GemPixObj.h"

/*-----------------------------------------------------------------
-------------------------------------------------------------------
CLASS
pix_movement

  KEYWORDS
  pix
  
	DESCRIPTION
	
-----------------------------------------------------------------*/
class GEM_EXTERN pix_movement : public GemPixObj
{
    CPPEXTERN_HEADER(pix_movement, GemPixObj);
		
public:
	
	//////////
	// Constructor
	pix_movement(t_floatarg f);
	
protected:
	
	//////////
	// Destructor
	virtual ~pix_movement();
	
	//////////
	// Do the processing
	virtual void 	processRGBAImage(imageStruct &image);
	virtual void 	processYUVImage(imageStruct &image);
	virtual void 	processGrayImage(imageStruct &image);
#ifdef __MMX__
        virtual void 	processGrayMMX(imageStruct &image);
#endif

#ifdef __VEC__
        virtual void 	processYUVAltivec(imageStruct &image);
#endif
	
	//////////
	// the last image (grey-scale)
	imageStruct    buffer;
	imageStruct    buffer2; // (the difference image for greyscales)
	//////////
	// the movement-mode
	unsigned char  threshold;

	//////////
	// the methods
	static void threshMessCallback(void *data, t_floatarg fthresh);
};

#endif	// for header file
