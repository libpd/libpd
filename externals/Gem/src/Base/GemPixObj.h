/*-----------------------------------------------------------------
LOG
    GEM - Graphics Environment for Multimedia

    Base class for pix class gem objects

    Copyright (c) 1997-1999 Mark Danks. mark@danks.org
    Copyright (c) Günther Geiger. geiger@epy.co.at
    Copyright (c) 2001-2011 IOhannes m zmölnig. forum::für::umläute. IEM. zmoelnig@iem.at
    For information on usage and redistribution, and for a DISCLAIMER OF ALL
    WARRANTIES, see the file, "GEM.LICENSE.TERMS" in this distribution.

-----------------------------------------------------------------*/

#ifndef _INCLUDE__GEM_BASE_GEMPIXOBJ_H_
#define _INCLUDE__GEM_BASE_GEMPIXOBJ_H_

#include "Base/GemBase.h"
#include "Gem/Image.h"
#include "stdlib.h"
#include "string.h"
#include "math.h"

/*-----------------------------------------------------------------
-------------------------------------------------------------------
CLASS
    GemPixObj
    
    Base class for pix class gem objects

DESCRIPTION

-----------------------------------------------------------------*/
class GEM_EXTERN GemPixObj : public GemBase
{
    public:

  //////////
  // Constructor
  GemPixObj();

    protected:
    	
  //////////
  // Destructor
  virtual ~GemPixObj()				{ }

  //////////
  // The derived class should override this if it provides
  // processing independent of the image.format
  // This is called whenever a new image comes through.
  // The default is to output an error
  virtual void 	processImage(imageStruct &image);

  //////////
  // The derived class should override this.
  // This is called whenever a new RGB image comes through.
  // The default is to call processImage().
  virtual void 	processRGBImage(imageStruct &image);

  //////////
  // The derived class should override this.
  // This is called whenever a new RGBA image comes through.
  // The default is to call processImage().
  virtual void 	processRGBAImage(imageStruct &image);
  // SIMD-optimized functions: by default the non-optimized function is called
  virtual void 	processRGBAMMX(imageStruct &image);
  virtual void 	processRGBASSE2(imageStruct &image);
  virtual void 	processRGBAAltivec(imageStruct &image);

  //////////
  // The derived class should override this.
  // This is called whenever a new gray8 image comes through.
  // The default is to call processImage().
  virtual void 	processGrayImage(imageStruct &image);
  // SIMD-optimized functions: by default the non-optimized function is called
  virtual void 	processGrayMMX(imageStruct &image);
  virtual void 	processGraySSE2(imageStruct &image);
  virtual void 	processGrayAltivec(imageStruct &image);     

  //////////
  // The derived class should override this.
  // This is called whenever a new YUV422 image comes through.
  // The default is to call processImage().
  virtual void 	processYUVImage(imageStruct &image);
  // SIMD-optimized functions: by default the non-optimized function is called
  virtual void 	processYUVMMX(imageStruct &image);
  virtual void 	processYUVSSE2(imageStruct &image);
  virtual void 	processYUVAltivec(imageStruct &image);
    	
  //////////
  // If the derived class needs the image resent.
  //  	This sets the dirty bit on the pixBlock.
  void	    	setPixModified();
  
  //////////
  // Turn on/off processing
  void            processOnOff(int on);
    
  //////////
  // the pixBlock-cache
  pixBlock    cachedPixBlock;
  pixBlock    *orgPixBlock;

  //////////
  int             m_processOnOff;
  int             m_simd;

  //////////
  // creation callback
  static void 	real_obj_setupCallback(t_class *classPtr) { 
    GemBase::real_obj_setupCallback(classPtr);
    GemPixObj::obj_setupCallback(classPtr); 
  }

  //////////
  // The derived class should NOT override this unless they have some
  //		very special behavior.
  // Do the rendering, which calls processImage or processGrayImage, etc...
  // save the image-information
  virtual void 	render(GemState *state);
  // turn the pointer back to the old data after rendering
  virtual void postrender(GemState *state);
  
  void startRendering(void) {
    //post("start rendering");
    setPixModified();
  }

 private:

  static inline GemPixObj *GetMyClass(void *data) {return((GemPixObj *)((Obj_header *)data)->data);}
    
  //////////
  // static member functions
  static void     obj_setupCallback(t_class *classPtr);
  static void 	floatMessCallback(void *data, float n);
  static void 	simdMessCallback(void *data, float n);
};


#endif	// for header file
