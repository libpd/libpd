/*-----------------------------------------------------------------
LOG
    GEM - Graphics Environment for Multimedia

    age an image

    Copyright (c) 1997-1999 Mark Danks. mark@danks.org
    Copyright (c) 2001-2011 IOhannes m zmölnig. forum::für::umläute. IEM. zmoelnig@iem.at
    For information on usage and redistribution, and for a DISCLAIMER OF ALL
    WARRANTIES, see the file, "GEM.LICENSE.TERMS" in this distribution.

    this is based on EffecTV by Fukuchi Kentauro
    * AgingTV - film-aging effect.
    * Copyright (C) 2001 FUKUCHI Kentarou

-----------------------------------------------------------------*/

#ifndef _INCLUDE__GEM_PIXES_PIX_CROP_H_
#define _INCLUDE__GEM_PIXES_PIX_CROP_H_

#include "Base/GemPixObj.h"

/*-----------------------------------------------------------------
-------------------------------------------------------------------
CLASS
    pix_crop
    
    Change pix from "any" color-space to GL_RGBA

KEYWORDS
    pix
    
DESCRIPTION
   
-----------------------------------------------------------------*/

class GEM_EXTERN pix_crop : public GemPixObj
{
  CPPEXTERN_HEADER(pix_crop, GemPixObj);

 public:

  //////////
  // Constructor
  pix_crop(t_floatarg,t_floatarg,t_floatarg,t_floatarg);
    	
 protected:
  
  //////////
  // Destructor
  virtual ~pix_crop();
  
	//////////
	// set dimension
	void dimenMess(int, int);
	void dimXMess(int);
	void dimYMess(int);

	//////////
	// set offset
	void offsetMess(int, int);
	void offXMess(int);
	void offYMess(int);

  	//////////
    	// Do the processing
    	void 	processImage(imageStruct &image);

	unsigned char *m_data;
	int            m_size;


	int sizeX, sizeY, sizeC;
	int offsetX, offsetY;
	int wantSizeX, wantSizeY;

 private:
    	//////////
    	// Static member functions
	static void offsetMessCallback(void *data, t_float x, t_float y);
	static void dimenMessCallback(void *data, t_float x, t_float y);
	static void dimXMessCallback(void *data, t_float x);
	static void dimYMessCallback(void *data, t_float x);
	static void offXMessCallback(void *data, t_float x);
	static void offYMessCallback(void *data, t_float x);
};

#endif	// for header file
