/*-----------------------------------------------------------------
LOG
GEM - Graphics Environment for Multimedia

Calculate the center of gravity of a pixBlock.

Copyright (c) 1997-1998 Mark Danks. mark@danks.org
Copyright (c) Günther Geiger. geiger@epy.co.at
Copyright (c) 2001-2011 IOhannes m zmölnig. forum::für::umläute. IEM. zmoelnig@iem.at
For information on usage and redistribution, and for a DISCLAIMER OF ALL
WARRANTIES, see the file, "GEM.LICENSE.TERMS" in this distribution.
	 
-----------------------------------------------------------------*/

/*-----------------------------------------------------------------
pix_blob

  0409:forum::für::umläute:2000
  IOhannes m zmoelnig
  mailto:zmoelnig@iem.kug.ac.at
-----------------------------------------------------------------*/

#ifndef _INCLUDE__GEM_PIXES_PIX_BLOB_H_
#define _INCLUDE__GEM_PIXES_PIX_BLOB_H_

#include "Base/GemPixObj.h"

/*-----------------------------------------------------------------
-------------------------------------------------------------------
CLASS

 pix_blob
 
  Get pixel information
  
   KEYWORDS
   pix
   
	DESCRIPTION
	
	 dumps the pix-data as a float-package
	 
-----------------------------------------------------------------*/
class GEM_EXTERN pix_blob : public GemPixObj
{
	CPPEXTERN_HEADER(pix_blob, GemPixObj);
		
public:
	
	//////////
	// Constructor
	pix_blob(int argc, t_atom *argv);
	
protected:
	
	//////////
	// Destructor
	virtual ~pix_blob();
	
	//////////
	// All we want is the pixel information, so this is a complete override.
	virtual void 	processRGBAImage(imageStruct &image);
	virtual void 	processGrayImage(imageStruct &image);
	virtual void 	processYUVImage(imageStruct &image);
	
	//////////
	void		ChannelMess(int  channel);
	void            GainMess(int argc, t_atom *argv);

	//////////
	// The color outlet
	t_outlet    	*m_xOut, *m_yOut;
	t_outlet        *m_zOut;
	
	//////////
	// user settings
	int              m_method;

	//////////
	// 
	t_float          m_gain[4];

private:
	
	//////////
	// Static member callbacks
	static void gainMessCallback(void *dump, t_symbol *, int argc, t_atom *argv);
	static void channelMessCallback(void *dump, t_float channel);
};

#endif	// for header file
