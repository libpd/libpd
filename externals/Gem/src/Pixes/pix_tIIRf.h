/*-----------------------------------------------------------------
  LOG
  GEM - Graphics Environment for Multimedia

  time-domain filtering of pixes (Float precision)
  
  Copyright (c) 2011-2011 IOhannes m zmölnig. forum::für::umläute. IEM. zmoelnig@iem.at
  For information on usage and redistribution, and for a DISCLAIMER OF ALL
  WARRANTIES, see the file, "GEM.LICENSE.TERMS" in this distribution.
	
  -----------------------------------------------------------------*/

#ifndef _INCLUDE__GEM_PIXES_PIX_TIIRF_H_
#define _INCLUDE__GEM_PIXES_PIX_TIIRF_H_

#include "Base/GemPixObj.h"

/*-----------------------------------------------------------------
  -------------------------------------------------------------------
  CLASS
  pix_tIIRf

  KEYWORDS
  pix
  
  DESCRIPTION
	
  -----------------------------------------------------------------*/
class GEM_EXTERN pix_tIIRf : public GemPixObj
{
  CPPEXTERN_HEADER(pix_tIIRf, GemPixObj);
		
    public:
	
  //////////
  // Constructor
  pix_tIIRf(t_floatarg,t_floatarg);
	
 protected:
	
  //////////
  // Destructor
  virtual ~pix_tIIRf();

  void allocate(imageStruct&img);
  void deallocate(void);
  void set(imageStruct*img=NULL);

  //////////
  // Do the processing
  virtual void 	processImage(imageStruct &image);

  //////////
  // set-flag
  enum { NONE, SET, CLEAR } m_set;
	
  //////////
  // the filter factors (feed-forward, feed-back)
  t_float *m_ff, *m_fb;
  int m_ffnum, m_fbnum;

  //////////
  // the image-latches
  t_float**m_buffer;
  int      m_bufnum;

  //////////
  // simple struct for checking whether the image has changed!
  imageStruct m_image;


  // which buffer-image is the current one ?
  int m_counter;
	
  //////////
  // the methods
  void setMess(t_symbol*,int,t_atom*);
  void listMess(t_symbol*,int,t_atom*);


  t_inlet **m_inlet;

};

#endif	// for header file
