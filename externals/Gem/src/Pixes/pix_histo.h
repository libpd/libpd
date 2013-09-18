/*-----------------------------------------------------------------
  LOG
  GEM - Graphics Environment for Multimedia

  calc the histogramm of the pixBuf and write it to table(s)
  
  Copyright (c) 1997-1999 Mark Danks. mark@danks.org
  Copyright (c) Günther Geiger. geiger@epy.co.at
  Copyright (c) 2001-2011 IOhannes m zmölnig. forum::für::umläute. IEM. zmoelnig@iem.at
  Copyright (c) 2002 James Tittle & Chris Clepper
  For information on usage and redistribution, and for a DISCLAIMER OF ALL
  WARRANTIES, see the file, "GEM.LICENSE.TERMS" in this distribution.
	
  -----------------------------------------------------------------*/

/*-----------------------------------------------------------------
  pix_histo

  IOhannes m zmoelnig
  mailto:zmoelnig@iem.kug.ac.at
	
  this code is published under the Gnu GeneralPublicLicense that should be distributed with gem & pd
	  
  -----------------------------------------------------------------*/

#ifndef _INCLUDE__GEM_PIXES_PIX_HISTO_H_
#define _INCLUDE__GEM_PIXES_PIX_HISTO_H_

#include "Base/GemPixObj.h"

/*-----------------------------------------------------------------
  -------------------------------------------------------------------
  CLASS
  pix_histo

  KEYWORDS
  pix
  
  DESCRIPTION
	
  -----------------------------------------------------------------*/
class GEM_EXTERN pix_histo : public GemPixObj
{
  CPPEXTERN_HEADER(pix_histo, GemPixObj);
		
    public:
	
  //////////
  // Constructor
  pix_histo(int argc, t_atom *argv);
	
 protected:
	
  //////////
  // Destructor
  virtual ~pix_histo();
	
  //////////
  // Do the processing
  virtual void 	processRGBAImage(imageStruct &image);
  virtual void 	processGrayImage(imageStruct &image);
  virtual void 	processYUVImage(imageStruct &image);

  //////////
  // tables to hold the curves
  t_symbol *name_R, *name_G, *name_B, *name_A;

  //////////
  // mode
  int m_mode;


  //////////
  // check for good arrays
  t_float* checkarray(t_symbol *s, int *length);
  
  //////////
  // do we need to redraw the table ?
  double updtime;

  //////////
  // update graphs
  void update_graphs(void);

  //////////
  // Set new arrays
  void	setMess(int argc, t_atom *argv);

  //////////
  // the methods
  static void setMessCallback(void *data, t_symbol *s, int argc, t_atom* argv);

};

#endif	// for header file
