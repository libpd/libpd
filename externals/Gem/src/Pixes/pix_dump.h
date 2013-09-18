/*-----------------------------------------------------------------
LOG
GEM - Graphics Environment for Multimedia

Get pixel information
	
Copyright (c) 1997-1998 Mark Danks. mark@danks.org
Copyright (c) Günther Geiger. geiger@epy.co.at
Copyright (c) 2001-2011 IOhannes m zmölnig. forum::für::umläute. IEM. zmoelnig@iem.at
Copyright (c) 2002 James Tittle & Chris Clepper
For information on usage and redistribution, and for a DISCLAIMER OF ALL
WARRANTIES, see the file, "GEM.LICENSE.TERMS" in this distribution.
	 
-----------------------------------------------------------------*/

/*-----------------------------------------------------------------
pix_dump

  0409:forum::für::umläute:2000
  IOhannes m zmoelnig
  mailto:zmoelnig@iem.kug.ac.at
-----------------------------------------------------------------*/

#ifndef _INCLUDE__GEM_PIXES_PIX_DUMP_H_
#define _INCLUDE__GEM_PIXES_PIX_DUMP_H_

#include "Base/GemPixObj.h"

/*-----------------------------------------------------------------
-------------------------------------------------------------------
CLASS

 pix_dump
 
  Get pixel information
  
   KEYWORDS
   pix
   
	DESCRIPTION
	
	 dumps the pix-data as a float-package
	 
-----------------------------------------------------------------*/
class GEM_EXTERN pix_dump : public GemPixObj
{
	CPPEXTERN_HEADER(pix_dump, GemPixObj);
		
public:
	
	//////////
	// Constructor
	pix_dump(t_floatarg fx, t_floatarg fy);
	
protected:
	
	//////////
	// Destructor
	virtual ~pix_dump();
	
	//////////
	// All we want is the pixel information, so this is a complete override.
	virtual void 	processImage(imageStruct &image);
	
	//////////
	virtual void 	processYUVImage(imageStruct &image);
	
	//////////
	void			trigger();

	//////////
	// The color outlet
	t_outlet    	*m_dataOut;
	
	//////////
	// the buffer
	int           xsize, ysize;      // proposed x/y-sizes
	int           m_xsize,  m_ysize;
	int           m_csize;
	t_atom       *m_buffer;
	int           m_bufsize;
	
	int           oldimagex;
	int           oldimagey;
	
	//////////
	// navigation
	float         m_xstep;
	float         m_ystep;
	
	/////////
	// pointer to the image data
	unsigned char *m_data;

private:
	
	//////////
	// Static member callbacks
	static void		triggerMessCallback(void *dump);
	static void		GREYMessCallback(void *dump);
	static void		RGBAMessCallback(void *dump);
	static void		RGBMessCallback(void *dump);

};

#endif	// for header file
