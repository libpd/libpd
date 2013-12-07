#ifndef _INCLUDE__GEM_PIXES_PIX_MOVEMENT__H_
#define _INCLUDE__GEM_PIXES_PIX_MOVEMENT__H_

#include "Base/GemPixObj.h"

class GEM_EXTERN pix_movement2 : public GemPixObj
{
  CPPEXTERN_HEADER(pix_movement2, GemPixObj);
		
    public:
  
  //////////
  // Constructor
  pix_movement2(t_float lothresh=0.392f, t_float hithresh=0.588f);
  
 protected:
  //Destructor
  ~pix_movement2();
  void processImage(imageStruct &image);

  imageStruct m_frame[3];
  imageStruct m_output, m_threshold, m_background;
  int m_frameIndex;

  unsigned char m_thresh, m_lowthresh;

  bool m_storeBackground, m_resetThreshold;

  t_inlet*m_threshInlet, *m_lowthreshInlet;


  void threshMess(int thresh);
  void lowThreshMess(int thresh);
  void bangMess();

 private:
  static void threshMessCallback(void *data, t_floatarg fthresh);
  static void lowThreshMessCallback(void *data, t_floatarg fthresh);
  static void bangMessCallback(void *data);
};


#endif 	// for header file
