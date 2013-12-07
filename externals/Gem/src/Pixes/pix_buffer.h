/*-----------------------------------------------------------------
LOG
    GEM - Graphics Environment for Multimedia



    Copyright (c) 1997-1999 Mark Danks. mark@danks.org
    Copyright (c) 2001-2011 IOhannes m zmölnig. forum::für::umläute. IEM. zmoelnig@iem.at
    Copyright (c) 2002 James Tittle & Chris Clepper
    For information on usage and redistribution, and for a DISCLAIMER OF ALL
    WARRANTIES, see the file, "GEM.LICENSE.TERMS" in this distribution.

-----------------------------------------------------------------*/

#ifndef _INCLUDE__GEM_PIXES_PIX_BUFFER_H_
#define _INCLUDE__GEM_PIXES_PIX_BUFFER_H_

#include "Base/CPPExtern.h"
#include "Gem/Image.h"

#define DEFAULT_NUM_FRAMES 100


/*-----------------------------------------------------------------
  -------------------------------------------------------------------
  CLASS
  pix_buffer
  
  this is a buffer object for storing frames of video
  
  KEYWORDS
  pix
    
  DESCRIPTION
   
  -----------------------------------------------------------------*/
class GEM_EXTERN pix_buffer : public CPPExtern
{
  CPPEXTERN_HEADER(pix_buffer, CPPExtern);

    public:

  //////////
  // Constructor
  pix_buffer(t_symbol *s,t_float f);
  virtual bool  putMess(imageStruct*img,int pos);
  virtual imageStruct* getMess(int pos);
  virtual int numFrames(void);
    	
 protected:
    	
  //////////
  // Destructor
  virtual ~pix_buffer();

  //////////
  // Do the processing

  imageStruct    *m_buffer;

  virtual void  allocateMess(int,int,int);
  virtual void  bangMess();
  virtual void  openMess(t_symbol*,int);
  virtual void  saveMess(t_symbol*,int);

  virtual void  copyMess(int,int);

  virtual void  resizeMess(int);

  int m_numframes;
  t_symbol *m_bindname;
   

 private:
  
  //////////
  // static member functions
  static void allocateMessCallback(void *data,t_symbol*,int,t_atom*);
  static void bangMessCallback(void *data);
  static void openMessCallback(void *data, t_symbol*, t_floatarg);
  static void saveMessCallback(void *data, t_symbol*, t_floatarg);
  static void resizeMessCallback(void *data, t_floatarg);
  static void copyMessCallback(void *data, t_floatarg, t_floatarg);
};

#endif	// for header file
