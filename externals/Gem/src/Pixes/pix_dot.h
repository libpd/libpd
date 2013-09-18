/*-----------------------------------------------------------------
LOG
    GEM - Graphics Environment for Multimedia

    change pixBuf into dots

    Copyright (c) 1997-1999 Mark Danks. mark@danks.org
    Copyright (c) 2001-2011 IOhannes m zmölnig. forum::für::umläute. IEM. zmoelnig@iem.at
    Copyright (c) 2003 James Tittle
    For information on usage and redistribution, and for a DISCLAIMER OF ALL
    WARRANTIES, see the file, "GEM.LICENSE.TERMS" in this distribution.

-----------------------------------------------------------------*/

#ifndef _INCLUDE__GEM_PIXES_PIX_DOT_H_
#define _INCLUDE__GEM_PIXES_PIX_DOT_H_

#include "Base/GemPixObj.h"

/*-----------------------------------------------------------------
  -------------------------------------------------------------------
  CLASS
  pix_dot
  
  change the pixBuf into dots
  
  KEYWORDS
  pix
    
  DESCRIPTION
   
  -----------------------------------------------------------------*/
class GEM_EXTERN pix_dot : public GemPixObj
{
  CPPEXTERN_HEADER(pix_dot, GemPixObj);

    public:

  //////////
  // Constructor
  pix_dot();
    	
 protected:
    	
  //////////
  // Destructor
  virtual ~pix_dot();

  //////////
  // Do the processing
  virtual void 	processRGBAImage(imageStruct &image);
  virtual void 	processYUVImage( imageStruct &image );
  virtual void 	processGrayImage( imageStruct &image );
  virtual void	drawDot( int xx, int yy, unsigned char c, U32 *dest );
  virtual void	drawDotYUV( int xx, int yy, unsigned char c, U16 *dest );
  virtual void	drawDotGray( int xx, int yy, unsigned char c, unsigned char *dest );
  virtual void  sampxy_table_init();
  virtual void  yuv_init();
  virtual void  makePattern(int format=GL_RGBA);
  virtual void  sizeMess(int width, int height);
  virtual void 	scaleMess( float state );
  int	sharedbuffer_init();
  void	sharedbuffer_reset();
  unsigned char *sharedbuffer_alloc(int size);
  unsigned char inline_RGB2Y( int rgb );

  imageStruct    myImage;

  //////////
  // Make dots
    
  int m_xsize, m_ysize, m_csize;
    unsigned char *sharedbuffer;
    int sharedbuffer_length;

    int tail;
    int alreadyInit;
    int DOTDEPTH, DOTMAX;
    int dots_width, dots_height;
    int dot_size, dot_hsize;
    int *sampx, *sampy;
    int state;
    t_float m_scale;
    U32 *pattern;
    U32 *heart_pattern;
    int mode;
    int R2Y[256];
    int G2Y[256];
    int B2Y[256];

    bool m_useScale;

 private:
  
  //////////
  // static member functions
  static void bangMessCallback(void *data);
  static void sizeMessCallback(void *data, t_floatarg width, t_floatarg height);
  static void scaleMessCallback(void *data, t_floatarg state);
};

#endif	// for header file
