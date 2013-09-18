/*-----------------------------------------------------------------
  LOG
  GEM - Graphics Environment for Multimedia

  Output a pixel-buffer as 4 signals

  Copyright (c) 2002-2011 IOhannes m zmölnig. forum::für::umläute. IEM. zmoelnig@iem.at
  Copyright (c) Günther Geiger. geiger@epy.co.at
  Copyright (c) 2001-2011 IOhannes m zmölnig. forum::für::umläute. IEM. zmoelnig@iem.at
  Copyright (c) 2002 James Tittle & Chris Clepper
  For information on usage and redistribution, and for a DISCLAIMER OF ALL
  WARRANTIES, see the file, "GEM.LICENSE.TERMS" in this distribution.

  -----------------------------------------------------------------*/

#ifndef _INCLUDE__GEM_PIXES_PIX_PIX_SIG_H_
#define _INCLUDE__GEM_PIXES_PIX_PIX_SIG_H_

#include <string.h>
#include "Base/GemPixObj.h"

/*-----------------------------------------------------------------
  -------------------------------------------------------------------
  CLASS
  pix_pix2sig
    
  Interprete a pixel-image as 4 RGBA-signals
    
  KEYWORDS
  pix
    
  DESCRIPTION

  4 outlets with signals : R~, G~, B~, A~
    
  -----------------------------------------------------------------*/
class GEM_EXTERN pix_pix2sig : public GemPixObj
{
  CPPEXTERN_HEADER(pix_pix2sig, GemPixObj);

    public:

  //////////
  // Constructor
  pix_pix2sig();
    	
 protected:
    	
  //////////
  // Destructor
  virtual      ~pix_pix2sig();

  //////////
  // DSP perform
  static t_int* perform(t_int* w);

  //////////
  // processImage
  virtual void  processImage(imageStruct &image);

  //////////
  // DSP-Message
  virtual void  dspMess(void *data, t_signal** sp);

  //-----------------------------------
  // GROUP:	Image data
  //-----------------------------------
    
  //////////
  // The pixBlock with the current image
  //  pixBlock    	m_pixBlock;
  unsigned char *m_data;
  long int       m_size;
  int            m_csize;
  int            m_format;

  //////////
  // the outlets~
  t_outlet *o_col[4];

 private:
    	
  //////////
  // static member functions
  static void   dspMessCallback(void* data,t_signal** sp);
};

#endif	// for header file
