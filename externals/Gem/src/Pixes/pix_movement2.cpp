////////////////////////////////////////////////////////
//
// GEM - Graphics Environment for Multimedia
//
// zmoelnig@iem.kug.ac.at
//
// Implementation file
//
//    Copyright (c) 1997-1998 Mark Danks.
//    Copyright (c) 2001-2011 IOhannes m zmölnig. forum::für::umläute. IEM. zmoelnig@iem.at
//    Copyright (c) 2004 Jakob Leiner & Theresa Rienmüller
//    For information on usage and redistribution, and for a DISCLAIMER OF ALL
//    WARRANTIES, see the file, "GEM.LICENSE.TERMS" in this distribution.
//
/////////////////////////////////////////////////////////

#include "pix_movement2.h"
#include <string.h>
#include <math.h>

CPPEXTERN_NEW_WITH_TWO_ARGS(pix_movement2, t_float,A_DEFFLOAT,t_float, A_DEFFLOAT);

/*------------------------------------------------------------
  Constructor
  initializes the pixBlocks and pixBlobs
  ------------------------------------------------------------*/
  pix_movement2 :: pix_movement2(t_float lothresh, t_float hithresh): 
    m_frameIndex(0),
    m_storeBackground(true), m_resetThreshold(true)
{
  int i=3;
  while(i--){
    m_frame[i].xsize=0;
    m_frame[i].ysize=0;
    m_frame[i].setCsizeByFormat(GL_LUMINANCE);
    m_frame[i].reallocate();
  }
  m_output.xsize=0;
  m_output.ysize=0;
  m_output.setCsizeByFormat(GL_LUMINANCE);
  m_output.reallocate();

  m_background.xsize=0;
  m_background.ysize=0;
  m_background.setCsizeByFormat(GL_LUMINANCE);
  m_background.reallocate();

  m_threshold.xsize=0;
  m_threshold.ysize=0;
  m_threshold.setCsizeByFormat(GL_LUMINANCE);
  m_threshold.reallocate();

  m_lowthresh=CLAMP(255.f*MIN(lothresh, hithresh));
  m_thresh=CLAMP(255.f*MAX(lothresh, hithresh));
  if(m_thresh==0)   m_thresh=150;
  if(m_lowthresh==0)m_lowthresh=100;

  /*
    m_thresh = 150;
    m_lowthresh = 100;
  */

  m_lowthreshInlet=inlet_new(this->x_obj, &this->x_obj->ob_pd, gensym("float"), gensym("low_thresh"));
  m_threshInlet=inlet_new(this->x_obj, &this->x_obj->ob_pd, gensym("float"), gensym("hi_thresh"));
}

pix_movement2 :: ~pix_movement2(){
  if(m_threshInlet)inlet_free(m_threshInlet);
  if(m_lowthreshInlet)inlet_free(m_lowthreshInlet);
}

/*------------------------------------------------------------
  processImage
  calculates the difference image between three frames
  if the difference is greater than the threshhold, the concerning
  pixel is set to 255
  ------------------------------------------------------------*/
void pix_movement2 :: processImage(imageStruct &image){
  int size = image.xsize * image.ysize;
  int i;
  // 0. check if we have a new dimension
  bool firstTime = ((image.xsize!=m_frame[0].xsize) || (image.ysize!=m_frame[0].ysize));
  if (firstTime){
    m_storeBackground=true;
    m_resetThreshold=true;
    m_output.xsize = image.xsize;
    m_output.ysize = image.ysize;
    m_output.reallocate();

    m_background.xsize = image.xsize;
    m_background.ysize = image.ysize;
    m_background.reallocate();
    
    m_threshold.xsize = image.xsize;
    m_threshold.ysize = image.ysize;
    m_threshold.reallocate();

    i = 3;
    while(i--){
      m_frame[i].xsize=image.xsize;
      m_frame[i].ysize=image.ysize;
      m_frame[i].reallocate();
    }
    m_frameIndex=0;
  }
  if(m_resetThreshold){
    // we can do this, because we are in Greyscale-mode
    memset(m_threshold.data, m_thresh, m_threshold.xsize * m_threshold.ysize);
    m_resetThreshold=false;
  }

  // 1. store the current frame as gray-image in the apropriate buffer
  switch (image.format) {
  case GL_RGBA:         m_frame[m_frameIndex].fromRGBA(image.data); break;
  case GL_BGRA_EXT:     m_frame[m_frameIndex].fromBGRA(image.data); break;
  case GL_YCBCR_422_GEM:m_frame[m_frameIndex].fromUYVY(image.data); break;
  case GL_LUMINANCE:    m_frame[m_frameIndex].fromGray(image.data); break;
  default: error("no method for this kind of color"); return;
  }

  // 2. if this is the first time, copy the current frame to the other frames as well
  if(m_storeBackground){
    m_storeBackground=false;
    m_background.fromGray(m_frame[0].data);
    for (i = 1; i < 3; i++)m_frame[i].fromGray(m_frame[0].data);
    // use our (black) "output"-image as "image" and return
    m_output.copy2ImageStruct(&image);

    if(firstTime)return;
  }
  // 3. now calc the difference
  unsigned char* cur   = m_frame[m_frameIndex].data;
  unsigned char* old2  = m_frame[(m_frameIndex+1)%3].data;
  unsigned char* old1  = m_frame[(m_frameIndex+2)%3].data;
  unsigned char* out   = m_output.data;
  unsigned char* thresh = m_threshold.data;
  unsigned char* back  = m_background.data;

  m_frameIndex++; m_frameIndex%=3;

  for(i=0; i<size; i++){

    if ((abs(cur[i]-old1[i]) > thresh[i]) &&
	(abs(cur[i]-old2[i]) > thresh[i])){
      out[i] = 255;
    } else {
      out[i] = 0;
      if(abs(cur[i]-back[i])>thresh[i]){
	out[i] = 255;
      }

      if(thresh[i] < m_lowthresh)thresh[i] = m_lowthresh;

      thresh[i] = ((256-26)*thresh[i]+(26)*5*abs(cur[i]-back[i]))>>8;
      back [i] = (26*back [i]+(256-26)*cur[i])>>8;
    }
  }
  m_output.upsidedown = image.upsidedown;
  m_output.copy2ImageStruct(&image);
}

/*------------------------------------------------------------
  threshMess
  ------------------------------------------------------------*/
void pix_movement2 :: threshMess(int thresh){
  if(thresh < static_cast<int>(m_lowthresh)){
    error("high threshold (%d) must not be less than low threshold(%d)", thresh, m_lowthresh);
    return;
  }
  m_thresh = CLAMP(thresh);
  m_resetThreshold=true;
}

/*------------------------------------------------------------
  lowThreshMess
  ------------------------------------------------------------*/
void pix_movement2 :: lowThreshMess(int thresh){
  if(thresh > static_cast<int>(m_thresh)){
    error("low threshold (%d) must not be be greater than high threshold(%d)", thresh, m_thresh);
    return;
  }
  m_lowthresh = CLAMP(thresh);
}

/*------------------------------------------------------------
  bangMess
  ------------------------------------------------------------*/
void pix_movement2 :: bangMess(){
  m_storeBackground = true;
}

/*------------------------------------------------------------

static member functions

------------------------------------------------------------*/
void pix_movement2 :: obj_setupCallback(t_class*classPtr){
  class_addmethod(classPtr, reinterpret_cast<t_method>(&pix_movement2::lowThreshMessCallback),
		  gensym("low_thresh"), A_FLOAT, A_NULL);
  class_addmethod(classPtr, reinterpret_cast<t_method>(&pix_movement2::threshMessCallback),
		  gensym("hi_thresh"), A_FLOAT, A_NULL);
  class_addbang(classPtr, reinterpret_cast<t_method>(&pix_movement2::bangMessCallback));
}

/*------------------------------------------------------------
  threshMessCallback
  ------------------------------------------------------------
*/
void pix_movement2 :: threshMessCallback(void *data, t_floatarg thresh)
{
  GetMyClass(data)->threshMess(static_cast<int>(255*thresh));
}
/*------------------------------------------------------------
  lowThreshMessCallback
  ------------------------------------------------------------*/
void pix_movement2 :: lowThreshMessCallback(void *data, t_floatarg thresh)
{
  GetMyClass(data)->lowThreshMess(static_cast<int>(255*thresh));
}
/*------------------------------------------------------------
  bangMessCallback
  ------------------------------------------------------------*/
void pix_movement2 :: bangMessCallback(void *data)
{
  GetMyClass(data)->bangMess();
}
